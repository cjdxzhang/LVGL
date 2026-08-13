#include "robot_chassis_log.h"

#include "../app_log.h"

#include <limits.h>

#define ROBOT_CHASSIS_LOG_MODULE "ROBOT_CHASSIS"
#define ROBOT_CHASSIS_LOG_CONTENT_LIMIT 1024U

static int log_print_length(size_t length)
{
    return length > (size_t)INT_MAX ? INT_MAX : (int)length;
}

void robot_chassis_log_send(const char *json, size_t length)
{
    size_t preview_length;

    if (json == NULL)
    {
        return;
    }
    preview_length = length > ROBOT_CHASSIS_LOG_CONTENT_LIMIT ?
                     ROBOT_CHASSIS_LOG_CONTENT_LIMIT : length;
    if (length > ROBOT_CHASSIS_LOG_CONTENT_LIMIT)
    {
        APP_LOG_USER(ROBOT_CHASSIS_LOG_MODULE,
                     "tcp send len=%zu: %.*s...（已截断）", length,
                     log_print_length(preview_length), json);
        return;
    }
    APP_LOG_USER(ROBOT_CHASSIS_LOG_MODULE, "tcp send len=%zu: %.*s",
                 length, log_print_length(preview_length), json);
}

void robot_chassis_log_send_failed(const char *json, size_t length, int error)
{
    size_t preview_length;

    if (json == NULL)
    {
        return;
    }
    preview_length = length > ROBOT_CHASSIS_LOG_CONTENT_LIMIT ?
                     ROBOT_CHASSIS_LOG_CONTENT_LIMIT : length;
    if (length > ROBOT_CHASSIS_LOG_CONTENT_LIMIT)
    {
        APP_LOG_ERROR(ROBOT_CHASSIS_LOG_MODULE,
                      "tcp send failed len=%zu error=%d: "
                      "%.*s...（已截断）", length, error,
                      log_print_length(preview_length), json);
        return;
    }
    APP_LOG_ERROR(ROBOT_CHASSIS_LOG_MODULE,
                  "tcp send failed len=%zu error=%d: %.*s", length, error,
                  log_print_length(preview_length), json);
}

void robot_chassis_log_receive(const char *json, size_t length)
{
    if (json == NULL)
    {
        return;
    }
    if (length > ROBOT_CHASSIS_LOG_CONTENT_LIMIT)
    {
        APP_LOG_USER(ROBOT_CHASSIS_LOG_MODULE,
                     "tcp recv len=%zu: 大数据已省略", length);
        return;
    }
    APP_LOG_USER(ROBOT_CHASSIS_LOG_MODULE, "tcp recv len=%zu: %.*s",
                 length, log_print_length(length), json);
}
