#ifndef ROBOT_CHASSIS_HEARTBEAT_H
#define ROBOT_CHASSIS_HEARTBEAT_H

#include "robot_chassis_types.h"

typedef enum {
    ROBOT_CHASSIS_ACTION_NONE = 0U,
    ROBOT_CHASSIS_ACTION_SEND_HEARTBEAT = 1U << 0,
    ROBOT_CHASSIS_ACTION_SEND_INITIAL_STATUS = 1U << 1,
    ROBOT_CHASSIS_ACTION_HEARTBEAT_LOST = 1U << 2,
    ROBOT_CHASSIS_ACTION_RECONNECT = 1U << 3,
    ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED = 1U << 4
} robot_chassis_service_action_t;

typedef struct {
    robot_chassis_connection_state_t connection;
    bool awaiting_response;
    uint64_t next_heartbeat_ms;
    uint64_t next_reconnect_ms;
} robot_chassis_heartbeat_t;

void robot_chassis_heartbeat_init(robot_chassis_heartbeat_t *heartbeat);
uint32_t robot_chassis_heartbeat_on_connected(robot_chassis_heartbeat_t *heartbeat,
                                              uint64_t now_ms);
uint32_t robot_chassis_heartbeat_on_valid_response(
    robot_chassis_heartbeat_t *heartbeat);
uint32_t robot_chassis_heartbeat_on_disconnected(robot_chassis_heartbeat_t *heartbeat,
                                                 uint64_t now_ms);
uint32_t robot_chassis_heartbeat_poll(robot_chassis_heartbeat_t *heartbeat,
                                     uint64_t now_ms);

#endif

