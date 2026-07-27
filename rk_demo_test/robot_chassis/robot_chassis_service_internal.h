#ifndef ROBOT_CHASSIS_SERVICE_INTERNAL_H
#define ROBOT_CHASSIS_SERVICE_INTERNAL_H

#include "robot_chassis_backup_controller.h"
#include "robot_chassis_client.h"
#include "robot_chassis_framer.h"
#include "robot_chassis_heartbeat.h"
#include "robot_chassis_mapping.h"
#include "robot_chassis_queue.h"
#include "robot_chassis_service.h"
#include "robot_chassis_transaction.h"
#include "robot_chassis_upgrade_controller.h"

typedef struct {
    bool initialized;
    bool running;
    bool thread_started;
    pthread_t thread;
    pthread_mutex_t state_mutex;
    pthread_mutex_t submission_mutex;
    pthread_mutex_t transfer_mutex;
    robot_chassis_service_config_t config;
    robot_chassis_client_t client;
    robot_chassis_framer_t framer;
    robot_chassis_queue_t queue;
    robot_chassis_transaction_t transaction;
    robot_chassis_heartbeat_t heartbeat;
    robot_chassis_mapping_poll_t mapping_poll;
    robot_chassis_backup_controller_t backup_controller;
    robot_chassis_upgrade_controller_t upgrade_controller;
} robot_chassis_service_context_t;

extern robot_chassis_service_context_t g_robot_chassis_service;

uint64_t robot_chassis_service_now_ms(void);
bool robot_chassis_service_is_running(void);
void robot_chassis_service_notify_connection(void);
void robot_chassis_service_disconnect(uint64_t now_ms);
int robot_chassis_service_handle_frame(const char *json, size_t length,
                                       void *user_data);
void *robot_chassis_service_thread_main(void *argument);
int robot_chassis_service_poll_mapping(uint64_t now_ms);
int robot_chassis_service_handle_realtime_map(const char *json, size_t length);
int robot_chassis_service_handle_map_list(const char *json, size_t length);
void robot_chassis_service_transfers_init(void);
int robot_chassis_service_handle_backup_response(const char *json, size_t length);
int robot_chassis_service_handle_upgrade_response(const char *json, size_t length);
void robot_chassis_service_handle_transfer_timeouts(uint64_t now_ms);
void robot_chassis_service_abort_transfers(int result);

#endif
