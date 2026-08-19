#include "tcp_service.h"

#include "serial.h"
#include "system_manager.h"
#include "robot_tcp.h"
#include "wifi_manager.h"
#include "device_binding_config.h"
#include "custom_imgbtn.h"
#include "cJSON.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define TCP_SERVICE_PORT 19092
#define TCP_SERVICE_BACKLOG 8
#define TCP_SERVICE_MAX_CLIENTS 8
#define TCP_SERVICE_FRAME_HEAD "@@JSON_BEGIN@@"
#define TCP_SERVICE_FRAME_TAIL "@@JSON_END@@"
#define TCP_SERVICE_RX_BUFFER_SIZE 8192

typedef struct
{
    int fd;
    size_t rx_len;
    char rx_buffer[TCP_SERVICE_RX_BUFFER_SIZE];
    struct sockaddr_in addr;
} tcp_client_t;

typedef struct
{
    int listen_fd;
    bool running;
    bool initialized;
    pthread_t thread;
    pthread_mutex_t lock;
    tcp_client_t clients[TCP_SERVICE_MAX_CLIENTS];
} tcp_service_context_t;

typedef struct
{
    uint8_t command;
    int int_value;
    uint8_t arg0;
    uint8_t arg1;
    uint8_t arg2;
    uint8_t arg3;
} async_serial_action_t;

enum
{
    ASYNC_CMD_NONE = 0,
    ASYNC_CMD_NAVIGATE_SCREEN,
    ASYNC_CMD_REFRESH_UI,
    ASYNC_CMD_START_FLOW,
    ASYNC_CMD_SELECT_SELF_CLEAN,
    ASYNC_CMD_STOP_FLOW,
};

typedef struct
{
    uint8_t command;
    int int_value;
} async_ui_action_t;

static tcp_service_context_t g_tcp_service =
{
    .listen_fd = -1,
    .running = false,
    .initialized = false,
    .lock = PTHREAD_MUTEX_INITIALIZER,
};

static const char *bucket_status_to_string(uint8_t status)
{
    switch (status)
    {
    case 0:
        return "power_on";
    case 1:
        return "self_check";
    case 2:
        return "shutdown";
    case 3:
        return "standby";
    case 4:
        return "running";
    case 5:
        return "low_power";
    default:
        return "unknown";
    }
}

static const char *base_status_to_string(uint8_t status)
{
    switch (status)
    {
    case 0:
        return "power_on";
    case 1:
        return "self_check";
    case 2:
        return "shutdown";
    case 3:
        return "standby";
    case 4:
        return "auto_water";
    case 5:
        return "keep_warm";
    case 6:
        return "wait_drain";
    case 7:
        return "clean_spray_hot";
    case 8:
        return "clean_drain_1";
    case 9:
        return "clean_spray_cold";
    case 10:
        return "clean_drain_2";
    case 11:
        return "hot_dry";
    case 12:
        return "force_drain";
    case 13:
        return "single_clean_spray";
    case 14:
        return "single_water_spray";
    case 15:
        return "single_hot_dry";
    default:
        return "unknown";
    }
}

static const char *link_mode_to_string(uint8_t link_mode)
{
    switch (link_mode)
    {
    case 0x01:
        return "bucket_only";
    case 0x02:
        return "bucket_base";
    case 0x03:
        return "direct";
    default:
        return "unknown";
    }
}

static uint8_t clamp_u8_local(int value, int min_value, int max_value)
{
    if (value < min_value)
    {
        return (uint8_t)min_value;
    }
    if (value > max_value)
    {
        return (uint8_t)max_value;
    }
    return (uint8_t)value;
}

static int add_client_locked(int client_fd, const struct sockaddr_in *addr)
{
    int index;

    for (index = 0; index < TCP_SERVICE_MAX_CLIENTS; ++index)
    {
        if (g_tcp_service.clients[index].fd < 0)
        {
            g_tcp_service.clients[index].fd = client_fd;
            g_tcp_service.clients[index].rx_len = 0u;
            g_tcp_service.clients[index].rx_buffer[0] = '\0';
            if (addr != NULL)
            {
                g_tcp_service.clients[index].addr = *addr;
            }
            else
            {
                memset(&g_tcp_service.clients[index].addr, 0, sizeof(g_tcp_service.clients[index].addr));
            }
            return index;
        }
    }

    return -1;
}

static void remove_client_locked(int index)
{
    if (index < 0 || index >= TCP_SERVICE_MAX_CLIENTS)
    {
        return;
    }

    if (g_tcp_service.clients[index].fd >= 0)
    {
        close(g_tcp_service.clients[index].fd);
    }

    g_tcp_service.clients[index].fd = -1;
    g_tcp_service.clients[index].rx_len = 0u;
    g_tcp_service.clients[index].rx_buffer[0] = '\0';
    memset(&g_tcp_service.clients[index].addr, 0, sizeof(g_tcp_service.clients[index].addr));
}

static int send_all(int fd, const char *buf, size_t len)
{
    size_t sent = 0u;

    while (sent < len)
    {
        ssize_t ret = send(fd, buf + sent, len - sent, MSG_NOSIGNAL);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        if (ret == 0)
        {
            return -1;
        }
        sent += (size_t)ret;
    }

    return 0;
}

static cJSON *build_modes_json(void)
{
    cJSON *array = cJSON_CreateArray();
    size_t index;

    if (array == NULL)
    {
        return NULL;
    }

    for (index = 0; index < system_mode_get_count(); ++index)
    {
        const system_mode_t *mode = system_mode_get(index);
        cJSON *item;

        if (mode == NULL)
        {
            continue;
        }

        item = cJSON_CreateObject();
        if (item == NULL)
        {
            continue;
        }

        cJSON_AddNumberToObject(item, "index", (double)index);
        cJSON_AddStringToObject(item, "name", mode->name);
        cJSON_AddNumberToObject(item, "icon_id", mode->icon_id);
        cJSON_AddNumberToObject(item, "temperature", mode->temperature);
        cJSON_AddNumberToObject(item, "time_sec", (double)mode->time_sec);
        cJSON_AddNumberToObject(item, "water_level", mode->water_level);
        cJSON_AddBoolToObject(item, "use_drug1", mode->use_drug1);
        cJSON_AddBoolToObject(item, "use_drug2", mode->use_drug2);
        cJSON_AddNumberToObject(item, "position", mode->position);
        cJSON_AddItemToArray(array, item);
    }

    return array;
}

