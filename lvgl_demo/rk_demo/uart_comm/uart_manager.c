#include <stdint.h>
#include <stdio.h>      /* Standard input/output */
#include <fcntl.h>      /* File control definitions */
#include <string.h>
#include <stdlib.h>
#include <termio.h>     /* POSIX terminal control definitions */
#include <sys/time.h>   /* Time structures for select() */
#include <unistd.h>     /* POSIX Symbolic Constants */
#include <assert.h>
#include <errno.h>      /* Error definitions */
#include <sys/mman.h>
#include <pthread.h>
#include "uart_manager.h"
#include <lvgl/lvgl.h>

#define CMSPAR 010000000000

#define SERIAL_FLUSH_TX     1
#define SERIAL_FLUSH_RX     2

#define SERIAL_PARITY_NO        0
#define SERIAL_PARITY_ODD       1
#define SERIAL_PARITY_EVENT     2
#define SERIAL_PARITY_MARK      3
#define SERIAL_PARITY_SPACE     4

#define SERIAL_STOPBIT_ONE      1
#define SERIAL_STOPBIT_TWO      2

static int serial_fd = -1;
static struct termios old_tios;
static pthread_t recv_thread;
static int thread_run = 0;
static uart_recv_cb_t user_cb = NULL;
static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;

void serial_print_buf(const char *direction, unsigned char *buf, int len)
{
    int i;
    char hex_str[1024] = {0};
    char temp[8] = {0};
    const char *tag = direction ? direction : "packet";
    
    // 打印字符串形式 (仅用于调试，注意二进制数据可能含有 \0) 
    // LV_LOG_USER("[UART] buf: %s", (char *)buf); 

    for (i = 0; i < len && i < 200; i++) {
        snprintf(temp, sizeof(temp), "%02X ", buf[i]);
        strcat(hex_str, temp);
    }
    LV_LOG_USER("[UART] %s message: %s", tag, hex_str);
}

static int termios_init(struct termios *tios, int baud, int parity, int data_bits, int stop_bits)
{
    speed_t baud_rate;

    if (tios == NULL) {
        return -1;
    }

    tios->c_line = 0;

    tios->c_cc[VMIN ] = 0;
    tios->c_cc[VTIME] = 0;

    /* configure the input modes... */
    tios->c_iflag = IGNBRK | IGNPAR | INPCK;

    /* configure the output modes... */
    tios->c_oflag = 0;     /* enable implementation-defined output processing */
    /* configure the control modes... */
    tios->c_cflag = CREAD | CLOCAL;

    if (data_bits == 5) {
        tios->c_cflag |= CS5;
    } else if (data_bits == 6) {
        tios->c_cflag |= CS6;
    } else if (data_bits == 7) {
        tios->c_cflag |= CS7;
    } else if (data_bits == 8) {
        tios->c_cflag |= CS8;
    } else {
        return -1;
    }

    if (stop_bits == 1) {
        tios->c_cflag &= ~ CSTOPB;
    } else if (stop_bits == 2) {
        tios->c_cflag |= CSTOPB;
    } else {
        return -1;
    }

    if (parity == 0) { /* none */
        tios->c_cflag &= ~ PARENB;
        tios->c_cflag &= ~ PARODD;
    } else if (parity == 2) { /* even */
        tios->c_cflag |= PARENB;
        tios->c_cflag &= ~ PARODD;
    } else if (parity == 1) { /* odd */
        tios->c_cflag |= PARENB;
        tios->c_cflag |= PARODD;
    } else if (parity == 3) { /* mark */
        tios->c_cflag |= PARENB;
        tios->c_cflag |= CMSPAR;
        tios->c_cflag |= PARODD;
    } else if (parity == 4) { /* space */
        tios->c_cflag |= PARENB;
        tios->c_cflag |= CMSPAR;
    } else {
        return -1;
    }

    /* configure the local modes... */
    tios->c_lflag = 0;    /* enable implementation-defined input processing   */

    /* Set the baud rate */
    switch (baud) {
        case 110:
            baud_rate = B110;    break;
        case 300:
            baud_rate = B300;    break;
        case 600:
            baud_rate = B600;    break;
        case 1200:
            baud_rate = B1200;   break;
        case 2400:
            baud_rate = B2400;   break;
        case 4800:
            baud_rate = B4800;   break;
        case 9600:
            baud_rate = B9600;   break;
        case 19200:
            baud_rate = B19200;  break;
        case 38400:
            baud_rate = B38400;  break;
        case 57600:
            baud_rate = B57600;  break;
        case 115200:
            baud_rate = B115200; break;
        case 230400:
            baud_rate = B230400; break;
        case 460800:
            baud_rate = B460800; break;
        case 576000:
            baud_rate = B576000; break;
        case 921600:
            baud_rate = B921600; break;
        default:
            return -1;
    }

    if ((cfsetispeed(tios, baud_rate) < 0) || (cfsetospeed(tios, baud_rate) < 0)) {
        return -1;
    }
    return 0;
}

