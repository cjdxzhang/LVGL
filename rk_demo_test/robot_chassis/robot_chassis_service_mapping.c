#include "robot_chassis_service_internal.h"

#include "robot_chassis_control.h"

int robot_chassis_service_set_mapping_active(bool active)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    const uint64_t now_ms = robot_chassis_service_now_ms();

    if (!service->initialized) {
        return ROBOT_CHASSIS_ERR_STATE;
    }

    pthread_mutex_lock(&service->state_mutex);
    if (active) {
        robot_chassis_mapping_poll_start(&service->mapping_poll, now_ms);
    } else {
        robot_chassis_mapping_poll_stop(&service->mapping_poll);
    }
    pthread_mutex_unlock(&service->state_mutex);
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_service_poll_mapping(uint64_t now_ms)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_command_t command;
    bool due;
    int result;

    pthread_mutex_lock(&service->state_mutex);
    due = robot_chassis_mapping_poll_due(&service->mapping_poll, now_ms);
    pthread_mutex_unlock(&service->state_mutex);
    if (!due) {
        return ROBOT_CHASSIS_OK;
    }

    result = robot_chassis_build_realtime_map_query(&command);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    result = robot_chassis_service_submit(&command);
    robot_chassis_command_release(&command);

    /* 上一个大地图响应尚未到达时跳过本周期，保持同 t 串行。 */
    return result == ROBOT_CHASSIS_ERR_CONFLICT ? ROBOT_CHASSIS_OK : result;
}
