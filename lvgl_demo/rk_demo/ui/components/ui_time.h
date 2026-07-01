#ifndef UI_TIME_H
#define UI_TIME_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define UI_TIME_TEXT_LEN 17

bool ui_time_format(const struct tm *tm_value, char *buf, size_t buf_len);
bool ui_time_format_now(char *buf, size_t buf_len);

#endif
