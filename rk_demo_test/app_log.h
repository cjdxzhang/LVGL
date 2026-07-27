#ifndef APP_LOG_H
#define APP_LOG_H

void app_log_user(const char *module, const char *fmt, ...);
void app_log_warn(const char *module, const char *fmt, ...);
void app_log_error(const char *module, const char *fmt, ...);

#define APP_LOG_USER(module, ...) app_log_user((module), __VA_ARGS__)
#define APP_LOG_WARN(module, ...) app_log_warn((module), __VA_ARGS__)
#define APP_LOG_ERROR(module, ...) app_log_error((module), __VA_ARGS__)

#endif
