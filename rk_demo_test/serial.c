#include "serial.h"
#include "system_manager.h"
#include "app_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEV_NAME        "/dev/ttyS3"
#define UART_BAUD       B115200
#define SERIAL_FRAME_LEN        30u
#define SERIAL_RX_BUFFER_SIZE   256u
#define SERIAL_TX_INTERVAL_MS   1000u
#define SERIAL_RECONNECT_INTERVAL_MS 2000u

#define SERIAL_LOG_USER(...) APP_LOG_USER("SERIAL", __VA_ARGS__)
#define SERIAL_LOG_WARN(...) APP_LOG_WARN("SERIAL", __VA_ARGS__)
#define SERIAL_LOG_ERROR(...) APP_LOG_ERROR("SERIAL", __VA_ARGS__)

#define FRAME_HEAD1     0x55u
#define FRAME_HEAD2     0xAAu

#define LINK_MODE_BUCKET_ONLY   0x01u
#define LINK_MODE_BUCKET_BASE   0x02u

#define CMD_KEEPALIVE           0x00u

// 桶体命令 (0xA0-0xA9)
#define CMD_BUCKET_SHUTDOWN     0xA0u
#define CMD_BUCKET_STANDBY      0xA1u
#define CMD_BUCKET_HEAT         0xA2u
#define CMD_BUCKET_STOP_HEAT    0xA3u
#define CMD_BUCKET_MASSAGE      0xA4u
#define CMD_BUCKET_UV           0xA5u
#define CMD_BUCKET_TIMER        0xA6u
#define CMD_BUCKET_STOP_ALL     0xA7u
#define CMD_BUCKET_SELF_CHECK   0xA8u
#define CMD_BUCKET_LOW_POWER    0xA9u

#define BUCKET_HEAT_TEMP_MIN    35u
#define BUCKET_HEAT_TEMP_MAX    48u
#define SERIAL_CMD_QUEUE_CAPACITY 16u

// 基站命令 (0xB0-0xB8)
#define CMD_BASE_SHUTDOWN       0xB0u
#define CMD_BASE_STANDBY        0xB1u
#define CMD_BASE_WATER          0xB2u
#define CMD_BASE_AUTO_CLEAN     0xB3u
#define CMD_BASE_FORCE_DRAIN    0xB4u
#define CMD_BASE_CLEAN_SPRAY    0xB5u
#define CMD_BASE_CLEAN_WATER    0xB6u
#define CMD_BASE_HOT_DRY        0xB7u
#define CMD_BASE_SELF_CHECK     0xB8u

// ========== 调试/发布模式配置 ==========
// 调试模式: 1 -> 使用TCP连接服务器测试, 0 -> 正式发布使用串口
#define IS_DEBUG        0   // 发布时改为0

#if IS_DEBUG
#define TCP_SERVER_IP   "192.168.0.138"   // 测试服务器IP
#define TCP_SERVER_PORT 8888            // 测试服务器端口
#endif

/** 待发送的一次性命令结构 */
typedef struct
{
    uint8_t cmd_type;           // 命令类型
    uint8_t link_mode;          // 链接模式
    int16_t temperature;        // 入队时的目标温度快照
    uint32_t timer_seconds;     // 入队时的定时档位快照
    int massage_level;          // 入队时的按摩档位快照
    bool sterilization_on;      // 入队时的除菌状态快照
    bool    base_data_valid;    // 是否使用自定义基站数据区
    uint8_t base_data[9];       // data[16]-data[24] 基站数据区
} pending_cmd_t;

typedef struct
{
    int fd;                     // 串口或socket的fd
    bool running;
    bool initialized;
    pthread_t thread;
    uint8_t rx_buffer[SERIAL_RX_BUFFER_SIZE];
    size_t rx_len;
    uint8_t last_link_status;
    bool has_link_status_report;
    uint8_t last_bucket_main_status;
    uint8_t last_base_main_status;
    int16_t last_bucket_temp;
    uint64_t last_rx_ms;
    pthread_mutex_t tx_lock;
    pending_cmd_t cmd_queue[SERIAL_CMD_QUEUE_CAPACITY];
    size_t cmd_queue_head;
    size_t cmd_queue_count;
} serial_context_t;

