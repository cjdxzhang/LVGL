#ifndef PAGE_LOCATION_H
#define PAGE_LOCATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

typedef struct {
    lv_obj_t * page_location_main;
} page_location_set_t;

page_location_set_t page_location_create(lv_obj_t * parent,
                                         void (*show_page_cb)(lv_obj_t * page),
                                         lv_obj_t * page_main);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PAGE_LOCATION_H */
