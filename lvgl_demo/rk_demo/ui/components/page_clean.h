#ifndef PAGE_CLEAN_H
#define PAGE_CLEAN_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    lv_obj_t * page_confirm;
    lv_obj_t * page_running;
    lv_obj_t * page_finish;
} page_clean_set_t;

page_clean_set_t page_clean_create(lv_obj_t * parent,
                                   void (*show_page_cb)(lv_obj_t * page),
                                   lv_obj_t * page_main);

void page_clean_set_countdown(int min, int sec);
void page_clean_set_countdown_seconds(int total_sec);

#ifdef __cplusplus
}
#endif

#endif /* PAGE_CLEAN_H */
