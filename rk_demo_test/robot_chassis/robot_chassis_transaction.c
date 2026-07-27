#include "robot_chassis_transaction.h"

#include <string.h>

static robot_chassis_pending_entry_t *find_entry(
    robot_chassis_transaction_t *transaction, int t)
{
    size_t index;

    for (index = 0U; index < ROBOT_CHASSIS_PENDING_CAPACITY; ++index) {
        if (transaction->entries[index].used && transaction->entries[index].t == t) {
            return &transaction->entries[index];
        }
    }
    return NULL;
}

static bool transfer_allows_t(robot_chassis_transfer_kind_t kind, int t)
{
    if (t == ROBOT_CHASSIS_T_STATUS) {
        return true;
    }

    switch (kind) {
    case ROBOT_CHASSIS_TRANSFER_MAP_BACKUP:
        return t == ROBOT_CHASSIS_T_MAP_BACKUP_BEGIN ||
               t == ROBOT_CHASSIS_T_MAP_BACKUP_NEXT;
    case ROBOT_CHASSIS_TRANSFER_UPGRADE:
        return t >= ROBOT_CHASSIS_T_UPGRADE_EXTRACT &&
               t <= ROBOT_CHASSIS_T_UPGRADE_FINISH;
    case ROBOT_CHASSIS_TRANSFER_NONE:
    default:
        return true;
    }
}

