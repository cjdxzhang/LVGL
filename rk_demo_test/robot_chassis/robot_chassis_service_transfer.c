#include "robot_chassis_service_internal.h"
#include <stdio.h>

static int emit_command(const robot_chassis_command_t *command, void *user_data)
{
    robot_chassis_command_t copy;

    (void)user_data;
    copy = *command;
    return robot_chassis_service_submit(&copy);
}

void robot_chassis_service_transfers_init(void)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;

    robot_chassis_backup_controller_init(&service->backup_controller,
                                         emit_command, NULL, service);
    robot_chassis_upgrade_controller_init(&service->upgrade_controller,
                                          emit_command, NULL, service);
}

int robot_chassis_service_backup_map(const char *map_name,
                                     const char *temporary_path,
                                     const char *final_path)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    int result;

    if (!service->initialized) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    pthread_mutex_lock(&service->transfer_mutex);
    result = robot_chassis_backup_controller_start(
        &service->backup_controller, map_name, temporary_path, final_path);
    pthread_mutex_unlock(&service->transfer_mutex);
    return result;
}

int robot_chassis_service_upgrade(const char *file_path)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    int result;

    if (!service->initialized) {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    pthread_mutex_lock(&service->transfer_mutex);
    result = robot_chassis_upgrade_controller_start(
        &service->upgrade_controller, file_path);
    pthread_mutex_unlock(&service->transfer_mutex);
    return result;
}

int robot_chassis_service_handle_backup_response(const char *json,
                                                 size_t length)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    char final_path[256] = {0};
    bool finished;
    int result;

    pthread_mutex_lock(&service->transfer_mutex);
    result = robot_chassis_backup_controller_accept(
        &service->backup_controller, json, length);
    finished = !service->backup_controller.active;
    if (finished && result == ROBOT_CHASSIS_OK) {
        (void)snprintf(final_path, sizeof(final_path), "%s",
                       service->backup_controller.final_path);
    }
    pthread_mutex_unlock(&service->transfer_mutex);
    if (finished) {
        robot_chassis_transaction_end_transfer(
            &service->transaction, ROBOT_CHASSIS_TRANSFER_MAP_BACKUP);
        if (service->config.backup_callback != NULL) {
            service->config.backup_callback(
                result, result == ROBOT_CHASSIS_OK ? final_path : NULL,
                service->config.user_data);
        }
    }
    return result;
}

int robot_chassis_service_handle_upgrade_response(const char *json,
                                                  size_t length)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    bool finished;
    int result;

    pthread_mutex_lock(&service->transfer_mutex);
    result = robot_chassis_upgrade_controller_accept(
        &service->upgrade_controller, json, length);
    finished = !service->upgrade_controller.active;
    pthread_mutex_unlock(&service->transfer_mutex);
    if (finished) {
        robot_chassis_transaction_end_transfer(
            &service->transaction, ROBOT_CHASSIS_TRANSFER_UPGRADE);
        if (service->config.upgrade_callback != NULL) {
            service->config.upgrade_callback(result,
                                             service->config.user_data);
        }
    }
    return result;
}

void robot_chassis_service_handle_transfer_timeouts(uint64_t now_ms)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    robot_chassis_transfer_kind_t kind = ROBOT_CHASSIS_TRANSFER_NONE;
    int expected_t = 0;
    bool pending = false;
    bool finished = false;
    int result = ROBOT_CHASSIS_OK;

    pthread_mutex_lock(&service->transfer_mutex);
    if (service->backup_controller.active) {
        kind = ROBOT_CHASSIS_TRANSFER_MAP_BACKUP;
        expected_t = service->backup_controller.transfer.expected_step <= 1U ?
                     ROBOT_CHASSIS_T_MAP_BACKUP_BEGIN :
                     ROBOT_CHASSIS_T_MAP_BACKUP_NEXT;
    } else if (service->upgrade_controller.active) {
        kind = ROBOT_CHASSIS_TRANSFER_UPGRADE;
        expected_t = robot_chassis_upgrade_controller_expected_t(
            &service->upgrade_controller);
    }
    if (expected_t > 0) {
        pending = robot_chassis_transaction_has_t(&service->transaction,
                                                  expected_t);
    }
    robot_chassis_transaction_expire_sent(&service->transaction, now_ms,
                                          5000U);
    if (pending && !robot_chassis_transaction_has_t(&service->transaction,
                                                    expected_t)) {
        if (kind == ROBOT_CHASSIS_TRANSFER_MAP_BACKUP) {
            result = robot_chassis_backup_controller_timeout(
                &service->backup_controller);
            finished = !service->backup_controller.active;
        } else {
            result = robot_chassis_upgrade_controller_timeout(
                &service->upgrade_controller);
            finished = !service->upgrade_controller.active;
        }
    }
    pthread_mutex_unlock(&service->transfer_mutex);

    if (finished) {
        robot_chassis_transaction_end_transfer(&service->transaction, kind);
        if (kind == ROBOT_CHASSIS_TRANSFER_MAP_BACKUP &&
            service->config.backup_callback != NULL) {
            service->config.backup_callback(result, NULL,
                                            service->config.user_data);
        } else if (kind == ROBOT_CHASSIS_TRANSFER_UPGRADE &&
                   service->config.upgrade_callback != NULL) {
            service->config.upgrade_callback(result,
                                             service->config.user_data);
        }
    }
}

void robot_chassis_service_abort_transfers(int result)
{
    robot_chassis_service_context_t *service = &g_robot_chassis_service;
    bool backup_active;
    bool upgrade_active;

    pthread_mutex_lock(&service->transfer_mutex);
    backup_active = service->backup_controller.active;
    upgrade_active = service->upgrade_controller.active;
    robot_chassis_backup_controller_abort(&service->backup_controller, result);
    robot_chassis_upgrade_controller_abort(&service->upgrade_controller, result);
    pthread_mutex_unlock(&service->transfer_mutex);
    if (backup_active && service->config.backup_callback != NULL) {
        service->config.backup_callback(result, NULL, service->config.user_data);
    }
    if (upgrade_active && service->config.upgrade_callback != NULL) {
        service->config.upgrade_callback(result, service->config.user_data);
    }
}