static serial_context_t g_serial =
{
    .fd = -1,
    .running = false,
    .initialized = false,
    .rx_len = 0u,
    .last_link_status = 0x01u,
    .has_link_status_report = false,
    .last_bucket_main_status = 0x03u,
    .last_base_main_status = 0x03u,
    .last_bucket_temp = 0,
    .last_rx_ms = 0u,
    .tx_lock = PTHREAD_MUTEX_INITIALIZER,
    .cmd_queue = {{0}},
    .cmd_queue_head = 0u,
    .cmd_queue_count = 0u,
};
static uint32_t g_mcu_status_report_sequence = 0u;
static uint8_t g_mcu_liquid_shortage_mask = 0u;
static serial_mcu_error_state_t g_mcu_error_state = {0};
static bool g_mcu_error_state_valid = false;
static pthread_mutex_t g_mcu_error_state_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    uint8_t mask;
    uint8_t bit;
    const char *name;
} serial_error_descriptor_t;

static const serial_error_descriptor_t g_bucket_error1_descriptors[] =
{
    {SERIAL_BUCKET_ERROR1_LACK_WATER, 0u, "桶体缺水"},
    {SERIAL_BUCKET_ERROR1_DRAIN_TIMEOUT, 1u, "排水超时"},
    {SERIAL_BUCKET_ERROR1_TEMPERATURE, 2u, "温度异常"},
    {SERIAL_BUCKET_ERROR1_HEATING_TIMEOUT, 3u, "加热超时"},
    {SERIAL_BUCKET_ERROR1_TEMP_SENSOR, 4u, "温度传感器故障"},
};

static const serial_error_descriptor_t g_bucket_error2_descriptors[] =
{
    {SERIAL_BUCKET_ERROR2_WATER_PUMP, 1u, "水泵故障"},
    {SERIAL_BUCKET_ERROR2_MASSAGE_MOTOR, 3u, "按摩电机故障"},
    {SERIAL_BUCKET_ERROR2_BATTERY_VOLTAGE, 5u, "电池电压异常"},
};

static const serial_error_descriptor_t g_base_error1_descriptors[] =
{
    {SERIAL_BASE_ERROR1_LIQUID_SENSOR_BOARD, 3u, "药液传感器板故障"},
};

static uint64_t get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000u) + ((uint64_t)tv.tv_usec / 1000u);
}

static uint8_t clamp_u8(int value, int min_value, int max_value)
{
    if (value < min_value) return (uint8_t)min_value;
    if (value > max_value) return (uint8_t)max_value;
    return (uint8_t)value;
}

// 二进制转换成16进制
static void dump_hex(const char *tag, const uint8_t *buf, size_t len)
{
    char hex[SERIAL_RX_BUFFER_SIZE * 3u + 1u] = {0};
    size_t index;
    size_t offset = 0u;
    size_t max_len = len;

    if (tag == NULL || buf == NULL)
    {
        return;
    }

    if (max_len > SERIAL_RX_BUFFER_SIZE)
    {
        max_len = SERIAL_RX_BUFFER_SIZE;
    }

    for (index = 0u; index < max_len && offset < sizeof(hex); ++index)
    {
        int written = snprintf(hex + offset, sizeof(hex) - offset,
                               "%s%02X", index == 0u ? "" : " ", buf[index]);
        if (written < 0 || (size_t)written >= sizeof(hex) - offset)
        {
            break;
        }
        offset += (size_t)written;
    }

    SERIAL_LOG_USER("%s len=%zu data=%s%s", tag, len, hex,
                    len > max_len ? " ..." : "");
}

static uint8_t calc_checksum(const uint8_t *frame)
{
    unsigned int sum = 0u;
    size_t index;
    for (index = 2u; index < (SERIAL_FRAME_LEN - 1u); ++index) sum += frame[index];
    return (uint8_t)(sum & 0xFFu);
}

