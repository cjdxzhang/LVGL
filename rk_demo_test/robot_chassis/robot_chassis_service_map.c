#include "robot_chassis_service_internal.h"

#include "robot_chassis_map.h"

#define ROBOT_CHASSIS_DEFAULT_MAX_MAP_CELLS (4U * 1024U * 1024U)

int robot_chassis_service_handle_realtime_map(const char *json, size_t length)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_grid_t grid = {0};
    const size_t max_cells = service->config.max_map_cells != 0U ?
                             service->config.max_map_cells : ROBOT_CHASSIS_DEFAULT_MAX_MAP_CELLS;
    int result;

    result = robot_chassis_map_parse_grid(json, length, max_cells, &grid);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    if (service->config.grid_callback != NULL)
    {
        service->config.grid_callback(&grid);
    }
    robot_chassis_map_grid_release(&grid);
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_service_handle_map_list(const char *json, size_t length)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_map_list_t map_list = {0};
    int result;

    result = robot_chassis_map_parse_list(json, length, &map_list);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    if (service->config.map_list_callback != NULL)
    {
        service->config.map_list_callback(&map_list);
    }
    robot_chassis_map_list_release(&map_list);
    return ROBOT_CHASSIS_OK;
}
