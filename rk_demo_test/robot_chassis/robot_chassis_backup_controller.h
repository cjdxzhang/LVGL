#ifndef ROBOT_CHASSIS_BACKUP_CONTROLLER_H
#define ROBOT_CHASSIS_BACKUP_CONTROLLER_H

#include "robot_chassis_transfer.h"

typedef int (*robot_chassis_backup_emit_t)(
    const robot_chassis_command_t *command);
typedef void (*robot_chassis_backup_result_t)(
    int result, const char *final_path);

typedef struct
{
    bool active;
    robot_chassis_backup_transfer_t transfer;
    char map_name[128];
    char temporary_path[256];
    char final_path[256];
    robot_chassis_backup_emit_t emit;
    robot_chassis_backup_result_t result;
} robot_chassis_backup_controller_t;

void robot_chassis_backup_controller_init(
    robot_chassis_backup_controller_t *controller,
    robot_chassis_backup_emit_t emit,
    robot_chassis_backup_result_t result);
int robot_chassis_backup_controller_start(
    robot_chassis_backup_controller_t *controller, const char *map_name,
    const char *temporary_path, const char *final_path);
int robot_chassis_backup_controller_accept(
    robot_chassis_backup_controller_t *controller, const char *json,
    size_t length);
int robot_chassis_backup_controller_timeout(
    robot_chassis_backup_controller_t *controller);
void robot_chassis_backup_controller_abort(
    robot_chassis_backup_controller_t *controller, int result);

#endif
