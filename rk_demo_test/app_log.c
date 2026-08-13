#include "app_log.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

#include <lvgl/lvgl.h>

#define APP_LOG_BODY_SIZE 1280u
#define APP_LOG_MESSAGE_SIZE (APP_LOG_BODY_SIZE + 64u)
#define APP_LOG_FILE_PATH "/var/log/messages"

typedef enum
{
    APP_LOG_LEVEL_USER = 0,
    APP_LOG_LEVEL_WARN,
    APP_LOG_LEVEL_ERROR,
} app_log_level_t;

static pthread_mutex_t g_app_log_file_mutex = PTHREAD_MUTEX_INITIALIZER;

static void app_log_append_message(const char *message)
{
    FILE *fp;

    if (message == NULL)
    {
        return;
    }

    pthread_mutex_lock(&g_app_log_file_mutex);
    fp = fopen(APP_LOG_FILE_PATH, "a");
    if (fp != NULL)
    {
        fprintf(fp, "rk_demo_test: %s\n", message);
        fclose(fp);
    }
    pthread_mutex_unlock(&g_app_log_file_mutex);
}

static void app_log_vwrite(app_log_level_t level, const char *module,
                           const char *fmt, va_list args)
{
    char body[APP_LOG_BODY_SIZE];
    char message[APP_LOG_MESSAGE_SIZE];
    int written;

    if (module == NULL || fmt == NULL)
    {
        return;
    }

    written = vsnprintf(body, sizeof(body), fmt, args);
    if (written < 0)
    {
        return;
    }

    written = snprintf(message, sizeof(message), "[%s] %s", module, body);
    if (written < 0)
    {
        return;
    }

    switch (level)
    {
    case APP_LOG_LEVEL_USER:
        LV_LOG_USER("%s", message);
        break;
    case APP_LOG_LEVEL_WARN:
        LV_LOG_WARN("%s", message);
        break;
    case APP_LOG_LEVEL_ERROR:
        LV_LOG_ERROR("%s", message);
        break;
    default:
        return;
    }

    app_log_append_message(message);
}

void app_log_user(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    app_log_vwrite(APP_LOG_LEVEL_USER, module, fmt, args);
    va_end(args);
}

void app_log_warn(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    app_log_vwrite(APP_LOG_LEVEL_WARN, module, fmt, args);
    va_end(args);
}

void app_log_error(const char *module, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    app_log_vwrite(APP_LOG_LEVEL_ERROR, module, fmt, args);
    va_end(args);
}
