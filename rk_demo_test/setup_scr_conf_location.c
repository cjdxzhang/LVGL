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



void setup_scr_conf_location(lv_ui *ui)
{
    //Write codes conf_location
    ui->conf_location = lv_obj_create(NULL);
    lv_obj_set_size(ui->conf_location, 800, 480);
    lv_obj_set_scrollbar_mode(ui->conf_location, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_location, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_location, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_img_1
    ui->conf_location_img_1 = lv_image_create(ui->conf_location);
    lv_obj_set_pos(ui->conf_location_img_1, 0, 0);
    lv_obj_set_size(ui->conf_location_img_1, 800, 480);
    lv_obj_add_flag(ui->conf_location_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_location_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->conf_location_img_1, 50,50);
    lv_image_set_rotation(ui->conf_location_img_1, 0);

    //Write style for conf_location_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_location_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_location_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_cont_1
    ui->conf_location_cont_1 = lv_obj_create(ui->conf_location);
    lv_obj_set_pos(ui->conf_location_cont_1, 14, 61);
    lv_obj_set_size(ui->conf_location_cont_1, 577, 403);
    lv_obj_set_scrollbar_mode(ui->conf_location_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_location_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_location_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->conf_location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->conf_location_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->conf_location_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_cont_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_cont_1, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_location_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_location_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_label_5
    ui->conf_location_label_5 = lv_label_create(ui->conf_location_cont_1);
    lv_obj_set_pos(ui->conf_location_label_5, 30, 13);
    lv_obj_set_size(ui->conf_location_label_5, 518, 34);
    lv_label_set_text(ui->conf_location_label_5, "定位设置");
    lv_label_set_long_mode(ui->conf_location_label_5, LV_LABEL_LONG_WRAP);

    //Write style for conf_location_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_location_label_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_location_label_5, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_location_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_location_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_cont_6
    ui->conf_location_cont_6 = lv_obj_create(ui->conf_location_cont_1);
    lv_obj_set_pos(ui->conf_location_cont_6, 25, 71);
    lv_obj_set_size(ui->conf_location_cont_6, 527, 296);
    lv_obj_set_scrollbar_mode(ui->conf_location_cont_6, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_location_cont_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_line_1
    ui->conf_location_line_1 = lv_line_create(ui->conf_location_cont_1);
    lv_obj_set_pos(ui->conf_location_line_1, 28, 62);
    lv_obj_set_size(ui->conf_location_line_1, 527, 1);
    static lv_point_precise_t conf_location_line_1[] = {{0, 0},{527, 0},{0, 0}};
    lv_line_set_points(ui->conf_location_line_1, conf_location_line_1, 3);

    //Write style for conf_location_line_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->conf_location_line_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->conf_location_line_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->conf_location_line_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->conf_location_line_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_cont_2
    ui->conf_location_cont_2 = lv_obj_create(ui->conf_location_cont_1);
    lv_obj_set_pos(ui->conf_location_cont_2, 55, 84);
    lv_obj_set_size(ui->conf_location_cont_2, 474, 249);
    lv_obj_set_scrollbar_mode(ui->conf_location_cont_2, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_location_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_label_6
    ui->conf_location_label_6 = lv_label_create(ui->conf_location_cont_2);
    lv_obj_set_pos(ui->conf_location_label_6, 1, 4);
    lv_obj_set_size(ui->conf_location_label_6, 469, 63);
    lv_label_set_text(ui->conf_location_label_6, "定点位置设定请移至\n移动端【地图界面】进行操作设置");
    lv_label_set_long_mode(ui->conf_location_label_6, LV_LABEL_LONG_WRAP);

    //Write style for conf_location_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_location_label_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_location_label_6, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_location_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_location_label_6, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_location_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_ta_1
    ui->conf_location_ta_1 = lv_textarea_create(ui->conf_location_cont_2);
    lv_obj_set_pos(ui->conf_location_ta_1, 104, 82);
    lv_obj_set_size(ui->conf_location_ta_1, 128, 38);
    lv_textarea_set_text(ui->conf_location_ta_1, "客厅");
    lv_textarea_set_placeholder_text(ui->conf_location_ta_1, "");
    lv_textarea_set_password_bullet(ui->conf_location_ta_1, "*");
    lv_textarea_set_password_mode(ui->conf_location_ta_1, false);
    lv_textarea_set_one_line(ui->conf_location_ta_1, true);
    lv_textarea_set_accepted_chars(ui->conf_location_ta_1, "");
    lv_textarea_set_max_length(ui->conf_location_ta_1, 32);
#if LV_USE_KEYBOARD
    lv_obj_add_event_cb(ui->conf_location_ta_1, ta_event_cb, LV_EVENT_ALL, ui->g_kb_top_layer);
#endif

    //Write style for conf_location_ta_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_location_ta_1, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_location_ta_1, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_location_ta_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_location_ta_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_location_ta_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_ta_1, 144, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_location_ta_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_location_ta_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_location_ta_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_ta_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_ta_1, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_ta_1, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_ta_1, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_ta_1, 10, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_location_ta_1, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_location_ta_1, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_location_ta_1, lv_color_hex(0x2195f6), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_location_ta_1, LV_GRAD_DIR_NONE, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_ta_1, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_location_ddlist_1
    ui->conf_location_ddlist_1 = lv_dropdown_create(ui->conf_location_cont_2);
    lv_obj_set_pos(ui->conf_location_ddlist_1, 237, 81);
    lv_obj_set_size(ui->conf_location_ddlist_1, 206, 38);
    lv_dropdown_set_options(ui->conf_location_ddlist_1, "助眠");
    lv_dropdown_set_symbol(ui->conf_location_ddlist_1, NULL);

    //Write style for conf_location_ddlist_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_location_ddlist_1, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_location_ddlist_1, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_location_ddlist_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_location_ddlist_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_ddlist_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_ddlist_1, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_ddlist_1, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_ddlist_1, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_ddlist_1, 121, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_location_ddlist_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_location_ddlist_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_ddlist_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_conf_location_ddlist_1_extra_list_selected_checked
    static lv_style_t style_conf_location_ddlist_1_extra_list_selected_checked;
    ui_init_style(&style_conf_location_ddlist_1_extra_list_selected_checked);

    lv_style_set_border_width(&style_conf_location_ddlist_1_extra_list_selected_checked, 1);
    lv_style_set_border_opa(&style_conf_location_ddlist_1_extra_list_selected_checked, 255);
    lv_style_set_border_color(&style_conf_location_ddlist_1_extra_list_selected_checked, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_conf_location_ddlist_1_extra_list_selected_checked, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_conf_location_ddlist_1_extra_list_selected_checked, 3);
    lv_style_set_bg_opa(&style_conf_location_ddlist_1_extra_list_selected_checked, 255);
    lv_style_set_bg_color(&style_conf_location_ddlist_1_extra_list_selected_checked, lv_color_hex(0x00a1b5));
    lv_style_set_bg_grad_dir(&style_conf_location_ddlist_1_extra_list_selected_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_location_ddlist_1), &style_conf_location_ddlist_1_extra_list_selected_checked, LV_PART_SELECTED|LV_STATE_CHECKED);

    //Write style state: LV_STATE_DEFAULT for &style_conf_location_ddlist_1_extra_list_main_default
    static lv_style_t style_conf_location_ddlist_1_extra_list_main_default;
    ui_init_style(&style_conf_location_ddlist_1_extra_list_main_default);

    lv_style_set_max_height(&style_conf_location_ddlist_1_extra_list_main_default, 90);
    lv_style_set_text_color(&style_conf_location_ddlist_1_extra_list_main_default, lv_color_hex(0x937dad));
    lv_style_set_text_font(&style_conf_location_ddlist_1_extra_list_main_default, &lv_font_SourceHanSansSC_Regular_25);
    lv_style_set_text_opa(&style_conf_location_ddlist_1_extra_list_main_default, 255);
    lv_style_set_border_width(&style_conf_location_ddlist_1_extra_list_main_default, 0);
    lv_style_set_radius(&style_conf_location_ddlist_1_extra_list_main_default, 10);
    lv_style_set_bg_opa(&style_conf_location_ddlist_1_extra_list_main_default, 121);
    lv_style_set_bg_color(&style_conf_location_ddlist_1_extra_list_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_conf_location_ddlist_1_extra_list_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_location_ddlist_1), &style_conf_location_ddlist_1_extra_list_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_conf_location_ddlist_1_extra_list_scrollbar_default
    static lv_style_t style_conf_location_ddlist_1_extra_list_scrollbar_default;
    ui_init_style(&style_conf_location_ddlist_1_extra_list_scrollbar_default);

    lv_style_set_radius(&style_conf_location_ddlist_1_extra_list_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_conf_location_ddlist_1_extra_list_scrollbar_default, 255);
    lv_style_set_bg_color(&style_conf_location_ddlist_1_extra_list_scrollbar_default, lv_color_hex(0x00ff00));
    lv_style_set_bg_grad_dir(&style_conf_location_ddlist_1_extra_list_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_location_ddlist_1), &style_conf_location_ddlist_1_extra_list_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_location_label_7
    ui->conf_location_label_7 = lv_label_create(ui->conf_location);
    lv_obj_set_pos(ui->conf_location_label_7, 68, 235);
    lv_obj_set_size(ui->conf_location_label_7, 100, 32);
    lv_label_set_text(ui->conf_location_label_7, "定位1");
    lv_label_set_long_mode(ui->conf_location_label_7, LV_LABEL_LONG_WRAP);

    //Write style for conf_location_label_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_location_label_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_location_label_7, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_location_label_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_location_label_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_btn_save
    ui->conf_location_btn_save = lv_button_create(ui->conf_location);
    lv_obj_set_pos(ui->conf_location_btn_save, 101, 398);
    lv_obj_set_size(ui->conf_location_btn_save, 178, 46);
    ui->conf_location_btn_save_label = lv_label_create(ui->conf_location_btn_save);
    lv_label_set_text(ui->conf_location_btn_save_label, "保存");
    lv_label_set_long_mode(ui->conf_location_btn_save_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->conf_location_btn_save_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->conf_location_btn_save, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->conf_location_btn_save_label, LV_PCT(100));

    //Write style for conf_location_btn_save, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_location_btn_save, 147, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_location_btn_save, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_location_btn_save, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_location_btn_save, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_btn_save, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_btn_save, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_location_btn_save, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_location_btn_save, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_location_btn_save, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_location_btn_save, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_location_btn_create_map
    ui->conf_location_btn_create_map = lv_button_create(ui->conf_location);
    lv_obj_set_pos(ui->conf_location_btn_create_map, 340, 398);
    lv_obj_set_size(ui->conf_location_btn_create_map, 178, 46);
    ui->conf_location_btn_create_map_label = lv_label_create(ui->conf_location_btn_create_map);
    lv_label_set_text(ui->conf_location_btn_create_map_label, "重新建图");
    lv_label_set_long_mode(ui->conf_location_btn_create_map_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->conf_location_btn_create_map_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->conf_location_btn_create_map, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->conf_location_btn_create_map_label, LV_PCT(100));

    //Write style for conf_location_btn_create_map, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_location_btn_create_map, 147, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_location_btn_create_map, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_location_btn_create_map, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_location_btn_create_map, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_location_btn_create_map, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_location_btn_create_map, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_location_btn_create_map, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_location_btn_create_map, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_location_btn_create_map, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_location_btn_create_map, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of conf_location.


    //Update current screen layout.
    lv_obj_update_layout(ui->conf_location);

    //Init events for screen.
    events_init_conf_location(ui);
}
