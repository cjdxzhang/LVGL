#include "voice_command_service.h"

#include "app_log.h"
#include "lvgl.h"
#include "system_manager.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

extern char **environ;

#define VOICE_LOG_USER(...) APP_LOG_USER("VOICE", __VA_ARGS__)
#define VOICE_LOG_WARN(...) APP_LOG_WARN("VOICE", __VA_ARGS__)
#define VOICE_LOG_ERROR(...) APP_LOG_ERROR("VOICE", __VA_ARGS__)

#define VOICE_UART_PATH "/dev/ttyS1"
#define VOICE_LINE_CAPACITY 64u
#define VOICE_READ_CAPACITY 128u
#define VOICE_RETRY_DELAY_MS 1000u
#define VOICE_SESSION_TIMEOUT_MS 25000u
#define VOICE_AUDIO_QUEUE_CAPACITY 16u
#define VOICE_ABANDON_AUDIO_PATH "/usr/share/voice/abandon.wav"

typedef enum
{
    VOICE_GATE_NONE = 0,
    VOICE_GATE_DETACHED,
    VOICE_GATE_DOCKED,
    VOICE_GATE_AWAKE_ONLY
} voice_gate_t;

typedef struct
{
    voice_command_t command;
    const char *text;
    const char *audio_path;
    voice_gate_t gate;
} voice_command_descriptor_t;

typedef struct
{
    char data[VOICE_LINE_CAPACITY];
    size_t length;
    bool discarding;
} voice_line_parser_t;

typedef struct
{
    pthread_mutex_t lock;
    pthread_cond_t audio_cond;
    pthread_t uart_thread;
    pthread_t audio_thread;
    bool initialized;
    bool running;
    bool uart_thread_started;
    bool audio_thread_started;
    int uart_fd;
    pid_t player_pid;
    const char *audio_queue[VOICE_AUDIO_QUEUE_CAPACITY];
    size_t audio_head;
    size_t audio_count;
    bool awake;
    lv_timer_t *session_timer;
} voice_service_state_t;

static const voice_command_descriptor_t g_voice_commands[] =
{
    {VOICE_COMMAND_WAKEUP_ACK, "CMD_WAKEUP_ACK", "/usr/share/voice/wakeup_ack.wav", VOICE_GATE_NONE},
    {VOICE_COMMAND_SLEEP_ACK, "CMD_SLEEP_ACK", "/usr/share/voice/sleep_ack.wav", VOICE_GATE_AWAKE_ONLY},
    {VOICE_COMMAND_FOOTBATH_START, "CMD_FOOTBATH_START", "/usr/share/voice/footbath_start.wav", VOICE_GATE_DETACHED},
    {VOICE_COMMAND_FOOTBATH_STOP, "CMD_FOOTBATH_STOP", "/usr/share/voice/footbath_stop.wav", VOICE_GATE_DETACHED},
    {VOICE_COMMAND_MASSAGE_START, "CMD_MASSAGE_START", "/usr/share/voice/massage_start.wav", VOICE_GATE_DETACHED},
    {VOICE_COMMAND_MASSAGE_STOP, "CMD_MASSAGE_STOP", "/usr/share/voice/massage_stop.wav", VOICE_GATE_DETACHED},
    {VOICE_COMMAND_TEMP_UP, "CMD_TEMP_UP", "/usr/share/voice/temp_up.wav", VOICE_GATE_DETACHED},
    {VOICE_COMMAND_TEMP_DOWN, "CMD_TEMP_DOWN", "/usr/share/voice/temp_down.wav", VOICE_GATE_DETACHED},
    {VOICE_COMMAND_GO_HOME, "CMD_GO_HOME", NULL, VOICE_GATE_NONE},
    {VOICE_COMMAND_DRY_START, "CMD_DRY_START", "/usr/share/voice/dry_start.wav", VOICE_GATE_DOCKED},
    {VOICE_COMMAND_CLEAN_START, "CMD_CLEAN_START", "/usr/share/voice/clean_start.wav", VOICE_GATE_DOCKED},
    {VOICE_COMMAND_CLEAN_STOP, "CMD_CLEAN_STOP", "/usr/share/voice/clean_stop.wav", VOICE_GATE_DOCKED},
    {VOICE_COMMAND_STOP_ALL, "CMD_STOP_ALL", "/usr/share/voice/stop_all.wav", VOICE_GATE_AWAKE_ONLY},
    {VOICE_COMMAND_WAKEUP, "WAKEUP", NULL, VOICE_GATE_NONE},
    {VOICE_COMMAND_NOISE, "NOISE", NULL, VOICE_GATE_NONE}
};

