#include "robot_tcp.h"

#include "system_manager.h"
#include "tcp_service.h"
#include "serial.h"
#include "app_log.h"

#include <stdint.h>
#include <math.h>

static pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_stations_mutex = PTHREAD_MUTEX_INITIALIZER;
static RobotState_t g_robot_state = {0};
static MapStations_t g_stations_cache = {0};
static unsigned int g_proxy_request_counter = 0u;

/* ========== 新增：位置更新标志 ========== */
static bool g_position_updated = false;
static pthread_mutex_t g_position_mutex = PTHREAD_MUTEX_INITIALIZER;

static void copy_text(char *dst, size_t dst_size, const char *src)
{
    if (dst == NULL || dst_size == 0u)
    {
        return;
    }

    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }

    strncpy(dst, src, dst_size - 1u);
    dst[dst_size - 1u] = '\0';
}

static const char *json_get_string(const cJSON *obj, const char *key)
{
    cJSON *item;

    if (obj == NULL || key == NULL)
    {
        return NULL;
    }

    item = cJSON_GetObjectItemCaseSensitive((cJSON *)obj, key);
    if (cJSON_IsString(item) && item->valuestring != NULL)
    {
        return item->valuestring;
    }

    return NULL;
}

static bool json_get_bool(const cJSON *obj, const char *key, bool default_value)
{
    cJSON *item;

    if (obj == NULL || key == NULL)
    {
        return default_value;
    }

    item = cJSON_GetObjectItemCaseSensitive((cJSON *)obj, key);
    if (cJSON_IsBool(item))
    {
        return cJSON_IsTrue(item);
    }
    if (cJSON_IsNumber(item))
    {
        return item->valueint != 0;
    }

    return default_value;
}

static void clear_station_cache_locked(void)
{
    free(g_stations_cache.stations);
    g_stations_cache.stations = NULL;
    g_stations_cache.station_count = 0;
    g_stations_cache.map_name[0] = '\0';
}

/* 无效名字不能截断后参与名称匹配；坐标不完整也不能兜底成原点。 */
static bool parse_saved_station(const cJSON *item, int fallback_id, StationInfo_t *station)
{
    const char *name = json_get_string(item, "name");
    const char *map_name = json_get_string(item, "mapName");
    cJSON *x = cJSON_GetObjectItemCaseSensitive(item, "x");
    cJSON *y = cJSON_GetObjectItemCaseSensitive(item, "y");
    cJSON *z = cJSON_GetObjectItemCaseSensitive(item, "z");
    cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "id");
    cJSON *type = cJSON_GetObjectItemCaseSensitive(item, "type");

    if (name == NULL || name[0] == '\0' || strlen(name) >= sizeof(station->name) ||
            strpbrk(name, "\r\n") != NULL ||
            (map_name != NULL && strlen(map_name) >= sizeof(station->map_name)) ||
            !cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z) ||
            !isfinite((float)x->valuedouble) || !isfinite((float)y->valuedouble) ||
            !isfinite((float)z->valuedouble))
    {
        APP_LOG_WARN("ROBOT_TCP", "忽略名称或坐标无效的站点");
        return false;
    }
    station->id = cJSON_IsNumber(id) ? id->valueint : fallback_id;
    copy_text(station->name, sizeof(station->name), name);
    copy_text(station->map_name, sizeof(station->map_name), map_name);
    station->x = (float)x->valuedouble;
    station->y = (float)y->valuedouble;
    station->z = (float)z->valuedouble;
    station->type = cJSON_IsNumber(type) ? type->valueint : 1;
    station->is_enable = json_get_bool(item, "is_enable",
                                       json_get_bool(item, "isEnable", true)) ? 1 : 0;
    return true;
}

static void append_saved_stations(cJSON *array, StationInfo_t *stations, int *count)
{
    cJSON *item;
    cJSON_ArrayForEach(item, array)
    {
        if (cJSON_IsObject(item) && parse_saved_station(item, *count + 1, &stations[*count]))
        {
            ++*count;
        }
    }
}

