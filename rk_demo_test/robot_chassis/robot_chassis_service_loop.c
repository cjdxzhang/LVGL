#include "robot_chassis_service_internal.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROBOT_CHASSIS_CONNECT_TIMEOUT_MS 1000
#define ROBOT_CHASSIS_SERVICE_IDLE_NS 10000000L
#define ROBOT_CHASSIS_RECEIVE_BUFFER_SIZE 65536U
#define ROBOT_CHASSIS_RESPONSE_TIMEOUT_MS 5000U

static void release_command(robot_chassis_command_t *command)
{
    free(command->data);
    memset(command, 0, sizeof(*command));
}

static void reset_transactions_and_queue(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_command_t command;

    pthread_mutex_lock(&service->submission_mutex);
    while (robot_chassis_queue_pop(&service->queue, &command) ==
           ROBOT_CHASSIS_OK) {
        release_command(&command);
    }

    robot_chassis_transaction_clear(&service->transaction);
    pthread_mutex_unlock(&service->submission_mutex);
}

void robot_chassis_service_disconnect(uint64_t now_ms)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    uint32_t actions;

    robot_chassis_client_close(&service->client);
    robot_chassis_framer_reset(&service->framer);
    robot_chassis_service_abort_transfers(ROBOT_CHASSIS_ERR_NOT_CONNECTED);
    reset_transactions_and_queue();

    pthread_mutex_lock(&service->state_mutex);
    actions = robot_chassis_heartbeat_on_disconnected(&service->heartbeat,
                                                      now_ms);
    pthread_mutex_unlock(&service->state_mutex);
    if ((actions & ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED) != 0U) {
        robot_chassis_service_notify_connection();
    }
}

static int send_literal(const char *json)
{
    return robot_chassis_client_send_request(
        &g_robot_chassis_service.client, json, strlen(json));
}

static int send_initial_status(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    int result;

    result = robot_chassis_transaction_reserve_t(
        &service->transaction, ROBOT_CHASSIS_T_STATUS);
    if (result == ROBOT_CHASSIS_ERR_CONFLICT) {
        return ROBOT_CHASSIS_OK;
    }
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    result = send_literal("{\"t\":42,\"p\":{\"data\":1}}");
    if (result == ROBOT_CHASSIS_OK) {
        result = robot_chassis_transaction_mark_sent(
            &service->transaction, ROBOT_CHASSIS_T_STATUS,
            robot_chassis_service_now_ms());
    }
    if (result != ROBOT_CHASSIS_OK) {
        robot_chassis_transaction_cancel_t(
            &service->transaction, ROBOT_CHASSIS_T_STATUS);
    }
    return result;
}

static int try_connect(uint64_t now_ms)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    uint32_t actions;
    int result;

    pthread_mutex_lock(&service->state_mutex);
    actions = robot_chassis_heartbeat_poll(&service->heartbeat, now_ms);
    pthread_mutex_unlock(&service->state_mutex);
    if ((actions & ROBOT_CHASSIS_ACTION_RECONNECT) == 0U) {
        return ROBOT_CHASSIS_ERR_WOULD_BLOCK;
    }

    result = robot_chassis_client_connect(&service->client,
                                          ROBOT_CHASSIS_CONNECT_TIMEOUT_MS);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    pthread_mutex_lock(&service->state_mutex);
    actions = robot_chassis_heartbeat_on_connected(&service->heartbeat,
                                                   now_ms);
    pthread_mutex_unlock(&service->state_mutex);
    robot_chassis_service_notify_connection();

    if ((actions & ROBOT_CHASSIS_ACTION_SEND_HEARTBEAT) != 0U) {
        result = send_literal("{\"cmd\":\"heatbeat\"}");
        if (result != ROBOT_CHASSIS_OK) {
            return result;
        }
    }
    if ((actions & ROBOT_CHASSIS_ACTION_SEND_INITIAL_STATUS) != 0U) {
        result = send_initial_status();
    }
    return result;
}