static cJSON *build_locations_json(void)
{
    cJSON *array = cJSON_CreateArray();
    size_t index;

    if (array == NULL)
    {
        return NULL;
    }

    for (index = 0; index < g_location_count; ++index)
    {
        const system_location_t *location = &g_location_settings[index];
        cJSON *item = cJSON_CreateObject();

        if (item == NULL)
        {
            continue;
        }

        cJSON_AddNumberToObject(item, "array_index", (double)index);
        cJSON_AddStringToObject(item, "name", location->name);
        cJSON_AddNumberToObject(item, "location_index", location->index);
        cJSON_AddNumberToObject(item, "default_mode_index", location->default_mode_index);
        cJSON_AddNumberToObject(item, "coord_x", location->coord.x);
        cJSON_AddNumberToObject(item, "coord_y", location->coord.y);
        cJSON_AddItemToArray(array, item);
    }

    return array;
}

static cJSON *build_robot_state_json(void)
{
    RobotState_t robot_state;
    cJSON *obj = cJSON_CreateObject();
    cJSON *pos;

    if (obj == NULL)
    {
        return NULL;
    }

    RobotTcp_GetState(&robot_state);
    cJSON_AddNumberToObject(obj, "battery_percent", robot_state.battery_percent);
    cJSON_AddNumberToObject(obj, "voltage", robot_state.voltage);
    cJSON_AddBoolToObject(obj, "is_charging", robot_state.is_charging != 0);
    cJSON_AddBoolToObject(obj, "is_emg_stop", robot_state.is_emg_stop != 0);
    cJSON_AddBoolToObject(obj, "in_nav_map", robot_state.in_nav_map != 0);
    cJSON_AddStringToObject(obj, "map_name", robot_state.map_name);

    pos = cJSON_CreateObject();
    if (pos != NULL)
    {
        cJSON_AddNumberToObject(pos, "x", robot_state.pos.x);
        cJSON_AddNumberToObject(pos, "y", robot_state.pos.y);
        cJSON_AddNumberToObject(pos, "z", robot_state.pos.z);
        cJSON_AddItemToObject(obj, "pos", pos);
    }

    return obj;
}

static cJSON *build_protocol_frames_json(void)
{
    cJSON *obj = cJSON_CreateObject();
    uint8_t frame[SERIAL_FRAME_LEN];
    cJSON *array;
    size_t index;

    if (obj == NULL)
    {
        return NULL;
    }

    if (serial_get_last_rx_frame(frame))
    {
        array = cJSON_CreateArray();
        if (array != NULL)
        {
            for (index = 0; index < SERIAL_FRAME_LEN; ++index)
            {
                cJSON_AddItemToArray(array, cJSON_CreateNumber(frame[index]));
            }
            cJSON_AddItemToObject(obj, "last_rx_frame", array);
            cJSON_AddNumberToObject(obj, "last_rx_link_mode", frame[2]);
            cJSON_AddStringToObject(obj, "last_rx_link_mode_name", link_mode_to_string(frame[2]));
            cJSON_AddNumberToObject(obj, "last_rx_cmd", frame[3]);
        }
    }

    if (serial_get_last_tx_frame(frame))
    {
        array = cJSON_CreateArray();
        if (array != NULL)
        {
            for (index = 0; index < SERIAL_FRAME_LEN; ++index)
            {
                cJSON_AddItemToArray(array, cJSON_CreateNumber(frame[index]));
            }
            cJSON_AddItemToObject(obj, "last_tx_frame", array);
            cJSON_AddNumberToObject(obj, "last_tx_link_mode", frame[2]);
            cJSON_AddStringToObject(obj, "last_tx_link_mode_name", link_mode_to_string(frame[2]));
            cJSON_AddNumberToObject(obj, "last_tx_cmd", frame[3]);
        }
    }

    return obj;
}

static cJSON *build_binding_json(void)
{
    device_binding_config_t config;
    cJSON *obj = cJSON_CreateObject();

    if (obj == NULL)
    {
        return NULL;
    }

    if (device_binding_config_load(&config) == 0)
    {
        cJSON_AddStringToObject(obj, "user_id", config.user_id);
        cJSON_AddStringToObject(obj, "wifi_ssid", config.wifi_ssid);
        cJSON_AddStringToObject(obj, "mqtt_json", config.mqtt_json);
        cJSON_AddNumberToObject(obj, "updated_at", (double)config.updated_at);
        cJSON_AddBoolToObject(obj, "loaded", true);
    }
    else
    {
        cJSON_AddBoolToObject(obj, "loaded", false);
    }

    return obj;
}