static int update_station_cache_from_response(cJSON *root)
{
    cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
    cJSON *robots = cJSON_GetObjectItemCaseSensitive(data, "robots");
    cJSON *array = cJSON_GetObjectItemCaseSensitive(data, "stations");
    cJSON *robot;
    StationInfo_t *stations = NULL;
    int total = 0;
    int count = 0;

    if (cJSON_IsArray(array))
    {
        total = cJSON_GetArraySize(array);
    }
    else if (cJSON_IsArray(robots))
    {
        cJSON_ArrayForEach(robot, robots)
        {
            cJSON *items = cJSON_GetObjectItemCaseSensitive(robot, "stations");
            if (!cJSON_IsArray(items))
            {
                return -1;
            }
            total += cJSON_GetArraySize(items);
        }
    }
    else
    {
        return -1;
    }
    if (total > 0)
    {
        stations = calloc((size_t)total, sizeof(*stations));
        if (stations == NULL)
        {
            return -1;
        }
    }
    if (cJSON_IsArray(array))
    {
        append_saved_stations(array, stations, &count);
    }
    else
    {
        cJSON_ArrayForEach(robot, robots)
        {
            append_saved_stations(cJSON_GetObjectItemCaseSensitive(robot, "stations"),
                                  stations, &count);
        }
    }
    pthread_mutex_lock(&g_stations_mutex);
    clear_station_cache_locked();
    g_stations_cache.stations = stations;
    g_stations_cache.station_count = count;
    if (count > 0)
    {
        copy_text(g_stations_cache.map_name, sizeof(g_stations_cache.map_name), stations[0].map_name);
    }
    pthread_mutex_unlock(&g_stations_mutex);
    APP_LOG_USER("ROBOT_TCP", "站点查询已应用: 数量=%d", count);
    return 0;
}

static int send_proxy_message(cJSON *message)
{
    if (message == NULL)
    {
        return -1;
    }

    char *text = cJSON_PrintUnformatted(message);
    if (text != NULL)
    {
        APP_LOG_USER("ROBOT_TCP", "发送代理消息: %s", text);
        cJSON_free(text);
    }
    return tcp_service_send_proxy_json(message);
}

static cJSON *build_goto_request(float x, float y, float z)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();
    cJSON *point = cJSON_CreateObject();
    char *point_text;
    char request_id[32];

    if (root == NULL || payload == NULL || point == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(payload);
        cJSON_Delete(point);
        return NULL;
    }

    cJSON_AddNumberToObject(point, "x", x);
    cJSON_AddNumberToObject(point, "y", y);
    cJSON_AddNumberToObject(point, "z", z);

    point_text = cJSON_PrintUnformatted(point);
    cJSON_Delete(point);
    if (point_text == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(payload);
        return NULL;
    }

    ++g_proxy_request_counter;
    snprintf(request_id, sizeof(request_id), "rk-nav-%03u", g_proxy_request_counter);

    cJSON_AddNumberToObject(root, "t", CMD_GOTO_POINT);
    //cJSON_AddStringToObject(root, "id", request_id);
    cJSON_AddStringToObject(payload, "data", point_text);
    cJSON_free(point_text);
    cJSON_AddItemToObject(root, "p", payload);
    return root;
}

static cJSON *build_return_base_request(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();
    char request_id[32];

    if (root == NULL || payload == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(payload);
        return NULL;
    }

    ++g_proxy_request_counter;
    snprintf(request_id, sizeof(request_id), "rk-charge-%03u", g_proxy_request_counter);

    cJSON_AddNumberToObject(root, "t", CMD_SET_CHARGE);
    //cJSON_AddStringToObject(root, "id", request_id);
    cJSON_AddItemToObject(root, "p", payload);
    return root;
}

