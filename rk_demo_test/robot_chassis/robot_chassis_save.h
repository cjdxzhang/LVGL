#ifndef ROBOT_CHASSIS_SAVE_H
#define ROBOT_CHASSIS_SAVE_H

#include "robot_chassis_map.h"

#define ROBOT_CHASSIS_SAVE_CONFIRM_INTERVAL_MS 2000U
#define ROBOT_CHASSIS_SAVE_CONFIRM_MAX_ATTEMPTS 3U

typedef enum {
    ROBOT_CHASSIS_SAVE_ACTION_NONE = 0,
    ROBOT_CHASSIS_SAVE_ACTION_QUERY_LIST,
    ROBOT_CHASSIS_SAVE_ACTION_SUCCEEDED,
    ROBOT_CHASSIS_SAVE_ACTION_FAILED
} robot_chassis_save_action_t;

typedef struct {
    bool active;
    bool waiting_response;
    uint8_t query_attempts;
    uint64_t next_query_ms;
    char target_map[128];
} robot_chassis_save_confirmation_t;

void robot_chassis_save_confirmation_init(
    robot_chassis_save_confirmation_t *confirmation);
int robot_chassis_save_confirmation_start(
    robot_chassis_save_confirmation_t *confirmation, const char *map_name,
    uint64_t now_ms);
robot_chassis_save_action_t robot_chassis_save_confirmation_poll(
    robot_chassis_save_confirmation_t *confirmation, uint64_t now_ms);
robot_chassis_save_action_t robot_chassis_save_confirmation_accept_list(
    robot_chassis_save_confirmation_t *confirmation,
    const robot_chassis_map_list_t *map_list, uint64_t now_ms);
void robot_chassis_save_confirmation_cancel(
    robot_chassis_save_confirmation_t *confirmation);

#endif
