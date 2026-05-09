#ifndef SETTING_1_H
#define SETTING_1_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

typedef void (*setting_1_back_cb_t)(void);
typedef void (*setting_1_home_cb_t)(void);

lv_obj_t * setting_1_create(lv_obj_t * parent);
void setting_1_set_back_callback(setting_1_back_cb_t cb);
void setting_1_set_home_callback(setting_1_home_cb_t cb);
lv_obj_t * setting_1_get_root(void);
void setting_1_destroy(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SETTING_1_H */

