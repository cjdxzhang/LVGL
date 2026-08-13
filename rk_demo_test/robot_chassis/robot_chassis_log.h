#ifndef ROBOT_CHASSIS_LOG_H
#define ROBOT_CHASSIS_LOG_H

#include <stddef.h>

void robot_chassis_log_send(const char *json, size_t length);
void robot_chassis_log_send_failed(const char *json, size_t length, int error);
void robot_chassis_log_receive(const char *json, size_t length);

#endif