static cJSON *build_status_payload(void)
{
    cJSON *root = cJSON_CreateObject();
    char ssid[64] = {0};
    char ip_addr[64] = {0};

    if (root == NULL)
    {
        return NULL;
    }

    cJSON_AddNumberToObject(root, "link_status", last_link_status);
    cJSON_AddBoolToObject(root, "base_connected", last_link_status == 0x01);
    /* 当前页面信息：供 App 判断启用/禁用哪些按钮 */
    cJSON_AddNumberToObject(root, "current_page_id", (double)get_current_screen_id());
    cJSON_AddStringToObject(root, "current_page", get_current_screen_name());
    cJSON_AddNumberToObject(root, "current_temp", system_current_temp_get());
    cJSON_AddNumberToObject(root, "temp_set", temp_set);
    cJSON_AddNumberToObject(root, "current_water_level", current_water_level);
    cJSON_AddNumberToObject(root, "water_level", water_level);
    cJSON_AddNumberToObject(root, "timer_set", (double)timer_set);
    cJSON_AddNumberToObject(root, "remaining_time", (double)remaining_seconds);
    cJSON_AddBoolToObject(root, "use_drug1", use_drug1);
    cJSON_AddBoolToObject(root, "use_drug2", use_drug2);
    cJSON_AddBoolToObject(root, "is_self_cleaning", is_self_cleaning);
    cJSON_AddBoolToObject(root, "constant_temperature", constant_temperature);
    cJSON_AddBoolToObject(root, "sterilization", sterilization);
    cJSON_AddNumberToObject(root, "massage_intensity", massage_intensity);
    cJSON_AddNumberToObject(root, "bucket_main_status", bucket_main_status);
    cJSON_AddStringToObject(root, "bucket_main_status_name",
                            bucket_status_to_string(bucket_main_status));
    cJSON_AddNumberToObject(root, "bucket_sub_status",
                            (double)(remaining_seconds / 60u));
    cJSON_AddNumberToObject(root, "base_main_status", base_main_status);
    cJSON_AddStringToObject(root, "base_main_status_name", base_status_to_string(base_main_status));
    cJSON_AddNumberToObject(root, "base_sub_status", 0);
    cJSON_AddNumberToObject(root, "current_location", current_location);
    cJSON_AddNumberToObject(root, "going_to_location", going_to_location);
    cJSON_AddNumberToObject(root, "selected_mode_index", current_selected_mode_index);
    cJSON_AddBoolToObject(root, "is_draining",
                          base_main_status == 8u || base_main_status == 10u);

    cJSON_AddBoolToObject(root, "wifi_connected", wifi_manager_is_connected());
    if (wifi_manager_get_current_ssid(ssid, sizeof(ssid)) == 0)
    {
        cJSON_AddStringToObject(root, "wifi_ssid", ssid);
    }
    if (wifi_manager_get_current_ipv4(ip_addr, sizeof(ip_addr)) == 0)
    {
        cJSON_AddStringToObject(root, "wifi_ip", ip_addr);
    }

    cJSON_AddItemToObject(root, "robot", build_robot_state_json());
    cJSON_AddItemToObject(root, "modes", build_modes_json());
    cJSON_AddItemToObject(root, "locations", build_locations_json());
    cJSON_AddItemToObject(root, "protocol_frames", build_protocol_frames_json());
    cJSON_AddItemToObject(root, "binding", build_binding_json());

    return root;
}

static void async_ui_action_cb(void *user_data)
{
    async_ui_action_t *action = (async_ui_action_t *)user_data;

    if (action == NULL)
    {
        return;
    }

    if (action->command == ASYNC_CMD_NAVIGATE_SCREEN)
    {
        navigate_to_screen((ui_screen_id_t)action->int_value);
        /* 切换页面后刷新目标页面，保证外部修改的全局状态能同步显示 */
        system_manager_refresh_current_page();
    }
    else if (action->command == ASYNC_CMD_REFRESH_UI)
    {
        /* 外部命令修改全局状态后，统一刷新各页面显示 */
        system_manager_refresh_ui();
    }
    else if (action->command == ASYNC_CMD_START_FLOW)
    {
        /* 开始：根据当前选择（模式/自清洁）进入 preparing 流程 */
        system_start_flow();
    }
    else if (action->command == ASYNC_CMD_SELECT_SELF_CLEAN)
    {
        /* 先选择自清洁（首页） */
        system_select_self_clean();
    }
    else if (action->command == ASYNC_CMD_STOP_FLOW)
    {
        /* 停止并返回上一页 */
        system_stop_flow();
    }

    free(action);
}

static void async_serial_action_cb(void *user_data)
{
    async_serial_action_t *action = (async_serial_action_t *)user_data;

    if (action == NULL)
    {
        return;
    }

    switch (action->command)
    {
    case 0xA0:
        serial_bucket_shutdown();
        break;
    case 0xA1:
        serial_bucket_standby();
        break;
    case 0xA2:
        serial_bucket_heat();
        break;
    case 0xA3:
        serial_bucket_stop_heat();
        break;
    case 0xA4:
        massage_intensity = clamp_u8_local(action->int_value, 0, 3);
        serial_bucket_massage();
        break;
    case 0xA5:
        sterilization = (action->int_value != 0);
        serial_bucket_uv();
        break;
    case 0xA6:
        serial_bucket_timer();
        break;
    case 0xA7:
        serial_bucket_stop_all();
        break;
    case 0xA8:
        serial_bucket_self_check();
        break;
    case 0xA9:
        serial_bucket_low_power();
        break;
    case 0xB0:
        serial_base_shutdown();
        break;
    case 0xB1:
        serial_base_standby();
        break;
    case 0xB2:
        serial_base_water_custom(action->arg0, action->arg1, action->arg2, action->arg3);
        break;
    case 0xB3:
        serial_base_auto_clean_custom(action->arg0, action->arg1, action->arg2);
        break;
    case 0xB4:
        serial_base_force_drain();
        break;
    case 0xB5:
        serial_base_clean_spray(action->arg0);
        break;
    case 0xB6:
        serial_base_clean_water_spray(action->arg0);
        break;
    case 0xB7:
        serial_base_hot_dry(action->arg0);
        break;
    case 0xB8:
        serial_base_self_check();
        break;
    default:
        break;
    }

    free(action);
}

static int schedule_ui_nav(ui_screen_id_t screen)
{
    async_ui_action_t *action = (async_ui_action_t *)malloc(sizeof(async_ui_action_t));

    if (action == NULL)
    {
        return -1;
    }

    action->command = ASYNC_CMD_NAVIGATE_SCREEN;
    action->int_value = (int)screen;
    if (lv_async_call(async_ui_action_cb, action) != LV_RESULT_OK)
    {
        free(action);
        return -1;
    }

    return 0;
}

