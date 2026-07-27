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



void setup_scr_setting(lv_ui *ui)
{
    //Write codes setting
    ui->setting = lv_obj_create(NULL);
    lv_obj_set_size(ui->setting, 800, 480);
    lv_obj_set_scrollbar_mode(ui->setting, LV_SCROLLBAR_MODE_OFF);

    //Write style for setting, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->setting, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_img_1
    ui->setting_img_1 = lv_image_create(ui->setting);
    lv_obj_set_pos(ui->setting_img_1, 0, 0);
    lv_obj_set_size(ui->setting_img_1, 800, 480);
    lv_obj_add_flag(ui->setting_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->setting_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->setting_img_1, 50,50);
    lv_image_set_rotation(ui->setting_img_1, 0);

    //Write style for setting_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->setting_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->setting_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_btn_2
    ui->setting_btn_2 = lv_button_create(ui->setting);
    lv_obj_set_pos(ui->setting_btn_2, -568, 59);
    lv_obj_set_size(ui->setting_btn_2, 160, 160);
    ui->setting_btn_2_label = lv_label_create(ui->setting_btn_2);
    lv_label_set_text(ui->setting_btn_2_label, "模式设置");
    lv_label_set_long_mode(ui->setting_btn_2_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->setting_btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->setting_btn_2, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->setting_btn_2_label, LV_PCT(100));

    //Write style for setting_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->setting_btn_2, 74, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->setting_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->setting_btn_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->setting_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_btn_2, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->setting_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->setting_btn_2, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->setting_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->setting_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_btn_3
    ui->setting_btn_3 = lv_button_create(ui->setting);
    lv_obj_set_pos(ui->setting_btn_3, -385, 59);
    lv_obj_set_size(ui->setting_btn_3, 160, 160);
    ui->setting_btn_3_label = lv_label_create(ui->setting_btn_3);
    lv_label_set_text(ui->setting_btn_3_label, "声光时钟");
    lv_label_set_long_mode(ui->setting_btn_3_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->setting_btn_3_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->setting_btn_3, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->setting_btn_3_label, LV_PCT(100));

    //Write style for setting_btn_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->setting_btn_3, 74, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->setting_btn_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->setting_btn_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->setting_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_btn_3, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->setting_btn_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->setting_btn_3, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->setting_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->setting_btn_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_btn_4
    ui->setting_btn_4 = lv_button_create(ui->setting);
    lv_obj_set_pos(ui->setting_btn_4, -196, 59);
    lv_obj_set_size(ui->setting_btn_4, 160, 160);
    ui->setting_btn_4_label = lv_label_create(ui->setting_btn_4);
    lv_label_set_text(ui->setting_btn_4_label, "WIFI设置");
    lv_label_set_long_mode(ui->setting_btn_4_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->setting_btn_4_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->setting_btn_4, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->setting_btn_4_label, LV_PCT(100));

    //Write style for setting_btn_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->setting_btn_4, 74, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->setting_btn_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->setting_btn_4, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->setting_btn_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_btn_4, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_btn_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->setting_btn_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->setting_btn_4, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->setting_btn_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->setting_btn_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_btn_5
    ui->setting_btn_5 = lv_button_create(ui->setting);
    lv_obj_set_pos(ui->setting_btn_5, -568, 257);
    lv_obj_set_size(ui->setting_btn_5, 160, 160);
    ui->setting_btn_5_label = lv_label_create(ui->setting_btn_5);
    lv_label_set_text(ui->setting_btn_5_label, "定位设置");
    lv_label_set_long_mode(ui->setting_btn_5_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->setting_btn_5_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->setting_btn_5, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->setting_btn_5_label, LV_PCT(100));

    //Write style for setting_btn_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->setting_btn_5, 74, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->setting_btn_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->setting_btn_5, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->setting_btn_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_btn_5, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_btn_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->setting_btn_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->setting_btn_5, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->setting_btn_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->setting_btn_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_btn_6
    ui->setting_btn_6 = lv_button_create(ui->setting);
    lv_obj_set_pos(ui->setting_btn_6, -385, 257);
    lv_obj_set_size(ui->setting_btn_6, 160, 160);
    ui->setting_btn_6_label = lv_label_create(ui->setting_btn_6);
    lv_label_set_text(ui->setting_btn_6_label, "高级设置");
    lv_label_set_long_mode(ui->setting_btn_6_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->setting_btn_6_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->setting_btn_6, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->setting_btn_6_label, LV_PCT(100));

    //Write style for setting_btn_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->setting_btn_6, 74, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->setting_btn_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->setting_btn_6, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->setting_btn_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_btn_6, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_btn_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->setting_btn_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->setting_btn_6, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->setting_btn_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->setting_btn_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_btn_7
    ui->setting_btn_7 = lv_button_create(ui->setting);
    lv_obj_set_pos(ui->setting_btn_7, -196, 257);
    lv_obj_set_size(ui->setting_btn_7, 160, 160);
    ui->setting_btn_7_label = lv_label_create(ui->setting_btn_7);
    lv_label_set_text(ui->setting_btn_7_label, "待定");
    lv_label_set_long_mode(ui->setting_btn_7_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->setting_btn_7_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->setting_btn_7, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->setting_btn_7_label, LV_PCT(100));

    //Write style for setting_btn_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->setting_btn_7, 74, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->setting_btn_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->setting_btn_7, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->setting_btn_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_btn_7, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_btn_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->setting_btn_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->setting_btn_7, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->setting_btn_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->setting_btn_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_cont_1
    ui->setting_cont_1 = lv_obj_create(ui->setting);
    lv_obj_set_pos(ui->setting_cont_1, 14, 61);
    lv_obj_set_size(ui->setting_cont_1, 577, 403);
    lv_obj_set_scrollbar_mode(ui->setting_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for setting_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->setting_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->setting_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->setting_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->setting_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_cont_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->setting_cont_1, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->setting_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->setting_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->setting_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->setting_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->setting_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->setting_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_label_1
    ui->setting_label_1 = lv_label_create(ui->setting_cont_1);
    lv_obj_set_pos(ui->setting_label_1, 30, 14);
    lv_obj_set_size(ui->setting_label_1, 518, 34);
    lv_label_set_text(ui->setting_label_1, "设置");
    lv_label_set_long_mode(ui->setting_label_1, LV_LABEL_LONG_WRAP);

    //Write style for setting_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->setting_label_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->setting_label_1, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->setting_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->setting_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_cont_2
    ui->setting_cont_2 = lv_obj_create(ui->setting_cont_1);
    lv_obj_set_pos(ui->setting_cont_2, 25, 71);
    lv_obj_set_size(ui->setting_cont_2, 527, 296);
    lv_obj_set_scrollbar_mode(ui->setting_cont_2, LV_SCROLLBAR_MODE_AUTO);

    //Write style for setting_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->setting_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes setting_line_1
    ui->setting_line_1 = lv_line_create(ui->setting_cont_1);
    lv_obj_set_pos(ui->setting_line_1, 28, 62);
    lv_obj_set_size(ui->setting_line_1, 527, 1);
    static lv_point_precise_t setting_line_1[] = {{0, 0},{527, 0},{0, 0}};
    lv_line_set_points(ui->setting_line_1, setting_line_1, 3);

    //Write style for setting_line_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->setting_line_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->setting_line_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->setting_line_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->setting_line_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of setting.


    //Update current screen layout.
    lv_obj_update_layout(ui->setting);

    //Init events for screen.
    events_init_setting(ui);
}
