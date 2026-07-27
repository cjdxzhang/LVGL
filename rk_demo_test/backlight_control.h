#ifndef BACKLIGHT_CONTROL_H
#define BACKLIGHT_CONTROL_H

#include <stdbool.h>

bool backlight_control_sleep(void);
bool backlight_control_wake(void);
void backlight_control_deinit(void);

#endif
