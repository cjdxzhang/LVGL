#include "robot_chassis_service_internal.h"

#include <stdio.h>

#include "robot_chassis_parser.h"
#include "robot_chassis_log.h"

static void complete_pending(const robot_chassis_message_info_t *info,
                             bool report_unmatched)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    int result;

    if (!info->has_t)
    {
        return;
    }
    result = robot_chassis_transaction_complete_t(&service->transaction,
             info->t);
    if (result != ROBOT_CHASSIS_OK)
    {
        if (report_unmatched)
        {
            fprintf(stderr, "底盘收到无待处理项的异步响应 t=%d\n",
                    info->t);
        }
        return;
    }
    if (service->config.response_callback != NULL)
    {
        service->config.response_callback(
            info->t, info->has_protocol_error ? info->protocol_error : 0);
    }
}

static void handle_valid_heartbeat(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    uint32_t actions;

    pthread_mutex_lock(&service->state_mutex);
    actions = robot_chassis_heartbeat_on_valid_response(&service->heartbeat);
    pthread_mutex_unlock(&service->state_mutex);
    if ((actions & ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED) != 0U)
    {
        robot_chassis_service_notify_connection();
    }
}

static void handle_status(const char *json, size_t length,
                          const robot_chassis_message_info_t *info)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_status_t status = {0};

    if (robot_chassis_status_parse(json, length, &status) ==
            ROBOT_CHASSIS_OK)
    {
        if (service->config.status_callback != NULL)
        {
            service->config.status_callback(&status);
        }
    }
    /* t=42 允许底盘主动推送，没有待处理请求时也必须完成解析。 */
    complete_pending(info, false);
}

int robot_chassis_service_handle_frame(const char *json, size_t length)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_message_info_t info;
    int result;

    robot_chassis_log_receive(json, length);
    result = robot_chassis_parser_inspect(json, length, &info);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }

    switch (info.route)
    {
    case ROBOT_CHASSIS_ROUTE_HEARTBEAT:
        if (info.heartbeat_valid)
        {
            handle_valid_heartbeat();
        }
        break;
    case ROBOT_CHASSIS_ROUTE_STATUS:
        handle_status(json, length, &info);
        break;
    case ROBOT_CHASSIS_ROUTE_REALTIME_MAP:
        result = robot_chassis_service_handle_realtime_map(json, length);
        if (result != ROBOT_CHASSIS_OK)
        {
            fprintf(stderr, "底盘实时地图解析失败，长度=%zu，错误=%d\n",
                    length, result);
        }
        complete_pending(&info, true);
        break;
    case ROBOT_CHASSIS_ROUTE_MAP_LIST:
        result = robot_chassis_service_handle_map_list(json, length);
        if (result != ROBOT_CHASSIS_OK)
        {
            fprintf(stderr, "底盘地图列表解析失败，长度=%zu，错误=%d\n",
                    length, result);
        }
        complete_pending(&info, true);
        break;
    case ROBOT_CHASSIS_ROUTE_MAP_BACKUP:
        complete_pending(&info, true);
        result = robot_chassis_service_handle_backup_response(json, length);
        if (result != ROBOT_CHASSIS_OK &&
                result != ROBOT_CHASSIS_ERR_WOULD_BLOCK)
        {
            fprintf(stderr, "底盘地图备份响应处理失败，错误=%d\n", result);
        }
        break;
    case ROBOT_CHASSIS_ROUTE_UPGRADE:
        complete_pending(&info, true);
        result = robot_chassis_service_handle_upgrade_response(json, length);
        if (result != ROBOT_CHASSIS_OK &&
                result != ROBOT_CHASSIS_ERR_WOULD_BLOCK)
        {
            fprintf(stderr, "底盘升级响应处理失败，错误=%d\n", result);
        }
        break;
    case ROBOT_CHASSIS_ROUTE_GENERIC:
        complete_pending(&info, true);
        break;
    case ROBOT_CHASSIS_ROUTE_REBOOT:
        if (service->config.response_callback != NULL)
        {
            service->config.response_callback(-1, 0);
        }
        break;
    case ROBOT_CHASSIS_ROUTE_UNKNOWN:
    default:
        fprintf(stderr, "底盘收到未知响应，长度=%zu\n", length);
        break;
    }
    return ROBOT_CHASSIS_OK;
}
