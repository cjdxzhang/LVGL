#ifndef UI_LABEL_H
#define UI_LABEL_H

#include "lvgl.h"


typedef struct
{
    const lv_font_t *font;
    lv_color_t color;
    lv_label_long_mode_t long_mode;
    lv_text_align_t text_align;
} ui_label_style_t;

typedef struct
{
    lv_coord_t x;
    lv_coord_t y;
    lv_coord_t width;
    lv_coord_t height;
    ui_label_style_t style;
} ui_label_fixed_config_t;

typedef struct
{
    lv_coord_t width;
    lv_align_t align;
    lv_coord_t x_offset;
    lv_coord_t y_offset;
    ui_label_style_t style;
} ui_label_button_config_t;

lv_obj_t *ui_label_create_common(lv_obj_t *parent, const char *text,
                                 const ui_label_fixed_config_t *config);
lv_obj_t *ui_label_create_button(lv_obj_t *parent, const char *text,
                                 const ui_label_button_config_t *config);

#endif
