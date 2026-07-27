#ifndef ROBOT_STATION_NAVIGATION_H
#define ROBOT_STATION_NAVIGATION_H
#include "robot_station_store.h"
typedef int (*robot_station_submit_t)(robot_chassis_command_t *command, void *user_data);
int robot_station_navigate(robot_station_store_t *store, const char *name, robot_station_submit_t submit, void *user_data);
int robot_station_return_to_base(robot_station_store_t *store, robot_station_submit_t submit, void *user_data);
#endif
