#ifndef ROBOT_CHASSIS_PARSER_H
#define ROBOT_CHASSIS_PARSER_H

#include "robot_chassis_types.h"

typedef struct {
    robot_chassis_route_t route;
    int t;
    bool has_t;
    bool heartbeat_valid;
    int protocol_error;
    bool has_protocol_error;
} robot_chassis_message_info_t;

int robot_chassis_parser_inspect(const char *json, size_t length,
                                 robot_chassis_message_info_t *info);

#endif