static voice_service_state_t g_voice =
{
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .audio_cond = PTHREAD_COND_INITIALIZER,
    .uart_fd = -1,
    .player_pid = -1
};

static bool voice_is_running(void)
{
    bool running;

    pthread_mutex_lock(&g_voice.lock);
    running = g_voice.running;
    pthread_mutex_unlock(&g_voice.lock);
    return running;
}

static const voice_command_descriptor_t *voice_descriptor(voice_command_t command)
{
    size_t index;

    for (index = 0u; index < sizeof(g_voice_commands) / sizeof(g_voice_commands[0]); index++)
    {
        if (g_voice_commands[index].command == command)
        {
            return &g_voice_commands[index];
        }
    }
    return NULL;
}

static voice_command_t voice_parse_command(const char *line)
{
    size_t index;

    if (line == NULL || line[0] == '\0')
    {
        return VOICE_COMMAND_UNKNOWN;
    }
    for (index = 0u; index < sizeof(g_voice_commands) / sizeof(g_voice_commands[0]); index++)
    {
        if (strcmp(line, g_voice_commands[index].text) == 0)
        {
            return g_voice_commands[index].command;
        }
    }
    return VOICE_COMMAND_UNKNOWN;
}

static bool voice_audio_enqueue(const char *path)
{
    size_t tail;

    if (path == NULL)
    {
        return false;
    }
    pthread_mutex_lock(&g_voice.lock);
    if (!g_voice.running || g_voice.audio_count >= VOICE_AUDIO_QUEUE_CAPACITY)
    {
        pthread_mutex_unlock(&g_voice.lock);
        VOICE_LOG_WARN("语音播放队列不可用，丢弃: %s", path);
        return false;
    }
    tail = (g_voice.audio_head + g_voice.audio_count) % VOICE_AUDIO_QUEUE_CAPACITY;
    g_voice.audio_queue[tail] = path;
    g_voice.audio_count++;
    pthread_cond_signal(&g_voice.audio_cond);
    pthread_mutex_unlock(&g_voice.lock);
    return true;
}

static int voice_play_file(const char *path)
{
    char *const arguments[] = {"aplay", "-q", (char *)path, NULL};
    pid_t pid;
    pid_t wait_result;
    int status;
    int spawn_result;

    if (access(path, R_OK) != 0)
    {
        VOICE_LOG_ERROR("语音文件不可读: path=%s error=%s", path, strerror(errno));
        return -1;
    }
    pthread_mutex_lock(&g_voice.lock);
    if (!g_voice.running)
    {
        pthread_mutex_unlock(&g_voice.lock);
        return -1;
    }
    spawn_result = posix_spawnp(&pid, "aplay", NULL, NULL, arguments, environ);
    if (spawn_result != 0)
    {
        pthread_mutex_unlock(&g_voice.lock);
        VOICE_LOG_ERROR("启动 aplay 失败: %s", strerror(spawn_result));
        return -1;
    }
    g_voice.player_pid = pid;
    pthread_mutex_unlock(&g_voice.lock);

    do
    {
        wait_result = waitpid(pid, &status, 0);
    }
    while (wait_result < 0 && errno == EINTR);

    pthread_mutex_lock(&g_voice.lock);
    if (g_voice.player_pid == pid)
    {
        g_voice.player_pid = -1;
    }
    pthread_mutex_unlock(&g_voice.lock);

    if (wait_result < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        VOICE_LOG_ERROR("语音播放失败: path=%s status=%d", path, status);
        return -1;
    }
    VOICE_LOG_USER("语音播放完成: %s", path);
    return 0;
}