static uint8_t water_level_to_protocol(int level)
{
    if (level <= 0) return 0x06u;
    if (level == 1) return 0x08u;
    return 0x0Cu;
}

static int protocol_water_level_to_ui_level(uint8_t level)
{
    if (level == 0u) return 0;
    if (level <= 0x06u) return 1;
    if (level <= 0x0Au) return 2;
    return 3;
}

static uint8_t timer_seconds_to_protocol(uint32_t seconds)
{
    switch (seconds)
    {
    case 0:
        return 0x00u;
    case 600:
        return 0x01u;
    case 900:
        return 0x02u;
    case 1200:
        return 0x03u;
    case 1500:
        return 0x04u;
    case 1800:
        return 0x05u;
    default:
        return 0x00u;
    }
}

static void disconnect_transport(const char *reason)
{
    if (g_serial.fd >= 0)
    {
        if (reason != NULL)
        {
            SERIAL_LOG_WARN("disconnected: %s", reason);
        }
        close(g_serial.fd);
        g_serial.fd = -1;
    }
    g_serial.rx_len = 0u;
    g_serial.last_rx_ms = 0u;
}

#if IS_DEBUG
static int tcp_connect(const char *ip, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        SERIAL_LOG_ERROR("socket failed: %s", strerror(errno));
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
    {
        SERIAL_LOG_ERROR("inet_pton failed: %s", strerror(errno));
        close(sock);
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        SERIAL_LOG_ERROR("connect failed: %s", strerror(errno));
        close(sock);
        return -1;
    }
    // 设置非阻塞以便select使用
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    return sock;
}
#endif

static int configure_serial_port(int fd)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        SERIAL_LOG_ERROR("tcgetattr failed: %s", strerror(errno));
        return -1;
    }
    cfmakeraw(&tty);
    cfsetispeed(&tty, UART_BAUD);
    cfsetospeed(&tty, UART_BAUD);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        SERIAL_LOG_ERROR("tcsetattr failed: %s", strerror(errno));
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    return 0;
}

static int connect_transport(void)
{
#if IS_DEBUG
    int fd = tcp_connect(TCP_SERVER_IP, TCP_SERVER_PORT);
    if (fd < 0)
    {
        SERIAL_LOG_ERROR("TCP connect to %s:%d failed", TCP_SERVER_IP, TCP_SERVER_PORT);
        return -1;
    }
    SERIAL_LOG_USER("TCP connected to %s:%d", TCP_SERVER_IP, TCP_SERVER_PORT);
    return fd;
#else
    int fd = open(DEV_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        SERIAL_LOG_ERROR("open %s failed: %s", DEV_NAME, strerror(errno));
        return -1;
    }
    if (configure_serial_port(fd) < 0)
    {
        close(fd);
        return -1;
    }
    SERIAL_LOG_USER("opened %s at 115200 baud", DEV_NAME);
    return fd;
#endif
}

static uint8_t get_keepalive_link_mode(void)
{
    if (!g_serial.has_link_status_report)
    {
        return LINK_MODE_BUCKET_BASE;
    }
    if (g_serial.last_link_status == 0x00u)
    {
        return LINK_MODE_BUCKET_ONLY;
    }
    return LINK_MODE_BUCKET_BASE;
}

/**
 * 构建发送帧
 *
 * 设计原则：
 * - 若一次性命令 FIFO 非空，则按入队顺序发送一条命令
 * - 业务命令使用自身命令族固定链接模式，摸鱼帧才根据上报的 LINK_STATUS 派生链接模式
 * - 温度、定时等参数在命令入队时快照，避免后续 UI 修改改变已排队帧
 */
