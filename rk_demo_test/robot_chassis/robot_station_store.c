#include "robot_station_store.h"

#include <math.h>
#include <string.h>

static bool station_name_is_valid(const char *name)
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

static size_t find_name_locked(const robot_station_store_t *store,
                               const char *name)
{
    size_t index;

    for (index = 0U; index < store->count; ++index)
    {
        if (strcmp(store->stations[index].name, name) == 0)
        {
            return index;
        }
    }
    return store->count;
}

static size_t find_base_locked(const robot_station_store_t *store)
{
    size_t index;

    for (index = 0U; index < store->count; ++index)
    {
        if (store->stations[index].is_charging_base)
        {
            return index;
        }
    }
    return store->count;
}

static int store_lock(const robot_station_store_t *store)
{
    robot_station_store_t *mutable_store = (robot_station_store_t *)store;

    if (store == NULL || !store->is_initialized)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    if (pthread_mutex_lock(&mutable_store->mutex) != 0)
    {
        return ROBOT_CHASSIS_ERR_IO;
    }
    if (!store->is_initialized)
    {
        (void)pthread_mutex_unlock(&mutable_store->mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    return ROBOT_CHASSIS_OK;
}

static void store_unlock(const robot_station_store_t *store)
{
    robot_station_store_t *mutable_store = (robot_station_store_t *)store;

    (void)pthread_mutex_unlock(&mutable_store->mutex);
}

int robot_station_store_init(robot_station_store_t *store, const char *path)
{
    size_t path_length;

    if (store == NULL || path == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    if (store->is_initialized)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    path_length = strlen(path);
    if (path_length == 0U || path_length >= sizeof(store->path))
    {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    memset(store, 0, sizeof(*store));
    if (pthread_mutex_init(&store->mutex, NULL) != 0)
    {
        return ROBOT_CHASSIS_ERR_IO;
    }
    memcpy(store->path, path, path_length + 1U);
    store->is_initialized = true;
    return ROBOT_CHASSIS_OK;
}

void robot_station_store_deinit(robot_station_store_t *store)
{
    if (store == NULL || !store->is_initialized)
    {
        return;
    }
    if (pthread_mutex_lock(&store->mutex) != 0)
    {
        return;
    }
    store->is_initialized = false;
    (void)pthread_mutex_unlock(&store->mutex);
    if (pthread_mutex_destroy(&store->mutex) != 0)
    {
        return;
    }
    memset(store, 0, sizeof(*store));
}

size_t robot_station_store_count(const robot_station_store_t *store)
{
    size_t count;

    if (store_lock(store) != ROBOT_CHASSIS_OK)
    {
        return 0U;
    }
    count = store->count;
    store_unlock(store);
    return count;
}

// 通过名字找到站点
int robot_station_store_find_by_name(const robot_station_store_t *store,
                                     const char *name,
                                     robot_station_t *station)
{
    size_t index;
    int result;

    if (!station_name_is_valid(name) || station == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = store_lock(store);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    index = find_name_locked(store, name);
    if (index >= store->count)
    {
        store_unlock(store);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    *station = store->stations[index];
    store_unlock(store);
    return ROBOT_CHASSIS_OK;
}

// 获取充电基站信息
int robot_station_store_get_charging_base(const robot_station_store_t *store,
        robot_station_t *station)
{
    size_t index;
    int result;

    if (station == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = store_lock(store);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    index = find_base_locked(store);
    if (index >= store->count)
    {
        store_unlock(store);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    *station = store->stations[index];
    store_unlock(store);
    return ROBOT_CHASSIS_OK;
}
