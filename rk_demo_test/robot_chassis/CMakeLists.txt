add_library(robot_chassis STATIC
    robot_chassis_module.c
    robot_chassis_framer.c
    robot_chassis_client.c
    robot_chassis_log.c
    robot_chassis_queue.c
    robot_chassis_transaction.c
    robot_chassis_parser.c
    robot_chassis_status.c
    robot_chassis_control.c
    robot_chassis_map.c
    robot_chassis_map_list.c
    robot_chassis_save.c
    robot_chassis_transfer.c
    robot_chassis_backup_controller.c
    robot_chassis_upgrade_controller.c
    robot_chassis_mapping.c
    robot_chassis_heartbeat.c
    robot_chassis_service.c
    robot_chassis_service_loop.c
    robot_chassis_service_dispatch.c
    robot_chassis_service_mapping.c
    robot_chassis_service_map.c
    robot_chassis_service_transfer.c
    robot_station_store.c
    robot_station_store_file.c
    robot_station_app.c
    robot_station_navigation.c
)

target_include_directories(robot_chassis PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cJSON
)

target_link_libraries(robot_chassis PUBLIC pthread z crypto)

set_source_files_properties(
    robot_station_store.c
    robot_station_store_file.c
    robot_station_app.c
    robot_station_navigation.c
    PROPERTIES
    COMPILE_OPTIONS "-std=c99;-Wall;-Wextra;-Werror")
