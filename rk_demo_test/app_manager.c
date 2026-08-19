#include "app_manager.h"
#include "status_bar.h"
#include "wifi_manager.h"
#include "system_manager.h"
#include "ble_manager.h"
#include "ble_provisioning.h"
#include "robot_tcp.h"
#include "serial.h"
#include "tcp_service.h"
#include "app_log.h"
#include "voice_command_service.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define APP_MANAGER_LOG_USER(...) APP_LOG_USER("APP_MANAGER", __VA_ARGS__)
#define APP_MANAGER_LOG_WARN(...) APP_LOG_WARN("APP_MANAGER", __VA_ARGS__)
#define APP_MANAGER_LOG_ERROR(...) APP_LOG_ERROR("APP_MANAGER", __VA_ARGS__)

// 静态变量，保存全局状态
static lv_ui *g_ui = NULL;
static lv_obj_t *g_status_bar = NULL;
static bool g_initialized = false;

/**
 * SNTP 对时 — 通过 NTP 服务器获取网络时间并更新系统时钟
 * 返回值: 0 成功, -1 失败
 */
static int app_manager_sntp_sync_time(void)
{
    const char *ntp_server = "pool.ntp.org";
    int sockfd;
    struct sockaddr_in server_addr;
    struct timeval tv;
    uint8_t ntp_buf[48] = {0};
    struct hostent *host;
    int ret = -1;

    /* 1. 解析 NTP 服务器域名 */
    host = gethostbyname(ntp_server);
    if (host == NULL)
    {
        APP_MANAGER_LOG_ERROR("NTP 域名解析失败: server=%s", ntp_server);
        return -1;
    }

    /* 2. 创建 UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        APP_MANAGER_LOG_ERROR("NTP socket 创建失败: %s", strerror(errno));
        return -1;
    }

    /* 3. 构造 NTP 请求包 (LI=0, VN=3, Mode=3 → 0x1B) */
    ntp_buf[0] = 0x1B;

    /* 4. 填写服务器地址 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(123);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    /* 5. 设置接收超时 5 秒 */
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* 6. 发送请求 */
    if (sendto(sockfd, ntp_buf, sizeof(ntp_buf), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        APP_MANAGER_LOG_ERROR("NTP 请求发送失败: %s", strerror(errno));
        close(sockfd);
        return -1;
    }

    /* 7. 接收响应 */
    {
        socklen_t addr_len = sizeof(server_addr);
        ssize_t n = recvfrom(sockfd, ntp_buf, sizeof(ntp_buf), 0,
                             (struct sockaddr *)&server_addr, &addr_len);
        if (n < 48)
        {
            if (n < 0)
            {
                APP_MANAGER_LOG_ERROR("NTP 响应接收失败: %s", strerror(errno));
            }
            else
            {
                APP_MANAGER_LOG_WARN("NTP 响应长度不足: received=%zd expected=48", n);
            }
            close(sockfd);
            return -1;
        }
    }
    close(sockfd);

    /* 8. 提取传输时间戳 (字节 40-43: 整数部分) */
    {
        uint32_t ntp_sec;
        time_t unix_time;
        struct timeval now;

        ntp_sec  = (uint32_t)ntp_buf[40] << 24;
        ntp_sec |= (uint32_t)ntp_buf[41] << 16;
        ntp_sec |= (uint32_t)ntp_buf[42] << 8;
        ntp_sec |= (uint32_t)ntp_buf[43];

        /* NTP 时间转 Unix 时间 (NTP epoch 1900-01-01, Unix epoch 1970-01-01) */
        unix_time = (time_t)(ntp_sec - 2208988800U);

        now.tv_sec  = unix_time;
        now.tv_usec = 0;

        /* 9. 设置系统时间 (需要 root 权限) */
        if (settimeofday(&now, NULL) == 0)
        {
            struct tm *tm_info = localtime(&unix_time);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
            APP_MANAGER_LOG_USER("NTP 对时成功: %s", time_str);
            ret = 0;
        }
        else
        {
            APP_MANAGER_LOG_ERROR("NTP 设置系统时间失败: %s", strerror(errno));
            ret = -1;
        }
    }

    return ret;
}

void app_manager_init(lv_ui *ui)
{
    if (ui == NULL)
    {
        APP_MANAGER_LOG_ERROR("初始化失败: ui 为空");
        return;
    }

    g_ui = ui;

    if (g_initialized)
    {
        APP_MANAGER_LOG_USER("已完成初始化，跳过重复调用");
        return;
    }

    // 1. 初始化 WiFi 管理模块
    wifi_manager_init();

    // 2. 创建状态栏（会自动注册 WiFi 回调，并更新时间和状态）
    g_status_bar = status_bar_create(ui);

    // 3. 初始化系统管理模块（设置默认温度和定时）
    system_manager_init(ui);

    // 4. 初始化串口协议模块（打开串口并启动后台收发）
    serial_init();

    // 5. 启动 ASRPRO ttyS1 语音命令和固定语音播放服务
    if (voice_command_service_init() != 0)
    {
        APP_MANAGER_LOG_ERROR("ASRPRO语音服务初始化失败，其他业务继续运行");
    }

    // 6. 初始化机器人 TCP 模块
    if (RobotTcp_Init() != 0)
    {
        APP_MANAGER_LOG_ERROR("机器人 TCP 模块初始化失败");
    }
    else if (RobotTcp_Start() != 0)
    {
        APP_MANAGER_LOG_ERROR("机器人 TCP 服务启动失败");
    }

    // 7. 启动手机本地 TCP 控制服务
    if (tcp_service_init() != 0)
    {
        APP_MANAGER_LOG_ERROR("本地 TCP 控制服务初始化失败");
    }

    // 8. 启动 BLE 配网和广播
    if (ble_manager_init() != 0)
    {
        APP_MANAGER_LOG_ERROR("BLE 管理模块初始化失败");
    }
    else if (ble_manager_start_advertising() != 0)
    {
        APP_MANAGER_LOG_ERROR("BLE 广播启动失败");
    }

    g_initialized = true;

    // 6. 如果已连接 WiFi，尝试 NTP 对时
    if (wifi_manager_is_connected())
    {
        APP_MANAGER_LOG_USER("Wi-Fi 已连接，开始 NTP 对时");
        app_manager_sntp_sync_time();
    }
    else
    {
        APP_MANAGER_LOG_WARN("Wi-Fi 未连接，跳过 NTP 对时");
    }

    APP_MANAGER_LOG_USER("初始化完成");
}

void app_manager_deinit(void)
{
    if (!g_initialized)
    {
        return;
    }
    g_initialized = false;

    /* 先停止语音入口，避免退出期间继续投递 LVGL 和 MCU 业务。 */
    voice_command_service_deinit();
    /* 先关闭外部入口，再停止机器人后台线程。 */
    tcp_service_deinit();
    (void)ble_manager_stop_advertising();
    ble_provisioning_stop();
    RobotTcp_Deinit();
    g_ui = NULL;
    g_status_bar = NULL;
}

lv_ui *app_manager_get_ui(void)
{
    return g_ui;
}

lv_obj_t *app_manager_get_status_bar(void)
{
    return g_status_bar;
}