static void *voice_audio_thread_main(void *user_data)
{
    LV_UNUSED(user_data);

    for (;;)
    {
        const char *path;

        pthread_mutex_lock(&g_voice.lock);
        while (g_voice.running && g_voice.audio_count == 0u)
        {
            pthread_cond_wait(&g_voice.audio_cond, &g_voice.lock);
        }
        if (!g_voice.running)
        {
            pthread_mutex_unlock(&g_voice.lock);
            break;
        }
        path = g_voice.audio_queue[g_voice.audio_head];
        g_voice.audio_head = (g_voice.audio_head + 1u) % VOICE_AUDIO_QUEUE_CAPACITY;
        g_voice.audio_count--;
        pthread_mutex_unlock(&g_voice.lock);
        (void)voice_play_file(path);
    }
    return NULL;
}

static void voice_session_sleep(void)
{
    if (!g_voice.awake)
    {
        return;
    }
    if (g_voice.session_timer != NULL)
    {
        lv_timer_del(g_voice.session_timer);
        g_voice.session_timer = NULL;
    }
    (void)voice_audio_enqueue("/usr/share/voice/sleep_ack.wav");
    g_voice.awake = false;
    VOICE_LOG_USER("语音会话进入休眠");
}

static void voice_session_timeout_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    g_voice.session_timer = NULL;
    voice_session_sleep();
}

static void voice_session_reset(void)
{
    if (g_voice.session_timer != NULL)
    {
        lv_timer_del(g_voice.session_timer);
        g_voice.session_timer = NULL;
    }
    g_voice.session_timer = lv_timer_create(voice_session_timeout_cb,
                                            VOICE_SESSION_TIMEOUT_MS, NULL);
    if (g_voice.session_timer == NULL)
    {
        VOICE_LOG_ERROR("创建25秒语音会话 timer 失败");
        (void)voice_audio_enqueue("/usr/share/voice/sleep_ack.wav");
        g_voice.awake = false;
        return;
    }
    lv_timer_set_repeat_count(g_voice.session_timer, 1);
}

static void voice_command_execute_async(void *user_data)
{
    voice_command_t command = (voice_command_t)(uintptr_t)user_data;
    const voice_command_descriptor_t *descriptor = voice_descriptor(command);
    system_command_decision_t decision;

    if (!voice_is_running())
    {
        return;
    }
    if (command == VOICE_COMMAND_WAKEUP_ACK)
    {
        g_voice.awake = true;
        (void)voice_audio_enqueue("/usr/share/voice/wakeup_ack.wav");
        voice_session_reset();
        VOICE_LOG_USER("收到唤醒命令，语音会话已开启");
        return;
    }
    if (!g_voice.awake)
    {
        VOICE_LOG_USER("休眠态忽略命令: %s",
                       descriptor == NULL ? "<unknown>" : descriptor->text);
        return;
    }
    if (command == VOICE_COMMAND_SLEEP_ACK)
    {
        voice_session_sleep();
        return;
    }
    if (command == VOICE_COMMAND_GO_HOME || command == VOICE_COMMAND_WAKEUP ||
            command == VOICE_COMMAND_NOISE || command == VOICE_COMMAND_UNKNOWN)
    {
        VOICE_LOG_USER("忽略非一期业务命令: %s",
                       descriptor == NULL ? "<unknown>" : descriptor->text);
        return;
    }

    decision = system_voice_command_execute(descriptor->command);
    if (decision.result == SYSTEM_COMMAND_ACCEPTED)
    {
        (void)voice_audio_enqueue(descriptor->audio_path);
        voice_session_reset();
        VOICE_LOG_USER("语音命令已执行: %s", descriptor->text);
    }
    else if (decision.result == SYSTEM_COMMAND_STATE_REJECTED)
    {
        VOICE_LOG_WARN("语音命令被状态门禁拒绝: command=%s has_status=%d link=0x%02X domain=%s state=%s reason=%s",
                       descriptor->text, has_mcu_status_report ? 1 : 0,
                       (unsigned int)last_link_status,
                       decision.domain, decision.state, decision.reason);
        (void)voice_audio_enqueue(VOICE_ABANDON_AUDIO_PATH);
    }
    else
    {
        VOICE_LOG_WARN("语音命令被互斥门禁拒绝: command=%s domain=%s state=%s reason=%s",
                       descriptor->text, decision.domain,
                       decision.state, decision.reason);
        (void)voice_audio_enqueue(VOICE_ABANDON_AUDIO_PATH);
    }
}

