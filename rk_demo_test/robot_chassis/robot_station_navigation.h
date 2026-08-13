#ifndef ROBOT_STATION_NAVIGATION_H
#define ROBOT_STATION_NAVIGATION_H
#include "robot_station_store.h"
typedef int (*robot_station_submit_t)(robot_chassis_command_t *command);
int robot_station_resolve_navigation_target(robot_station_store_t *store,
        const char *name,
        robot_station_t *target);
int robot_station_submit_navigation_target(const robot_station_t *target,
        robot_station_submit_t submit);
int robot_station_navigate_with_target(robot_station_store_t *store,
                                       const char *name,
                                       robot_station_t *target,
                                       robot_station_submit_t submit);
int robot_station_return_to_base(robot_station_store_t *store,
                                 robot_station_t *target,
                                 robot_station_submit_t submit);
#endif