static int schedule_ui_command(uint8_t command)
{
    async_ui_action_t *action = (async_ui_action_t *)malloc(sizeof(async_ui_action_t));

    if (action == NULL)
    {
        return -1;
    }

    action->command = command;
    action->int_value = 0;
    if (lv_async_call(async_ui_action_cb, action) != LV_RESULT_OK)
    {
        free(action);
        return -1;
    }

    return 0;
}

static int schedule_ui_refresh(void)
{
    return schedule_ui_command(ASYNC_CMD_REFRESH_UI);
}

static int schedule_serial_action(uint8_t command,
                                  int int_value,
                                  uint8_t arg0,
                                  uint8_t arg1,
                                  uint8_t arg2,
                                  uint8_t arg3)
{
    async_serial_action_t *action = (async_serial_action_t *)malloc(sizeof(async_serial_action_t));

    if (action == NULL)
    {
        return -1;
    }

    action->command = command;
    action->int_value = int_value;
    action->arg0 = arg0;
    action->arg1 = arg1;
    action->arg2 = arg2;
    action->arg3 = arg3;

    if (lv_async_call(async_serial_action_cb, action) != LV_RESULT_OK)
    {
        free(action);
        return -1;
    }

    return 0;
}

static cJSON *make_response(const char *request_id,
                            const char *cmd,
                            bool ok,
                            const char *code,
                            const char *message,
                            cJSON *data)
{
    cJSON *root = cJSON_CreateObject();

    if (root == NULL)
    {
        if (data != NULL)
        {
            cJSON_Delete(data);
        }
        return NULL;
    }

    cJSON_AddStringToObject(root, "type", "response");
    cJSON_AddStringToObject(root, "cmd", cmd != NULL ? cmd : "unknown");
    if (request_id != NULL && request_id[0] != '\0')
    {
        cJSON_AddStringToObject(root, "request_id", request_id);
    }
    cJSON_AddBoolToObject(root, "ok", ok);
    cJSON_AddStringToObject(root, "code", code != NULL ? code : (ok ? "OK" : "ERROR"));
    cJSON_AddStringToObject(root, "message", message != NULL ? message : "");
    if (data != NULL)
    {
        cJSON_AddItemToObject(root, "data", data);
    }
    else
    {
        cJSON_AddItemToObject(root, "data", cJSON_CreateObject());
    }

    return root;
}

static cJSON *make_error_response(const char *request_id,
                                  const char *cmd,
                                  const char *code,
                                  const char *message)
{
    return make_response(request_id, cmd, false, code, message, NULL);
}

static bool json_get_bool_default(cJSON *obj, const char *key, bool default_value)
{
    cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);

    if (cJSON_IsBool(item))
    {
        return cJSON_IsTrue(item);
    }

    return default_value;
}

static int json_get_int_default(cJSON *obj, const char *key, int default_value)
{
    cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);

    if (cJSON_IsNumber(item))
    {
        return item->valueint;
    }

    return default_value;
}

static const char *json_get_string(cJSON *obj, const char *key)
{
    cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);

    if (cJSON_IsString(item) && item->valuestring != NULL)
    {
        return item->valuestring;
    }

    return NULL;
}

static cJSON *handle_get_status(const char *request_id, const char *cmd)
{
    return make_response(request_id, cmd, true, "OK", "", build_status_payload());
}

static cJSON *handle_list_modes(const char *request_id, const char *cmd)
{
    cJSON *data = cJSON_CreateObject();
    if (data == NULL)
    {
        return make_error_response(request_id, cmd, "NO_MEMORY", "创建响应失败");
    }
    cJSON_AddItemToObject(data, "modes", build_modes_json());
    return make_response(request_id, cmd, true, "OK", "", data);
}

static cJSON *handle_list_locations(const char *request_id, const char *cmd)
{
    cJSON *data = cJSON_CreateObject();
    if (data == NULL)
    {
        return make_error_response(request_id, cmd, "NO_MEMORY", "创建响应失败");
    }
    cJSON_AddItemToObject(data, "locations", build_locations_json());
    return make_response(request_id, cmd, true, "OK", "", data);
}

static cJSON *handle_robot_status_refresh(const char *request_id, const char *cmd)
{
    cJSON *data;
    int ret = RobotTcp_RequestStatus();

    data = cJSON_CreateObject();
    if (data == NULL)
    {
        return make_error_response(request_id, cmd, "NO_MEMORY", "创建响应失败");
    }
    cJSON_AddNumberToObject(data, "request_ret", ret);
    cJSON_AddItemToObject(data, "robot", build_robot_state_json());
    return make_response(request_id, cmd, ret == 0, ret == 0 ? "OK" : "ROBOT_REQ_FAILED",
                         ret == 0 ? "" : "机器人状态请求发送失败", data);
}

static cJSON *handle_set_runtime(cJSON *payload, const char *request_id, const char *cmd)
{
    int maybe_temp = json_get_int_default(payload, "temp_set", temp_set);
    int maybe_timer = json_get_int_default(payload, "timer_set", (int)timer_set);
    int maybe_water = json_get_int_default(payload, "water_level", water_level);
    bool has_drug1 = json_get_bool_default(payload, "use_drug1", use_drug1);
    bool has_drug2 = json_get_bool_default(payload, "use_drug2", use_drug2);
    int maybe_massage = json_get_int_default(payload, "massage_intensity", massage_intensity);
    bool maybe_constant = json_get_bool_default(payload, "constant_temperature", constant_temperature);
    bool maybe_uv = json_get_bool_default(payload, "sterilization", sterilization);

    if (maybe_temp < 25 || maybe_temp > 50)
    {
        return make_error_response(request_id, cmd, "INVALID_TEMP", "temp_set 范围应为 25-50");
    }
    if (maybe_timer < 0 || maybe_timer > 86400)
    {
        return make_error_response(request_id, cmd, "INVALID_TIMER", "timer_set 范围应为 0-86400 秒");
    }
    if (maybe_water < 0 || maybe_water > 2)
    {
        return make_error_response(request_id, cmd, "INVALID_WATER_LEVEL", "water_level 范围应为 0-2");
    }
    if (maybe_massage < 0 || maybe_massage > 3)
    {
        return make_error_response(request_id, cmd, "INVALID_MASSAGE",
                                   "massage_intensity 范围应为 0-3");
    }

    temp_set = (int16_t)maybe_temp;
    timer_set = (uint32_t)maybe_timer;
    water_level = maybe_water;
    use_drug1 = has_drug1;
    use_drug2 = has_drug2;
    massage_intensity = maybe_massage;
    constant_temperature = maybe_constant;
    sterilization = maybe_uv;

    /* 同步刷新界面显示 */
    schedule_ui_refresh();
    return make_response(request_id, cmd, true, "OK", "", build_status_payload());
}