static void voice_schedule_command(voice_command_t command)
{
    if (command == VOICE_COMMAND_UNKNOWN)
    {
        VOICE_LOG_WARN("收到未知语音文本");
        return;
    }
    if (lv_async_call(voice_command_execute_async,
                      (void *)(uintptr_t)command) != LV_RESULT_OK)
    {
        VOICE_LOG_ERROR("投递语音命令到 LVGL 线程失败: command=%d", command);
    }
}

static void voice_handle_line(voice_line_parser_t *parser)
{
    voice_command_t command;

    while (parser->length > 0u && parser->data[parser->length - 1u] == '\r')
    {
        parser->length--;
    }
    parser->data[parser->length] = '\0';
    if (parser->length == 0u)
    {
        return;
    }
    command = voice_parse_command(parser->data);
    if (command == VOICE_COMMAND_UNKNOWN)
    {
        VOICE_LOG_WARN("忽略未知语音文本: %s", parser->data);
        return;
    }
    voice_schedule_command(command);
}

static void voice_parser_feed(voice_line_parser_t *parser,
                              const uint8_t *data, size_t length)
{
    size_t index;

    for (index = 0u; index < length; index++)
    {
        uint8_t value = data[index];

        if (value == '\n')
        {
            if (parser->discarding)
            {
                VOICE_LOG_WARN("超长语音输入行已丢弃");
            }
            else
            {
                voice_handle_line(parser);
            }
            parser->length = 0u;
            parser->discarding = false;
            continue;
        }
        if (parser->discarding)
        {
            continue;
        }
        if (parser->length >= VOICE_LINE_CAPACITY - 1u)
        {
            parser->length = 0u;
            parser->discarding = true;
            continue;
        }
        parser->data[parser->length++] = (char)value;
    }
}

static int voice_uart_open(void)
{
    struct termios tty;
    int fd = open(VOICE_UART_PATH, O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);

    if (fd < 0)
    {
        return -1;
    }
    if (tcgetattr(fd, &tty) != 0)
    {
        close(fd);
        return -1;
    }
    cfmakeraw(&tty);
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~(CSTOPB | CRTSCTS | PARENB | CSIZE);
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        close(fd);
        return -1;
    }
    tcflush(fd, TCIFLUSH);
    return fd;
}

static void voice_wait_retry(void)
{
    unsigned int elapsed = 0u;

    while (voice_is_running() && elapsed < VOICE_RETRY_DELAY_MS)
    {
        usleep(100000u);
        elapsed += 100u;
    }
}