static int goto_station_locked(const StationInfo_t *station)
{
    float x;
    float y;
    float z;
    char name[sizeof(station->name)];

    if (station == NULL)
    {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    if (station->is_enable == 0)
    {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    x = station->x;
    y = station->y;
    z = station->z;
    copy_text(name, sizeof(name), station->name);

    pthread_mutex_unlock(&g_stations_mutex);
    printf("[robot_tcp] goto station %s (%.2f, %.2f, %.2f) via proxy\n", name, x, y, z);
    return RobotTcp_SendGoto(x, y, z);
}

int RobotTcp_Init(void)
{
    pthread_mutex_lock(&g_state_mutex);
    memset(&g_robot_state, 0, sizeof(g_robot_state));
    copy_text(g_robot_state.map_name, sizeof(g_robot_state.map_name), "unknown");
    pthread_mutex_unlock(&g_state_mutex);

    pthread_mutex_lock(&g_stations_mutex);
    clear_station_cache_locked();
    pthread_mutex_unlock(&g_stations_mutex);

    g_proxy_request_counter = 0u;
    return 0;
}

int RobotTcp_Start(void)
{
    return 0;
}

int RobotTcp_SendMove(float linear_speed, float angular_speed)
{
    cJSON *root;
    cJSON *payload;

    if (!isfinite(linear_speed) || !isfinite(angular_speed))
    {
        return -1;
    }
    root = cJSON_CreateObject();
    if (root == NULL)
    {
        return -1;
    }
    payload = cJSON_AddObjectToObject(root, "p");
    if (payload == NULL ||
            cJSON_AddNumberToObject(root, "t", CMD_MOVE) == NULL ||
            cJSON_AddNumberToObject(payload, "angle", angular_speed) == NULL ||
            cJSON_AddNumberToObject(payload, "speed", linear_speed) == NULL)
    {
        cJSON_Delete(root);
        return -1;
    }
    return send_proxy_message(root);
}

int RobotTcp_SendGoto(float x, float y, float z)
{
    return send_proxy_message(build_goto_request(x, y, z));
}

int RobotTcp_ReturnToBase(void)
{
    return send_proxy_message(build_return_base_request());
}

int RobotTcp_SetCharge(int enable)
{
    (void)enable;
    return -1;
}

int RobotTcp_EmergencyStop(int stop)
{
    (void)stop;
    return -1;
}

void RobotTcp_GetState(RobotState_t *state)
{
    if (state == NULL)
    {
        return;
    }

    pthread_mutex_lock(&g_position_mutex);

    if (g_position_updated)
    {
        /* 有新的位置数据，直接从 tcp_service 获取 */
        robot_position_t pos;
        if (tcp_service_get_robot_position(&pos))
        {
            state->pos.x = (float)pos.x;
            state->pos.y = (float)pos.y;
            state->pos.z = (float)pos.z;
            /* 电量统一来自MCU，停用底盘电量赋值。 */
            // state->battery_percent = pos.powquantity >= 0 ? pos.powquantity : 0;
            state->battery_percent = serial_mcu_battery_percent();
            state->voltage = (float)pos.power;
            state->is_emg_stop = pos.emgStop;
            state->in_nav_map = pos.innavmap ? 1 : 0;
            copy_text(state->map_name, sizeof(state->map_name), pos.mapname);

            /* 同步更新内部缓存 */
            pthread_mutex_lock(&g_state_mutex);
            g_robot_state.pos = state->pos;
            g_robot_state.voltage = state->voltage;
            g_robot_state.is_emg_stop = state->is_emg_stop;
            g_robot_state.in_nav_map = state->in_nav_map;
            copy_text(g_robot_state.map_name, sizeof(g_robot_state.map_name), pos.mapname);
            pthread_mutex_unlock(&g_state_mutex);

            pthread_mutex_unlock(&g_position_mutex);
            return;
        }
    }

    pthread_mutex_unlock(&g_position_mutex);

    /* 没有新位置数据，使用内部缓存 */
    pthread_mutex_lock(&g_state_mutex);
    memcpy(state, &g_robot_state, sizeof(RobotState_t));
    pthread_mutex_unlock(&g_state_mutex);
    state->battery_percent = serial_mcu_battery_percent();
}

/* ========== 新增：从 tcp_service 更新位置 ========== */
void RobotTcp_UpdatePosition(float x, float y, float z, int battery_percent,
                             float voltage, float vel_speed, float vel_angle,
                             int emg_stop, int inbuildmap, int innavmap,
                             const char *mapname)
{
    /* 保留位置更新接口兼容性，底盘电量不再参与状态更新。 */
    (void)battery_percent;
    pthread_mutex_lock(&g_position_mutex);

    /* 更新内部缓存 */
    pthread_mutex_lock(&g_state_mutex);
    g_robot_state.pos.x = x;
    g_robot_state.pos.y = y;
    g_robot_state.pos.z = z;
    // g_robot_state.battery_percent = battery_percent >= 0 ? battery_percent : 0;
    g_robot_state.voltage = voltage;
    g_robot_state.is_emg_stop = emg_stop;
    g_robot_state.in_nav_map = innavmap;
    if (mapname != NULL && mapname[0] != '\0')
    {
        copy_text(g_robot_state.map_name, sizeof(g_robot_state.map_name), mapname);
    }
    pthread_mutex_unlock(&g_state_mutex);

    g_position_updated = true;
    pthread_mutex_unlock(&g_position_mutex);
}

int RobotTcp_RequestStatus(void)
{
    return -1;
}

void RobotTcp_Deinit(void)
{
    pthread_mutex_lock(&g_stations_mutex);
    clear_station_cache_locked();
    pthread_mutex_unlock(&g_stations_mutex);
}

int RobotTcp_StartMapping(void)
{
    return -1;
}

int RobotTcp_BackupMap(const char *map_name)
{
    (void)map_name;
    return -1;
}

int RobotTcp_FetchAndSaveStations(void)
{
    return send_proxy_message(RobotTcp_BuildStationsQueryRequest());
}

int RobotTcp_GotoStation(int station_index)
{
    int ret;

    pthread_mutex_lock(&g_stations_mutex);
    if (station_index < 0 || station_index >= g_stations_cache.station_count ||
            g_stations_cache.stations == NULL)
    {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    ret = goto_station_locked(&g_stations_cache.stations[station_index]);
    return ret;
}

int RobotTcp_GetStationList(MapStations_t *out_stations)
{
    if (out_stations == NULL)
    {
        return -1;
    }

    memset(out_stations, 0, sizeof(*out_stations));

    pthread_mutex_lock(&g_stations_mutex);
    if (g_stations_cache.station_count > 0 && g_stations_cache.stations != NULL)
    {
        out_stations->stations = (StationInfo_t *)malloc((size_t)g_stations_cache.station_count * sizeof(
                                     StationInfo_t));
        if (out_stations->stations == NULL)
        {
            pthread_mutex_unlock(&g_stations_mutex);
            return -1;
        }

        memcpy(out_stations->stations,
               g_stations_cache.stations,
               (size_t)g_stations_cache.station_count * sizeof(StationInfo_t));
    }

    out_stations->station_count = g_stations_cache.station_count;
    copy_text(out_stations->map_name, sizeof(out_stations->map_name), g_stations_cache.map_name);
    pthread_mutex_unlock(&g_stations_mutex);
    return 0;
}

void RobotTcp_FreeStationList(MapStations_t *stations)
{
    if (stations == NULL)
    {
        return;
    }

    free(stations->stations);
    stations->stations = NULL;
    stations->station_count = 0;
    stations->map_name[0] = '\0';
}

int RobotTcp_GotoStationByName(const char *station_name)
{
    int index;

    if (station_name == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_stations_mutex);
    if (g_stations_cache.stations == NULL)
    {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    for (index = 0; index < g_stations_cache.station_count; ++index)
    {
        if (strcmp(g_stations_cache.stations[index].name, station_name) == 0)
        {
            return goto_station_locked(&g_stations_cache.stations[index]);
        }
    }

    pthread_mutex_unlock(&g_stations_mutex);
    return -1;
}

int RobotTcp_GotoStationById(int station_id)
{
    int index;

    pthread_mutex_lock(&g_stations_mutex);
    if (g_stations_cache.stations == NULL)
    {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    for (index = 0; index < g_stations_cache.station_count; ++index)
    {
        if (g_stations_cache.stations[index].id == station_id)
        {
            return goto_station_locked(&g_stations_cache.stations[index]);
        }
    }

    pthread_mutex_unlock(&g_stations_mutex);
    return -1;
}

int RobotTcp_GetStationCoordsById(int station_id, float *x, float *y, float *z,
                                  char *name, int name_size)
{
    int index;

    pthread_mutex_lock(&g_stations_mutex);
    if (g_stations_cache.stations == NULL)
    {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    for (index = 0; index < g_stations_cache.station_count; ++index)
    {
        StationInfo_t *target = &g_stations_cache.stations[index];
        if (target->id != station_id)
        {
            continue;
        }

        if (x != NULL)
        {
            *x = target->x;
        }
        if (y != NULL)
        {
            *y = target->y;
        }
        if (z != NULL)
        {
            *z = target->z;
        }
        if (name != NULL && name_size > 0)
        {
            copy_text(name, (size_t)name_size, target->name);
        }

        pthread_mutex_unlock(&g_stations_mutex);
        return 0;
    }

    pthread_mutex_unlock(&g_stations_mutex);
    return -1;
}

cJSON *RobotTcp_BuildStationsQueryRequest(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *payload = cJSON_CreateObject();
    char request_id[32];

    if (root == NULL || payload == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(payload);
        return NULL;
    }

    ++g_proxy_request_counter;
    snprintf(request_id, sizeof(request_id), "stations-%03u", g_proxy_request_counter);
    cJSON_AddStringToObject(root, "cmd", "get_saved_stations");
    cJSON_AddStringToObject(root, "request_id", request_id);
    cJSON_AddItemToObject(root, "payload", payload);
    return root;
}

bool RobotTcp_IsProxyMessage(const cJSON *root)
{
    cJSON *t_item;
    const char *cmd;

    if (!cJSON_IsObject(root))
    {
        return false;
    }

    cmd = json_get_string(root, "cmd");
    if (cmd != NULL &&
            (strcmp(cmd, "get_saved_stations") == 0 || strcmp(cmd, "get_stations") == 0))
    {
        return true;
    }

    t_item = cJSON_GetObjectItemCaseSensitive((cJSON *)root, "t");
    return cJSON_IsNumber(t_item) && t_item->valueint == CMD_GOTO_POINT;
}

int RobotTcp_HandleProxyMessage(cJSON *root)
{
    const char *cmd;

    if (!cJSON_IsObject(root))
    {
        return 0;
    }

    cmd = json_get_string(root, "cmd");
    if (cmd != NULL &&
            (strcmp(cmd, "get_saved_stations") == 0 || strcmp(cmd, "get_stations") == 0))
    {
        if (!json_get_bool(root, "ok", false))
        {
            printf("[robot_tcp] proxy station query failed: %s\n",
                   json_get_string(root, "message") != NULL ? json_get_string(root, "message") : "unknown");
            return 1;
        }

        if (update_station_cache_from_response(root) != 0)
        {
            printf("[robot_tcp] failed to parse proxy station response\n");
            return 1;
        }

        system_location_sync_from_robot_cache();
        printf("[robot_tcp] station cache updated from proxy\n");
        return 1;
    }

    if (RobotTcp_IsProxyMessage(root))
    {
        return 1;
    }

    return 0;
}
