#include "robot_chassis_save.h"

#include <string.h>

static bool has_yaml_suffix(const char *name)
{
    const size_t length = strlen(name);

    return length >= 5U && strcmp(name + length - 5U, ".yaml") == 0;
}

static bool list_contains(const robot_chassis_map_list_t *map_list,
                          const char *target)
{
    size_t index;

    if (map_list == NULL || target == NULL) {
        return false;
    }
    for (index = 0U; index < map_list->count; ++index) {
        if (map_list->names[index] != NULL &&
            strcmp(map_list->names[index], target) == 0) {
            return true;
        }
    }
    return false;
}

void robot_chassis_save_confirmation_init(
    robot_chassis_save_confirmation_t *confirmation)
{
    if (confirmation != NULL) {
        memset(confirmation, 0, sizeof(*confirmation));
    }
}

int robot_chassis_save_confirmation_start(
    robot_chassis_save_confirmation_t *confirmation, const char *map_name,
    uint64_t now_ms)
{
    size_t length;

    if (confirmation == NULL || map_name == NULL || map_name[0] == '\0') {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    length = strlen(map_name);
    if (length + (has_yaml_suffix(map_name) ? 1U : 6U) >
        sizeof(confirmation->target_map)) {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }

    robot_chassis_save_confirmation_init(confirmation);
    memcpy(confirmation->target_map, map_name, length + 1U);
    if (!has_yaml_suffix(map_name)) {
        memcpy(confirmation->target_map + length, ".yaml", 6U);
    }
    confirmation->active = true;
    confirmation->next_query_ms =
        now_ms + ROBOT_CHASSIS_SAVE_CONFIRM_INTERVAL_MS;
    return ROBOT_CHASSIS_OK;
}

robot_chassis_save_action_t robot_chassis_save_confirmation_poll(
    robot_chassis_save_confirmation_t *confirmation, uint64_t now_ms)
{
    if (confirmation == NULL || !confirmation->active ||
        confirmation->waiting_response ||
        now_ms < confirmation->next_query_ms) {
        return ROBOT_CHASSIS_SAVE_ACTION_NONE;
    }
    confirmation->waiting_response = true;
    confirmation->query_attempts++;
    return ROBOT_CHASSIS_SAVE_ACTION_QUERY_LIST;
}

robot_chassis_save_action_t robot_chassis_save_confirmation_accept_list(
    robot_chassis_save_confirmation_t *confirmation,
    const robot_chassis_map_list_t *map_list, uint64_t now_ms)
{
    if (confirmation == NULL || map_list == NULL || !confirmation->active ||
        !confirmation->waiting_response) {
        return ROBOT_CHASSIS_SAVE_ACTION_NONE;
    }
    confirmation->waiting_response = false;
    if (list_contains(map_list, confirmation->target_map)) {
        confirmation->active = false;
        return ROBOT_CHASSIS_SAVE_ACTION_SUCCEEDED;
    }
    if (confirmation->query_attempts >=
        ROBOT_CHASSIS_SAVE_CONFIRM_MAX_ATTEMPTS) {
        confirmation->active = false;
        return ROBOT_CHASSIS_SAVE_ACTION_FAILED;
    }
    confirmation->next_query_ms =
        now_ms + ROBOT_CHASSIS_SAVE_CONFIRM_INTERVAL_MS;
    return ROBOT_CHASSIS_SAVE_ACTION_NONE;
}

void robot_chassis_save_confirmation_cancel(
    robot_chassis_save_confirmation_t *confirmation)
{
    robot_chassis_save_confirmation_init(confirmation);
}