static cJSON *handle_apply_mode(cJSON *payload, const char *request_id, const char *cmd)
{
    int mode_index = json_get_int_default(payload, "mode_index", -1);

    if (system_mode_apply_index(mode_index) != 0)
    {
        return make_error_response(request_id, cmd, "INVALID_MODE", "mode_index 无效");
    }

    is_self_cleaning = false;
    remaining_seconds = 0u;
    /* 同步刷新界面显示 */
    schedule_ui_refresh();
    return make_response(request_id, cmd, true, "OK", "", build_status_payload());
}

static cJSON *handle_bucket_command(cJSON *payload, const char *request_id, const char *cmd)
{
    const char *action = json_get_string(payload, "action");
    int ret = 0;

    if (action == NULL)
    {
        return make_error_response(request_id, cmd, "MISSING_ACTION", "缺少 action 字段");
    }

    if (strcmp(action, "shutdown") == 0)
    {
        ret = schedule_serial_action(0xA0, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "standby") == 0)
    {
        ret = schedule_serial_action(0xA1, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "heat") == 0)
    {
        ret = schedule_serial_action(0xA2, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "stop_heat") == 0)
    {
        ret = schedule_serial_action(0xA3, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "massage") == 0)
    {
        ret = schedule_serial_action(0xA4, json_get_int_default(payload, "intensity", massage_intensity), 0,
                                     0, 0, 0);
    }
    else if (strcmp(action, "uv") == 0)
    {
        ret = schedule_serial_action(0xA5, json_get_bool_default(payload, "enable", sterilization) ? 1 : 0,
                                     0, 0, 0, 0);
    }
    else if (strcmp(action, "timer") == 0)
    {
        int timer_value = json_get_int_default(payload, "timer_set", (int)timer_set);
        if (timer_value < 0 || timer_value > 86400)
        {
            return make_error_response(request_id, cmd, "INVALID_TIMER", "timer_set 范围应为 0-86400 秒");
        }
        timer_set = (uint32_t)timer_value;
        ret = schedule_serial_action(0xA6, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "stop_all") == 0)
    {
        ret = schedule_serial_action(0xA7, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "self_check") == 0)
    {
        ret = schedule_serial_action(0xA8, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "low_power") == 0)
    {
        ret = schedule_serial_action(0xA9, 0, 0, 0, 0, 0);
    }
    else
    {
        return make_error_response(request_id, cmd, "UNKNOWN_ACTION", "不支持的 bucket action");
    }

    if (ret != 0)
    {
        return make_error_response(request_id, cmd, "SCHEDULE_FAILED", "命令调度失败");
    }

    /* 同步刷新界面显示（定时/按摩/除菌等会修改全局状态） */
    schedule_ui_refresh();
    return make_response(request_id, cmd, true, "OK", "", build_status_payload());
}

static cJSON *handle_base_command(cJSON *payload, const char *request_id, const char *cmd)
{
    const char *action = json_get_string(payload, "action");
    int ret = 0;

    if (action == NULL)
    {
        return make_error_response(request_id, cmd, "MISSING_ACTION", "缺少 action 字段");
    }

    if (strcmp(action, "shutdown") == 0)
    {
        ret = schedule_serial_action(0xB0, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "standby") == 0)
    {
        ret = schedule_serial_action(0xB1, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "water") == 0)
    {
        ret = schedule_serial_action(0xB2,
                                     0,
                                     clamp_u8_local(json_get_int_default(payload, "water_level", water_level), 0, 2),
                                     clamp_u8_local(json_get_int_default(payload, "herb1_seconds", use_drug1 ? 5 : 0), 0, 15),
                                     clamp_u8_local(json_get_int_default(payload, "herb2_seconds", use_drug2 ? 5 : 0), 0, 15),
                                     clamp_u8_local(json_get_int_default(payload, "cleaner_seconds", 0), 0, 15));
    }
    else if (strcmp(action, "auto_clean") == 0)
    {
        ret = schedule_serial_action(0xB3,
                                     0,
                                     clamp_u8_local(json_get_int_default(payload, "spray_hot_minutes", 5), 0, 10),
                                     clamp_u8_local(json_get_int_default(payload, "spray_cold_minutes", 5), 0, 10),
                                     clamp_u8_local(json_get_int_default(payload, "dry_time_x10min", 1), 0, 9),
                                     0);
    }
    else if (strcmp(action, "force_drain") == 0)
    {
        ret = schedule_serial_action(0xB4, 0, 0, 0, 0, 0);
    }
    else if (strcmp(action, "clean_spray") == 0)
    {
        ret = schedule_serial_action(0xB5,
                                     0,
                                     clamp_u8_local(json_get_int_default(payload, "minutes", 1), 0, 10),
                                     0,
                                     0,
                                     0);
    }
    else if (strcmp(action, "clean_water_spray") == 0)
    {
        ret = schedule_serial_action(0xB6,
                                     0,
                                     clamp_u8_local(json_get_int_default(payload, "minutes", 1), 0, 10),
                                     0,
                                     0,
                                     0);
    }
    else if (strcmp(action, "hot_dry") == 0)
    {
        ret = schedule_serial_action(0xB7,
                                     0,
                                     clamp_u8_local(json_get_int_default(payload, "time_x10min", 1), 0, 9),
                                     0,
                                     0,
                                     0);
    }
    else if (strcmp(action, "self_check") == 0)
    {
        ret = schedule_serial_action(0xB8, 0, 0, 0, 0, 0);
    }
    else
    {
        return make_error_response(request_id, cmd, "UNKNOWN_ACTION", "不支持的 base action");
    }

    if (ret != 0)
    {
        return make_error_response(request_id, cmd, "SCHEDULE_FAILED", "命令调度失败");
    }

    /* 同步刷新界面显示 */
    schedule_ui_refresh();
    return make_response(request_id, cmd, true, "OK", "", build_status_payload());
}