static int serial_open(const char *serial_name, int baud, int parity, int data_bits, int stop_bits)
{
    struct termios settings;
    int fd;

    if (serial_fd >= 0) {
        return 0;
    }

    if ((fd = open(serial_name, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
        return 0;
    }

    if (tcgetattr(fd, &old_tios) < 0) {
        close(fd);
        return 0;
    }

    settings = old_tios;
    if (termios_init(&settings, baud, parity, data_bits, stop_bits) < 0) {
        close(fd);
        return 0;
    }

    if (tcsetattr(fd, TCSANOW, &settings) < 0) {
        close(fd);
        return 0;
    }

    serial_fd = fd;
    return 1;
}

static int serial_close(void)
{
    if (serial_fd < 0) {
        return 0;
    }
    tcsetattr(serial_fd, TCSANOW, &(old_tios));
    close(serial_fd);
    serial_fd = -1;

    return 1;
}

static int serial_read(unsigned char *buf, size_t size)
{
    int readsize = 0;

    if (size <= 0) {
        return 0;
    }

    readsize = read(serial_fd, buf, size);
    if (readsize > 0) {
        LV_LOG_USER("[UART] read: length=%d", readsize);
        serial_print_buf("read", buf, readsize);
    }
    return readsize;
}

static int serial_poll(int timeout_ms)
{
    if (serial_fd < 0) {
        return -1;
    }
    
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(serial_fd, &rfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return select(serial_fd + 1, &rfds, NULL, NULL, &tv);
}

static void *uart_recv_thread_func(void *arg)
{
    unsigned char buffer[256];
    LV_LOG_USER("[UART] Receiver thread started using serial.c logic");

    while (thread_run) {
        if (serial_poll(100) > 0) {
            int len = serial_read(buffer, sizeof(buffer));
            if (len > 0 && user_cb) {
                user_cb(buffer, (uint16_t)len);
            }
        }
    }
    return NULL;
}

int uart_manager_init(const char *device, int baudrate)
{
    LV_LOG_USER("[UART] Initializing %s at %d baud", device, baudrate);

    if (serial_open(device, baudrate, SERIAL_PARITY_NO, 8, SERIAL_STOPBIT_ONE) != 1) {
        LV_LOG_ERROR("[UART] Failed to open %s", device);
        return -1;
    }

    thread_run = 1;
    if (pthread_create(&recv_thread, NULL, uart_recv_thread_func, NULL) != 0) {
        LV_LOG_ERROR("[UART] Failed to create recv thread");
        serial_close();
        return -1;
    }

    return 0;
}

void uart_manager_register_cb(uart_recv_cb_t cb)
{
    user_cb = cb;
}

int uart_manager_send(uint8_t *data, uint16_t len)
{
    if (serial_fd < 0) {
        return -1;
    }
    pthread_mutex_lock(&send_mutex);
    int ret = write(serial_fd, data, len);
    pthread_mutex_unlock(&send_mutex);
    if (ret > 0) {
        LV_LOG_USER("[UART] sent %d bytes", ret);
        serial_print_buf("send", data, ret);
    }
    return ret;
}

void uart_manager_deinit(void)
{
    thread_run = 0;
    pthread_join(recv_thread, NULL);
    serial_close();
}
