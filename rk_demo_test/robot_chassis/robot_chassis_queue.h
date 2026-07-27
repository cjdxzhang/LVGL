#ifndef ROBOT_CHASSIS_QUEUE_H
#define ROBOT_CHASSIS_QUEUE_H

#include <pthread.h>

#include "robot_chassis_types.h"

typedef struct {
    robot_chassis_command_t entries[ROBOT_CHASSIS_QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
    pthread_mutex_t mutex;
} robot_chassis_queue_t;

int robot_chassis_queue_init(robot_chassis_queue_t *queue);
void robot_chassis_queue_deinit(robot_chassis_queue_t *queue);
int robot_chassis_queue_push(robot_chassis_queue_t *queue, const robot_chassis_command_t *command);
int robot_chassis_queue_pop(robot_chassis_queue_t *queue, robot_chassis_command_t *command);
size_t robot_chassis_queue_size(robot_chassis_queue_t *queue);

#endif
