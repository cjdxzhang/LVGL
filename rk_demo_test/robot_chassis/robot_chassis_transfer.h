#ifndef ROBOT_CHASSIS_TRANSFER_H
#define ROBOT_CHASSIS_TRANSFER_H

#include <stdio.h>

#include "robot_chassis_types.h"

typedef enum
{
    ROBOT_CHASSIS_UPGRADE_IDLE = 0,
    ROBOT_CHASSIS_UPGRADE_WAIT_BEGIN,
    ROBOT_CHASSIS_UPGRADE_WAIT_CHUNK,
    ROBOT_CHASSIS_UPGRADE_WAIT_FINISH,
    ROBOT_CHASSIS_UPGRADE_WAIT_EXTRACT,
    ROBOT_CHASSIS_UPGRADE_SUCCEEDED,
    ROBOT_CHASSIS_UPGRADE_FAILED
} robot_chassis_upgrade_stage_t;

typedef struct
{
    robot_chassis_transfer_kind_t kind;
    FILE *file;
    char temporary_path[256];
    char final_path[256];
    uint32_t expected_step;
    uint32_t step_total;
    uint64_t expected_size;
    uint64_t received_size;
    bool restart_required;
} robot_chassis_backup_transfer_t;

typedef struct
{
    robot_chassis_transfer_kind_t kind;
    FILE *file;
    char file_path[256];
    char file_name[128];
    char whole_file_md5[33];
    uint64_t file_size;
    uint32_t current_step;
    uint32_t step_total;
    uint8_t retries;
    robot_chassis_upgrade_stage_t stage;
} robot_chassis_upgrade_transfer_t;

int robot_chassis_hex_decode(const char *hex, size_t hex_length, uint8_t *output,
                             size_t output_capacity, size_t *output_length);
int robot_chassis_backup_begin(robot_chassis_backup_transfer_t *transfer,
                               const char *temporary_path, const char *final_path);
int robot_chassis_backup_accept_response(robot_chassis_backup_transfer_t *transfer,
        const char *json, size_t length);
void robot_chassis_backup_abort(robot_chassis_backup_transfer_t *transfer);
int robot_chassis_upgrade_begin(robot_chassis_upgrade_transfer_t *transfer,
                                const char *file_path);
int robot_chassis_upgrade_build_next(robot_chassis_upgrade_transfer_t *transfer,
                                     robot_chassis_command_t *command);
int robot_chassis_upgrade_accept_response(robot_chassis_upgrade_transfer_t *transfer,
        const char *json, size_t length);
int robot_chassis_upgrade_timeout(robot_chassis_upgrade_transfer_t *transfer);
void robot_chassis_upgrade_abort(robot_chassis_upgrade_transfer_t *transfer);

#endif
