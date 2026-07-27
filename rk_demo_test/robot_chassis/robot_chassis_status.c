#include "robot_chassis_status.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"

static bool json_integer(const cJSON *item, int *value)
{
    if (!cJSON_IsNumber(item) || item->valuedouble < (double)INT_MIN ||
        item->valuedouble > (double)INT_MAX) {
        return false;
    }
    *value = (int)item->valuedouble;
    return item->valuedouble == (double)*value;
}

static void parse_bool(const cJSON *object, const char *name, bool *output,
                       uint64_t valid_bit, robot_chassis_status_t *status)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, name);

    if (cJSON_IsBool(item)) {
        *output = cJSON_IsTrue(item);
        status->valid_fields |= valid_bit;
    }
}

static void parse_int(const cJSON *object, const char *name, int *output,
                      uint64_t valid_bit, robot_chassis_status_t *status)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, name);
    int value;

    if (json_integer(item, &value)) {
        *output = value;
        status->valid_fields |= valid_bit;
    }
}

static void parse_number(const cJSON *object, const char *name, double *output,
                         uint64_t valid_bit,
                         robot_chassis_status_t *status)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, name);

    if (cJSON_IsNumber(item) && isfinite(item->valuedouble)) {
        *output = item->valuedouble;
        status->valid_fields |= valid_bit;
    }
}

static void parse_string(const cJSON *object, const char *name, char *output,
                         size_t capacity, uint64_t valid_bit,
                         robot_chassis_status_t *status)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, name);
    size_t length;

    if (!cJSON_IsString(item) || item->valuestring == NULL) {
        return;
    }
    length = strlen(item->valuestring);
    if (length >= capacity) {
        return;
    }
    memcpy(output, item->valuestring, length + 1U);
    status->valid_fields |= valid_bit;
}

static void parse_pose(const cJSON *robot, robot_chassis_status_t *status)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(robot, "pos");
    robot_chassis_pose_t pose;
    char extra;

    if (!cJSON_IsString(item) || item->valuestring == NULL) {
        return;
    }
    if (sscanf(item->valuestring, "%f,%f,%f%c", &pose.x, &pose.y, &pose.z,
               &extra) != 3) {
        return;
    }
    if (!isfinite(pose.x) || !isfinite(pose.y) || !isfinite(pose.z)) {
        return;
    }
    status->pose = pose;
    status->valid_fields |= ROBOT_CHASSIS_STATUS_VALID_POSE;
}

static void parse_power_board(const cJSON *robot,
                              robot_chassis_status_t *status)
{
    const cJSON *board =
        cJSON_GetObjectItemCaseSensitive(robot, "powerBoard");

    if (!cJSON_IsObject(board)) {
        return;
    }
    parse_int(board, "Battery_failure", &status->battery_failure,
              ROBOT_CHASSIS_STATUS_VALID_BATTERY_FAILURE, status);
    parse_int(board, "errCode1", &status->error_code_1,
              ROBOT_CHASSIS_STATUS_VALID_ERROR_CODE_1, status);
    parse_int(board, "errCode2", &status->error_code_2,
              ROBOT_CHASSIS_STATUS_VALID_ERROR_CODE_2, status);
    parse_int(board, "errCode3", &status->error_code_3,
              ROBOT_CHASSIS_STATUS_VALID_ERROR_CODE_3, status);
    parse_int(board, "power_status", &status->power_status,
              ROBOT_CHASSIS_STATUS_VALID_POWER_STATUS, status);
    parse_number(board, "SOC", &status->power_board_soc,
                 ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_SOC, status);
    parse_number(board, "SOH", &status->power_board_soh,
                 ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_SOH, status);
    parse_number(board, "capacity", &status->power_board_capacity,
                 ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_CAPACITY, status);
    parse_number(board, "chargeTime", &status->power_board_charge_time,
                 ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_CHARGE_TIME, status);
    parse_number(board, "current", &status->power_board_current,
                 ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_CURRENT, status);
    parse_number(
        board, "max_temperature", &status->power_board_max_temperature,
        ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_MAX_TEMPERATURE, status);
    parse_number(
        board, "min_temperature", &status->power_board_min_temperature,
        ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_MIN_TEMPERATURE, status);
    parse_int(board, "switch_status", &status->power_board_switch_status,
              ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_SWITCH_STATUS, status);
    parse_number(board, "voltage", &status->power_board_voltage,
                 ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_VOLTAGE, status);
}

