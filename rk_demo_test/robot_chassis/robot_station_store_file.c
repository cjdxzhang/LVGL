#define _POSIX_C_SOURCE 200809L

#include "robot_station_store.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cJSON.h"

#define ROBOT_STATION_FILE_MAX_SIZE (1024U * 1024U)

static bool name_is_valid(const char *name)
{
    size_t index;

    if (name == NULL)
    {
        return false;
    }
    for (index = 0U; index < sizeof(((robot_station_t *)0)->name); ++index)
    {
        if (name[index] == '\0')
        {
            return index > 0U;
        }
    }
    return false;
}

// 验证站点信息是否有效
static int validate_snapshot(const robot_station_t *stations, size_t count)
{
    size_t index;
    size_t compared;
    size_t base_count = 0U;

    if (count > ROBOT_STATION_MAX_COUNT)
    {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    for (index = 0U; index < count; ++index)
    {
        if (!name_is_valid(stations[index].name) ||
                !isfinite(stations[index].x) || !isfinite(stations[index].y) ||
                !isfinite(stations[index].z))
        {
            return ROBOT_CHASSIS_ERR_FORMAT;
        }
        if (stations[index].is_charging_base)
        {
            ++base_count;
        }
        for (compared = 0U; compared < index; ++compared)
        {
            if (strcmp(stations[index].name, stations[compared].name) == 0)
            {
                return ROBOT_CHASSIS_ERR_CONFLICT;
            }
        }
    }
    return base_count <= 1U ? ROBOT_CHASSIS_OK : ROBOT_CHASSIS_ERR_CONFLICT;
}

// 解析站点信息保存到station中
static int parse_station(const cJSON *item, robot_station_t *station)
{
    const cJSON *name;
    const cJSON *x;
    const cJSON *y;
    const cJSON *z;
    const cJSON *is_base;
    size_t name_length;

    if (!cJSON_IsObject(item))
    {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    name = cJSON_GetObjectItemCaseSensitive(item, "name");
    x = cJSON_GetObjectItemCaseSensitive(item, "x");
    y = cJSON_GetObjectItemCaseSensitive(item, "y");
    z = cJSON_GetObjectItemCaseSensitive(item, "z");
    is_base = cJSON_GetObjectItemCaseSensitive(item, "is_charging_base");
    if (!cJSON_IsString(name) || name->valuestring == NULL ||
            !cJSON_IsNumber(x) || !cJSON_IsNumber(y) || !cJSON_IsNumber(z) ||
            !cJSON_IsBool(is_base) || !isfinite(x->valuedouble) ||
            !isfinite(y->valuedouble) || !isfinite(z->valuedouble))
    {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    name_length = strlen(name->valuestring);
    if (name_length == 0U || name_length >= sizeof(station->name))
    {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    memset(station, 0, sizeof(*station));
    memcpy(station->name, name->valuestring, name_length + 1U);
    station->x = x->valuedouble;
    station->y = y->valuedouble;
    station->z = z->valuedouble;
    station->is_charging_base = cJSON_IsTrue(is_base);
    return ROBOT_CHASSIS_OK;
}

static int read_file(const char *path, char **text, size_t *length)
{
    struct stat status;
    FILE *file;
    char *buffer;

    if (stat(path, &status) != 0)
    {
        return errno == ENOENT ? ROBOT_CHASSIS_ERR_WOULD_BLOCK :
               ROBOT_CHASSIS_ERR_IO;
    }
    if (status.st_size < 0 || (uint64_t)status.st_size >
            ROBOT_STATION_FILE_MAX_SIZE)
    {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    file = fopen(path, "rb");
    if (file == NULL)
    {
        return ROBOT_CHASSIS_ERR_IO;
    }
    buffer = malloc((size_t)status.st_size + 1U);
    if (buffer == NULL)
    {
        (void)fclose(file);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    if (fread(buffer, 1U, (size_t)status.st_size, file) !=
            (size_t)status.st_size || ferror(file))
    {
        (void)fclose(file);
        free(buffer);
        return ROBOT_CHASSIS_ERR_IO;
    }
    if (fclose(file) != 0)
    {
        free(buffer);
        return ROBOT_CHASSIS_ERR_IO;
    }
    buffer[status.st_size] = '\0';
    *text = buffer;
    *length = (size_t)status.st_size;
    return ROBOT_CHASSIS_OK;
}

// 加载站点信息到store中
int robot_station_store_load(robot_station_store_t *store)
{
    robot_station_t stations[ROBOT_STATION_MAX_COUNT] = {0};
    char *text = NULL;
    size_t length = 0U;
    const char *parse_end = NULL;
    cJSON *root = NULL;
    const cJSON *array;
    size_t count;
    size_t index;
    int result;

    if (store == NULL || !store->is_initialized)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = read_file(store->path, &text, &length);
    if (result == ROBOT_CHASSIS_ERR_WOULD_BLOCK)
    {
        return ROBOT_CHASSIS_OK;
    }
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    root = cJSON_ParseWithLengthOpts(text, length, &parse_end, 0);
    if (root == NULL)
    {
        result = ROBOT_CHASSIS_ERR_JSON;
        goto out;
    }
    while (parse_end < text + length && isspace((unsigned char)*parse_end))
    {
        ++parse_end;
    }
    array = cJSON_GetObjectItemCaseSensitive(root, "stations");
    if (parse_end != text + length || !cJSON_IsObject(root) ||
            !cJSON_IsArray(array))
    {
        result = ROBOT_CHASSIS_ERR_FORMAT;
        goto out;
    }
    count = (size_t)cJSON_GetArraySize(array);
    if (count > ROBOT_STATION_MAX_COUNT)
    {
        result = ROBOT_CHASSIS_ERR_LIMIT;
        goto out;
    }
    for (index = 0U; index < count; ++index)
    {
        result = parse_station(cJSON_GetArrayItem(array, (int)index),
                               &stations[index]);
        if (result != ROBOT_CHASSIS_OK)
        {
            goto out;
        }
    }
    result = validate_snapshot(stations, count);
    if (result != ROBOT_CHASSIS_OK)
    {
        goto out;
    }
    if (pthread_mutex_lock(&store->mutex) != 0)
    {
        result = ROBOT_CHASSIS_ERR_IO;
        goto out;
    }
    memcpy(store->stations, stations, sizeof(stations));
    store->count = count;
    (void)pthread_mutex_unlock(&store->mutex);

out:
    cJSON_Delete(root);
    free(text);
    return result;
}

static int apply_update(robot_station_t *stations, size_t *count,
                        const robot_station_t *station)
{
    size_t index;
    size_t base_index = *count;

    if (station == NULL || !name_is_valid(station->name) ||
            !isfinite(station->x) || !isfinite(station->y) ||
            !isfinite(station->z))
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    for (index = 0U; index < *count; ++index)
    {
        if (stations[index].is_charging_base)
        {
            base_index = index;
        }
        if (strcmp(stations[index].name, station->name) == 0)
        {
            if (stations[index].is_charging_base !=
                    station->is_charging_base)
            {
                return ROBOT_CHASSIS_ERR_CONFLICT;
            }
            stations[index] = *station;
            return ROBOT_CHASSIS_OK;
        }
    }
    if (station->is_charging_base && base_index < *count)
    {
        stations[base_index] = *station;
        return ROBOT_CHASSIS_OK;
    }
    if (*count >= ROBOT_STATION_MAX_COUNT)
    {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    stations[*count] = *station;
    ++(*count);
    return ROBOT_CHASSIS_OK;
}

static cJSON *build_json(const robot_station_t *stations, size_t count)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
    size_t index;

    if (root == NULL || array == NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(array);
        return NULL;
    }
    cJSON_AddItemToObject(root, "stations", array);
    for (index = 0U; index < count; ++index)
    {
        cJSON *item = cJSON_CreateObject();

        if (item == NULL ||
                !cJSON_AddStringToObject(item, "name", stations[index].name) ||
                !cJSON_AddNumberToObject(item, "x", stations[index].x) ||
                !cJSON_AddNumberToObject(item, "y", stations[index].y) ||
                !cJSON_AddNumberToObject(item, "z", stations[index].z) ||
                !cJSON_AddBoolToObject(item, "is_charging_base",
                                       stations[index].is_charging_base))
        {
            cJSON_Delete(item);
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToArray(array, item);
    }
    return root;
}

// 站点信息保存到文件中
static int write_snapshot(const robot_station_store_t *store,
                          const robot_station_t *stations, size_t count)
{
    char temporary[sizeof(((robot_station_store_t *)0)->path) + 5U];
    cJSON *root;
    char *text;
    FILE *file;
    size_t length;
    int result = ROBOT_CHASSIS_OK;
    int path_length;

    path_length = snprintf(temporary, sizeof(temporary), "%s.tmp", store->path);
    if (path_length < 0 || (size_t)path_length >= sizeof(temporary))
    {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    root = build_json(stations, count);
    if (root == NULL)
    {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (text == NULL)
    {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    file = fopen(temporary, "wb");
    if (file == NULL)
    {
        free(text);
        return ROBOT_CHASSIS_ERR_IO;
    }
    length = strlen(text);
    if (store->test_io_failure_point == ROBOT_STATION_TEST_IO_WRITE ||
            fwrite(text, 1U, length, file) != length ||
            store->test_io_failure_point == ROBOT_STATION_TEST_IO_FLUSH ||
            fflush(file) != 0 ||
            store->test_io_failure_point == ROBOT_STATION_TEST_IO_SYNC ||
            fsync(fileno(file)) != 0)
    {
        result = ROBOT_CHASSIS_ERR_IO;
    }
    free(text);
    if (fclose(file) != 0 ||
            store->test_io_failure_point == ROBOT_STATION_TEST_IO_CLOSE)
    {
        result = ROBOT_CHASSIS_ERR_IO;
    }
    if (result == ROBOT_CHASSIS_OK &&
            (store->test_io_failure_point == ROBOT_STATION_TEST_IO_RENAME ||
             rename(temporary, store->path) != 0))
    {
        result = ROBOT_CHASSIS_ERR_IO;
    }
    if (result != ROBOT_CHASSIS_OK)
    {
        (void)unlink(temporary);
    }
    return result;
}

// 更新站点信息到store中
int robot_station_store_update(robot_station_store_t *store,
                               const robot_station_t *station)
{
    robot_station_t snapshot[ROBOT_STATION_MAX_COUNT];
    size_t count;
    int result;

    if (store == NULL || !store->is_initialized)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    if (pthread_mutex_lock(&store->mutex) != 0)
    {
        return ROBOT_CHASSIS_ERR_IO;
    }
    memcpy(snapshot, store->stations, sizeof(snapshot));
    count = store->count;
    result = apply_update(snapshot, &count, station);
    if (result == ROBOT_CHASSIS_OK)
    {
        result = write_snapshot(store, snapshot, count);
    }
    if (result == ROBOT_CHASSIS_OK)
    {
        memcpy(store->stations, snapshot, sizeof(snapshot));
        store->count = count;
    }
    (void)pthread_mutex_unlock(&store->mutex);
    return result;
}
