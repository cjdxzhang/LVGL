#ifndef ROBOT_CHASSIS_TRANSACTION_H
#define ROBOT_CHASSIS_TRANSACTION_H

#include <pthread.h>

#include "robot_chassis_types.h"

#define ROBOT_CHASSIS_PENDING_CAPACITY 16U

typedef struct {
    bool used;
    bool sent;
    int t;
    uint64_t sent_at_ms;
} robot_chassis_pending_entry_t;

typedef struct {
    robot_chassis_pending_entry_t entries[ROBOT_CHASSIS_PENDING_CAPACITY];
    robot_chassis_transfer_kind_t active_transfer;
    pthread_mutex_t mutex;
} robot_chassis_transaction_t;

int robot_chassis_transaction_init(robot_chassis_transaction_t *transaction);
void robot_chassis_transaction_deinit(robot_chassis_transaction_t *transaction);
int robot_chassis_transaction_reserve_t(robot_chassis_transaction_t *transaction, int t);
int robot_chassis_transaction_mark_sent(robot_chassis_transaction_t *transaction,
                                        int t, uint64_t sent_at_ms);
int robot_chassis_transaction_complete_t(robot_chassis_transaction_t *transaction, int t);
int robot_chassis_transaction_cancel_t(robot_chassis_transaction_t *transaction, int t);
size_t robot_chassis_transaction_expire_sent(robot_chassis_transaction_t *transaction,
                                             uint64_t now_ms,
                                             uint64_t timeout_ms);
void robot_chassis_transaction_clear(robot_chassis_transaction_t *transaction);
bool robot_chassis_transaction_has_t(robot_chassis_transaction_t *transaction, int t);
bool robot_chassis_transaction_allows_t(robot_chassis_transaction_t *transaction, int t);
int robot_chassis_transaction_begin_transfer(robot_chassis_transaction_t *transaction,
                                             robot_chassis_transfer_kind_t kind);
void robot_chassis_transaction_end_transfer(robot_chassis_transaction_t *transaction,
                                            robot_chassis_transfer_kind_t kind);
robot_chassis_transfer_kind_t robot_chassis_transaction_active_transfer(
    robot_chassis_transaction_t *transaction);

#endif
