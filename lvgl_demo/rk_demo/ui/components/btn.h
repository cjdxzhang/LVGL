#ifndef BTN_H
#define BTN_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BTN_EVENT_MODE_SLEEP = 1,
    BTN_EVENT_MODE_RELAX,
    BTN_EVENT_MODE_WARM,
    BTN_EVENT_MODE_CLEAN,
    BTN_EVENT_START,
    BTN_EVENT_BATH1,
    BTN_EVENT_BATH2,
    BTN_EVENT_LOCATE,
    BTN_EVENT_SETTING,
    BTN_EVENT_BACK_HOME,
} btn_event_id_t;

typedef void (*btn_event_cb_t)(btn_event_id_t id);

void btn_set_event_callback(btn_event_cb_t cb);

void btn_create_left_group(lv_obj_t * parent);
void btn_create_right_page1(lv_obj_t * parent);
void btn_create_right_page2(lv_obj_t * parent);

#ifdef __cplusplus
}
#endif

#endif /* BTN_H */
