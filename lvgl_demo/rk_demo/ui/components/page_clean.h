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

typedef enum {
    PAGE_CLEAN_STEP_DRAIN_1 = 0,
    PAGE_CLEAN_STEP_SPRAY_1,
    PAGE_CLEAN_STEP_DRAIN_2,
    PAGE_CLEAN_STEP_SPRAY_2,
    PAGE_CLEAN_STEP_DRAIN_3,
    PAGE_CLEAN_STEP_DRY,
    PAGE_CLEAN_STEP_COUNT,
    PAGE_CLEAN_STEP_FINISH = 0xFF
} page_clean_step_t;

page_clean_set_t page_clean_create(lv_obj_t * parent,
                                   void (*show_page_cb)(lv_obj_t * page),
                                   lv_obj_t * page_main);

void page_clean_set_step(page_clean_step_t step);

#ifdef __cplusplus
}
#endif

#endif /* PAGE_CLEAN_H */
