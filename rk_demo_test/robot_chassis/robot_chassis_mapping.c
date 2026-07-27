#include "robot_chassis_mapping.h"

#include <string.h>

void robot_chassis_mapping_poll_init(robot_chassis_mapping_poll_t *poll)
{
    if (poll != NULL) {
        memset(poll, 0, sizeof(*poll));
    }
}

void robot_chassis_mapping_poll_start(robot_chassis_mapping_poll_t *poll,
                                      uint64_t now_ms)
{
    if (poll == NULL) {
        return;
    }
    poll->active = true;
    poll->next_request_ms = now_ms + ROBOT_CHASSIS_MAP_POLL_INTERVAL_MS;
}

void robot_chassis_mapping_poll_stop(robot_chassis_mapping_poll_t *poll)
{
    if (poll == NULL) {
        return;
    }
    poll->active = false;
    poll->next_request_ms = 0U;
}

bool robot_chassis_mapping_poll_due(robot_chassis_mapping_poll_t *poll,
                                    uint64_t now_ms)
{
    if (poll == NULL || !poll->active || now_ms < poll->next_request_ms) {
        return false;
    }
    do {
        poll->next_request_ms += ROBOT_CHASSIS_MAP_POLL_INTERVAL_MS;
    } while (poll->next_request_ms <= now_ms);
    return true;
}

