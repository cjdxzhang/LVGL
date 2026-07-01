#ifndef WATER_ADD_H
#define WATER_ADD_H

#include "lvgl/lvgl.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_obj_t * page_running;
    lv_obj_t * page_confirm;
} page_water_add_set_t;

page_water_add_set_t page_water_add_create(lv_obj_t * parent,
                                           void (*show_page_cb)(lv_obj_t * page),
                                           lv_obj_t * page_main);
void page_water_add_set_info(uint8_t temp,
                             uint16_t timer_minutes,
                             uint8_t water_level,
                             uint8_t herb1,
                             uint8_t herb2);

#ifdef __cplusplus
}
#endif

#endif /* WATER_ADD_H */