int robot_chassis_transaction_init(robot_chassis_transaction_t *transaction)
{
    if (transaction == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    memset(transaction, 0, sizeof(*transaction));
    if (pthread_mutex_init(&transaction->mutex, NULL) != 0) {
        return ROBOT_CHASSIS_ERR_IO;
    }
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_transaction_deinit(robot_chassis_transaction_t *transaction)
{
    if (transaction == NULL) {
        return;
    }
    pthread_mutex_destroy(&transaction->mutex);
    memset(transaction, 0, sizeof(*transaction));
}

int robot_chassis_transaction_reserve_t(robot_chassis_transaction_t *transaction,
                                        int t)
{
    size_t index;
    size_t free_index = ROBOT_CHASSIS_PENDING_CAPACITY;

    if (transaction == NULL || t <= 0) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&transaction->mutex);
    for (index = 0U; index < ROBOT_CHASSIS_PENDING_CAPACITY; ++index) {
        if (transaction->entries[index].used && transaction->entries[index].t == t) {
            pthread_mutex_unlock(&transaction->mutex);
            return ROBOT_CHASSIS_ERR_CONFLICT;
        }
        if (!transaction->entries[index].used &&
            free_index == ROBOT_CHASSIS_PENDING_CAPACITY) {
            free_index = index;
        }
    }
    if (free_index == ROBOT_CHASSIS_PENDING_CAPACITY) {
        pthread_mutex_unlock(&transaction->mutex);
        return ROBOT_CHASSIS_ERR_LIMIT;
    }

    transaction->entries[free_index].used = true;
    transaction->entries[free_index].t = t;
    pthread_mutex_unlock(&transaction->mutex);
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_transaction_mark_sent(robot_chassis_transaction_t *transaction,
                                        int t, uint64_t sent_at_ms)
{
    robot_chassis_pending_entry_t *entry;

    if (transaction == NULL || t <= 0) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&transaction->mutex);
    entry = find_entry(transaction, t);
    if (entry == NULL || entry->sent) {
        pthread_mutex_unlock(&transaction->mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    entry->sent = true;
    entry->sent_at_ms = sent_at_ms;
    pthread_mutex_unlock(&transaction->mutex);
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_transaction_complete_t(robot_chassis_transaction_t *transaction,
                                         int t)
{
    robot_chassis_pending_entry_t *entry;

    if (transaction == NULL || t <= 0) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&transaction->mutex);
    entry = find_entry(transaction, t);
    if (entry == NULL || !entry->sent) {
        pthread_mutex_unlock(&transaction->mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    memset(entry, 0, sizeof(*entry));
    pthread_mutex_unlock(&transaction->mutex);
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_transaction_cancel_t(robot_chassis_transaction_t *transaction,
                                       int t)
{
    robot_chassis_pending_entry_t *entry;

    if (transaction == NULL || t <= 0) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&transaction->mutex);
    entry = find_entry(transaction, t);
    if (entry == NULL) {
        pthread_mutex_unlock(&transaction->mutex);
        return ROBOT_CHASSIS_ERR_STATE;
    }
    memset(entry, 0, sizeof(*entry));
    pthread_mutex_unlock(&transaction->mutex);
    return ROBOT_CHASSIS_OK;
}

size_t robot_chassis_transaction_expire_sent(robot_chassis_transaction_t *transaction,
                                             uint64_t now_ms,
                                             uint64_t timeout_ms)
{
    size_t index;
    size_t expired = 0U;

    if (transaction == NULL || timeout_ms == 0U) {
        return 0U;
    }

    pthread_mutex_lock(&transaction->mutex);
    for (index = 0U; index < ROBOT_CHASSIS_PENDING_CAPACITY; ++index) {
        robot_chassis_pending_entry_t *entry = &transaction->entries[index];

        if (entry->used && entry->sent && now_ms >= entry->sent_at_ms &&
            now_ms - entry->sent_at_ms >= timeout_ms) {
            memset(entry, 0, sizeof(*entry));
            expired++;
        }
    }
    pthread_mutex_unlock(&transaction->mutex);
    return expired;
}

void robot_chassis_transaction_clear(robot_chassis_transaction_t *transaction)
{
    if (transaction == NULL) {
        return;
    }

    pthread_mutex_lock(&transaction->mutex);
    memset(transaction->entries, 0, sizeof(transaction->entries));
    transaction->active_transfer = ROBOT_CHASSIS_TRANSFER_NONE;
    pthread_mutex_unlock(&transaction->mutex);
}

bool robot_chassis_transaction_has_t(robot_chassis_transaction_t *transaction,
                                     int t)
{
    bool found;

    if (transaction == NULL || t <= 0) {
        return false;
    }

    pthread_mutex_lock(&transaction->mutex);
    found = find_entry(transaction, t) != NULL;
    pthread_mutex_unlock(&transaction->mutex);
    return found;
}

bool robot_chassis_transaction_allows_t(robot_chassis_transaction_t *transaction,
                                        int t)
{
    bool allowed;

    if (transaction == NULL || t <= 0) {
        return false;
    }

    pthread_mutex_lock(&transaction->mutex);
    allowed = transfer_allows_t(transaction->active_transfer, t);
    pthread_mutex_unlock(&transaction->mutex);
    return allowed;
}

int robot_chassis_transaction_begin_transfer(
    robot_chassis_transaction_t *transaction, robot_chassis_transfer_kind_t kind)
{
    if (transaction == NULL || kind == ROBOT_CHASSIS_TRANSFER_NONE) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&transaction->mutex);
    if (transaction->active_transfer != ROBOT_CHASSIS_TRANSFER_NONE) {
        pthread_mutex_unlock(&transaction->mutex);
        return ROBOT_CHASSIS_ERR_BUSY;
    }
    transaction->active_transfer = kind;
    pthread_mutex_unlock(&transaction->mutex);
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_transaction_end_transfer(
    robot_chassis_transaction_t *transaction, robot_chassis_transfer_kind_t kind)
{
    if (transaction == NULL || kind == ROBOT_CHASSIS_TRANSFER_NONE) {
        return;
    }

    pthread_mutex_lock(&transaction->mutex);
    if (transaction->active_transfer == kind) {
        transaction->active_transfer = ROBOT_CHASSIS_TRANSFER_NONE;
    }
    pthread_mutex_unlock(&transaction->mutex);
}

robot_chassis_transfer_kind_t robot_chassis_transaction_active_transfer(
    robot_chassis_transaction_t *transaction)
{
    robot_chassis_transfer_kind_t kind;

    if (transaction == NULL) {
        return ROBOT_CHASSIS_TRANSFER_NONE;
    }

    pthread_mutex_lock(&transaction->mutex);
    kind = transaction->active_transfer;
    pthread_mutex_unlock(&transaction->mutex);
    return kind;
}
