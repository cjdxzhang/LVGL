#ifndef CLK_LIGHT_H
#define CLK_LIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

lv_obj_t * clk_light_create(lv_obj_t * parent);
void clk_light_set_back_cb(lv_event_cb_t cb);
void clk_light_set_home_cb(lv_event_cb_t cb);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CLK_LIGHT_H */
