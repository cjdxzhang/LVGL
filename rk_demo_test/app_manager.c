#include "app_manager.h"
#include "status_bar.h"
#include "wifi_manager.h"
#include "system_manager.h"
#include "robot_chassis/robot_chassis_control.h"
#include "robot_chassis/robot_chassis_service.h"
#include "robot_chassis/robot_station_app.h"
#include "robot_chassis/robot_station_navigation.h"
#include "robot_gateway_ipc.h"
#include "serial.h"
#include "app_log.h"
#include "voice_command_service.h"

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define APP_ROBOT_STATION_PATH "/mnt/udisk/stations.json"
#define APP_ROBOT_MAP_NAME_SIZE 128U
#define APP_ROBOT_BACKUP_PATH_SIZE 256U

#define APP_MANAGER_LOG_USER(...) APP_LOG_USER("APP_MANAGER", __VA_ARGS__)
#define APP_MANAGER_LOG_WARN(...) APP_LOG_WARN("APP_MANAGER", __VA_ARGS__)
#define APP_MANAGER_LOG_ERROR(...) APP_LOG_ERROR("APP_MANAGER", __VA_ARGS__)

// 静态变量，保存全局状态
static lv_ui *g_ui = NULL;
static lv_obj_t *g_status_bar = NULL;
static bool g_initialized = false;
static bool g_robot_available = false;
static robot_station_store_t g_station_store;
static robot_chassis_status_t g_robot_status;
static bool g_robot_status_valid = false;
static pthread_mutex_t g_robot_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_robot_access_mutex = PTHREAD_MUTEX_INITIALIZER;

static void app_manager_robot_status_callback(
    const robot_chassis_status_t *status)
{
    if (status == NULL)
    {
        return;
    }
    pthread_mutex_lock(&g_robot_state_mutex);
    g_robot_status = *status;
    g_robot_status_valid = true;
    pthread_mutex_unlock(&g_robot_state_mutex);
}

static int app_manager_robot_init(void)
{
    robot_chassis_service_config_t config = {0};
    int result;

    memset(&g_station_store, 0, sizeof(g_station_store));
    result = robot_station_store_init(&g_station_store,
                                      APP_ROBOT_STATION_PATH);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    result = robot_station_store_load(&g_station_store);
    if (result != ROBOT_CHASSIS_OK)
    {
        robot_station_store_deinit(&g_station_store);
        return result;
    }
    config.status_callback = app_manager_robot_status_callback;
    result = robot_chassis_service_init(&config);
    if (result != ROBOT_CHASSIS_OK)
    {
        robot_station_store_deinit(&g_station_store);
        return result;
    }
    result = robot_chassis_service_start();
    if (result != ROBOT_CHASSIS_OK)
    {
        robot_chassis_service_deinit();
        robot_station_store_deinit(&g_station_store);
        return result;
    }
    g_robot_available = true;
    return ROBOT_CHASSIS_OK;
}

static int app_manager_robot_submit(robot_chassis_command_t *command)
{
    int result;

    if (command == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = g_robot_available ? robot_chassis_service_submit(command) :
             ROBOT_CHASSIS_ERR_STATE;
    robot_chassis_command_release(command);
    return result;
}

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

    // 6. 加载 App 站点数据并启动唯一的底盘服务线程
    {
        int result = app_manager_robot_init();
        if (result != ROBOT_CHASSIS_OK)
        {
            APP_MANAGER_LOG_ERROR("机器人模块初始化失败: ret=%d", result);
        }
    }

    pthread_mutex_lock(&g_robot_access_mutex);
    g_initialized = true;
    pthread_mutex_unlock(&g_robot_access_mutex);

    if (g_robot_available && robot_gateway_ipc_start() != 0)
    {
        APP_MANAGER_LOG_ERROR("机器人调试 IPC 启动失败");
    }

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
    /* 先停止语音入口，避免退出期间继续投递 LVGL 和 MCU 业务。 */
    voice_command_service_deinit();
    /* 先停止 IPC，避免退出期间有新的机器人命令进入。 */
    robot_gateway_ipc_stop();
    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return;
    }
    /* 先阻止新的 App/业务请求，再停止底盘线程并释放站点存储。 */
    g_initialized = false;
    {
        bool robot_available = g_robot_available;
        g_robot_available = false;
        pthread_mutex_unlock(&g_robot_access_mutex);
        if (robot_available)
        {
            robot_chassis_service_stop();
            robot_chassis_service_deinit();
            robot_station_store_deinit(&g_station_store);
        }
    }
    pthread_mutex_lock(&g_robot_state_mutex);
    memset(&g_robot_status, 0, sizeof(g_robot_status));
    g_robot_status_valid = false;
    pthread_mutex_unlock(&g_robot_state_mutex);
    g_ui = NULL;
    g_status_bar = NULL;
}

