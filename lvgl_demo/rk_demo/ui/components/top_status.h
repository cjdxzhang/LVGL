#ifndef TOOP_STATUS_H
#define TOOP_STATUS_H

#include "lvgl/lvgl.h"
#include <stdint.h>

void status_bar_create(lv_obj_t * parent);

void status_bar_set_wifi(uint8_t level);
void status_bar_set_connect(uint8_t state);
void status_bar_set_voice(uint8_t state);
void status_bar_set_battery(uint8_t level);
void status_bar_set_time(const char * text);

#endif

