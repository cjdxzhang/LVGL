#include "ui_label.h"

#include <stdbool.h>
#include <stddef.h>

static bool ui_label_parameters_are_valid(const lv_obj_t *parent,
        const ui_label_style_t *style)
{
    return parent != NULL && lv_obj_is_valid(parent) &&
           style != NULL && style->font != NULL;
}

static void ui_label_set_content_and_style(lv_obj_t *label, const char *text,
        const ui_label_style_t *style)
{
    lv_label_set_text(label, text != NULL ? text : "");
    lv_label_set_long_mode(label, style->long_mode);
    lv_obj_set_style_text_color(label, style->color,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, style->font,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    if (style->text_align != LV_TEXT_ALIGN_LEFT)
    {
        lv_obj_set_style_text_align(label, style->text_align,
                                    LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

lv_obj_t *ui_label_create_common(lv_obj_t *parent, const char *text,
                                 const ui_label_fixed_config_t *config)
{
    lv_obj_t *label;

    if (config == NULL ||
            !ui_label_parameters_are_valid(parent, &config->style) ||
            config->width <= 0 || config->height <= 0)
    {
        return NULL;
    }

    label = lv_label_create(parent);
    if (label == NULL)
    {
        return NULL;
    }
    lv_obj_set_pos(label, config->x, config->y);
    lv_obj_set_size(label, config->width, config->height);
    ui_label_set_content_and_style(label, text, &config->style);
    return label;
}

lv_obj_t *ui_label_create_button(lv_obj_t *parent, const char *text,
                                 const ui_label_button_config_t *config)
{
    lv_obj_t *label;

    if (config == NULL ||
            !ui_label_parameters_are_valid(parent, &config->style) ||
            config->width <= 0)
    {
        return NULL;
    }

    label = lv_label_create(parent);
    if (label == NULL)
    {
        return NULL;
    }
    lv_obj_set_width(label, config->width);
    ui_label_set_content_and_style(label, text, &config->style);
    lv_obj_align(label, config->align, config->x_offset, config->y_offset);
    return label;
}
