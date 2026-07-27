#include "robot_chassis_heartbeat.h"

#include <string.h>

void robot_chassis_heartbeat_init(robot_chassis_heartbeat_t *heartbeat)
{
    if (heartbeat != NULL) {
        memset(heartbeat, 0, sizeof(*heartbeat));
    }
}

uint32_t robot_chassis_heartbeat_on_connected(robot_chassis_heartbeat_t *heartbeat,
                                              uint64_t now_ms)
{
    if (heartbeat == NULL) {
        return ROBOT_CHASSIS_ACTION_NONE;
    }

    heartbeat->connection.tcp_connected = true;
    heartbeat->connection.heartbeat_alive = false;
    heartbeat->connection.heartbeat_misses = 0U;
    heartbeat->awaiting_response = true;
    heartbeat->next_heartbeat_ms =
        now_ms + ROBOT_CHASSIS_HEARTBEAT_INTERVAL_MS;
    heartbeat->next_reconnect_ms = 0U;
    return ROBOT_CHASSIS_ACTION_SEND_HEARTBEAT |
           ROBOT_CHASSIS_ACTION_SEND_INITIAL_STATUS |
           ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED;
}

uint32_t robot_chassis_heartbeat_on_valid_response(
    robot_chassis_heartbeat_t *heartbeat)
{
    bool changed;

    if (heartbeat == NULL || !heartbeat->connection.tcp_connected) {
        return ROBOT_CHASSIS_ACTION_NONE;
    }

    changed = !heartbeat->connection.heartbeat_alive ||
              heartbeat->connection.heartbeat_misses != 0U;
    heartbeat->connection.heartbeat_alive = true;
    heartbeat->connection.heartbeat_misses = 0U;
    heartbeat->awaiting_response = false;
    return changed ? ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED
                   : ROBOT_CHASSIS_ACTION_NONE;
}

uint32_t robot_chassis_heartbeat_on_disconnected(robot_chassis_heartbeat_t *heartbeat,
                                                 uint64_t now_ms)
{
    bool changed;

    if (heartbeat == NULL) {
        return ROBOT_CHASSIS_ACTION_NONE;
    }

    changed = heartbeat->connection.tcp_connected ||
              heartbeat->connection.heartbeat_alive;
    heartbeat->connection.tcp_connected = false;
    heartbeat->connection.heartbeat_alive = false;
    heartbeat->awaiting_response = false;
    heartbeat->next_heartbeat_ms = 0U;
    heartbeat->next_reconnect_ms =
        now_ms + ROBOT_CHASSIS_RECONNECT_INTERVAL_MS;
    return changed ? ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED
                   : ROBOT_CHASSIS_ACTION_NONE;
}

uint32_t robot_chassis_heartbeat_poll(robot_chassis_heartbeat_t *heartbeat,
                                     uint64_t now_ms)
{
    uint32_t actions = ROBOT_CHASSIS_ACTION_NONE;

    if (heartbeat == NULL) {
        return actions;
    }

    if (!heartbeat->connection.tcp_connected) {
        if (now_ms >= heartbeat->next_reconnect_ms) {
            heartbeat->next_reconnect_ms =
                now_ms + ROBOT_CHASSIS_RECONNECT_INTERVAL_MS;
            actions |= ROBOT_CHASSIS_ACTION_RECONNECT;
        }
        return actions;
    }

    while (now_ms >= heartbeat->next_heartbeat_ms) {
        if (heartbeat->awaiting_response &&
            heartbeat->connection.heartbeat_misses < UINT8_MAX) {
            heartbeat->connection.heartbeat_misses++;
        }
        if (heartbeat->connection.heartbeat_misses >=
            ROBOT_CHASSIS_HEARTBEAT_MISS_LIMIT) {
            heartbeat->connection.tcp_connected = false;
            heartbeat->connection.heartbeat_alive = false;
            heartbeat->awaiting_response = false;
            heartbeat->next_heartbeat_ms = 0U;
            heartbeat->next_reconnect_ms =
                now_ms + ROBOT_CHASSIS_RECONNECT_INTERVAL_MS;
            actions &= ~ROBOT_CHASSIS_ACTION_SEND_HEARTBEAT;
            actions |= ROBOT_CHASSIS_ACTION_HEARTBEAT_LOST |
                       ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED;
            return actions;
        }

        heartbeat->awaiting_response = true;
        heartbeat->next_heartbeat_ms +=
            ROBOT_CHASSIS_HEARTBEAT_INTERVAL_MS;
        actions |= ROBOT_CHASSIS_ACTION_SEND_HEARTBEAT;
    }

    return actions;
}