static void build_tx_frame(uint8_t frame[SERIAL_FRAME_LEN])
{
    uint8_t link_mode;
    uint8_t cmd_type;
    pending_cmd_t pending = {0};
    bool has_pending = false;

    pthread_mutex_lock(&g_serial.tx_lock);
    if (g_serial.cmd_queue_count > 0u)
    {
        pending = g_serial.cmd_queue[g_serial.cmd_queue_head];
        g_serial.cmd_queue_head = (g_serial.cmd_queue_head + 1u) %
                                  SERIAL_CMD_QUEUE_CAPACITY;
        g_serial.cmd_queue_count--;
        cmd_type = pending.cmd_type;
        link_mode = pending.link_mode;
        has_pending = true;
    }
    pthread_mutex_unlock(&g_serial.tx_lock);

    if (!has_pending)
    {
        // 无事可做 → 发送摸鱼帧
        link_mode = get_keepalive_link_mode();
        cmd_type = CMD_KEEPALIVE;
    }

    memset(frame, 0, SERIAL_FRAME_LEN);
    frame[0] = FRAME_HEAD1;
    frame[1] = FRAME_HEAD2;
    frame[2] = link_mode;
    frame[3] = cmd_type;
    frame[4] = 0x00;
    frame[5] = 0x00;
    frame[6] = (cmd_type == CMD_BUCKET_HEAT || cmd_type == CMD_BASE_WATER) ?
               clamp_u8(pending.temperature,
                        BUCKET_HEAT_TEMP_MIN, BUCKET_HEAT_TEMP_MAX) :
               0x00;
    frame[7] = (cmd_type == CMD_BUCKET_MASSAGE) ?
               clamp_u8(pending.massage_level, 0, 3) :
               0x00;
    frame[8] = (cmd_type == CMD_BUCKET_UV && pending.sterilization_on) ?
               0x01u : 0x00u;
    frame[9] = (cmd_type == CMD_BUCKET_TIMER) ?
               timer_seconds_to_protocol(pending.timer_seconds) :
               0x00;

    if (pending.base_data_valid)
    {
        /* 使用命令入队时保存的基站数据快照。 */
        memcpy(&frame[16], pending.base_data, sizeof(pending.base_data));
    }

    frame[SERIAL_FRAME_LEN - 1u] = calc_checksum(frame);
}

static void log_error_field_changes(
    uint8_t data_index,
    uint8_t previous_value,
    uint8_t current_value,
    const serial_error_descriptor_t *descriptors,
    size_t descriptor_count)
{
    uint8_t changed_value;
    size_t index;

    changed_value = previous_value ^ current_value;
    for (index = 0u; index < descriptor_count; ++index)
    {
        const serial_error_descriptor_t *descriptor = &descriptors[index];

        if ((changed_value & descriptor->mask) == 0u)
        {
            continue;
        }
        SERIAL_LOG_USER("MCU故障%s: %s (data[%u] Bit%u)",
                        (current_value & descriptor->mask) != 0u ?
                        "出现" : "恢复",
                        descriptor->name,
                        data_index,
                        descriptor->bit);
    }
}

static void update_mcu_error_state(const uint8_t frame[SERIAL_FRAME_LEN])
{
    serial_mcu_error_state_t previous_state;
    serial_mcu_error_state_t current_state =
    {
        .bucket_error1 = frame[14] & SERIAL_BUCKET_ERROR1_MASK,
        .bucket_error2 = frame[15] & SERIAL_BUCKET_ERROR2_MASK,
        .base_error1 = frame[25] & SERIAL_BASE_ERROR1_MASK,
    };

    pthread_mutex_lock(&g_mcu_error_state_lock);
    previous_state = g_mcu_error_state;
    g_mcu_error_state = current_state;
    g_mcu_error_state_valid = true;
    pthread_mutex_unlock(&g_mcu_error_state_lock);

    log_error_field_changes(14u, previous_state.bucket_error1,
                            current_state.bucket_error1,
                            g_bucket_error1_descriptors,
                            sizeof(g_bucket_error1_descriptors) /
                            sizeof(g_bucket_error1_descriptors[0]));
    log_error_field_changes(15u, previous_state.bucket_error2,
                            current_state.bucket_error2,
                            g_bucket_error2_descriptors,
                            sizeof(g_bucket_error2_descriptors) /
                            sizeof(g_bucket_error2_descriptors[0]));
    log_error_field_changes(25u, previous_state.base_error1,
                            current_state.base_error1,
                            g_base_error1_descriptors,
                            sizeof(g_base_error1_descriptors) /
                            sizeof(g_base_error1_descriptors[0]));
}

