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



void setup_scr_conf_advance(lv_ui *ui)
{
    //Write codes conf_advance
    ui->conf_advance = lv_obj_create(NULL);
    lv_obj_set_size(ui->conf_advance, 800, 480);
    lv_obj_set_scrollbar_mode(ui->conf_advance, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_advance, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_img_1
    ui->conf_advance_img_1 = lv_image_create(ui->conf_advance);
    lv_obj_set_pos(ui->conf_advance_img_1, 0, 0);
    lv_obj_set_size(ui->conf_advance_img_1, 800, 480);
    lv_obj_add_flag(ui->conf_advance_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_advance_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->conf_advance_img_1, 50,50);
    lv_image_set_rotation(ui->conf_advance_img_1, 0);

    //Write style for conf_advance_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_advance_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_advance_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_3
    ui->conf_advance_cont_3 = lv_obj_create(ui->conf_advance);
    lv_obj_set_pos(ui->conf_advance_cont_3, 14, 61);
    lv_obj_set_size(ui->conf_advance_cont_3, 577, 403);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_advance_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_3, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->conf_advance_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->conf_advance_cont_3, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->conf_advance_cont_3, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_3, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_3, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_cont_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_cont_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_4
    ui->conf_advance_label_4 = lv_label_create(ui->conf_advance_cont_3);
    lv_obj_set_pos(ui->conf_advance_label_4, 30, 14);
    lv_obj_set_size(ui->conf_advance_label_4, 518, 34);
    lv_label_set_text(ui->conf_advance_label_4, "模式设置");
    lv_label_set_long_mode(ui->conf_advance_label_4, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_4, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_4
    ui->conf_advance_cont_4 = lv_obj_create(ui->conf_advance_cont_3);
    lv_obj_set_pos(ui->conf_advance_cont_4, 25, 71);
    lv_obj_set_size(ui->conf_advance_cont_4, 527, 296);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_4, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_advance_cont_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_line_1
    ui->conf_advance_line_1 = lv_line_create(ui->conf_advance_cont_3);
    lv_obj_set_pos(ui->conf_advance_line_1, 28, 62);
    lv_obj_set_size(ui->conf_advance_line_1, 527, 1);
    static lv_point_precise_t conf_advance_line_1[] = {{0, 0},{527, 0},{0, 0}};
    lv_line_set_points(ui->conf_advance_line_1, conf_advance_line_1, 3);

    //Write style for conf_advance_line_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->conf_advance_line_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->conf_advance_line_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->conf_advance_line_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->conf_advance_line_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_9
    ui->conf_advance_cont_9 = lv_obj_create(ui->conf_advance_cont_3);
    lv_obj_set_pos(ui->conf_advance_cont_9, 57, 91);
    lv_obj_set_size(ui->conf_advance_cont_9, 472, 288);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_9, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_advance_cont_9, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_2
    ui->conf_advance_cont_2 = lv_obj_create(ui->conf_advance_cont_9);
    lv_obj_set_pos(ui->conf_advance_cont_2, 0, 0);
    lv_obj_set_size(ui->conf_advance_cont_2, 459, 111);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_advance_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_2, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_2, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_3
    ui->conf_advance_label_3 = lv_label_create(ui->conf_advance_cont_2);
    lv_obj_set_pos(ui->conf_advance_label_3, 5, 73);
    lv_obj_set_size(ui->conf_advance_label_3, 400, 22);
    lv_label_set_text(ui->conf_advance_label_3, "（开启后，回到基地自动开启自清洁）");
    lv_label_set_long_mode(ui->conf_advance_label_3, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_3, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_3, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_3, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_2
    ui->conf_advance_label_2 = lv_label_create(ui->conf_advance_cont_2);
    lv_obj_set_pos(ui->conf_advance_label_2, 19, 16);
    lv_obj_set_size(ui->conf_advance_label_2, 352, 53);
    lv_label_set_text(ui->conf_advance_label_2, "返回自动洗烘");
    lv_label_set_long_mode(ui->conf_advance_label_2, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_2, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_2, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_sw_1
    ui->conf_advance_sw_1 = lv_switch_create(ui->conf_advance_cont_2);
    lv_obj_set_pos(ui->conf_advance_sw_1, 370, 24);
    lv_obj_set_size(ui->conf_advance_sw_1, 65, 34);

    //Write style for conf_advance_sw_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_1, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_1, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_advance_sw_1, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_1, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->conf_advance_sw_1, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for conf_advance_sw_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_1, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_1, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_1, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_1, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_1, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_8
    ui->conf_advance_cont_8 = lv_obj_create(ui->conf_advance_cont_9);
    lv_obj_set_pos(ui->conf_advance_cont_8, 0, 533);
    lv_obj_set_size(ui->conf_advance_cont_8, 459, 111);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_8, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_advance_cont_8, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_8, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_8, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_cont_8, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_cont_8, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_12
    ui->conf_advance_label_12 = lv_label_create(ui->conf_advance_cont_8);
    lv_obj_set_pos(ui->conf_advance_label_12, 5, 73);
    lv_obj_set_size(ui->conf_advance_label_12, 459, 22);
    lv_label_set_text(ui->conf_advance_label_12, "（开启后，缺失中药液2，仍可开启自动注水足疗）");
    lv_label_set_long_mode(ui->conf_advance_label_12, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_12, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_12, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_12, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_12, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_12, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_11
    ui->conf_advance_label_11 = lv_label_create(ui->conf_advance_cont_8);
    lv_obj_set_pos(ui->conf_advance_label_11, 19, 16);
    lv_obj_set_size(ui->conf_advance_label_11, 352, 53);
    lv_label_set_text(ui->conf_advance_label_11, "无中药液体2·药浴");
    lv_label_set_long_mode(ui->conf_advance_label_11, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_11, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_11, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_11, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_11, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_sw_5
    ui->conf_advance_sw_5 = lv_switch_create(ui->conf_advance_cont_8);
    lv_obj_set_pos(ui->conf_advance_sw_5, 370, 24);
    lv_obj_set_size(ui->conf_advance_sw_5, 65, 34);

    //Write style for conf_advance_sw_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_5, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_5, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_5, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_sw_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_advance_sw_5, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_5, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_5, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_5, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->conf_advance_sw_5, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for conf_advance_sw_5, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_5, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_5, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_5, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_5, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_5, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_7
    ui->conf_advance_cont_7 = lv_obj_create(ui->conf_advance_cont_9);
    lv_obj_set_pos(ui->conf_advance_cont_7, 0, 400);
    lv_obj_set_size(ui->conf_advance_cont_7, 459, 111);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_7, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_advance_cont_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_7, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_7, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_cont_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_cont_7, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_10
    ui->conf_advance_label_10 = lv_label_create(ui->conf_advance_cont_7);
    lv_obj_set_pos(ui->conf_advance_label_10, 5, 73);
    lv_obj_set_size(ui->conf_advance_label_10, 460, 22);
    lv_label_set_text(ui->conf_advance_label_10, "（开启后，缺失中药液1,，仍可开启自动注水足疗）");
    lv_label_set_long_mode(ui->conf_advance_label_10, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_10, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_10, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_10, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_10, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_9
    ui->conf_advance_label_9 = lv_label_create(ui->conf_advance_cont_7);
    lv_obj_set_pos(ui->conf_advance_label_9, 19, 16);
    lv_obj_set_size(ui->conf_advance_label_9, 352, 53);
    lv_label_set_text(ui->conf_advance_label_9, "无中药液体1·药浴");
    lv_label_set_long_mode(ui->conf_advance_label_9, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_9, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_9, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_9, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_9, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_9, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_sw_4
    ui->conf_advance_sw_4 = lv_switch_create(ui->conf_advance_cont_7);
    lv_obj_set_pos(ui->conf_advance_sw_4, 370, 24);
    lv_obj_set_size(ui->conf_advance_sw_4, 65, 34);

    //Write style for conf_advance_sw_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_4, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_4, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_4, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_sw_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_advance_sw_4, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_4, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_4, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_4, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->conf_advance_sw_4, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for conf_advance_sw_4, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_4, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_4, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_4, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_4, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_4, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_6
    ui->conf_advance_cont_6 = lv_obj_create(ui->conf_advance_cont_9);
    lv_obj_set_pos(ui->conf_advance_cont_6, 0, 267);
    lv_obj_set_size(ui->conf_advance_cont_6, 459, 111);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_6, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_advance_cont_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_6, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_6, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_cont_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_cont_6, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_8
    ui->conf_advance_label_8 = lv_label_create(ui->conf_advance_cont_6);
    lv_obj_set_pos(ui->conf_advance_label_8, 5, 73);
    lv_obj_set_size(ui->conf_advance_label_8, 400, 22);
    lv_label_set_text(ui->conf_advance_label_8, "（开启后，缺失清洁液，仍可开启自动清洗）");
    lv_label_set_long_mode(ui->conf_advance_label_8, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_8, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_8, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_8, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_8, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_7
    ui->conf_advance_label_7 = lv_label_create(ui->conf_advance_cont_6);
    lv_obj_set_pos(ui->conf_advance_label_7, 19, 16);
    lv_obj_set_size(ui->conf_advance_label_7, 352, 53);
    lv_label_set_text(ui->conf_advance_label_7, "无清洁液·自清洗");
    lv_label_set_long_mode(ui->conf_advance_label_7, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_7, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_7, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_7, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_sw_3
    ui->conf_advance_sw_3 = lv_switch_create(ui->conf_advance_cont_6);
    lv_obj_set_pos(ui->conf_advance_sw_3, 370, 24);
    lv_obj_set_size(ui->conf_advance_sw_3, 65, 34);

    //Write style for conf_advance_sw_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_3, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_3, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_sw_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_advance_sw_3, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_3, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_3, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_3, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->conf_advance_sw_3, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for conf_advance_sw_3, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_3, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_3, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_3, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_3, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_3, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes conf_advance_cont_5
    ui->conf_advance_cont_5 = lv_obj_create(ui->conf_advance_cont_9);
    lv_obj_set_pos(ui->conf_advance_cont_5, 0, 134);
    lv_obj_set_size(ui->conf_advance_cont_5, 459, 111);
    lv_obj_set_scrollbar_mode(ui->conf_advance_cont_5, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_advance_cont_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_cont_5, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_cont_5, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_cont_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_cont_5, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_6
    ui->conf_advance_label_6 = lv_label_create(ui->conf_advance_cont_5);
    lv_obj_set_pos(ui->conf_advance_label_6, 5, 73);
    lv_obj_set_size(ui->conf_advance_label_6, 400, 22);
    lv_label_set_text(ui->conf_advance_label_6, "（关闭后，自清洁后不再自动烘干）");
    lv_label_set_long_mode(ui->conf_advance_label_6, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_6, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_6, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_6, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_label_5
    ui->conf_advance_label_5 = lv_label_create(ui->conf_advance_cont_5);
    lv_obj_set_pos(ui->conf_advance_label_5, 19, 16);
    lv_obj_set_size(ui->conf_advance_label_5, 352, 53);
    lv_label_set_text(ui->conf_advance_label_5, "自动烘干足疗桶");
    lv_label_set_long_mode(ui->conf_advance_label_5, LV_LABEL_LONG_WRAP);

    //Write style for conf_advance_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_advance_label_5, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_advance_label_5, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_advance_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_advance_label_5, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_advance_sw_2
    ui->conf_advance_sw_2 = lv_switch_create(ui->conf_advance_cont_5);
    lv_obj_set_pos(ui->conf_advance_sw_2, 370, 24);
    lv_obj_set_size(ui->conf_advance_sw_2, 65, 34);

    //Write style for conf_advance_sw_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_2, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_2, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_advance_sw_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_advance_sw_2, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_2, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_2, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_2, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->conf_advance_sw_2, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for conf_advance_sw_2, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_advance_sw_2, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_advance_sw_2, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_advance_sw_2, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_advance_sw_2, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_advance_sw_2, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //The custom code of conf_advance.


    //Update current screen layout.
    lv_obj_update_layout(ui->conf_advance);

    //Init events for screen.
    events_init_conf_advance(ui);
}
