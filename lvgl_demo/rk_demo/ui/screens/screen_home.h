#ifndef SCREEN_HOME_H
#define SCREEN_HOME_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

void screen_home_create(void);
void screen_home_set_clean_pending(bool pending);

#endif