static void *voice_uart_thread_main(void *user_data)
{
    voice_line_parser_t parser = {0};
    uint8_t buffer[VOICE_READ_CAPACITY];

    LV_UNUSED(user_data);
    while (voice_is_running())
    {
        fd_set read_fds;
        struct timeval timeout = {.tv_sec = 0, .tv_usec = 200000};
        int fd;
        int select_result;

        pthread_mutex_lock(&g_voice.lock);
        fd = g_voice.uart_fd;
        pthread_mutex_unlock(&g_voice.lock);
        if (fd < 0)
        {
            fd = voice_uart_open();
            if (fd < 0)
            {
                VOICE_LOG_WARN("打开 %s 失败: %s", VOICE_UART_PATH, strerror(errno));
                voice_wait_retry();
                continue;
            }
            pthread_mutex_lock(&g_voice.lock);
            g_voice.uart_fd = fd;
            pthread_mutex_unlock(&g_voice.lock);
            memset(&parser, 0, sizeof(parser));
            VOICE_LOG_USER("已打开 %s，参数115200 8N1", VOICE_UART_PATH);
        }

        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        select_result = select(fd + 1, &read_fds, NULL, NULL, &timeout);
        if (select_result < 0 && errno != EINTR)
        {
            VOICE_LOG_ERROR("ttyS1 select 失败: %s", strerror(errno));
        }
        else if (select_result > 0 && FD_ISSET(fd, &read_fds))
        {
            ssize_t received = read(fd, buffer, sizeof(buffer));

            if (received > 0)
            {
                voice_parser_feed(&parser, buffer, (size_t)received);
                continue;
            }
            if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
            {
                continue;
            }
            VOICE_LOG_WARN("ttyS1 已断开或读取失败: %s",
                           received == 0 ? "EOF" : strerror(errno));
        }
        else
        {
            continue;
        }

        pthread_mutex_lock(&g_voice.lock);
        if (g_voice.uart_fd == fd)
        {
            close(g_voice.uart_fd);
            g_voice.uart_fd = -1;
        }
        pthread_mutex_unlock(&g_voice.lock);
        voice_wait_retry();
    }
    return NULL;
}

int voice_command_service_init(void)
{
    int result;

    pthread_mutex_lock(&g_voice.lock);
    if (g_voice.initialized)
    {
        pthread_mutex_unlock(&g_voice.lock);
        return 0;
    }
    g_voice.running = true;
    g_voice.awake = false;
    g_voice.audio_head = 0u;
    g_voice.audio_count = 0u;
    pthread_mutex_unlock(&g_voice.lock);

    result = pthread_create(&g_voice.audio_thread, NULL,
                            voice_audio_thread_main, NULL);
    if (result != 0)
    {
        VOICE_LOG_ERROR("创建语音播放线程失败: %s", strerror(result));
        goto fail;
    }
    g_voice.audio_thread_started = true;
    result = pthread_create(&g_voice.uart_thread, NULL,
                            voice_uart_thread_main, NULL);
    if (result != 0)
    {
        VOICE_LOG_ERROR("创建语音串口线程失败: %s", strerror(result));
        goto fail;
    }
    g_voice.uart_thread_started = true;

    pthread_mutex_lock(&g_voice.lock);
    g_voice.initialized = true;
    pthread_mutex_unlock(&g_voice.lock);
    VOICE_LOG_USER("ASRPRO语音服务初始化完成，默认休眠");
    return 0;

fail:
    voice_command_service_deinit();
    return -1;
}

void voice_command_service_deinit(void)
{
    pid_t player_pid;
    int uart_fd;
    int command;

    pthread_mutex_lock(&g_voice.lock);
    g_voice.running = false;
    g_voice.initialized = false;
    uart_fd = g_voice.uart_fd;
    g_voice.uart_fd = -1;
    player_pid = g_voice.player_pid;
    g_voice.audio_count = 0u;
    pthread_cond_broadcast(&g_voice.audio_cond);
    pthread_mutex_unlock(&g_voice.lock);

    for (command = (int)VOICE_COMMAND_UNKNOWN;
            command <= (int)VOICE_COMMAND_NOISE; command++)
    {
        (void)lv_async_call_cancel(voice_command_execute_async,
                                   (void *)(uintptr_t)command);
    }
    if (g_voice.session_timer != NULL)
    {
        lv_timer_del(g_voice.session_timer);
        g_voice.session_timer = NULL;
    }
    g_voice.awake = false;
    if (uart_fd >= 0)
    {
        close(uart_fd);
    }
    if (player_pid > 0)
    {
        kill(player_pid, SIGTERM);
    }
    if (g_voice.uart_thread_started)
    {
        pthread_join(g_voice.uart_thread, NULL);
        g_voice.uart_thread_started = false;
    }
    if (g_voice.audio_thread_started)
    {
        pthread_join(g_voice.audio_thread, NULL);
        g_voice.audio_thread_started = false;
    }
    VOICE_LOG_USER("ASRPRO语音服务已停止");
}
