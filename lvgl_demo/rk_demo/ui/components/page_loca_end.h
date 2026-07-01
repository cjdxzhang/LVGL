#ifndef PAGE_LOCA_END_H
#define PAGE_LOCA_END_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

lv_obj_t * page_loca_end_create(lv_obj_t * parent,
                                void (*show_page_cb)(lv_obj_t * page),
                                lv_obj_t * page_main);

void page_loca_end_show(void);
void page_loca_end_stop_auto_back(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PAGE_LOCA_END_H */
