#ifndef ROBOT_CHASSIS_UPGRADE_CONTROLLER_H
#define ROBOT_CHASSIS_UPGRADE_CONTROLLER_H

#include "robot_chassis_transfer.h"

typedef int (*robot_chassis_upgrade_emit_t)(
    const robot_chassis_command_t *command, void *user_data);
typedef void (*robot_chassis_upgrade_result_t)(int result, void *user_data);

typedef struct {
    bool active;
    robot_chassis_upgrade_transfer_t transfer;
    robot_chassis_upgrade_emit_t emit;
    robot_chassis_upgrade_result_t result;
    void *user_data;
} robot_chassis_upgrade_controller_t;

void robot_chassis_upgrade_controller_init(
    robot_chassis_upgrade_controller_t *controller,
    robot_chassis_upgrade_emit_t emit,
    robot_chassis_upgrade_result_t result,
    void *user_data);
int robot_chassis_upgrade_controller_start(
    robot_chassis_upgrade_controller_t *controller, const char *file_path);
int robot_chassis_upgrade_controller_accept(
    robot_chassis_upgrade_controller_t *controller, const char *json,
    size_t length);
int robot_chassis_upgrade_controller_timeout(
    robot_chassis_upgrade_controller_t *controller);
void robot_chassis_upgrade_controller_abort(
    robot_chassis_upgrade_controller_t *controller, int result);
int robot_chassis_upgrade_controller_expected_t(
    const robot_chassis_upgrade_controller_t *controller);

#endif