static cJSON *handle_robot_command(cJSON *payload, const char *request_id, const char *cmd)
{
    const char *action = json_get_string(payload, "action");
    int ret = -1;
    cJSON *data = cJSON_CreateObject();

    if (action == NULL)
    {
        return make_error_response(request_id, cmd, "MISSING_ACTION", "缺少 action 字段");
    }
    if (data == NULL)
    {
        return make_error_response(request_id, cmd, "NO_MEMORY", "创建响应失败");
    }

    if (strcmp(action, "request_status") == 0)
    {
        ret = RobotTcp_RequestStatus();
    }
    else if (strcmp(action, "move") == 0)
    {
        ret = RobotTcp_SendMove((float)json_get_int_default(payload, "linear_speed_mm", 0) / 1000.0f,
                                (float)json_get_int_default(payload, "angular_speed_millirad", 0) / 1000.0f);
    }
    else if (strcmp(action, "goto") == 0)
    {
        cJSON *x = cJSON_GetObjectItemCaseSensitive(payload, "x");
        cJSON *y = cJSON_GetObjectItemCaseSensitive(payload, "y");
        cJSON *z = cJSON_GetObjectItemCaseSensitive(payload, "z");
        if (!cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z))
        {
            cJSON_Delete(data);
            return make_error_response(request_id, cmd, "INVALID_COORD", "goto 需要 x/y/z 数值");
        }
        ret = RobotTcp_SendGoto((float)x->valuedouble, (float)y->valuedouble, (float)z->valuedouble);
    }
    else if (strcmp(action, "goto_station_id") == 0)
    {
        ret = RobotTcp_GotoStationById(json_get_int_default(payload, "station_id", -1));
    }
    else if (strcmp(action, "goto_station_name") == 0)
    {
        const char *name = json_get_string(payload, "station_name");
        if (name == NULL)
        {
            cJSON_Delete(data);
            return make_error_response(request_id, cmd, "INVALID_STATION", "缺少 station_name");
        }
        ret = RobotTcp_GotoStationByName(name);
    }
    else if (strcmp(action, "charge") == 0)
    {
        ret = RobotTcp_SetCharge(json_get_bool_default(payload, "enable", true) ? 1 : 0);
    }
    else if (strcmp(action, "emergency_stop") == 0)
    {
        ret = RobotTcp_EmergencyStop(json_get_bool_default(payload, "stop", true) ? 1 : 0);
    }
    else if (strcmp(action, "start_mapping") == 0)
    {
        ret = RobotTcp_StartMapping();
    }
    else if (strcmp(action, "backup_map") == 0)
    {
        const char *map_name = json_get_string(payload, "map_name");
        if (map_name == NULL)
        {
            cJSON_Delete(data);
            return make_error_response(request_id, cmd, "INVALID_MAP_NAME", "缺少 map_name");
        }
        ret = RobotTcp_BackupMap(map_name);
    }
    else if (strcmp(action, "fetch_stations") == 0)
    {
        ret = RobotTcp_FetchAndSaveStations();
    }
    else
    {
        cJSON_Delete(data);
        return make_error_response(request_id, cmd, "UNKNOWN_ACTION", "不支持的 robot action");
    }

    cJSON_AddNumberToObject(data, "ret", ret);
    cJSON_AddItemToObject(data, "robot", build_robot_state_json());
    return make_response(request_id, cmd, ret == 0, ret == 0 ? "OK" : "ROBOT_FAILED",
                         ret == 0 ? "" : "机器人命令执行失败", data);
}

static cJSON *handle_ui_command(cJSON *payload, const char *request_id, const char *cmd)
{
    const char *action = json_get_string(payload, "action");
    int ret;

    if (action == NULL)
    {
        return make_error_response(request_id, cmd, "MISSING_ACTION", "缺少 action 字段");
    }

    if (strcmp(action, "show_index") == 0)
    {
        ret = schedule_ui_nav(UI_SCREEN_INDEX);
    }
    else if (strcmp(action, "show_working") == 0)
    {
        ret = schedule_ui_nav(UI_SCREEN_WORKING);
    }
    else if (strcmp(action, "show_preparing") == 0)
    {
        ret = schedule_ui_nav(UI_SCREEN_PREPARING);
    }
    else if (strcmp(action, "show_location") == 0)
    {
        ret = schedule_ui_nav(UI_SCREEN_LOCATION);
    }
    else if (strcmp(action, "show_setting") == 0)
    {
        ret = schedule_ui_nav(UI_SCREEN_SETTING);
    }
    else
    {
        return make_error_response(request_id, cmd, "UNKNOWN_ACTION", "不支持的 ui action");
    }

    if (ret != 0)
    {
        return make_error_response(request_id, cmd, "SCHEDULE_FAILED", "界面切换调度失败");
    }

    return make_response(request_id, cmd, true, "OK", "", build_status_payload());
}