/**
 * 从接收到的协议帧中解析状态，并更新运行时状态变量
 */
static void update_runtime_state_from_rx(const uint8_t frame[SERIAL_FRAME_LEN])
{
    uint8_t liquid_shortage_mask;
    uint8_t previous_liquid_shortage_mask;

    bucket_main_status = frame[12];
    base_main_status = frame[23];

    /* 缺液三位维持独立UI链路，不与通用故障快照重复。 */
    liquid_shortage_mask = frame[25] & SERIAL_LIQUID_SHORTAGE_MASK;
    previous_liquid_shortage_mask = __atomic_exchange_n(
                                        &g_mcu_liquid_shortage_mask,
                                        liquid_shortage_mask,
                                        __ATOMIC_ACQ_REL);
    update_mcu_error_state(frame);

    g_serial.last_link_status = frame[4];
    g_serial.has_link_status_report = true;
    g_serial.last_bucket_main_status = bucket_main_status;
    g_serial.last_base_main_status = base_main_status;
    g_serial.last_bucket_temp = frame[6];
    g_serial.last_rx_ms = get_time_ms();

    last_link_status = frame[4];
    has_mcu_status_report = true;
    system_ui_schedule_link_status(frame[4]);
    if (liquid_shortage_mask != previous_liquid_shortage_mask)
    {
        system_ui_schedule_liquid_shortage(liquid_shortage_mask);
    }
    if (last_link_status == 0x00)
    {
        bucket_temp_update_actual_from_mcu((int16_t)frame[6]);
    }
    current_water_level = protocol_water_level_to_ui_level(frame[5]);
    // 更新 MCU 状态上报序列号，每收到一个 g_mcu_status_report_sequence就+1
    (void)__atomic_add_fetch(&g_mcu_status_report_sequence, 1u,
                             __ATOMIC_RELEASE);
}

// 获取最近一次 MCU 状态上报的序列号g_mcu_status_report_sequence
uint32_t serial_mcu_status_report_sequence(void)
{
    return __atomic_load_n(&g_mcu_status_report_sequence, __ATOMIC_ACQUIRE);
}

uint8_t serial_mcu_liquid_shortage_mask(void)
{
    return __atomic_load_n(&g_mcu_liquid_shortage_mask, __ATOMIC_ACQUIRE);
}

bool serial_mcu_error_state_get(serial_mcu_error_state_t *state)
{
    bool valid;

    if (state == NULL)
    {
        return false;
    }

    pthread_mutex_lock(&g_mcu_error_state_lock);
    valid = g_mcu_error_state_valid;
    if (valid)
    {
        *state = g_mcu_error_state;
    }
    pthread_mutex_unlock(&g_mcu_error_state_lock);
    return valid;
}

/**
 * 发送当前状态到串口或TCP服务器
 */
static void send_protocol_frame(void)
{
    uint8_t frame[SERIAL_FRAME_LEN];
    ssize_t written;

    if (g_serial.fd < 0) return;

    build_tx_frame(frame);

#if IS_DEBUG
    // TCP发送
    written = write(g_serial.fd, frame, sizeof(frame));
#else
    // 串口发送
    written = write(g_serial.fd, frame, sizeof(frame));
    if (written >= 0) tcdrain(g_serial.fd);
#endif

    if (written < 0)
    {
        SERIAL_LOG_ERROR("write failed: %s", strerror(errno));
        disconnect_transport(strerror(errno));
        return;
    }
    if ((size_t)written != sizeof(frame))
    {
        SERIAL_LOG_WARN("write incomplete: %zd/%u", written, SERIAL_FRAME_LEN);
        return;
    }

    dump_hex("TX", frame, sizeof(frame));
}

/**
 * 将接收缓冲区中从offset开始的数据前移，丢弃前offset字节
 */
