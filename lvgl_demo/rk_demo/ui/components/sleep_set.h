#ifndef SLEEP_SET_H
#define SLEEP_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

lv_obj_t * sleep_set_create(lv_obj_t * parent);
void sleep_set_set_back_cb(lv_event_cb_t cb);
void sleep_set_set_home_cb(lv_event_cb_t cb);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SLEEP_SET_H */