static int find_robot_object(const char *json, size_t length, cJSON **root_out,
                             const cJSON **robot_out)
{
    cJSON *root;
    const char *parse_end = NULL;
    const cJSON *t;
    const cJSON *p;
    const cJSON *data;
    const cJSON *robot;
    int t_value;

    root = cJSON_ParseWithLengthOpts(json, length, &parse_end, 0);
    if (root == NULL) {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (parse_end < json + length && isspace((unsigned char)*parse_end)) {
        parse_end++;
    }
    if (parse_end != json + length || !cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }

    t = cJSON_GetObjectItemCaseSensitive(root, "t");
    if (!json_integer(t, &t_value) || t_value != ROBOT_CHASSIS_T_STATUS) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    p = cJSON_GetObjectItemCaseSensitive(root, "p");
    data = cJSON_IsObject(p)
               ? cJSON_GetObjectItemCaseSensitive(p, "data")
               : NULL;
    robot = cJSON_IsObject(data)
                ? cJSON_GetObjectItemCaseSensitive(data, "8")
                : NULL;
    if (!cJSON_IsObject(robot)) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }

    *root_out = root;
    *robot_out = robot;
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_status_parse(const char *json, size_t length,
                               robot_chassis_status_t *status)
{
    cJSON *root = NULL;
    const cJSON *robot = NULL;
    const cJSON *devstate;
    const cJSON *real_state;
    int result;

    if (status != NULL) {
        status->valid_fields = 0U;
    }
    if (json == NULL || length == 0U || status == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    result = find_robot_object(json, length, &root, &robot);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }

    parse_bool(robot, "agvStop", &status->agv_stop,
               ROBOT_CHASSIS_STATUS_VALID_AGV_STOP, status);
    devstate = cJSON_GetObjectItemCaseSensitive(robot, "devstate");
    if (cJSON_IsObject(devstate)) {
        parse_int(devstate, "batteryDev", &status->battery_dev,
                  ROBOT_CHASSIS_STATUS_VALID_BATTERY_DEV, status);
    }
    parse_bool(robot, "inbuildmap", &status->in_build_map,
               ROBOT_CHASSIS_STATUS_VALID_IN_BUILD_MAP, status);
    parse_bool(robot, "innavmap", &status->in_nav_map,
               ROBOT_CHASSIS_STATUS_VALID_IN_NAV_MAP, status);
    parse_int(robot, "emgStop", &status->emergency_stop,
              ROBOT_CHASSIS_STATUS_VALID_EMERGENCY_STOP, status);
    parse_int(robot, "softStop", &status->soft_stop,
              ROBOT_CHASSIS_STATUS_VALID_SOFT_STOP, status);
    parse_int(robot, "navServerBusy", &status->navigation_server_busy,
              ROBOT_CHASSIS_STATUS_VALID_NAV_BUSY, status);
    parse_number(robot, "powquantity", &status->battery_percent,
                 ROBOT_CHASSIS_STATUS_VALID_BATTERY_PERCENT, status);
    parse_number(robot, "power", &status->power,
                 ROBOT_CHASSIS_STATUS_VALID_POWER, status);
    parse_pose(robot, status);
    parse_string(robot, "mapname", status->map_name,
                 sizeof(status->map_name),
                 ROBOT_CHASSIS_STATUS_VALID_MAP_NAME, status);

    real_state = cJSON_GetObjectItemCaseSensitive(robot, "realState");
    if (cJSON_IsObject(real_state)) {
        parse_string(real_state, "lockmap", status->lock_map,
                     sizeof(status->lock_map),
                     ROBOT_CHASSIS_STATUS_VALID_LOCK_MAP, status);
    }
    parse_power_board(robot, status);

    cJSON_Delete(root);
    return ROBOT_CHASSIS_OK;
}
