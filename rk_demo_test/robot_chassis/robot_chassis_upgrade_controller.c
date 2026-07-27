#include "robot_chassis_upgrade_controller.h"

#include "robot_chassis_control.h"
#include <string.h>

static int emit_current(robot_chassis_upgrade_controller_t *controller)
{
    robot_chassis_command_t command = {0};
    int result;

    result = robot_chassis_upgrade_build_next(&controller->transfer, &command);
    if (result == ROBOT_CHASSIS_OK) {
        result = controller->emit(&command, controller->user_data);
    }
    robot_chassis_command_release(&command);
    return result;
}

static void finish(robot_chassis_upgrade_controller_t *controller, int result)
{
    controller->active = false;
    if (controller->result != NULL) {
        controller->result(result, controller->user_data);
    }
}

void robot_chassis_upgrade_controller_init(
    robot_chassis_upgrade_controller_t *controller,
    robot_chassis_upgrade_emit_t emit,
    robot_chassis_upgrade_result_t result,
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

int robot_chassis_upgrade_controller_start(
    robot_chassis_upgrade_controller_t *controller, const char *file_path)
{
    int result;

    if (controller == NULL || controller->emit == NULL || file_path == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    if (controller->active) {
        return ROBOT_CHASSIS_ERR_BUSY;
    }
    result = robot_chassis_upgrade_begin(&controller->transfer, file_path);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    controller->active = true;
    result = emit_current(controller);
    if (result != ROBOT_CHASSIS_OK) {
        robot_chassis_upgrade_abort(&controller->transfer);
        controller->active = false;
    }
    return result;
}

int robot_chassis_upgrade_controller_accept(
    robot_chassis_upgrade_controller_t *controller, const char *json,
    size_t length)
{
    int result;
    int emit_result;

    if (controller == NULL || !controller->active) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_chassis_upgrade_accept_response(&controller->transfer,
                                                    json, length);
    if (result == ROBOT_CHASSIS_OK &&
        controller->transfer.kind == ROBOT_CHASSIS_TRANSFER_NONE) {
        finish(controller, ROBOT_CHASSIS_OK);
        return ROBOT_CHASSIS_OK;
    }
    if (result == ROBOT_CHASSIS_OK ||
        result == ROBOT_CHASSIS_ERR_WOULD_BLOCK) {
        emit_result = emit_current(controller);
        if (emit_result == ROBOT_CHASSIS_OK) {
            return result;
        }
        robot_chassis_upgrade_controller_abort(controller, emit_result);
        return emit_result;
    }
    finish(controller, result);
    return result;
}

int robot_chassis_upgrade_controller_timeout(
    robot_chassis_upgrade_controller_t *controller)
{
    int result;
    int emit_result;

    if (controller == NULL || !controller->active) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = robot_chassis_upgrade_timeout(&controller->transfer);
    if (result == ROBOT_CHASSIS_ERR_WOULD_BLOCK) {
        emit_result = emit_current(controller);
        if (emit_result == ROBOT_CHASSIS_OK) {
            return result;
        }
        robot_chassis_upgrade_controller_abort(controller, emit_result);
        return emit_result;
    }
    finish(controller, result);
    return result;
}

void robot_chassis_upgrade_controller_abort(
    robot_chassis_upgrade_controller_t *controller, int result)
{
    bool was_active;

    if (controller == NULL) {
        return;
    }
    was_active = controller->active;
    robot_chassis_upgrade_abort(&controller->transfer);
    controller->active = false;
    if (was_active && controller->result != NULL) {
        controller->result(result, controller->user_data);
    }
}

int robot_chassis_upgrade_controller_expected_t(
    const robot_chassis_upgrade_controller_t *controller)
{
    if (controller == NULL || !controller->active) {
        return 0;
    }
    switch (controller->transfer.stage) {
    case ROBOT_CHASSIS_UPGRADE_WAIT_BEGIN:
        return ROBOT_CHASSIS_T_UPGRADE_BEGIN;
    case ROBOT_CHASSIS_UPGRADE_WAIT_CHUNK:
        return ROBOT_CHASSIS_T_UPGRADE_CHUNK;
    case ROBOT_CHASSIS_UPGRADE_WAIT_FINISH:
        return ROBOT_CHASSIS_T_UPGRADE_FINISH;
    case ROBOT_CHASSIS_UPGRADE_WAIT_EXTRACT:
        return ROBOT_CHASSIS_T_UPGRADE_EXTRACT;
    default:
        return 0;
    }
}
