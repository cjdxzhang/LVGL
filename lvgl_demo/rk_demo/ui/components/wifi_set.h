#ifndef WIFI_SET_H
#define WIFI_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

lv_obj_t * wifi_set_create(lv_obj_t * parent);
void wifi_set_set_back_cb(lv_event_cb_t cb);
void wifi_set_set_home_cb(lv_event_cb_t cb);
lv_obj_t * wifi_set_get_root(void);
void wifi_set_destroy(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WIFI_SET_H */