static cJSON *handle_start_command(cJSON *payload, const char *request_id, const char *cmd)
{
    const char *action = json_get_string(payload, "action");
    int ret;

    if (action == NULL)
    {
        return make_error_response(request_id, cmd, "MISSING_ACTION", "缺少 action 字段");
    }

    if (strcmp(action, "select_self_clean") == 0)
    {
        /* 先选择自清洁（首页），再执行 start 进入自清洁流程 */
        ret = schedule_ui_command(ASYNC_CMD_SELECT_SELF_CLEAN);
    }
    else if (strcmp(action, "start") == 0)
    {
        /* 开始：根据当前选择（模式/自清洁）跳转 preparing 页面 */
        ret = schedule_ui_command(ASYNC_CMD_START_FLOW);
    }
    else if (strcmp(action, "stop") == 0)
    {
        /* 停止并返回上一页 */
        ret = schedule_ui_command(ASYNC_CMD_STOP_FLOW);
    }
    else
    {
        return make_error_response(request_id, cmd, "UNKNOWN_ACTION", "不支持的 start action");
    }

    if (ret != 0)
    {
        return make_error_response(request_id, cmd, "SCHEDULE_FAILED", "命令调度失败");
    }

    return make_response(request_id, cmd, true, "OK", "", build_status_payload());
}

static cJSON *dispatch_request(cJSON *root)
{
    const char *cmd = json_get_string(root, "cmd");
    const char *request_id = json_get_string(root, "request_id");
    cJSON *payload = cJSON_GetObjectItemCaseSensitive(root, "payload");
    cJSON *empty_payload = NULL;

    if (cmd == NULL)
    {
        return make_error_response(request_id, "unknown", "MISSING_CMD", "缺少 cmd 字段");
    }

    if (payload == NULL || !cJSON_IsObject(payload))
    {
        empty_payload = cJSON_CreateObject();
        if (empty_payload == NULL)
        {
            return make_error_response(request_id, cmd, "NO_MEMORY", "创建 payload 失败");
        }
        payload = empty_payload;
    }

    if (strcmp(cmd, "get_status") == 0)
    {
        cJSON_Delete(empty_payload);
        return handle_get_status(request_id, cmd);
    }
    if (strcmp(cmd, "list_modes") == 0)
    {
        cJSON_Delete(empty_payload);
        return handle_list_modes(request_id, cmd);
    }
    if (strcmp(cmd, "list_locations") == 0)
    {
        cJSON_Delete(empty_payload);
        return handle_list_locations(request_id, cmd);
    }
    if (strcmp(cmd, "robot_status_refresh") == 0)
    {
        cJSON_Delete(empty_payload);
        return handle_robot_status_refresh(request_id, cmd);
    }
    if (strcmp(cmd, "set_runtime") == 0)
    {
        cJSON *response = handle_set_runtime(payload, request_id, cmd);
        cJSON_Delete(empty_payload);
        return response;
    }
    if (strcmp(cmd, "apply_mode") == 0)
    {
        cJSON *response = handle_apply_mode(payload, request_id, cmd);
        cJSON_Delete(empty_payload);
        return response;
    }
    if (strcmp(cmd, "bucket_command") == 0)
    {
        cJSON *response = handle_bucket_command(payload, request_id, cmd);
        cJSON_Delete(empty_payload);
        return response;
    }
    if (strcmp(cmd, "base_command") == 0)
    {
        cJSON *response = handle_base_command(payload, request_id, cmd);
        cJSON_Delete(empty_payload);
        return response;
    }
    if (strcmp(cmd, "robot_command") == 0)
    {
        cJSON *response = handle_robot_command(payload, request_id, cmd);
        cJSON_Delete(empty_payload);
        return response;
    }
    if (strcmp(cmd, "ui_command") == 0)
    {
        cJSON *response = handle_ui_command(payload, request_id, cmd);
        cJSON_Delete(empty_payload);
        return response;
    }
    if (strcmp(cmd, "start_command") == 0)
    {
        cJSON *response = handle_start_command(payload, request_id, cmd);
        cJSON_Delete(empty_payload);
        return response;
    }

    cJSON_Delete(empty_payload);
    return make_error_response(request_id, cmd, "UNKNOWN_CMD", "不支持的 cmd");
}

static int send_json_response(int fd, cJSON *response)
{
    char *json_text;
    char *frame_text;
    size_t json_len;
    size_t frame_len;
    int ret;

    if (response == NULL)
    {
        return -1;
    }

    json_text = cJSON_PrintUnformatted(response);
    cJSON_Delete(response);
    if (json_text == NULL)
    {
        return -1;
    }

    json_len = strlen(json_text);
    frame_len = strlen(TCP_SERVICE_FRAME_HEAD) + json_len + strlen(TCP_SERVICE_FRAME_TAIL);
    frame_text = (char *)malloc(frame_len + 1u);
    if (frame_text == NULL)
    {
        cJSON_free(json_text);
        return -1;
    }

    snprintf(frame_text, frame_len + 1u, "%s%s%s", TCP_SERVICE_FRAME_HEAD, json_text,
             TCP_SERVICE_FRAME_TAIL);
    ret = send_all(fd, frame_text, frame_len);
    free(frame_text);
    cJSON_free(json_text);
    return ret;
}

static int process_one_frame(int client_fd, const char *json_text, size_t json_len)
{
    cJSON *root;
    cJSON *response;
    char *json_copy = (char *)malloc(json_len + 1u);

    if (json_copy == NULL)
    {
        response = make_error_response(NULL, "unknown", "NO_MEMORY", "内存不足");
        return send_json_response(client_fd, response);
    }

    memcpy(json_copy, json_text, json_len);
    json_copy[json_len] = '\0';
    root = cJSON_Parse(json_copy);
    free(json_copy);

    if (root == NULL)
    {
        response = make_error_response(NULL, "unknown", "BAD_JSON", "JSON 解析失败");
        return send_json_response(client_fd, response);
    }

    response = dispatch_request(root);
    cJSON_Delete(root);
    return send_json_response(client_fd, response);
}

