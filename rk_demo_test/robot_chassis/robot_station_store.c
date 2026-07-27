#include "robot_station_store.h"

#include <math.h>
#include <string.h>

static bool station_name_is_valid(const char *name)
{
    size_t index;

    if (name == NULL) {
        return false;
    }
    for (index = 0U; index < sizeof(((robot_station_t *)0)->name); ++index) {
        if (name[index] == '\0') {
            return index > 0U;
        }
    }
    return false;
}

static bool station_is_valid(const robot_station_t *station)
{
    return station != NULL && station_name_is_valid(station->name) &&
           isfinite(station->x) && isfinite(station->y) &&
           isfinite(station->z);
}

static size_t find_name_locked(const robot_station_store_t *store,
                               const char *name)
{
    size_t index;

    for (index = 0U; index < store->count; ++index) {
        if (strcmp(store->stations[index].name, name) == 0) {
            return index;
        }
    }
    return store->count;
}

static size_t find_base_locked(const robot_station_store_t *store)
{
    size_t index;

    for (index = 0U; index < store->count; ++index) {
        if (store->stations[index].is_charging_base) {
            return index;
        }
    }
    return store->count;
}

static int store_lock(const robot_station_store_t *store)
{
    robot_station_store_t *mutable_store = (robot_station_store_t *)store;

    if (store == NULL || !store->is_initialized) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    if (pthread_mutex_lock(&mutable_store->mutex) != 0) {
        return ROBOT_CHASSIS_ERR_IO;
    }
    if (!store->is_initialized) {
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

    if (store == NULL || path == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    if (store->is_initialized) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    path_length = strlen(path);
    if (path_length == 0U || path_length >= sizeof(store->path)) {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    memset(store, 0, sizeof(*store));
    if (pthread_mutex_init(&store->mutex, NULL) != 0) {
        return ROBOT_CHASSIS_ERR_IO;
    }
    memcpy(store->path, path, path_length + 1U);
    store->is_initialized = true;
    return ROBOT_CHASSIS_OK;
}

void robot_station_store_deinit(robot_station_store_t *store)
{
    if (store == NULL || !store->is_initialized) {
        return;
    }
    if (pthread_mutex_lock(&store->mutex) != 0) {
        return;
    }
    store->is_initialized = false;
    (void)pthread_mutex_unlock(&store->mutex);
    if (pthread_mutex_destroy(&store->mutex) != 0) {
        return;
    }
    memset(store, 0, sizeof(*store));
}

int robot_station_store_upsert(robot_station_store_t *store,
                               const robot_station_t *station)
{
    size_t name_index;
    size_t base_index;
    int result;

    if (!station_is_valid(station)) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = store_lock(store);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }

    name_index = find_name_locked(store, station->name);
    if (name_index < store->count) {
        if (store->stations[name_index].is_charging_base !=
            station->is_charging_base) {
            result = ROBOT_CHASSIS_ERR_CONFLICT;
        } else {
            store->stations[name_index] = *station;
            result = ROBOT_CHASSIS_OK;
        }
        store_unlock(store);
        return result;
    }

    if (station->is_charging_base) {
        base_index = find_base_locked(store);
        if (base_index < store->count) {
            store->stations[base_index] = *station;
            store_unlock(store);
            return ROBOT_CHASSIS_OK;
        }
    }
    if (store->count >= ROBOT_STATION_MAX_COUNT) {
        store_unlock(store);
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    store->stations[store->count] = *station;
    ++store->count;
    store_unlock(store);
    return ROBOT_CHASSIS_OK;
}

size_t robot_station_store_count(const robot_station_store_t *store)
{
    size_t count;

    if (store_lock(store) != ROBOT_CHASSIS_OK) {
        return 0U;
    }
    count = store->count;
    store_unlock(store);
    return count;
}

size_t robot_station_store_count_charging_bases(
    const robot_station_store_t *store)
{
    size_t count = 0U;
    size_t index;

    if (store_lock(store) != ROBOT_CHASSIS_OK) {
        return 0U;
    }
    for (index = 0U; index < store->count; ++index) {
        if (store->stations[index].is_charging_base) {
            ++count;
        }
    }
    store_unlock(store);
    return count;
}

int robot_station_store_find_by_name(const robot_station_store_t *store,
                                     const char *name,
                                     robot_station_t *station)
{
    size_t index;
    int result;

    if (!station_name_is_valid(name) || station == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = store_lock(store);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    index = find_name_locked(store, name);
    if (index >= store->count) {
        store_unlock(store);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    *station = store->stations[index];
    store_unlock(store);
    return ROBOT_CHASSIS_OK;
}

int robot_station_store_get_charging_base(const robot_station_store_t *store,
                                          robot_station_t *station)
{
    size_t index;
    int result;

    if (station == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = store_lock(store);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    index = find_base_locked(store);
    if (index >= store->count) {
        store_unlock(store);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    *station = store->stations[index];
    store_unlock(store);
    return ROBOT_CHASSIS_OK;
}
