/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_location(lv_ui *ui)
{
    //Write codes location
    ui->location = lv_obj_create(NULL);
    lv_obj_set_size(ui->location, 800, 480);
    lv_obj_set_scrollbar_mode(ui->location, LV_SCROLLBAR_MODE_OFF);

    //Write style for location, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->location, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes location_img_1
    ui->location_img_1 = lv_image_create(ui->location);
    lv_obj_set_pos(ui->location_img_1, 0, 0);
    lv_obj_set_size(ui->location_img_1, 800, 480);
    lv_obj_add_flag(ui->location_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->location_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->location_img_1, 50,50);
    lv_image_set_rotation(ui->location_img_1, 0);

    //Write style for location_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->location_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->location_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes location_cont_1
    ui->location_cont_1 = lv_obj_create(ui->location);
    lv_obj_set_pos(ui->location_cont_1, 14, 59);
    lv_obj_set_size(ui->location_cont_1, 380, 152);
    lv_obj_set_scrollbar_mode(ui->location_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for location_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->location_cont_1, 50, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->location_cont_1, 79, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->location_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->location_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes location_label_moving_tips
    ui->location_label_moving_tips = lv_label_create(ui->location_cont_1);
    lv_obj_set_pos(ui->location_label_moving_tips, 130, 63);
    lv_obj_set_size(ui->location_label_moving_tips, 269, 53);
    lv_label_set_text(ui->location_label_moving_tips, "移动至");
    lv_label_set_long_mode(ui->location_label_moving_tips, LV_LABEL_LONG_WRAP);

    //Write style for location_label_moving_tips, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->location_label_moving_tips, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->location_label_moving_tips, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->location_label_moving_tips, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->location_label_moving_tips, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->location_label_moving_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes location_img_2
    ui->location_img_2 = lv_image_create(ui->location_cont_1);
    lv_obj_set_pos(ui->location_img_2, 13, 25);
    lv_obj_set_size(ui->location_img_2, 100, 100);
    lv_obj_add_flag(ui->location_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->location_img_2, &_local_RGB565A8_100x100);
    lv_image_set_pivot(ui->location_img_2, 100,50);
    lv_image_set_rotation(ui->location_img_2, 0);

    //Write style for location_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->location_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->location_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes location_cont_2
    ui->location_cont_2 = lv_obj_create(ui->location);
    lv_obj_set_pos(ui->location_cont_2, 421, 59);
    lv_obj_set_size(ui->location_cont_2, 372, 409);
    lv_obj_set_scrollbar_mode(ui->location_cont_2, LV_SCROLLBAR_MODE_AUTO);

    //Write style for location_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of location.


    //Update current screen layout.
    lv_obj_update_layout(ui->location);

    //Init events for screen.
    events_init_location(ui);
}
