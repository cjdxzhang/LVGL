#ifndef ROBOT_CHASSIS_MAP_H
#define ROBOT_CHASSIS_MAP_H

#include "robot_chassis_types.h"

typedef struct {
    uint32_t width;
    uint32_t height;
    double origin_x;
    double origin_y;
    double resolution;
    bool coordinate_conversion_available;
    uint8_t *cells;
    size_t cell_count;
} robot_chassis_grid_t;

typedef struct {
    char **names;
    size_t count;
} robot_chassis_map_list_t;

int robot_chassis_map_parse_grid(const char *json, size_t length, size_t max_cells,
                                 robot_chassis_grid_t *grid);
void robot_chassis_map_grid_release(robot_chassis_grid_t *grid);
int robot_chassis_map_parse_list(const char *json, size_t length,
                                 robot_chassis_map_list_t *map_list);
void robot_chassis_map_list_release(robot_chassis_map_list_t *map_list);

#endif

