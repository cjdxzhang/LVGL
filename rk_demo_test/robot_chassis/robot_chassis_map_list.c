#include "robot_chassis_map.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

#define ROBOT_CHASSIS_MAP_LIST_MAX_BYTES (64U * 1024U)
#define ROBOT_CHASSIS_MAP_LIST_MAX_COUNT 512U

static int release_with_error(cJSON *root, robot_chassis_map_list_t *map_list,
                              int error)
{
    cJSON_Delete(root);
    robot_chassis_map_list_release(map_list);
    return error;
}

int robot_chassis_map_parse_list(const char *json, size_t length,
                                 robot_chassis_map_list_t *map_list)
{
    const char *parse_end = NULL;
    const char *inner_end = NULL;
    cJSON *array;
    cJSON *root;
    const cJSON *t;
    const cJSON *p;
    const cJSON *er;
    const cJSON *data;
    size_t count = 0U;
    size_t index = 0U;

    if (json == NULL || length == 0U || map_list == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    memset(map_list, 0, sizeof(*map_list));
    root = cJSON_ParseWithLengthOpts(json, length, &parse_end, 0);
    if (root == NULL) {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (parse_end < json + length && isspace((unsigned char)*parse_end)) {
        parse_end++;
    }
    t = cJSON_GetObjectItemCaseSensitive(root, "t");
    p = cJSON_GetObjectItemCaseSensitive(root, "p");
    er = cJSON_IsObject(p) ? cJSON_GetObjectItemCaseSensitive(p, "er") : NULL;
    data = cJSON_IsObject(p) ?
        cJSON_GetObjectItemCaseSensitive(p, "data") : NULL;
    if (parse_end != json + length || !cJSON_IsObject(root) ||
        !cJSON_IsNumber(t) || t->valuedouble != 58.0 ||
        !cJSON_IsNumber(er) || er->valuedouble != 0.0 ||
        !cJSON_IsString(data) || data->valuestring == NULL) {
        return release_with_error(root, map_list,
                                  ROBOT_CHASSIS_ERR_FORMAT);
    }
    if (strlen(data->valuestring) > ROBOT_CHASSIS_MAP_LIST_MAX_BYTES) {
        return release_with_error(root, map_list, ROBOT_CHASSIS_ERR_LIMIT);
    }

    array = cJSON_ParseWithOpts(data->valuestring, &inner_end, 0);
    if (array == NULL) {
        return release_with_error(root, map_list, ROBOT_CHASSIS_ERR_JSON);
    }
    while (*inner_end != '\0' &&
           isspace((unsigned char)*inner_end)) {
        inner_end++;
    }
    if (*inner_end != '\0' || !cJSON_IsArray(array)) {
        cJSON_Delete(array);
        return release_with_error(root, map_list,
                                  ROBOT_CHASSIS_ERR_FORMAT);
    }
    count = (size_t)cJSON_GetArraySize(array);
    if (count > ROBOT_CHASSIS_MAP_LIST_MAX_COUNT) {
        cJSON_Delete(array);
        return release_with_error(root, map_list, ROBOT_CHASSIS_ERR_LIMIT);
    }
    if (count > 0U) {
        map_list->names = calloc(count, sizeof(*map_list->names));
        if (map_list->names == NULL) {
            cJSON_Delete(array);
            return release_with_error(root, map_list,
                                      ROBOT_CHASSIS_ERR_NO_MEMORY);
        }
    }
    map_list->count = count;
    for (index = 0U; index < count; ++index) {
        const cJSON *name = cJSON_GetArrayItem(array, (int)index);
        size_t name_length;

        if (!cJSON_IsString(name) || name->valuestring == NULL ||
            name->valuestring[0] == '\0') {
            cJSON_Delete(array);
            return release_with_error(root, map_list,
                                      ROBOT_CHASSIS_ERR_FORMAT);
        }
        name_length = strlen(name->valuestring);
        if (name_length > ROBOT_CHASSIS_MAP_LIST_MAX_BYTES) {
            cJSON_Delete(array);
            return release_with_error(root, map_list,
                                      ROBOT_CHASSIS_ERR_LIMIT);
        }
        map_list->names[index] = malloc(name_length + 1U);
        if (map_list->names[index] == NULL) {
            cJSON_Delete(array);
            return release_with_error(root, map_list,
                                      ROBOT_CHASSIS_ERR_NO_MEMORY);
        }
        memcpy(map_list->names[index], name->valuestring, name_length + 1U);
    }
    cJSON_Delete(array);
    cJSON_Delete(root);
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_map_list_release(robot_chassis_map_list_t *map_list)
{
    size_t index;

    if (map_list == NULL) {
        return;
    }
    for (index = 0U; index < map_list->count; ++index) {
        free(map_list->names[index]);
    }
    free(map_list->names);
    memset(map_list, 0, sizeof(*map_list));
}
