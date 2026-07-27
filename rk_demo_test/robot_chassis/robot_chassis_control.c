#include "robot_chassis_control.h"

#include <math.h>
#include <string.h>

#include "cJSON.h"

static bool valid_text(const char *text)
{
    return text != NULL && text[0] != '\0';
}

static cJSON *create_t_root(int t)
{
    cJSON *root = cJSON_CreateObject();

    if (root != NULL && cJSON_AddNumberToObject(root, "t", t) == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    return root;
}

static cJSON *add_p_object(cJSON *root)
{
    cJSON *p;

    if (root == NULL) {
        return NULL;
    }
    p = cJSON_CreateObject();
    if (p == NULL) {
        return NULL;
    }
    cJSON_AddItemToObject(root, "p", p);
    return p;
}

static cJSON *add_data_object(cJSON *root)
{
    cJSON *p = add_p_object(root);
    cJSON *data;

    if (p == NULL) {
        return NULL;
    }
    data = cJSON_CreateObject();
    if (data == NULL) {
        return NULL;
    }
    cJSON_AddItemToObject(p, "data", data);
    return data;
}

static int finalize_command(cJSON *root, int response_t, bool dangerous,
                            bool special,
                            robot_chassis_special_command_t special_command,
                            robot_chassis_command_t *command)
{
    char *json;

    if (root == NULL || command == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (json == NULL) {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }

    memset(command, 0, sizeof(*command));
    command->data = json;
    command->length = strlen(json);
    command->response_t = response_t;
    command->dangerous = dangerous;
    command->uses_special_command = special;
    command->special_command = special_command;
    return ROBOT_CHASSIS_OK;
}

static int build_simple_t(int t, bool dangerous,
                          robot_chassis_command_t *command)
{
    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    return finalize_command(create_t_root(t), t, dangerous, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

static int add_inner_data_string(cJSON *root, cJSON *inner)
{
    cJSON *p;
    char *inner_json;

    if (root == NULL || inner == NULL) {
        cJSON_Delete(inner);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    inner_json = cJSON_PrintUnformatted(inner);
    cJSON_Delete(inner);
    if (inner_json == NULL) {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    p = add_p_object(root);
    if (p == NULL ||
        cJSON_AddStringToObject(p, "data", inner_json) == NULL) {
        cJSON_free(inner_json);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    cJSON_free(inner_json);
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_command_release(robot_chassis_command_t *command)
{
    if (command == NULL) {
        return;
    }
    cJSON_free(command->data);
    memset(command, 0, sizeof(*command));
}

int robot_chassis_build_heartbeat(robot_chassis_command_t *command)
{
    cJSON *root;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = cJSON_CreateObject();
    if (root == NULL ||
        cJSON_AddStringToObject(root, "cmd", "heatbeat") == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, 0, false, true,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_reboot(robot_chassis_command_t *command)
{
    cJSON *root;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = cJSON_CreateObject();
    if (root == NULL || add_p_object(root) == NULL ||
        cJSON_AddStringToObject(root, "cmd", "reBoot") == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, 0, true, true,
                            ROBOT_CHASSIS_CMD_REBOOT, command);
}

int robot_chassis_build_status_query(robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *p;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_STATUS);
    p = add_p_object(root);
    if (p == NULL || cJSON_AddNumberToObject(p, "data", 1) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_STATUS, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_manual_move(double angle, double speed,
                                    robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *p;

    if (command == NULL || !isfinite(angle) || !isfinite(speed)) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_MANUAL_MOVE);
    p = add_p_object(root);
    if (p == NULL || cJSON_AddNumberToObject(p, "angle", angle) == NULL ||
        cJSON_AddNumberToObject(p, "speed", speed) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_MANUAL_MOVE, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_goto(double x, double y, double z, double tolerance,
                             robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *data;

    if (command == NULL || !isfinite(x) || !isfinite(y) || !isfinite(z) ||
        !isfinite(tolerance) || tolerance <= 0.0) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_GOTO_POINT);
    data = add_data_object(root);
    if (data == NULL || cJSON_AddNumberToObject(data, "x", x) == NULL ||
        cJSON_AddNumberToObject(data, "y", y) == NULL ||
        cJSON_AddNumberToObject(data, "z", z) == NULL ||
        cJSON_AddNumberToObject(data, "tolerance", tolerance) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_GOTO_POINT, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_start_charge(double x, double y, double z,
                                     robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (command == NULL || !isfinite(x) || !isfinite(y) || !isfinite(z)) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_START_CHARGE);
    inner = cJSON_CreateObject();
    if (inner == NULL || cJSON_AddNumberToObject(inner, "x", x) == NULL ||
        cJSON_AddNumberToObject(inner, "y", y) == NULL ||
        cJSON_AddNumberToObject(inner, "z", z) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_START_CHARGE, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_cancel_charge(robot_chassis_command_t *command)
{
    cJSON *root;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_CANCEL_CHARGE);
    if (add_p_object(root) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_CANCEL_CHARGE, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_auto_explore(bool enabled,
                                     robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *data;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_AUTO_EXPLORE);
    data = add_data_object(root);
    if (data == NULL ||
        cJSON_AddBoolToObject(data, "explore", enabled) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_AUTO_EXPLORE, true, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_lock_map(const char *map_name,
                                 robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (!valid_text(map_name) || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_LOCK_MAP);
    inner = cJSON_CreateObject();
    if (inner == NULL ||
        cJSON_AddStringToObject(inner, "mapname", map_name) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_LOCK_MAP, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_init_pose(double x, double y, double z,
                                  robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (command == NULL || !isfinite(x) || !isfinite(y) || !isfinite(z)) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_INIT_POSE);
    inner = cJSON_CreateObject();
    if (inner == NULL || cJSON_AddNumberToObject(inner, "x", x) == NULL ||
        cJSON_AddNumberToObject(inner, "y", y) == NULL ||
        cJSON_AddNumberToObject(inner, "z", z) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_INIT_POSE, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_soft_stop(bool stop,
                                  robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *data;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_SOFT_STOP);
    data = add_data_object(root);
    if (data == NULL ||
        cJSON_AddNumberToObject(data, "stop", stop ? 1 : 0) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_SOFT_STOP, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_start_mapping(robot_chassis_command_t *command)
{
    return build_simple_t(ROBOT_CHASSIS_T_START_MAPPING, true, command);
}

int robot_chassis_build_close_mapping(robot_chassis_command_t *command)
{
    return build_simple_t(ROBOT_CHASSIS_T_CLOSE_MAPPING, true, command);
}

int robot_chassis_build_save_map(const char *name, const char *alias,
                                 robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (!valid_text(name) || alias == NULL || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_SAVE_MAP);
    inner = cJSON_CreateObject();
    if (inner == NULL ||
        cJSON_AddStringToObject(inner, "name", name) == NULL ||
        cJSON_AddStringToObject(inner, "alias", alias) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_SAVE_MAP, true, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_realtime_map_query(robot_chassis_command_t *command)
{
    return build_simple_t(ROBOT_CHASSIS_T_REALTIME_MAP, false, command);
}

int robot_chassis_build_map_list_query(robot_chassis_command_t *command)
{
    return build_simple_t(ROBOT_CHASSIS_T_MAP_LIST, false, command);
}

int robot_chassis_build_clear_navigation(robot_chassis_command_t *command)
{
    return build_simple_t(ROBOT_CHASSIS_T_CLEAR_NAVIGATION, false, command);
}

int robot_chassis_build_factory_reset(robot_chassis_command_t *command)
{
    cJSON *root;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_FACTORY_RESET);
    if (add_p_object(root) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_FACTORY_RESET, true, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_set_time(const char *time_text,
                                 robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *data;

    if (!valid_text(time_text) || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_SET_TIME);
    data = add_data_object(root);
    if (data == NULL ||
        cJSON_AddStringToObject(data, "time", time_text) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_SET_TIME, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_map_backup_begin(const char *map_name,
                                         robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (!valid_text(map_name) || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_MAP_BACKUP_BEGIN);
    inner = cJSON_CreateObject();
    if (inner == NULL ||
        cJSON_AddStringToObject(inner, "name", map_name) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_MAP_BACKUP_BEGIN, false,
                            false, ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_map_backup_next(robot_chassis_command_t *command)
{
    return build_simple_t(ROBOT_CHASSIS_T_MAP_BACKUP_NEXT, false, command);
}

int robot_chassis_build_upgrade_begin(const char *file_name, uint64_t file_size,
                                      uint32_t step_total,
                                      robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (!valid_text(file_name) || file_size == 0U || step_total == 0U ||
        command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_UPGRADE_BEGIN);
    inner = cJSON_CreateObject();
    if (inner == NULL ||
        cJSON_AddStringToObject(inner, "fileName", file_name) == NULL ||
        cJSON_AddNumberToObject(inner, "fileSize", (double)file_size) == NULL ||
        cJSON_AddNumberToObject(inner, "stepTotal", step_total) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_UPGRADE_BEGIN, true, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_upgrade_chunk(const char *file_data,
                                      const char *file_name, const char *md5,
                                      uint32_t step,
                                      robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (!valid_text(file_data) || !valid_text(file_name) || !valid_text(md5) ||
        strlen(file_data) > ROBOT_CHASSIS_UPGRADE_HEX_CHUNK_MAX ||
        step == 0U || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_UPGRADE_CHUNK);
    inner = cJSON_CreateObject();
    if (inner == NULL ||
        cJSON_AddStringToObject(inner, "fileData", file_data) == NULL ||
        cJSON_AddStringToObject(inner, "fileName", file_name) == NULL ||
        cJSON_AddStringToObject(inner, "md5", md5) == NULL ||
        cJSON_AddNumberToObject(inner, "step", step) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_UPGRADE_CHUNK, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_upgrade_finish(const char *file_name, const char *md5,
                                       robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (!valid_text(file_name) || !valid_text(md5) || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_UPGRADE_FINISH);
    inner = cJSON_CreateObject();
    if (inner == NULL ||
        cJSON_AddStringToObject(inner, "fileName", file_name) == NULL ||
        cJSON_AddStringToObject(inner, "md5", md5) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_UPGRADE_FINISH, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_upgrade_extract(const char *file_name,
                                        robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *inner;
    int result;

    if (!valid_text(file_name) || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_UPGRADE_EXTRACT);
    inner = cJSON_CreateObject();
    if (inner == NULL ||
        cJSON_AddStringToObject(inner, "fileName", file_name) == NULL) {
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = add_inner_data_string(root, inner);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_UPGRADE_EXTRACT, false, false,
                            ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}

int robot_chassis_build_wheel_enable(bool enabled,
                                     robot_chassis_command_t *command)
{
    cJSON *root;
    cJSON *data;

    if (command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = create_t_root(ROBOT_CHASSIS_T_WHEEL_ENABLE);
    data = add_data_object(root);
    if (data == NULL ||
        cJSON_AddBoolToObject(data, "enable", enabled) == NULL) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    return finalize_command(root, ROBOT_CHASSIS_T_WHEEL_ENABLE, !enabled,
                            false, ROBOT_CHASSIS_CMD_HEARTBEAT, command);
}