static int handle_heartbeat_schedule(uint64_t now_ms)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    uint32_t actions;

    pthread_mutex_lock(&service->state_mutex);
    actions = robot_chassis_heartbeat_poll(&service->heartbeat, now_ms);
    pthread_mutex_unlock(&service->state_mutex);
    if ((actions & ROBOT_CHASSIS_ACTION_CONNECTION_CHANGED) != 0U) {
        robot_chassis_service_notify_connection();
    }
    if ((actions & ROBOT_CHASSIS_ACTION_HEARTBEAT_LOST) != 0U) {
        robot_chassis_client_close(&service->client);
        robot_chassis_framer_reset(&service->framer);
        reset_transactions_and_queue();
        return ROBOT_CHASSIS_ERR_TIMEOUT;
    }
    if ((actions & ROBOT_CHASSIS_ACTION_SEND_HEARTBEAT) != 0U) {
        return send_literal("{\"cmd\":\"heatbeat\"}");
    }
    return ROBOT_CHASSIS_OK;
}

static int send_next_command(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_command_t command;
    int result;

    result = robot_chassis_queue_pop(&service->queue, &command);
    if (result == ROBOT_CHASSIS_ERR_STATE) {
        return ROBOT_CHASSIS_OK;
    }
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }

    result = robot_chassis_client_send_request(
        &service->client, command.data, command.length);
    if (command.response_t > 0) {
        if (result == ROBOT_CHASSIS_OK) {
            result = robot_chassis_transaction_mark_sent(
                &service->transaction, command.response_t,
                robot_chassis_service_now_ms());
        } else {
            robot_chassis_transaction_cancel_t(&service->transaction,
                                               command.response_t);
        }
    }
    release_command(&command);
    return result;
}

static int receive_available(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    uint8_t buffer[ROBOT_CHASSIS_RECEIVE_BUFFER_SIZE];

    for (;;) {
        const ssize_t received = robot_chassis_client_receive(
            &service->client, buffer, sizeof(buffer));
        size_t frame_count = 0U;
        int result;

        if (received == ROBOT_CHASSIS_ERR_WOULD_BLOCK) {
            return ROBOT_CHASSIS_OK;
        }
        if (received < 0) {
            return (int)received;
        }
        result = robot_chassis_framer_feed(
            &service->framer, buffer, (size_t)received,
            robot_chassis_service_handle_frame, NULL, &frame_count);
        if (result != ROBOT_CHASSIS_OK) {
            return result;
        }
    }
}

void *robot_chassis_service_thread_main(void *argument)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    const struct timespec idle = {
        .tv_sec = 0,
        .tv_nsec = ROBOT_CHASSIS_SERVICE_IDLE_NS
    };

    (void)argument;
    while (robot_chassis_service_is_running()) {
        const uint64_t now_ms = robot_chassis_service_now_ms();
        int result;

        if (service->client.fd < 0) {
            result = try_connect(now_ms);
            if (result != ROBOT_CHASSIS_OK &&
                result != ROBOT_CHASSIS_ERR_WOULD_BLOCK) {
                robot_chassis_service_disconnect(now_ms);
            }
            nanosleep(&idle, NULL);
            continue;
        }

        robot_chassis_service_handle_transfer_timeouts(now_ms);
        result = handle_heartbeat_schedule(now_ms);
        if (result == ROBOT_CHASSIS_OK) {
            result = robot_chassis_service_poll_mapping(now_ms);
        }
        if (result == ROBOT_CHASSIS_OK) {
            result = send_next_command();
        }
        if (result == ROBOT_CHASSIS_OK) {
            result = receive_available();
        }
        if (result != ROBOT_CHASSIS_OK) {
            robot_chassis_service_disconnect(now_ms);
        }
        nanosleep(&idle, NULL);
    }

    robot_chassis_service_disconnect(robot_chassis_service_now_ms());
    return NULL;
}
