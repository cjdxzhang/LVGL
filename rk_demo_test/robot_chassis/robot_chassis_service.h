#ifndef ROBOT_CHASSIS_SERVICE_H
#define ROBOT_CHASSIS_SERVICE_H

#include "robot_chassis_map.h"
#include "robot_chassis_status.h"
#include "robot_chassis_types.h"

typedef void (*robot_chassis_connection_callback_t)(
    const robot_chassis_connection_state_t *state);
typedef void (*robot_chassis_status_callback_t)(const robot_chassis_status_t *status);
typedef void (*robot_chassis_response_callback_t)(int t, int result);
typedef void (*robot_chassis_grid_callback_t)(const robot_chassis_grid_t *grid);
typedef void (*robot_chassis_map_list_callback_t)(const robot_chassis_map_list_t *map_list);
typedef void (*robot_chassis_backup_callback_t)(int result, const char *final_path);
typedef void (*robot_chassis_upgrade_callback_t)(int result);

typedef struct
{
    const char *host;
    uint16_t port;
    robot_chassis_connection_callback_t connection_callback;
    robot_chassis_status_callback_t status_callback;
    robot_chassis_response_callback_t response_callback;
    robot_chassis_grid_callback_t grid_callback;
    robot_chassis_map_list_callback_t map_list_callback;
    robot_chassis_backup_callback_t backup_callback;
    robot_chassis_upgrade_callback_t upgrade_callback;
    size_t max_map_cells;
} robot_chassis_service_config_t;

int robot_chassis_service_init(const robot_chassis_service_config_t *config);
int robot_chassis_service_start(void);
void robot_chassis_service_stop(void);
void robot_chassis_service_deinit(void);
int robot_chassis_service_submit(robot_chassis_command_t *command);
int robot_chassis_service_get_connection_state(robot_chassis_connection_state_t *state);
int robot_chassis_service_set_mapping_active(bool active);
int robot_chassis_service_backup_map(const char *map_name, const char *temporary_path,
                                     const char *final_path);
int robot_chassis_service_upgrade(const char *file_path);

#endif
