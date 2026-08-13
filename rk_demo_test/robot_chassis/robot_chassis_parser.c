#include "robot_chassis_parser.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>

#include "cJSON.h"

static bool number_to_int(const cJSON *item, int *value)
{
    if (!cJSON_IsNumber(item) || item->valuedouble < (double)INT_MIN ||
            item->valuedouble > (double)INT_MAX)
    {
        return false;
    }
    *value = (int)item->valuedouble;
    return item->valuedouble == (double) * value;
}

static robot_chassis_route_t route_for_t(int t)
{
    switch (t)
    {
    case ROBOT_CHASSIS_T_STATUS:
        return ROBOT_CHASSIS_ROUTE_STATUS;
    case ROBOT_CHASSIS_T_REALTIME_MAP:
        return ROBOT_CHASSIS_ROUTE_REALTIME_MAP;
    case ROBOT_CHASSIS_T_MAP_LIST:
        return ROBOT_CHASSIS_ROUTE_MAP_LIST;
    case ROBOT_CHASSIS_T_MAP_BACKUP_BEGIN:
    case ROBOT_CHASSIS_T_MAP_BACKUP_NEXT:
        return ROBOT_CHASSIS_ROUTE_MAP_BACKUP;
    case ROBOT_CHASSIS_T_UPGRADE_EXTRACT:
    case ROBOT_CHASSIS_T_UPGRADE_BEGIN:
    case ROBOT_CHASSIS_T_UPGRADE_CHUNK:
    case ROBOT_CHASSIS_T_UPGRADE_FINISH:
        return ROBOT_CHASSIS_ROUTE_UPGRADE;
    default:
        return ROBOT_CHASSIS_ROUTE_GENERIC;
    }
}

int robot_chassis_parser_inspect(const char *json, size_t length,
                                 robot_chassis_message_info_t *info)
{
    cJSON *root;
    const char *parse_end = NULL;
    const cJSON *cmd;
    const cJSON *t;
    const cJSON *p;
    const cJSON *er;

    if (json == NULL || length == 0U || info == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    memset(info, 0, sizeof(*info));

    root = cJSON_ParseWithLengthOpts(json, length, &parse_end, 0);
    if (root == NULL)
    {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (parse_end < json + length && isspace((unsigned char)*parse_end))
    {
        parse_end++;
    }
    if (parse_end != json + length || !cJSON_IsObject(root))
    {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }

    cmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    if (cmd != NULL)
    {
        if (!cJSON_IsString(cmd) || cmd->valuestring == NULL)
        {
            cJSON_Delete(root);
            return ROBOT_CHASSIS_ERR_FORMAT;
        }
        if (strcmp(cmd->valuestring, "heatbeat") == 0)
        {
            info->route = ROBOT_CHASSIS_ROUTE_HEARTBEAT;
            /* 底盘的 result 可能为 false，收到合法心跳响应即表示链路正常。 */
            info->heartbeat_valid = true;
        }
        else if (strcmp(cmd->valuestring, "reBoot") == 0)
        {
            info->route = ROBOT_CHASSIS_ROUTE_REBOOT;
        }
        else
        {
            info->route = ROBOT_CHASSIS_ROUTE_UNKNOWN;
        }
        cJSON_Delete(root);
        return ROBOT_CHASSIS_OK;
    }

    t = cJSON_GetObjectItemCaseSensitive(root, "t");
    if (t == NULL)
    {
        info->route = ROBOT_CHASSIS_ROUTE_UNKNOWN;
        cJSON_Delete(root);
        return ROBOT_CHASSIS_OK;
    }
    if (!number_to_int(t, &info->t))
    {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    info->has_t = true;
    info->route = route_for_t(info->t);

    p = cJSON_GetObjectItemCaseSensitive(root, "p");
    if (cJSON_IsObject(p))
    {
        er = cJSON_GetObjectItemCaseSensitive(p, "er");
        if (er != NULL)
        {
            if (!number_to_int(er, &info->protocol_error))
            {
                cJSON_Delete(root);
                return ROBOT_CHASSIS_ERR_FORMAT;
            }
            info->has_protocol_error = true;
        }
    }

    cJSON_Delete(root);
    return ROBOT_CHASSIS_OK;
}
