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



void setup_scr_conf_other(lv_ui *ui)
{
    //Write codes conf_other
    ui->conf_other = lv_obj_create(NULL);
    lv_obj_set_size(ui->conf_other, 800, 480);
    lv_obj_set_scrollbar_mode(ui->conf_other, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_other, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_other, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_img_1
    ui->conf_other_img_1 = lv_image_create(ui->conf_other);
    lv_obj_set_pos(ui->conf_other_img_1, 0, 0);
    lv_obj_set_size(ui->conf_other_img_1, 800, 480);
    lv_obj_add_flag(ui->conf_other_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_other_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->conf_other_img_1, 50,50);
    lv_image_set_rotation(ui->conf_other_img_1, 0);

    //Write style for conf_other_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_other_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_other_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_cont_1
    ui->conf_other_cont_1 = lv_obj_create(ui->conf_other);
    lv_obj_set_pos(ui->conf_other_cont_1, 14, 61);
    lv_obj_set_size(ui->conf_other_cont_1, 577, 403);
    lv_obj_set_scrollbar_mode(ui->conf_other_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_other_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->conf_other_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->conf_other_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->conf_other_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_cont_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_cont_1, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_label_1
    ui->conf_other_label_1 = lv_label_create(ui->conf_other_cont_1);
    lv_obj_set_pos(ui->conf_other_label_1, 30, 14);
    lv_obj_set_size(ui->conf_other_label_1, 518, 34);
    lv_label_set_text(ui->conf_other_label_1, "声光时钟");
    lv_label_set_long_mode(ui->conf_other_label_1, LV_LABEL_LONG_WRAP);

    //Write style for conf_other_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_other_label_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_other_label_1, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_other_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_other_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_cont_2
    ui->conf_other_cont_2 = lv_obj_create(ui->conf_other_cont_1);
    lv_obj_set_pos(ui->conf_other_cont_2, 25, 71);
    lv_obj_set_size(ui->conf_other_cont_2, 527, 296);
    lv_obj_set_scrollbar_mode(ui->conf_other_cont_2, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_other_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_line_1
    ui->conf_other_line_1 = lv_line_create(ui->conf_other_cont_1);
    lv_obj_set_pos(ui->conf_other_line_1, 28, 62);
    lv_obj_set_size(ui->conf_other_line_1, 527, 1);
    static lv_point_precise_t conf_other_line_1[] = {{0, 0},{527, 0},{0, 0}};
    lv_line_set_points(ui->conf_other_line_1, conf_other_line_1, 3);

    //Write style for conf_other_line_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->conf_other_line_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->conf_other_line_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->conf_other_line_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->conf_other_line_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_cont_3
    ui->conf_other_cont_3 = lv_obj_create(ui->conf_other_cont_1);
    lv_obj_set_pos(ui->conf_other_cont_3, 57, 91);
    lv_obj_set_size(ui->conf_other_cont_3, 472, 288);
    lv_obj_set_scrollbar_mode(ui->conf_other_cont_3, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_other_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_cont_4
    ui->conf_other_cont_4 = lv_obj_create(ui->conf_other_cont_3);
    lv_obj_set_pos(ui->conf_other_cont_4, 0, 0);
    lv_obj_set_size(ui->conf_other_cont_4, 459, 73);
    lv_obj_set_scrollbar_mode(ui->conf_other_cont_4, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_other_cont_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_cont_4, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_cont_4, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_cont_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_cont_4, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_cont_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_label_2
    ui->conf_other_label_2 = lv_label_create(ui->conf_other_cont_4);
    lv_obj_set_pos(ui->conf_other_label_2, 20, 21);
    lv_obj_set_size(ui->conf_other_label_2, 157, 35);
    lv_label_set_text(ui->conf_other_label_2, "基站氛围灯");
    lv_label_set_long_mode(ui->conf_other_label_2, LV_LABEL_LONG_WRAP);

    //Write style for conf_other_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_other_label_2, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_other_label_2, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_other_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_other_label_2, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_sw_1
    ui->conf_other_sw_1 = lv_switch_create(ui->conf_other_cont_4);
    lv_obj_set_pos(ui->conf_other_sw_1, 370, 24);
    lv_obj_set_size(ui->conf_other_sw_1, 65, 34);

    //Write style for conf_other_sw_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_other_sw_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_sw_1, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_sw_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_other_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_sw_1, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_sw_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_other_sw_1, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->conf_other_sw_1, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->conf_other_sw_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_sw_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->conf_other_sw_1, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for conf_other_sw_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_other_sw_1, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_sw_1, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_sw_1, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_other_sw_1, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_sw_1, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes conf_other_cont_5
    ui->conf_other_cont_5 = lv_obj_create(ui->conf_other_cont_3);
    lv_obj_set_pos(ui->conf_other_cont_5, -2, 91);
    lv_obj_set_size(ui->conf_other_cont_5, 459, 73);
    lv_obj_set_scrollbar_mode(ui->conf_other_cont_5, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_other_cont_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_cont_5, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_cont_5, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_cont_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_cont_5, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_cont_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_label_3
    ui->conf_other_label_3 = lv_label_create(ui->conf_other_cont_5);
    lv_obj_set_pos(ui->conf_other_label_3, 22, 20);
    lv_obj_set_size(ui->conf_other_label_3, 222, 36);
    lv_label_set_text(ui->conf_other_label_3, "语音播报");
    lv_label_set_long_mode(ui->conf_other_label_3, LV_LABEL_LONG_WRAP);

    //Write style for conf_other_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_other_label_3, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_other_label_3, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_other_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_other_label_3, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_sw_2
    ui->conf_other_sw_2 = lv_switch_create(ui->conf_other_cont_5);
    lv_obj_set_pos(ui->conf_other_sw_2, 370, 24);
    lv_obj_set_size(ui->conf_other_sw_2, 65, 34);

    //Write style for conf_other_sw_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_other_sw_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_sw_2, lv_color_hex(0xe6e2e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_sw_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_other_sw_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_sw_2, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_sw_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_other_sw_2, Part: LV_PART_INDICATOR, State: LV_STATE_CHECKED.
    lv_obj_set_style_bg_opa(ui->conf_other_sw_2, 255, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui->conf_other_sw_2, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_sw_2, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_CHECKED);
    lv_obj_set_style_border_width(ui->conf_other_sw_2, 0, LV_PART_INDICATOR|LV_STATE_CHECKED);

    //Write style for conf_other_sw_2, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_other_sw_2, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_sw_2, lv_color_hex(0xffffff), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_sw_2, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_other_sw_2, 0, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_sw_2, 10, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes conf_other_cont_6
    ui->conf_other_cont_6 = lv_obj_create(ui->conf_other_cont_3);
    lv_obj_set_pos(ui->conf_other_cont_6, -4, 187);
    lv_obj_set_size(ui->conf_other_cont_6, 459, 73);
    lv_obj_set_scrollbar_mode(ui->conf_other_cont_6, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_other_cont_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_cont_6, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_cont_6, 195, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_other_cont_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_other_cont_6, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_label_4
    ui->conf_other_label_4 = lv_label_create(ui->conf_other_cont_6);
    lv_obj_set_pos(ui->conf_other_label_4, 22, 20);
    lv_obj_set_size(ui->conf_other_label_4, 151, 36);
    lv_label_set_text(ui->conf_other_label_4, "时间同步");
    lv_label_set_long_mode(ui->conf_other_label_4, LV_LABEL_LONG_WRAP);

    //Write style for conf_other_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_other_label_4, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_other_label_4, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_other_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_other_label_4, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_label_5
    ui->conf_other_label_5 = lv_label_create(ui->conf_other_cont_6);
    lv_obj_set_pos(ui->conf_other_label_5, 160, 20);
    lv_obj_set_size(ui->conf_other_label_5, 247, 36);
    lv_label_set_text(ui->conf_other_label_5, "2026-05-24 09:02");
    lv_label_set_long_mode(ui->conf_other_label_5, LV_LABEL_LONG_WRAP);

    //Write style for conf_other_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_other_label_5, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_other_label_5, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_other_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_other_label_5, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_other_imgbtn_1
    ui->conf_other_imgbtn_1 = lv_imagebutton_create(ui->conf_other);
    lv_obj_set_pos(ui->conf_other_imgbtn_1, 476, 364);
    lv_obj_set_size(ui->conf_other_imgbtn_1, 30, 30);
    lv_obj_add_flag(ui->conf_other_imgbtn_1, LV_OBJ_FLAG_CHECKABLE);
    lv_imagebutton_set_src(ui->conf_other_imgbtn_1, LV_IMAGEBUTTON_STATE_RELEASED, &_timer_refresh_RGB565A8_30x30, NULL, NULL);
    ui->conf_other_imgbtn_1_label = lv_label_create(ui->conf_other_imgbtn_1);
    lv_label_set_text(ui->conf_other_imgbtn_1_label, "");
    lv_label_set_long_mode(ui->conf_other_imgbtn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->conf_other_imgbtn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->conf_other_imgbtn_1, 0, LV_STATE_DEFAULT);

    //Write style for conf_other_imgbtn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_other_imgbtn_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_other_imgbtn_1, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_other_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_other_imgbtn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_other_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_other_imgbtn_1, Part: LV_PART_MAIN, State: LV_STATE_PRESSED.
    lv_obj_set_style_image_recolor_opa(ui->conf_other_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_PRESSED);
    lv_obj_set_style_image_opa(ui->conf_other_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_PRESSED);
    lv_obj_set_style_text_color(ui->conf_other_imgbtn_1, lv_color_hex(0xFF33FF), LV_PART_MAIN|LV_STATE_PRESSED);
    lv_obj_set_style_text_font(ui->conf_other_imgbtn_1, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_PRESSED);
    lv_obj_set_style_text_opa(ui->conf_other_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(ui->conf_other_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_PRESSED);

    //Write style for conf_other_imgbtn_1, Part: LV_PART_MAIN, State: LV_STATE_CHECKED.
    lv_obj_set_style_image_recolor_opa(ui->conf_other_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_image_opa(ui->conf_other_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_text_color(ui->conf_other_imgbtn_1, lv_color_hex(0xFF33FF), LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_text_font(ui->conf_other_imgbtn_1, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_text_opa(ui->conf_other_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(ui->conf_other_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_CHECKED);

    //Write style for conf_other_imgbtn_1, Part: LV_PART_MAIN, State: LV_IMAGEBUTTON_STATE_RELEASED.
    lv_obj_set_style_image_recolor_opa(ui->conf_other_imgbtn_1, 0, LV_PART_MAIN|LV_IMAGEBUTTON_STATE_RELEASED);
    lv_obj_set_style_image_opa(ui->conf_other_imgbtn_1, 255, LV_PART_MAIN|LV_IMAGEBUTTON_STATE_RELEASED);

    //The custom code of conf_other.


    //Update current screen layout.
    lv_obj_update_layout(ui->conf_other);

    //Init events for screen.
    events_init_conf_other(ui);
}
