#ifndef ROBOT_CHASSIS_CONTROL_H
#define ROBOT_CHASSIS_CONTROL_H

#include "robot_chassis_types.h"

void robot_chassis_command_release(robot_chassis_command_t *command);
int robot_chassis_build_heartbeat(robot_chassis_command_t *command);
int robot_chassis_build_reboot(robot_chassis_command_t *command);
int robot_chassis_build_status_query(robot_chassis_command_t *command);
int robot_chassis_build_manual_move(double angle, double speed, robot_chassis_command_t *command);
int robot_chassis_build_goto(double x, double y, double z, double tolerance,
                             robot_chassis_command_t *command);
int robot_chassis_build_start_charge(double x, double y, double z,
                                     robot_chassis_command_t *command);
int robot_chassis_build_cancel_charge(robot_chassis_command_t *command);
int robot_chassis_build_auto_explore(bool enabled, robot_chassis_command_t *command);
int robot_chassis_build_lock_map(const char *map_name, robot_chassis_command_t *command);
int robot_chassis_build_init_pose(double x, double y, double z, robot_chassis_command_t *command);
int robot_chassis_build_soft_stop(bool stop, robot_chassis_command_t *command);
int robot_chassis_build_start_mapping(robot_chassis_command_t *command);
int robot_chassis_build_close_mapping(robot_chassis_command_t *command);
int robot_chassis_build_save_map(const char *name, const char *alias,
                                 robot_chassis_command_t *command);
int robot_chassis_build_realtime_map_query(robot_chassis_command_t *command);
int robot_chassis_build_map_list_query(robot_chassis_command_t *command);
int robot_chassis_build_clear_navigation(robot_chassis_command_t *command);
int robot_chassis_build_factory_reset(robot_chassis_command_t *command);
int robot_chassis_build_set_time(const char *time_text, robot_chassis_command_t *command);
int robot_chassis_build_map_backup_begin(const char *map_name, robot_chassis_command_t *command);
int robot_chassis_build_map_backup_next(robot_chassis_command_t *command);
int robot_chassis_build_upgrade_begin(const char *file_name, uint64_t file_size,
                                      uint32_t step_total,
                                      robot_chassis_command_t *command);
int robot_chassis_build_upgrade_chunk(const char *file_data, const char *file_name,
                                      const char *md5, uint32_t step,
                                      robot_chassis_command_t *command);
int robot_chassis_build_upgrade_finish(const char *file_name, const char *md5,
                                       robot_chassis_command_t *command);
int robot_chassis_build_upgrade_extract(const char *file_name,
                                        robot_chassis_command_t *command);
int robot_chassis_build_wheel_enable(bool enabled, robot_chassis_command_t *command);

#endif

