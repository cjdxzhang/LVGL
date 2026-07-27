cmake_minimum_required(VERSION 3.8)

project(rk_demo_test)

add_subdirectory(robot_chassis)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/guider_fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/guider_customer_fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/images
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/guider_fonts SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/images SRCS)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} PRIVATE robot_chassis)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)