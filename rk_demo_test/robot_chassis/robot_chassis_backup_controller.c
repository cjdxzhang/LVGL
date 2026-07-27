#include "robot_chassis_backup_controller.h"

#include <string.h>

#include "robot_chassis_control.h"

static int emit_command(robot_chassis_backup_controller_t *controller,
                        robot_chassis_command_t *command)
{
    int result;

    result = controller->emit(command, controller->user_data);
    robot_chassis_command_release(command);
    return result;
}

static int emit_begin(robot_chassis_backup_controller_t *controller)
{
    robot_chassis_command_t command;
    int result;

    result = robot_chassis_build_map_backup_begin(controller->map_name,
                                                   &command);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    return emit_command(controller, &command);
}

static int restart_from_first_segment(
    robot_chassis_backup_controller_t *controller)
{
    int result;

    result = robot_chassis_backup_begin(&controller->transfer,
                                        controller->temporary_path,
                                        controller->final_path);
    if (result == ROBOT_CHASSIS_OK) {
        result = emit_begin(controller);
    }
    if (result != ROBOT_CHASSIS_OK) {
        robot_chassis_backup_abort(&controller->transfer);
        controller->active = false;
        if (controller->result != NULL) {
            controller->result(result, NULL, controller->user_data);
        }
    }
    return result;
}

void robot_chassis_backup_controller_init(
    robot_chassis_backup_controller_t *controller,
    robot_chassis_backup_emit_t emit,
    robot_chassis_backup_result_t result,
    void *user_data)
{
    if (controller == NULL) {
        return;
    }
    memset(controller, 0, sizeof(*controller));
    controller->emit = emit;
    controller->result = result;
    controller->user_data = user_data;
}

int robot_chassis_backup_controller_start(
    robot_chassis_backup_controller_t *controller, const char *map_name,
    const char *temporary_path, const char *final_path)
{
    size_t map_length;
    size_t temporary_length;
    size_t final_length;
    int result;

    if (controller == NULL || controller->emit == NULL || map_name == NULL ||
        temporary_path == NULL || final_path == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    if (controller->active) {
        return ROBOT_CHASSIS_ERR_BUSY;
    }
    map_length = strlen(map_name);
    temporary_length = strlen(temporary_path);
    final_length = strlen(final_path);
    if (map_length == 0U || map_length >= sizeof(controller->map_name) ||
        temporary_length == 0U ||
        temporary_length >= sizeof(controller->temporary_path) ||
        final_length == 0U || final_length >= sizeof(controller->final_path)) {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }

    memcpy(controller->map_name, map_name, map_length + 1U);
    memcpy(controller->temporary_path, temporary_path, temporary_length + 1U);
    memcpy(controller->final_path, final_path, final_length + 1U);
    result = robot_chassis_backup_begin(&controller->transfer,
                                        controller->temporary_path,
                                        controller->final_path);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    controller->active = true;
    result = emit_begin(controller);
    if (result != ROBOT_CHASSIS_OK) {
        robot_chassis_backup_abort(&controller->transfer);
        controller->active = false;
    }
    return result;
}

int robot_chassis_backup_controller_accept(
    robot_chassis_backup_controller_t *controller, const char *json,
    size_t length)
{
    robot_chassis_command_t command;
    int result;
    int emit_result;

    if (controller == NULL || !controller->active) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_chassis_backup_accept_response(&controller->transfer, json,
                                                  length);
    if (result == ROBOT_CHASSIS_ERR_WOULD_BLOCK) {
        emit_result = robot_chassis_build_map_backup_next(&command);
        if (emit_result == ROBOT_CHASSIS_OK) {
            emit_result = emit_command(controller, &command);
        }
        if (emit_result != ROBOT_CHASSIS_OK) {
            robot_chassis_backup_controller_abort(controller, emit_result);
            return emit_result;
        }
        return result;
    }
    if (result == ROBOT_CHASSIS_OK) {
        controller->active = false;
        if (controller->result != NULL) {
            controller->result(ROBOT_CHASSIS_OK, controller->final_path,
                               controller->user_data);
        }
        return ROBOT_CHASSIS_OK;
    }

    emit_result = restart_from_first_segment(controller);
    return emit_result == ROBOT_CHASSIS_OK ? result : emit_result;
}

int robot_chassis_backup_controller_timeout(
    robot_chassis_backup_controller_t *controller)
{
    if (controller == NULL || !controller->active) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    robot_chassis_backup_abort(&controller->transfer);
    return restart_from_first_segment(controller);
}

void robot_chassis_backup_controller_abort(
    robot_chassis_backup_controller_t *controller, int result)
{
    bool was_active;

    if (controller == NULL) {
        return;
    }
    was_active = controller->active;
    robot_chassis_backup_abort(&controller->transfer);
    controller->active = false;
    if (was_active && controller->result != NULL) {
        controller->result(result, NULL, controller->user_data);
    }
}
