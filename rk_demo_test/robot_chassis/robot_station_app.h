#ifndef ROBOT_STATION_APP_H
#define ROBOT_STATION_APP_H

#include "robot_station_store.h"

int robot_station_app_handle_json(robot_station_store_t *store,
                                  const char *json, size_t length);

#endif