static void shift_rx_buffer(size_t offset)
{
    if (offset >= g_serial.rx_len)
    {
        g_serial.rx_len = 0u;
        return;
    }
    memmove(g_serial.rx_buffer, g_serial.rx_buffer + offset, g_serial.rx_len - offset);
    g_serial.rx_len -= offset;
}

/**
 * 处理接收缓冲区中的数据，尝试解析出完整的协议帧，并更新运行时状态
 */
static void consume_rx_frames(void)
{
    while (g_serial.rx_len >= SERIAL_FRAME_LEN)
    {
        size_t start_index = 0u;
        while (start_index + 1u < g_serial.rx_len)
        {
            if (g_serial.rx_buffer[start_index] == FRAME_HEAD1 &&
                    g_serial.rx_buffer[start_index + 1u] == FRAME_HEAD2) break;
            start_index++;
        }
        if (start_index > 0u)
        {
            SERIAL_LOG_WARN("drop %zu noise byte(s) before frame", start_index);
            shift_rx_buffer(start_index);
            if (g_serial.rx_len < SERIAL_FRAME_LEN)
            {
                return;
            }
        }
        if (g_serial.rx_buffer[0] != FRAME_HEAD1 || g_serial.rx_buffer[1] != FRAME_HEAD2)
        {
            SERIAL_LOG_WARN("parse failed: invalid frame header");
            g_serial.rx_len = 0u;
            return;
        }
        uint8_t calc = calc_checksum(g_serial.rx_buffer);
        if (calc != g_serial.rx_buffer[SERIAL_FRAME_LEN - 1u])
        {
            SERIAL_LOG_WARN("parse failed: checksum calc=0x%02X recv=0x%02X",
                            calc, g_serial.rx_buffer[SERIAL_FRAME_LEN - 1u]);
            dump_hex("RX BAD", g_serial.rx_buffer, SERIAL_FRAME_LEN);
            shift_rx_buffer(1u);
            continue;
        }
        dump_hex("RX", g_serial.rx_buffer, SERIAL_FRAME_LEN);
        update_runtime_state_from_rx(g_serial.rx_buffer);
        SERIAL_LOG_USER("parse ok: link_status=0x%02X bucket_status=0x%02X base_status=0x%02X temp=%d water=%d",
                        g_serial.last_link_status,
                        g_serial.last_bucket_main_status,
                        g_serial.last_base_main_status,
                        g_serial.last_bucket_temp,
                        current_water_level);
        shift_rx_buffer(SERIAL_FRAME_LEN);
    }
}

/**
 * 尝试从接收缓冲区中读取数据，并调用consume_rx_frames解析
 */
static void try_read_data(void)
{
    ssize_t bytes_read;
    if (g_serial.fd < 0) return;
    if (g_serial.rx_len >= SERIAL_RX_BUFFER_SIZE) g_serial.rx_len = 0u;
    bytes_read = read(g_serial.fd,
                      g_serial.rx_buffer + g_serial.rx_len,
                      SERIAL_RX_BUFFER_SIZE - g_serial.rx_len);
    if (bytes_read < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        {
            SERIAL_LOG_ERROR("read failed: %s", strerror(errno));
            disconnect_transport(strerror(errno));
        }
        return;
    }
    if (bytes_read == 0)
    {
#if IS_DEBUG
        disconnect_transport("peer closed connection");
#endif
        return;
    }
    SERIAL_LOG_USER("MCU upload received: len=%zd", bytes_read);
    dump_hex("MCU RX CHUNK", g_serial.rx_buffer + g_serial.rx_len, (size_t)bytes_read);
    g_serial.rx_len += (size_t)bytes_read;
    consume_rx_frames();
}

