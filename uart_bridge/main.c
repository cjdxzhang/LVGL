#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define UART_TTYS3 "/dev/ttyS3"
#define UART_TTYS4 "/dev/ttyS4"
#define UART_BAUD B115200
#define RX_BUF_SIZE 512
#define FISH_INTERVAL_MS 1000

static const uint8_t k_fish_frame[] = {
    0x55, 0xAA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
};

static volatile sig_atomic_t g_running = 1;

static void handle_signal(int sig)
{
    (void)sig;
    g_running = 0;
}

static long long now_ms(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;

    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

static void dump_hex(const uint8_t *data, size_t len)
{
    size_t i;

    for (i = 0; i < len; ++i)
        printf("%02X%s", data[i], (i + 1 == len) ? "" : " ");
}

static void log_bytes(const char *prefix, const char *from, const char *to,
                      const uint8_t *data, size_t len)
{
    printf("[%lld] %s %s -> %s len=%zu hex=", now_ms(), prefix, from, to, len);
    dump_hex(data, len);
    printf("\n");
    fflush(stdout);
}

static int configure_uart(int fd)
{
    struct termios tio;

    if (tcgetattr(fd, &tio) != 0)
        return -1;

    cfmakeraw(&tio);
    cfsetispeed(&tio, UART_BAUD);
    cfsetospeed(&tio, UART_BAUD);

    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CRTSCTS;
    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tio) != 0)
        return -1;

    tcflush(fd, TCIOFLUSH);
    return 0;
}

static int open_uart(const char *path)
{
    int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd < 0) {
        fprintf(stderr, "open %s failed: %s\n", path, strerror(errno));
        return -1;
    }

    if (configure_uart(fd) != 0) {
        fprintf(stderr, "configure %s failed: %s\n", path, strerror(errno));
        close(fd);
        return -1;
    }

    printf("opened %s: 115200 8N1\n", path);
    return fd;
}

static int wait_writable(int fd)
{
    struct pollfd pfd;

    pfd.fd = fd;
    pfd.events = POLLOUT;
    pfd.revents = 0;

    return poll(&pfd, 1, 1000);
}

static ssize_t write_all(int fd, const char *path, const uint8_t *data, size_t len)
{
    size_t written = 0;

    while (written < len && g_running) {
        ssize_t ret = write(fd, data + written, len - written);

        if (ret > 0) {
            written += (size_t)ret;
            continue;
        }

        if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            int poll_ret = wait_writable(fd);
            if (poll_ret > 0)
                continue;
            if (poll_ret == 0) {
                fprintf(stderr, "write %s timeout\n", path);
                break;
            }
        }

        if (ret < 0)
            fprintf(stderr, "write %s failed: %s\n", path, strerror(errno));
        break;
    }

    return (ssize_t)written;
}

static void forward_data(int from_fd, const char *from_path,
                         int to_fd, const char *to_path)
{
    uint8_t buf[RX_BUF_SIZE];
    ssize_t len;

    for (;;) {
        len = read(from_fd, buf, sizeof(buf));
        if (len > 0) {
            log_bytes("rx", from_path, to_path, buf, (size_t)len);
            if (write_all(to_fd, to_path, buf, (size_t)len) == len)
                log_bytes("tx", from_path, to_path, buf, (size_t)len);
            continue;
        }

        if (len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return;

        if (len < 0)
            fprintf(stderr, "read %s failed: %s\n", from_path, strerror(errno));
        return;
    }
}

static void send_fish_frame(int fd_ttys4)
{
    if (write_all(fd_ttys4, UART_TTYS4, k_fish_frame, sizeof(k_fish_frame)) ==
        (ssize_t)sizeof(k_fish_frame))
        log_bytes("auto", "fish", UART_TTYS4, k_fish_frame,
                  sizeof(k_fish_frame));
}

int main(void)
{
    int fd_ttys3;
    int fd_ttys4;
    long long next_fish_ms;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    fd_ttys3 = open_uart(UART_TTYS3);
    if (fd_ttys3 < 0)
        return EXIT_FAILURE;

    fd_ttys4 = open_uart(UART_TTYS4);
    if (fd_ttys4 < 0) {
        close(fd_ttys3);
        return EXIT_FAILURE;
    }

    printf("uart_bridge started: %s <-> %s, auto fish frame to %s every %d ms\n",
           UART_TTYS3, UART_TTYS4, UART_TTYS4, FISH_INTERVAL_MS);
    fflush(stdout);

    next_fish_ms = now_ms() + FISH_INTERVAL_MS;

    while (g_running) {
        struct pollfd pfds[2];
        long long now = now_ms();
        int timeout_ms = 0;
        int ret;

        if (now >= next_fish_ms) {
            send_fish_frame(fd_ttys4);
            next_fish_ms = now + FISH_INTERVAL_MS;
        }

        timeout_ms = (int)(next_fish_ms - now_ms());
        if (timeout_ms < 0)
            timeout_ms = 0;

        pfds[0].fd = fd_ttys3;
        pfds[0].events = POLLIN;
        pfds[0].revents = 0;
        pfds[1].fd = fd_ttys4;
        pfds[1].events = POLLIN;
        pfds[1].revents = 0;

        ret = poll(pfds, 2, timeout_ms);
        if (ret < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "poll failed: %s\n", strerror(errno));
            break;
        }

        if (ret == 0)
            continue;

        if (pfds[0].revents & POLLIN)
            forward_data(fd_ttys3, UART_TTYS3, fd_ttys4, UART_TTYS4);
        if (pfds[1].revents & POLLIN)
            forward_data(fd_ttys4, UART_TTYS4, fd_ttys3, UART_TTYS3);

        if (pfds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
            fprintf(stderr, "%s poll event error: 0x%x\n", UART_TTYS3,
                    pfds[0].revents);
        if (pfds[1].revents & (POLLERR | POLLHUP | POLLNVAL))
            fprintf(stderr, "%s poll event error: 0x%x\n", UART_TTYS4,
                    pfds[1].revents);
    }

    close(fd_ttys4);
    close(fd_ttys3);
    printf("uart_bridge stopped\n");
    return EXIT_SUCCESS;
}
