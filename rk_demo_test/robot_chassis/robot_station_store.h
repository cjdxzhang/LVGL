#ifndef ROBOT_STATION_STORE_H
#define ROBOT_STATION_STORE_H

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

#include "robot_chassis_types.h"

#define ROBOT_STATION_MAX_COUNT 32U

typedef struct
{
    char name[64];
    double x;
    double y;
    double z;
    bool is_charging_base;
} robot_station_t;

typedef struct
{
    pthread_mutex_t mutex;
    robot_station_t stations[ROBOT_STATION_MAX_COUNT];
    size_t count;
    bool is_initialized;
    char path[256];
    int test_io_failure_point;
} robot_station_store_t;

typedef enum
{
    ROBOT_STATION_TEST_IO_NONE = 0,
    ROBOT_STATION_TEST_IO_WRITE,
    ROBOT_STATION_TEST_IO_FLUSH,
    ROBOT_STATION_TEST_IO_SYNC,
    ROBOT_STATION_TEST_IO_CLOSE,
    ROBOT_STATION_TEST_IO_RENAME
} robot_station_test_io_failure_t;

/* 调用方必须保证 init/deinit 不与其他接口并发。 */
int robot_station_store_init(robot_station_store_t *store, const char *path);
void robot_station_store_deinit(robot_station_store_t *store);
int robot_station_store_load(robot_station_store_t *store);
int robot_station_store_update(robot_station_store_t *store,
                               const robot_station_t *station);
size_t robot_station_store_count(const robot_station_store_t *store);
int robot_station_store_find_by_name(const robot_station_store_t *store,
                                     const char *name,
                                     robot_station_t *station);
int robot_station_store_get_charging_base(const robot_station_store_t *store,
        robot_station_t *station);

#endif