static void *worker_thread(void *arg)
{
    uint64_t next_tx_ms = get_time_ms();
    uint64_t next_reconnect_ms = 0u;
    (void)arg;

    while (g_serial.running)
    {
        fd_set read_fds;
        struct timeval timeout;
        uint64_t now_ms = get_time_ms();
        uint64_t wait_ms = (now_ms >= next_tx_ms) ? 0u : (next_tx_ms - now_ms);
        int ret;

        if (g_serial.fd < 0)
        {
            if (now_ms >= next_reconnect_ms)
            {
                g_serial.fd = connect_transport();
                next_reconnect_ms = now_ms + SERIAL_RECONNECT_INTERVAL_MS;
                if (g_serial.fd >= 0) next_tx_ms = now_ms;
            }
            now_ms = get_time_ms();
            wait_ms = (now_ms >= next_reconnect_ms) ? 0u : (next_reconnect_ms - now_ms);
        }


        timeout.tv_sec = (time_t)(wait_ms / 1000u);
        timeout.tv_usec = (suseconds_t)((wait_ms % 1000u) * 1000u);

        FD_ZERO(&read_fds);
        if (g_serial.fd >= 0)
        {
            FD_SET(g_serial.fd, &read_fds);
            ret = select(g_serial.fd + 1, &read_fds, NULL, NULL, &timeout);
        }
        else
        {
            ret = select(0, NULL, NULL, NULL, &timeout);
        }
        if (ret < 0)
        {
            if (errno != EINTR) SERIAL_LOG_ERROR("select failed: %s", strerror(errno));
            continue;
        }
        if (g_serial.fd >= 0 && ret > 0 && FD_ISSET(g_serial.fd, &read_fds)) try_read_data();

        now_ms = get_time_ms();
        if (g_serial.fd >= 0 && now_ms >= next_tx_ms)
        {
            send_protocol_frame();
            next_tx_ms = now_ms + SERIAL_TX_INTERVAL_MS;
        }
    }
    disconnect_transport("worker stopped");
    return NULL;
}

void serial_init(void)
{
    int ret;

    if (g_serial.initialized) return;

    g_serial.running = true;
    ret = pthread_create(&g_serial.thread, NULL, worker_thread, NULL);
    if (ret != 0)
    {
        SERIAL_LOG_ERROR("pthread_create failed: %s", strerror(ret));
        g_serial.running = false;
        disconnect_transport("pthread_create failed");
        g_serial.fd = -1;
        return;
    }
    pthread_detach(g_serial.thread);
    g_serial.initialized = true;
}

// ========== 待发送命令辅助函数 ==========

static void enqueue_cmd_locked(const pending_cmd_t *cmd)
{
    size_t tail;

    if (g_serial.cmd_queue_count >= SERIAL_CMD_QUEUE_CAPACITY)
    {
        SERIAL_LOG_WARN("命令队列已满，丢弃 cmd_type=0x%02X", cmd->cmd_type);
        return;
    }

    tail = (g_serial.cmd_queue_head + g_serial.cmd_queue_count) %
           SERIAL_CMD_QUEUE_CAPACITY;
    g_serial.cmd_queue[tail] = *cmd;
    g_serial.cmd_queue_count++;
}

/** 设置一个不带基站数据的一次性命令 */
static void set_pending_cmd(uint8_t cmd_type, uint8_t link_mode)
{
    pending_cmd_t cmd =
    {
        .cmd_type = cmd_type,
        .link_mode = link_mode,
        .temperature = temp_set,
        .timer_seconds = timer_set,
        .massage_level = massage_intensity,
        .sterilization_on = sterilization,
        .base_data_valid = false,
    };

    pthread_mutex_lock(&g_serial.tx_lock);
    enqueue_cmd_locked(&cmd);
    pthread_mutex_unlock(&g_serial.tx_lock);
}

/** 设置一个携带基站数据区的一次性命令 */
static void set_pending_cmd_with_base(uint8_t cmd_type, uint8_t link_mode,
                                      const uint8_t base_data[9])
{
    pending_cmd_t cmd =
    {
        .cmd_type = cmd_type,
        .link_mode = link_mode,
        .temperature = temp_set,
        .timer_seconds = timer_set,
        .massage_level = massage_intensity,
        .sterilization_on = sterilization,
        .base_data_valid = base_data != NULL,
    };

    if (base_data != NULL)
    {
        memcpy(cmd.base_data, base_data, sizeof(cmd.base_data));
    }

    pthread_mutex_lock(&g_serial.tx_lock);
    enqueue_cmd_locked(&cmd);
    pthread_mutex_unlock(&g_serial.tx_lock);
}


