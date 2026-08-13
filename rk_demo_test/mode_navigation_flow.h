#ifndef MODE_NAVIGATION_FLOW_H
#define MODE_NAVIGATION_FLOW_H

#include <stdbool.h>
#include <stdint.h>

#include "robot_chassis/robot_station_store.h"

#define MODE_NAVIGATION_TARGET_NAME_LEN 64U

typedef int (*mode_navigation_resolve_t)(const char *name,
        robot_station_t *target,
        void *user_data);
typedef void (*mode_navigation_standby_t)(void *user_data);
typedef int (*mode_navigation_submit_t)(const robot_station_t *target,
                                        void *user_data);

typedef struct
{
    char target_name[MODE_NAVIGATION_TARGET_NAME_LEN];
    bool active;
    bool completion_handled;
    uint8_t previous_base_status;
    uint32_t last_report_sequence;
} auto_water_navigation_flow_t;

int mode_navigation_start(const char *name,
                          mode_navigation_resolve_t resolve,
                          mode_navigation_standby_t standby,
                          mode_navigation_submit_t submit,
                          void *user_data,
                          robot_station_t *target);
void auto_water_navigation_flow_arm(auto_water_navigation_flow_t *flow,
                                    const char *target_name,
                                    uint32_t report_sequence);
void auto_water_navigation_flow_cancel(auto_water_navigation_flow_t *flow);
bool auto_water_navigation_flow_observe(auto_water_navigation_flow_t *flow,
                                        uint8_t base_status,
                                        uint32_t report_sequence);
const char *auto_water_navigation_flow_target(
    const auto_water_navigation_flow_t *flow);
void mode_navigation_confirm_self_clean_stop(bool confirmed_self_clean,
        mode_navigation_standby_t standby,
        void *user_data);

#endif
