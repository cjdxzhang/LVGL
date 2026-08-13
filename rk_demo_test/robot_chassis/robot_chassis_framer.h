#ifndef ROBOT_CHASSIS_FRAMER_H
#define ROBOT_CHASSIS_FRAMER_H

#include "robot_chassis_types.h"

typedef int (*robot_chassis_frame_callback_t)(const char *json, size_t length);

typedef struct
{
    uint8_t *buffer;
    size_t length;
    size_t capacity;
    size_t max_frame_size;
} robot_chassis_framer_t;

int robot_chassis_framer_init(robot_chassis_framer_t *framer, size_t max_frame_size);
void robot_chassis_framer_reset(robot_chassis_framer_t *framer);
void robot_chassis_framer_deinit(robot_chassis_framer_t *framer);
int robot_chassis_framer_feed(robot_chassis_framer_t *framer, const uint8_t *data, size_t length,
                              robot_chassis_frame_callback_t callback, size_t *frame_count);

#endif