// ---------- 桶体命令 (0xA0-0xA9) ----------

void serial_bucket_shutdown(void)
{
    set_pending_cmd(CMD_BUCKET_SHUTDOWN, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_standby(void)
{
    set_pending_cmd(CMD_BUCKET_STANDBY, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_heat(void)
{
    set_pending_cmd(CMD_BUCKET_HEAT, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_stop_heat(void)
{
    set_pending_cmd(CMD_BUCKET_STOP_HEAT, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_massage(void)
{
    set_pending_cmd(CMD_BUCKET_MASSAGE, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_uv(void)
{
    set_pending_cmd(CMD_BUCKET_UV, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_timer(void)
{
    set_pending_cmd(CMD_BUCKET_TIMER, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_stop_all(void)
{
    set_pending_cmd(CMD_BUCKET_STOP_ALL, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_self_check(void)
{
    set_pending_cmd(CMD_BUCKET_SELF_CHECK, LINK_MODE_BUCKET_ONLY);
}

void serial_bucket_low_power(void)
{
    set_pending_cmd(CMD_BUCKET_LOW_POWER, LINK_MODE_BUCKET_ONLY);
}

// ---------- 基站命令 (0xB0-0xB8) ----------

void serial_base_shutdown(void)
{
    set_pending_cmd(CMD_BASE_SHUTDOWN, LINK_MODE_BUCKET_BASE);
}

void serial_base_standby(void)
{
    set_pending_cmd(CMD_BASE_STANDBY, LINK_MODE_BUCKET_BASE);
}

void serial_base_water(void)
{
    uint8_t base_data[9] = {0};
    base_data[0] = water_level_to_protocol(water_level);    // data[16] 目标水位
    base_data[4] = use_drug1 ? 0x05u : 0x00u;              // data[20] 药液1
    base_data[5] = use_drug2 ? 0x05u : 0x00u;              // data[21] 药液2
    set_pending_cmd_with_base(CMD_BASE_WATER, LINK_MODE_BUCKET_BASE, base_data);
}

void serial_base_auto_clean()
{
    uint8_t base_data[9] = {0};
    base_data[1] = 2;       // data[17] 热水喷淋时间(min)
    base_data[2] = 2;       // data[18] 清水喷淋时间(min)
    base_data[3] = 1;       // data[19] 烘干时间(x10min)
    set_pending_cmd_with_base(CMD_BASE_AUTO_CLEAN, LINK_MODE_BUCKET_BASE, base_data);
}

void serial_base_force_drain(void)
{
    set_pending_cmd(CMD_BASE_FORCE_DRAIN, LINK_MODE_BUCKET_BASE);
}

void serial_base_clean_spray(uint8_t minutes)
{
    uint8_t base_data[9] = {0};
    base_data[1] = minutes;            // data[17] 热水喷淋时间(min)
    set_pending_cmd_with_base(CMD_BASE_CLEAN_SPRAY, LINK_MODE_BUCKET_BASE, base_data);
}

void serial_base_clean_water_spray(uint8_t minutes)
{
    uint8_t base_data[9] = {0};
    base_data[2] = minutes;            // data[18] 清水喷淋时间(min)
    set_pending_cmd_with_base(CMD_BASE_CLEAN_WATER, LINK_MODE_BUCKET_BASE, base_data);
}

void serial_base_hot_dry(uint8_t time_x10min)
{
    uint8_t base_data[9] = {0};
    base_data[3] = time_x10min;        // data[19] 烘干时间(x10min)
    set_pending_cmd_with_base(CMD_BASE_HOT_DRY, LINK_MODE_BUCKET_BASE, base_data);
}

void serial_base_self_check(void)
{
    set_pending_cmd(CMD_BASE_SELF_CHECK, LINK_MODE_BUCKET_BASE);
}
