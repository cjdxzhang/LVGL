#include "robot_chassis_queue.h"

#include <stdlib.h>
#include <string.h>

static void release_entry(robot_chassis_command_t *command)
{
    free(command->data);
    memset(command, 0, sizeof(*command));
}

int robot_chassis_queue_init(robot_chassis_queue_t *queue)
{
    if (queue == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    memset(queue, 0, sizeof(*queue));
    if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
        return ROBOT_CHASSIS_ERR_IO;
    }
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_queue_deinit(robot_chassis_queue_t *queue)
{
    size_t index;

    if (queue == NULL) {
        return;
    }

    pthread_mutex_lock(&queue->mutex);
    for (index = 0U; index < ROBOT_CHASSIS_QUEUE_CAPACITY; ++index) {
        release_entry(&queue->entries[index]);
    }
    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
    pthread_mutex_unlock(&queue->mutex);
    pthread_mutex_destroy(&queue->mutex);
}

int robot_chassis_queue_push(robot_chassis_queue_t *queue,
                             const robot_chassis_command_t *command)
{
    robot_chassis_command_t copy;

    if (queue == NULL || command == NULL || command->data == NULL ||
        command->length == 0U || command->length == SIZE_MAX) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    memset(&copy, 0, sizeof(copy));
    copy = *command;
    copy.data = malloc(command->length + 1U);
    if (copy.data == NULL) {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    memcpy(copy.data, command->data, command->length);
    copy.data[command->length] = '\0';

    pthread_mutex_lock(&queue->mutex);
    if (queue->count == ROBOT_CHASSIS_QUEUE_CAPACITY) {
        pthread_mutex_unlock(&queue->mutex);
        free(copy.data);
        return ROBOT_CHASSIS_ERR_QUEUE_FULL;
    }
    queue->entries[queue->tail] = copy;
    queue->tail = (queue->tail + 1U) % ROBOT_CHASSIS_QUEUE_CAPACITY;
    queue->count++;
    pthread_mutex_unlock(&queue->mutex);
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_queue_pop(robot_chassis_queue_t *queue,
                            robot_chassis_command_t *command)
{
    if (queue == NULL || command == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&queue->mutex);
    if (queue->count == 0U) {
        pthread_mutex_unlock(&queue->mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    *command = queue->entries[queue->head];
    memset(&queue->entries[queue->head], 0, sizeof(queue->entries[queue->head]));
    queue->head = (queue->head + 1U) % ROBOT_CHASSIS_QUEUE_CAPACITY;
    queue->count--;
    pthread_mutex_unlock(&queue->mutex);
    return ROBOT_CHASSIS_OK;
}

size_t robot_chassis_queue_size(robot_chassis_queue_t *queue)
{
    size_t count;

    if (queue == NULL) {
        return 0U;
    }

    pthread_mutex_lock(&queue->mutex);
    count = queue->count;
    pthread_mutex_unlock(&queue->mutex);
    return count;
}

