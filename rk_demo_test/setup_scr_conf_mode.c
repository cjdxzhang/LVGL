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



void setup_scr_conf_mode(lv_ui *ui)
{
    //Write codes conf_mode
    ui->conf_mode = lv_obj_create(NULL);
    lv_obj_set_size(ui->conf_mode, 800, 480);
    lv_obj_set_scrollbar_mode(ui->conf_mode, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_mode, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_mode, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_img_1
    ui->conf_mode_img_1 = lv_image_create(ui->conf_mode);
    lv_obj_set_pos(ui->conf_mode_img_1, 0, -1);
    lv_obj_set_size(ui->conf_mode_img_1, 800, 480);
    lv_obj_add_flag(ui->conf_mode_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_mode_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->conf_mode_img_1, 50,50);
    lv_image_set_rotation(ui->conf_mode_img_1, 0);

    //Write style for conf_mode_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_mode_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_mode_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_cont_1
    ui->conf_mode_cont_1 = lv_obj_create(ui->conf_mode);
    lv_obj_set_pos(ui->conf_mode_cont_1, 14, 61);
    lv_obj_set_size(ui->conf_mode_cont_1, 577, 403);
    lv_obj_set_scrollbar_mode(ui->conf_mode_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_mode_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->conf_mode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->conf_mode_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->conf_mode_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_cont_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_cont_1, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_label_1
    ui->conf_mode_label_1 = lv_label_create(ui->conf_mode_cont_1);
    lv_obj_set_pos(ui->conf_mode_label_1, 30, 14);
    lv_obj_set_size(ui->conf_mode_label_1, 518, 34);
    lv_label_set_text(ui->conf_mode_label_1, "模式设置");
    lv_label_set_long_mode(ui->conf_mode_label_1, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_label_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_label_1, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_cont_2
    ui->conf_mode_cont_2 = lv_obj_create(ui->conf_mode_cont_1);
    lv_obj_set_pos(ui->conf_mode_cont_2, 25, 71);
    lv_obj_set_size(ui->conf_mode_cont_2, 527, 296);
    lv_obj_set_scrollbar_mode(ui->conf_mode_cont_2, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_mode_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_line_1
    ui->conf_mode_line_1 = lv_line_create(ui->conf_mode_cont_1);
    lv_obj_set_pos(ui->conf_mode_line_1, 28, 62);
    lv_obj_set_size(ui->conf_mode_line_1, 527, 1);
    static lv_point_precise_t conf_mode_line_1[] = {{0, 0},{527, 0},{0, 0}};
    lv_line_set_points(ui->conf_mode_line_1, conf_mode_line_1, 3);

    //Write style for conf_mode_line_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->conf_mode_line_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->conf_mode_line_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->conf_mode_line_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->conf_mode_line_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of conf_mode.


    //Update current screen layout.
    lv_obj_update_layout(ui->conf_mode);

    //Init events for screen.
    events_init_conf_mode(ui);
}
