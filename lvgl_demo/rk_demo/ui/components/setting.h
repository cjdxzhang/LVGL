#ifndef SETTING_H
#define SETTING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

typedef void (*setting_back_cb_t)(void);

typedef enum {
    SETTING_ITEM_MODE = 0,
    SETTING_ITEM_CLOCK,
    SETTING_ITEM_WLAN,
    SETTING_ITEM_LOCATION,
    SETTING_ITEM_ADVANCED,
    SETTING_ITEM_TBD,
} setting_item_id_t;

typedef void (*setting_item_cb_t)(setting_item_id_t item_id);

void setting_set_item_callback(setting_item_cb_t cb);

/* 创建设置页，parent 一般传 lv_scr_act() */
lv_obj_t * setting_create(lv_obj_t * parent);

/* 注册返回回调：点击设置页右下角返回按钮时触发 */
void setting_set_back_callback(setting_back_cb_t cb);

/* 获取当前设置页根对象 */
lv_obj_t * setting_get_root(void);

/* 删除设置页 */
void setting_destroy(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SETTING_H */