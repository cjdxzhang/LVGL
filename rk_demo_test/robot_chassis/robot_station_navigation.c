#include "robot_station_navigation.h"
#include "robot_chassis_control.h"
#include "robot_chassis_service.h"
#define ROBOT_STATION_GOTO_TOLERANCE 0.1

// 提交命令到服务
static int submit_to_service(robot_chassis_command_t *command, void *user_data)
{
    (void)user_data;
    return robot_chassis_service_submit(command);
}
static int submit_command(robot_chassis_command_t *command, robot_station_submit_t submit,
                          void *user_data)
{
    int result;
    if (submit == NULL)
    {
        submit = submit_to_service;
    }
    result = submit(command, user_data);
    robot_chassis_command_release(command);
    return result;
}
int robot_station_navigate(robot_station_store_t *store, const char *name,
                           robot_station_submit_t submit, void *user_data)
{
    return robot_station_navigate_with_target(store, name, NULL, submit,
            user_data);
}

// 普通站点导航
int robot_station_navigate_with_target(robot_station_store_t *store,
                                       const char *name,
                                       robot_station_t *target,
                                       robot_station_submit_t submit,
                                       void *user_data)
{
    robot_station_t station;
    robot_chassis_command_t command = {0};
    int result = robot_station_store_find_by_name(store, name, &station);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    if (station.is_charging_base)
    {
        return ROBOT_CHASSIS_ERR_CONFLICT;
    }
    if (target != NULL)
    {
        *target = station;
    }
    result = robot_chassis_build_goto(station.x, station.y, station.z, ROBOT_STATION_GOTO_TOLERANCE,
                                      &command);
    return result == ROBOT_CHASSIS_OK ? submit_command(&command, submit, user_data) : result;
}

// 充电基站导航
int robot_station_return_to_base(robot_station_store_t *store, robot_station_submit_t submit,
                                 void *user_data)
{
    return robot_station_return_to_base_with_target(store, NULL, submit,
            user_data);
}

int robot_station_return_to_base_with_target(robot_station_store_t *store,
        robot_station_t *target,
        robot_station_submit_t submit,
        void *user_data)
{
    robot_station_t station;
    robot_chassis_command_t command = {0};
    int result = robot_station_store_get_charging_base(store, &station);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    if (target != NULL)
    {
        *target = station;
    }
    result = robot_chassis_build_start_charge(station.x, station.y, station.z, &command);
    return result == ROBOT_CHASSIS_OK ? submit_command(&command, submit, user_data) : result;
}
