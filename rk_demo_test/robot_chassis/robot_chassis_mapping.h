#ifndef ROBOT_CHASSIS_MAPPING_H
#define ROBOT_CHASSIS_MAPPING_H

#include "robot_chassis_types.h"

typedef struct {
    bool active;
    uint64_t next_request_ms;
} robot_chassis_mapping_poll_t;

void robot_chassis_mapping_poll_init(robot_chassis_mapping_poll_t *poll);
void robot_chassis_mapping_poll_start(robot_chassis_mapping_poll_t *poll,
                                      uint64_t now_ms);
void robot_chassis_mapping_poll_stop(robot_chassis_mapping_poll_t *poll);
bool robot_chassis_mapping_poll_due(robot_chassis_mapping_poll_t *poll,
                                    uint64_t now_ms);

#endif

