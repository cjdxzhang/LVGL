#include "ui_time.h"

#include <stdio.h>

bool ui_time_format(const struct tm *tm_value, char *buf, size_t buf_len)
{
    int written;

    if (tm_value == NULL || buf == NULL || buf_len == 0) {
        return false;
    }

    written = snprintf(buf, buf_len, "%04d-%02d-%02d %02d:%02d",
                       tm_value->tm_year + 1900,
                       tm_value->tm_mon + 1,
                       tm_value->tm_mday,
                       tm_value->tm_hour,
                       tm_value->tm_min);
    return written > 0 && (size_t)written < buf_len;
}

bool ui_time_format_now(char *buf, size_t buf_len)
{
    time_t now;
    struct tm tm_value;

    now = time(NULL);
    if (localtime_r(&now, &tm_value) == NULL) {
        return false;
    }

    return ui_time_format(&tm_value, buf, buf_len);
}
