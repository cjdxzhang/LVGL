#include "robot_chassis_service_internal.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

robot_chassis_service_context_t g_robot_chassis_service;

uint64_t robot_chassis_service_now_ms(void)
{
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return 0U;
    }
    return (uint64_t)now.tv_sec * 1000U + (uint64_t)now.tv_nsec / 1000000U;
}

bool robot_chassis_service_is_running(void)
{
    bool running;

    pthread_mutex_lock(&g_robot_chassis_service.state_mutex);
    running = g_robot_chassis_service.running;
    pthread_mutex_unlock(&g_robot_chassis_service.state_mutex);
    return running;
}

void robot_chassis_service_notify_connection(void)
{
    robot_chassis_connection_state_t state;
    robot_chassis_connection_callback_t callback;
    void *user_data;

    pthread_mutex_lock(&g_robot_chassis_service.state_mutex);
    state = g_robot_chassis_service.heartbeat.connection;
    callback = g_robot_chassis_service.config.connection_callback;
    user_data = g_robot_chassis_service.config.user_data;
    pthread_mutex_unlock(&g_robot_chassis_service.state_mutex);
    if (callback != NULL) {
        callback(&state, user_data);
    }
}

int robot_chassis_service_init(const robot_chassis_service_config_t *config)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    const char *host;
    uint16_t port;
    int result;

    if (config == NULL || service->initialized) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    memset(service, 0, sizeof(*service));
    if (pthread_mutex_init(&service->state_mutex, NULL) != 0) {
        return ROBOT_CHASSIS_ERR_IO;
    }
    if (pthread_mutex_init(&service->submission_mutex, NULL) != 0) {
        pthread_mutex_destroy(&service->state_mutex);
        return ROBOT_CHASSIS_ERR_IO;
    }
    if (pthread_mutex_init(&service->transfer_mutex, NULL) != 0) {
        pthread_mutex_destroy(&service->submission_mutex);
        pthread_mutex_destroy(&service->state_mutex);
        return ROBOT_CHASSIS_ERR_IO;
    }

    host = config->host != NULL ? config->host : ROBOT_CHASSIS_DEFAULT_IP;
    port = config->port != 0U ? config->port : ROBOT_CHASSIS_DEFAULT_PORT;
    result = robot_chassis_client_init(&service->client, host, port);
    if (result != ROBOT_CHASSIS_OK) {
        pthread_mutex_destroy(&service->transfer_mutex);
        pthread_mutex_destroy(&service->submission_mutex);
        pthread_mutex_destroy(&service->state_mutex);
        return result;
    }
    result = robot_chassis_framer_init(&service->framer,
                                       ROBOT_CHASSIS_MAX_FRAME_SIZE);
    if (result != ROBOT_CHASSIS_OK) {
        pthread_mutex_destroy(&service->transfer_mutex);
        pthread_mutex_destroy(&service->submission_mutex);
        pthread_mutex_destroy(&service->state_mutex);
        return result;
    }
    result = robot_chassis_queue_init(&service->queue);
    if (result != ROBOT_CHASSIS_OK) {
        robot_chassis_framer_deinit(&service->framer);
        pthread_mutex_destroy(&service->transfer_mutex);
        pthread_mutex_destroy(&service->submission_mutex);
        pthread_mutex_destroy(&service->state_mutex);
        return result;
    }
    result = robot_chassis_transaction_init(&service->transaction);
    if (result != ROBOT_CHASSIS_OK) {
        robot_chassis_queue_deinit(&service->queue);
        robot_chassis_framer_deinit(&service->framer);
        pthread_mutex_destroy(&service->transfer_mutex);
        pthread_mutex_destroy(&service->submission_mutex);
        pthread_mutex_destroy(&service->state_mutex);
        return result;
    }

    service->config = *config;
    robot_chassis_heartbeat_init(&service->heartbeat);
    robot_chassis_mapping_poll_init(&service->mapping_poll);
    robot_chassis_service_transfers_init();
    service->initialized = true;
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_service_start(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;

    if (!service->initialized) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    pthread_mutex_lock(&service->state_mutex);
    if (service->running) {
        pthread_mutex_unlock(&service->state_mutex);
        return ROBOT_CHASSIS_ERR_CONFLICT;
    }
    service->running = true;
    pthread_mutex_unlock(&service->state_mutex);

    if (pthread_create(&service->thread, NULL,
                       robot_chassis_service_thread_main, NULL) != 0) {
        pthread_mutex_lock(&service->state_mutex);
        service->running = false;
        pthread_mutex_unlock(&service->state_mutex);
        return ROBOT_CHASSIS_ERR_IO;
    }
    service->thread_started = true;
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_service_stop(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;

    if (!service->initialized || !service->thread_started) {
        return;
    }
    pthread_mutex_lock(&service->state_mutex);
    service->running = false;
    pthread_mutex_unlock(&service->state_mutex);
    pthread_join(service->thread, NULL);
    service->thread_started = false;
}

void robot_chassis_service_deinit(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;

    if (!service->initialized) {
        return;
    }
    robot_chassis_service_stop();
    robot_chassis_client_close(&service->client);
    robot_chassis_transaction_deinit(&service->transaction);
    robot_chassis_queue_deinit(&service->queue);
    robot_chassis_framer_deinit(&service->framer);
    pthread_mutex_destroy(&service->transfer_mutex);
    pthread_mutex_destroy(&service->submission_mutex);
    pthread_mutex_destroy(&service->state_mutex);
    memset(service, 0, sizeof(*service));
}

static bool command_allowed_during_transfer(
    robot_chassis_transaction_t *transaction,
    const robot_chassis_command_t *command)
{
    if (robot_chassis_transaction_active_transfer(transaction) ==
        ROBOT_CHASSIS_TRANSFER_NONE) {
        return true;
    }
    if (command->uses_special_command &&
        command->special_command == ROBOT_CHASSIS_CMD_HEARTBEAT) {
        return true;
    }
    return command->response_t > 0 &&
           robot_chassis_transaction_allows_t(transaction, command->response_t);
}

int robot_chassis_service_submit(robot_chassis_command_t *command)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_transfer_kind_t transfer = ROBOT_CHASSIS_TRANSFER_NONE;
    robot_chassis_transfer_kind_t active_transfer;
    bool started_transfer = false;
    int result;

    if (!service->initialized || command == NULL || command->data == NULL ||
        command->length == 0U) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&service->submission_mutex);
    if (!command_allowed_during_transfer(&service->transaction, command)) {
        result = ROBOT_CHASSIS_ERR_BUSY;
        goto out;
    }
    if (command->response_t == ROBOT_CHASSIS_T_MAP_BACKUP_BEGIN) {
        transfer = ROBOT_CHASSIS_TRANSFER_MAP_BACKUP;
    } else if (command->response_t == ROBOT_CHASSIS_T_UPGRADE_BEGIN) {
        transfer = ROBOT_CHASSIS_TRANSFER_UPGRADE;
    }
    if (transfer != ROBOT_CHASSIS_TRANSFER_NONE) {
        active_transfer = robot_chassis_transaction_active_transfer(
            &service->transaction);
        if (active_transfer == ROBOT_CHASSIS_TRANSFER_NONE) {
            result = robot_chassis_transaction_begin_transfer(
                &service->transaction, transfer);
            if (result != ROBOT_CHASSIS_OK) {
                goto out;
            }
            started_transfer = true;
        } else if (active_transfer != transfer) {
            result = ROBOT_CHASSIS_ERR_BUSY;
            goto out;
        }
    }
    if (command->response_t > 0) {
        result = robot_chassis_transaction_reserve_t(&service->transaction,
                                                     command->response_t);
        if (result != ROBOT_CHASSIS_OK) {
            if (started_transfer) {
                robot_chassis_transaction_end_transfer(
                    &service->transaction, transfer);
            }
            goto out;
        }
    }

    result = robot_chassis_queue_push(&service->queue, command);
    if (result != ROBOT_CHASSIS_OK) {
        if (command->response_t > 0) {
            robot_chassis_transaction_cancel_t(&service->transaction,
                                               command->response_t);
        }
        if (started_transfer) {
            robot_chassis_transaction_end_transfer(&service->transaction,
                                                   transfer);
        }
    }
out:
    pthread_mutex_unlock(&service->submission_mutex);
    return result;
}

int robot_chassis_service_get_connection_state(
    robot_chassis_connection_state_t *state)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;

    if (!service->initialized || state == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    pthread_mutex_lock(&service->state_mutex);
    *state = service->heartbeat.connection;
    pthread_mutex_unlock(&service->state_mutex);
    return ROBOT_CHASSIS_OK;
}
