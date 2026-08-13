#include "mode_navigation_flow.h"

#include <string.h>

// 启动导航流程
int mode_navigation_start(const char *name,
                          mode_navigation_resolve_t resolve,
                          mode_navigation_standby_t standby,
                          mode_navigation_submit_t submit,
                          void *user_data,
                          robot_station_t *target)
{
    int result;

    if (name == NULL || resolve == NULL || standby == NULL || submit == NULL ||
            target == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = resolve(name, target, user_data);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    standby(user_data);
    return submit(target, user_data);
}

// 配置导航flow，指定目标名称和上次报告序列号
void auto_water_navigation_flow_arm(auto_water_navigation_flow_t *flow,
                                    const char *target_name,
                                    uint32_t report_sequence)
{
    if (flow == NULL)
    {
        return;
    }
    memset(flow, 0, sizeof(*flow));
    if (target_name != NULL)
    {
        strncpy(flow->target_name, target_name,
                sizeof(flow->target_name) - 1U);
    }
    flow->active = true;
    flow->last_report_sequence = report_sequence;
}

void auto_water_navigation_flow_cancel(auto_water_navigation_flow_t *flow)
{
    if (flow != NULL)
    {
        memset(flow, 0, sizeof(*flow));
    }
}

// 观察基站状态变化，判断是否可以自动上水导航
bool auto_water_navigation_flow_observe(auto_water_navigation_flow_t *flow,
                                        uint8_t base_status,
                                        uint32_t report_sequence)
{
    bool completed;

    if (flow == NULL || !flow->active || flow->completion_handled)
    {
        return false;
    }
    if (report_sequence == flow->last_report_sequence)
    {
        return false;
    }
    flow->last_report_sequence = report_sequence;
    completed = flow->previous_base_status != 5U && base_status == 5U;
    flow->previous_base_status = base_status;
    if (completed)
    {
        flow->completion_handled = true;
    }
    return completed;
}

const char *auto_water_navigation_flow_target(
    const auto_water_navigation_flow_t *flow)
{
    return flow == NULL ? "" : flow->target_name;
}

void mode_navigation_confirm_self_clean_stop(bool confirmed_self_clean,
        mode_navigation_standby_t standby,
        void *user_data)
{
    if (confirmed_self_clean && standby != NULL)
    {
        standby(user_data);
    }
}
