#ifndef ROBOT_CHASSIS_STATUS_H
#define ROBOT_CHASSIS_STATUS_H

#include "robot_chassis_types.h"

typedef enum {
    ROBOT_CHASSIS_STATUS_VALID_AGV_STOP = UINT64_C(1) << 0,
    ROBOT_CHASSIS_STATUS_VALID_BATTERY_DEV = UINT64_C(1) << 1,
    ROBOT_CHASSIS_STATUS_VALID_IN_BUILD_MAP = UINT64_C(1) << 2,
    ROBOT_CHASSIS_STATUS_VALID_IN_NAV_MAP = UINT64_C(1) << 3,
    ROBOT_CHASSIS_STATUS_VALID_EMERGENCY_STOP = UINT64_C(1) << 4,
    ROBOT_CHASSIS_STATUS_VALID_SOFT_STOP = UINT64_C(1) << 5,
    ROBOT_CHASSIS_STATUS_VALID_NAV_BUSY = UINT64_C(1) << 6,
    ROBOT_CHASSIS_STATUS_VALID_BATTERY_PERCENT = UINT64_C(1) << 7,
    ROBOT_CHASSIS_STATUS_VALID_POWER = UINT64_C(1) << 8,
    ROBOT_CHASSIS_STATUS_VALID_POSE = UINT64_C(1) << 9,
    ROBOT_CHASSIS_STATUS_VALID_MAP_NAME = UINT64_C(1) << 10,
    ROBOT_CHASSIS_STATUS_VALID_LOCK_MAP = UINT64_C(1) << 11,
    ROBOT_CHASSIS_STATUS_VALID_BATTERY_FAILURE = UINT64_C(1) << 12,
    ROBOT_CHASSIS_STATUS_VALID_ERROR_CODE_1 = UINT64_C(1) << 13,
    ROBOT_CHASSIS_STATUS_VALID_ERROR_CODE_2 = UINT64_C(1) << 14,
    ROBOT_CHASSIS_STATUS_VALID_ERROR_CODE_3 = UINT64_C(1) << 15,
    ROBOT_CHASSIS_STATUS_VALID_POWER_STATUS = UINT64_C(1) << 16,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_SOC = UINT64_C(1) << 17,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_SOH = UINT64_C(1) << 18,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_CAPACITY = UINT64_C(1) << 19,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_CHARGE_TIME = UINT64_C(1) << 20,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_CURRENT = UINT64_C(1) << 21,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_MAX_TEMPERATURE = UINT64_C(1) << 22,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_MIN_TEMPERATURE = UINT64_C(1) << 23,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_SWITCH_STATUS = UINT64_C(1) << 24,
    ROBOT_CHASSIS_STATUS_VALID_POWER_BOARD_VOLTAGE = UINT64_C(1) << 25
} robot_chassis_status_valid_field_t;

typedef struct {
    uint64_t valid_fields;
    bool agv_stop;
    int battery_dev;
    bool in_build_map;
    bool in_nav_map;
    int emergency_stop;
    int soft_stop;
    int navigation_server_busy;
    double battery_percent;
    double power;
    robot_chassis_pose_t pose;
    char map_name[128];
    char lock_map[128];
    int battery_failure;
    int error_code_1;
    int error_code_2;
    int error_code_3;
    int power_status;
    double power_board_soc;
    double power_board_soh;
    double power_board_capacity;
    double power_board_charge_time;
    double power_board_current;
    double power_board_max_temperature;
    double power_board_min_temperature;
    int power_board_switch_status;
    double power_board_voltage;
} robot_chassis_status_t;

int robot_chassis_status_parse(const char *json, size_t length, robot_chassis_status_t *status);

#endif