static int process_client_buffer(tcp_client_t *client)
{
    const char *head;
    const char *tail;
    size_t prefix_len = strlen(TCP_SERVICE_FRAME_HEAD);
    size_t suffix_len = strlen(TCP_SERVICE_FRAME_TAIL);

    while (1)
    {
        size_t remaining;
        size_t frame_len;

        client->rx_buffer[client->rx_len] = '\0';
        head = strstr(client->rx_buffer, TCP_SERVICE_FRAME_HEAD);
        if (head == NULL)
        {
            if (client->rx_len >= prefix_len)
            {
                client->rx_len = 0u;
                client->rx_buffer[0] = '\0';
            }
            return 0;
        }

        if (head != client->rx_buffer)
        {
            remaining = client->rx_len - (size_t)(head - client->rx_buffer);
            memmove(client->rx_buffer, head, remaining);
            client->rx_len = remaining;
            client->rx_buffer[client->rx_len] = '\0';
        }

        tail = strstr(client->rx_buffer + prefix_len, TCP_SERVICE_FRAME_TAIL);
        if (tail == NULL)
        {
            return 0;
        }

        frame_len = (size_t)(tail - (client->rx_buffer + prefix_len));
        if (process_one_frame(client->fd, client->rx_buffer + prefix_len, frame_len) != 0)
        {
            return -1;
        }

        remaining = client->rx_len - (size_t)((tail - client->rx_buffer) + suffix_len);
        memmove(client->rx_buffer,
                tail + suffix_len,
                remaining);
        client->rx_len = remaining;
        client->rx_buffer[client->rx_len] = '\0';
    }
}

static int create_server_socket(void)
{
    int fd;
    int optval = 1;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("[tcp_service] socket");
        return -1;
    }

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(TCP_SERVICE_PORT);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        perror("[tcp_service] bind");
        close(fd);
        return -1;
    }

    if (listen(fd, TCP_SERVICE_BACKLOG) != 0)
    {
        perror("[tcp_service] listen");
        close(fd);
        return -1;
    }

    return fd;
}

static void *tcp_service_thread(void *arg)
{
    (void)arg;

    while (g_tcp_service.running)
    {
        fd_set read_fds;
        struct timeval timeout;
        int max_fd = -1;
        int index;
        int ready_count;

        FD_ZERO(&read_fds);
        pthread_mutex_lock(&g_tcp_service.lock);
        if (g_tcp_service.listen_fd >= 0)
        {
            FD_SET(g_tcp_service.listen_fd, &read_fds);
            max_fd = g_tcp_service.listen_fd;
        }

        for (index = 0; index < TCP_SERVICE_MAX_CLIENTS; ++index)
        {
            if (g_tcp_service.clients[index].fd >= 0)
            {
                FD_SET(g_tcp_service.clients[index].fd, &read_fds);
                if (g_tcp_service.clients[index].fd > max_fd)
                {
                    max_fd = g_tcp_service.clients[index].fd;
                }
            }
        }
        pthread_mutex_unlock(&g_tcp_service.lock);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        ready_count = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready_count < 0)
        {
            if (errno != EINTR)
            {
                perror("[tcp_service] select");
            }
            continue;
        }
        if (ready_count == 0)
        {
            continue;
        }

        pthread_mutex_lock(&g_tcp_service.lock);
        if (g_tcp_service.listen_fd >= 0 && FD_ISSET(g_tcp_service.listen_fd, &read_fds))
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(g_tcp_service.listen_fd, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd >= 0)
            {
                if (add_client_locked(client_fd, &client_addr) < 0)
                {
                    close(client_fd);
                }
            }
        }

        for (index = 0; index < TCP_SERVICE_MAX_CLIENTS; ++index)
        {
            tcp_client_t *client = &g_tcp_service.clients[index];
            ssize_t bytes_read;

            if (client->fd < 0 || !FD_ISSET(client->fd, &read_fds))
            {
                continue;
            }

            if (client->rx_len >= (TCP_SERVICE_RX_BUFFER_SIZE - 1u))
            {
                remove_client_locked(index);
                continue;
            }

            bytes_read = recv(client->fd,
                              client->rx_buffer + client->rx_len,
                              (TCP_SERVICE_RX_BUFFER_SIZE - 1u) - client->rx_len,
                              0);
            if (bytes_read <= 0)
            {
                remove_client_locked(index);
                continue;
            }

            client->rx_len += (size_t)bytes_read;
            if (process_client_buffer(client) != 0)
            {
                remove_client_locked(index);
            }
        }
        pthread_mutex_unlock(&g_tcp_service.lock);
    }

    pthread_mutex_lock(&g_tcp_service.lock);
    for (int index = 0; index < TCP_SERVICE_MAX_CLIENTS; ++index)
    {
        remove_client_locked(index);
    }
    if (g_tcp_service.listen_fd >= 0)
    {
        close(g_tcp_service.listen_fd);
        g_tcp_service.listen_fd = -1;
    }
    pthread_mutex_unlock(&g_tcp_service.lock);
    return NULL;
}

int tcp_service_init(void)
{
    int index;

    if (g_tcp_service.initialized)
    {
        return 0;
    }

    for (index = 0; index < TCP_SERVICE_MAX_CLIENTS; ++index)
    {
        g_tcp_service.clients[index].fd = -1;
        g_tcp_service.clients[index].rx_len = 0u;
        g_tcp_service.clients[index].rx_buffer[0] = '\0';
    }

    g_tcp_service.listen_fd = create_server_socket();
    if (g_tcp_service.listen_fd < 0)
    {
        return -1;
    }

    g_tcp_service.running = true;
    if (pthread_create(&g_tcp_service.thread, NULL, tcp_service_thread, NULL) != 0)
    {
        perror("[tcp_service] pthread_create");
        close(g_tcp_service.listen_fd);
        g_tcp_service.listen_fd = -1;
        g_tcp_service.running = false;
        return -1;
    }

    pthread_detach(g_tcp_service.thread);
    g_tcp_service.initialized = true;
    printf("[tcp_service] listen on 0.0.0.0:%d\n", TCP_SERVICE_PORT);
    return 0;
}

void tcp_service_deinit(void)
{
    g_tcp_service.running = false;
}
