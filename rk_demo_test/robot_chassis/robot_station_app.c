#include "robot_station_app.h"

#include <ctype.h>
#include <math.h>
#include <string.h>

#include "cJSON.h"

// 解析站点信息保存到station中
int robot_station_app_handle_json(robot_station_store_t *store,
                                  const char *json, size_t length)
{
    const char *parse_end = NULL;
    cJSON *root;
    const cJSON *name;
    const cJSON *x;
    const cJSON *y;
    const cJSON *z;
    const cJSON *is_base;
    robot_station_t station = {0};
    size_t name_length;
    int result;

    if (store == NULL || json == NULL || length == 0U)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    root = cJSON_ParseWithLengthOpts(json, length, &parse_end, 0);
    if (root == NULL)
    {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (parse_end < json + length && isspace((unsigned char)*parse_end))
    {
        ++parse_end;
    }
    name = cJSON_GetObjectItemCaseSensitive(root, "name");
    x = cJSON_GetObjectItemCaseSensitive(root, "x");
    y = cJSON_GetObjectItemCaseSensitive(root, "y");
    z = cJSON_GetObjectItemCaseSensitive(root, "z");
    is_base = cJSON_GetObjectItemCaseSensitive(root, "is_charging_base");
    if (parse_end != json + length || !cJSON_IsObject(root) ||
            !cJSON_IsString(name) || name->valuestring == NULL ||
            !cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z) ||
            !cJSON_IsBool(is_base) || !isfinite(x->valuedouble) ||
            !isfinite(y->valuedouble) || !isfinite(z->valuedouble))
    {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    name_length = strlen(name->valuestring);
    if (name_length == 0U || name_length >= sizeof(station.name))
    {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    memcpy(station.name, name->valuestring, name_length + 1U);
    station.x = x->valuedouble;
    station.y = y->valuedouble;
    station.z = z->valuedouble;
    station.is_charging_base = cJSON_IsTrue(is_base);
    result = robot_station_store_update(store, &station);
    cJSON_Delete(root);
    return result;
}