// app下发json站点信息
int app_manager_robot_station_update_json(const char *json, size_t length)
{
    int result;

    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized || !g_robot_available)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_station_app_handle_json(&g_station_store, json, length);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return result;
}

// 机器人导航到指定站点
int app_manager_robot_navigate(const char *name, robot_station_t *target)
{
    int result;

    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized || !g_robot_available || target == NULL)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_station_navigate_with_target(&g_station_store, name, target,
             NULL);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return result;
}

// 解析站点名称，获取对应的坐标信息
int app_manager_robot_resolve_navigation_target(const char *name,
        robot_station_t *target)
{
    int result;

    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized || !g_robot_available || target == NULL)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_station_resolve_navigation_target(&g_station_store, name, target);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return result;
}

// 提交导航目标给底盘
int app_manager_robot_submit_navigation_target(const robot_station_t *target)
{
    int result;

    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized || !g_robot_available || target == NULL)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_station_submit_navigation_target(target, NULL);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return result;
}

// 充电回仓
int app_manager_robot_return_to_base(robot_station_t *target)
{
    int result;

    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized || !g_robot_available || target == NULL)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_station_return_to_base(&g_station_store, target,
                                          NULL);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return result;
}

int app_manager_robot_request_status(void)
{
    robot_chassis_command_t command = {0};
    int result;

    pthread_mutex_lock(&g_robot_access_mutex);
    result = robot_chassis_build_status_query(&command);
    if (result != ROBOT_CHASSIS_OK)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return result;
    }
    result = app_manager_robot_submit(&command);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return result;
}

int app_manager_robot_get_status(robot_chassis_status_t *status)
{
    if (status == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized || !g_robot_available)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    pthread_mutex_lock(&g_robot_state_mutex);
    if (!g_robot_status_valid)
    {
        pthread_mutex_unlock(&g_robot_state_mutex);
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    *status = g_robot_status;
    pthread_mutex_unlock(&g_robot_state_mutex);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return ROBOT_CHASSIS_OK;
}

static bool app_manager_robot_valid_map_name(const char *map_name)
{
    size_t index;
    size_t length;

    if (map_name == NULL || map_name[0] == '\0')
    {
        return false;
    }
    length = strlen(map_name);
    if (length >= APP_ROBOT_MAP_NAME_SIZE || strcmp(map_name, ".") == 0 ||
            strcmp(map_name, "..") == 0)
    {
        return false;
    }
    for (index = 0U; index < length; index++)
    {
        unsigned char value = (unsigned char)map_name[index];

        if (value == '/' || value == '\\' || iscntrl(value))
        {
            return false;
        }
    }
    return true;
}

int app_manager_robot_backup_map(const char *map_name)
{
    char temporary_path[APP_ROBOT_BACKUP_PATH_SIZE];
    char final_path[APP_ROBOT_BACKUP_PATH_SIZE];
    int temporary_length;
    int final_length;
    int result;

    if (!app_manager_robot_valid_map_name(map_name))
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    temporary_length = snprintf(temporary_path, sizeof(temporary_path),
                                "/mnt/udisk/.%s.part", map_name);
    final_length = snprintf(final_path, sizeof(final_path),
                            "/mnt/udisk/%s", map_name);
    if (temporary_length < 0 ||
            (size_t)temporary_length >= sizeof(temporary_path) ||
            final_length < 0 || (size_t)final_length >= sizeof(final_path))
    {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }

    pthread_mutex_lock(&g_robot_access_mutex);
    if (!g_initialized || !g_robot_available)
    {
        pthread_mutex_unlock(&g_robot_access_mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_chassis_service_backup_map(map_name, temporary_path,
             final_path);
    pthread_mutex_unlock(&g_robot_access_mutex);
    return result;
}

lv_ui *app_manager_get_ui(void)
{
    return g_ui;
}

lv_obj_t *app_manager_get_status_bar(void)
{
    return g_status_bar;
}
