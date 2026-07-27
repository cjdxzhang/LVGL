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



void setup_scr_conf_wifi(lv_ui *ui)
{
    //Write codes conf_wifi
    ui->conf_wifi = lv_obj_create(NULL);
    lv_obj_set_size(ui->conf_wifi, 800, 480);
    lv_obj_set_scrollbar_mode(ui->conf_wifi, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_img_1
    ui->conf_wifi_img_1 = lv_image_create(ui->conf_wifi);
    lv_obj_set_pos(ui->conf_wifi_img_1, 0, 0);
    lv_obj_set_size(ui->conf_wifi_img_1, 800, 480);
    lv_obj_add_flag(ui->conf_wifi_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_wifi_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->conf_wifi_img_1, 50,50);
    lv_image_set_rotation(ui->conf_wifi_img_1, 0);

    //Write style for conf_wifi_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_wifi_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_wifi_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_cont_1
    ui->conf_wifi_cont_1 = lv_obj_create(ui->conf_wifi);
    lv_obj_set_pos(ui->conf_wifi_cont_1, 14, 61);
    lv_obj_set_size(ui->conf_wifi_cont_1, 577, 403);
    lv_obj_set_scrollbar_mode(ui->conf_wifi_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_wifi_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_wifi_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->conf_wifi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->conf_wifi_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->conf_wifi_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_cont_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_wifi_cont_1, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_wifi_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_wifi_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_wifi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_wifi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_wifi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_wifi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_wifi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_cont_2
    ui->conf_wifi_cont_2 = lv_obj_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_cont_2, 25, 71);
    lv_obj_set_size(ui->conf_wifi_cont_2, 527, 296);
    lv_obj_set_scrollbar_mode(ui->conf_wifi_cont_2, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_wifi_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_wifi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_line_1
    ui->conf_wifi_line_1 = lv_line_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_line_1, 28, 62);
    lv_obj_set_size(ui->conf_wifi_line_1, 527, 1);
    static lv_point_precise_t conf_wifi_line_1[] = {{0, 0},{527, 0},{0, 0}};
    lv_line_set_points(ui->conf_wifi_line_1, conf_wifi_line_1, 3);

    //Write style for conf_wifi_line_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->conf_wifi_line_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->conf_wifi_line_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->conf_wifi_line_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->conf_wifi_line_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_btn_2
    ui->conf_wifi_btn_2 = lv_button_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_btn_2, 170, 327);
    lv_obj_set_size(ui->conf_wifi_btn_2, 192, 40);
    ui->conf_wifi_btn_2_label = lv_label_create(ui->conf_wifi_btn_2);
    lv_label_set_text(ui->conf_wifi_btn_2_label, "连接");
    lv_label_set_long_mode(ui->conf_wifi_btn_2_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->conf_wifi_btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->conf_wifi_btn_2, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->conf_wifi_btn_2_label, LV_PCT(100));

    //Write style for conf_wifi_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_wifi_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_wifi_btn_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_wifi_btn_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_wifi_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_btn_2, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_wifi_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_wifi_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_wifi_btn_2, &lv_font_SourceHanSansSC_Regular_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_wifi_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_wifi_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_wifi_btn_2_label, &lv_font_SourceHanSansSC_Regular_18, LV_PART_MAIN|LV_STATE_DEFAULT);

    // Keep the button visible while a background connection is in progress.
    lv_obj_set_style_bg_color(ui->conf_wifi_btn_2, lv_color_hex(0x9b91a6), LV_PART_MAIN|LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(ui->conf_wifi_btn_2, 220, LV_PART_MAIN|LV_STATE_DISABLED);
    lv_obj_set_style_text_color(ui->conf_wifi_btn_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DISABLED);

    // Connection result shown independently so the button text always remains "连接".
    ui->conf_wifi_status_label = lv_label_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_status_label, 30, 334);
    lv_obj_set_size(ui->conf_wifi_status_label, 250, 26);
    lv_label_set_text(ui->conf_wifi_status_label, "");
    lv_label_set_long_mode(ui->conf_wifi_status_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(ui->conf_wifi_status_label, &lv_font_SourceHanSansSC_Regular_18,
                               LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_wifi_status_label, lv_color_hex(0xffffff),
                                LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_wifi_status_label, LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_label_3
    ui->conf_wifi_label_3 = lv_label_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_label_3, 30, 14);
    lv_obj_set_size(ui->conf_wifi_label_3, 518, 34);
    lv_label_set_text(ui->conf_wifi_label_3, "wifi设置");
    lv_label_set_long_mode(ui->conf_wifi_label_3, LV_LABEL_LONG_WRAP);

    //Write style for conf_wifi_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_wifi_label_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_wifi_label_3, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_wifi_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_wifi_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_wifi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_wifi_ssid
    ui->conf_wifi_wifi_ssid = lv_dropdown_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_wifi_ssid, 51, 83);
    lv_obj_set_size(ui->conf_wifi_wifi_ssid, 470, 54);
    lv_dropdown_set_options(ui->conf_wifi_wifi_ssid, "");
    lv_dropdown_set_symbol(ui->conf_wifi_wifi_ssid, NULL);

    //Write style for conf_wifi_wifi_ssid, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_wifi_wifi_ssid, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_wifi_wifi_ssid, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_wifi_wifi_ssid, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_wifi_wifi_ssid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_wifi_wifi_ssid, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_wifi_wifi_ssid, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_wifi_wifi_ssid, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_wifi_ssid, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_wifi_wifi_ssid, 121, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_wifi_wifi_ssid, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_wifi_wifi_ssid, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_wifi_wifi_ssid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_conf_wifi_wifi_ssid_extra_list_selected_checked
    static lv_style_t style_conf_wifi_wifi_ssid_extra_list_selected_checked;
    ui_init_style(&style_conf_wifi_wifi_ssid_extra_list_selected_checked);

    lv_style_set_border_width(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, 1);
    lv_style_set_border_opa(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, 255);
    lv_style_set_border_color(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, 3);
    lv_style_set_bg_opa(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, 255);
    lv_style_set_bg_color(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, lv_color_hex(0x00a1b5));
    lv_style_set_bg_grad_dir(&style_conf_wifi_wifi_ssid_extra_list_selected_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_wifi_wifi_ssid), &style_conf_wifi_wifi_ssid_extra_list_selected_checked, LV_PART_SELECTED|LV_STATE_CHECKED);

    //Write style state: LV_STATE_DEFAULT for &style_conf_wifi_wifi_ssid_extra_list_main_default
    static lv_style_t style_conf_wifi_wifi_ssid_extra_list_main_default;
    ui_init_style(&style_conf_wifi_wifi_ssid_extra_list_main_default);

    lv_style_set_max_height(&style_conf_wifi_wifi_ssid_extra_list_main_default, 90);
    lv_style_set_text_color(&style_conf_wifi_wifi_ssid_extra_list_main_default, lv_color_hex(0x937dad));
    lv_style_set_text_font(&style_conf_wifi_wifi_ssid_extra_list_main_default, &lv_font_SourceHanSansSC_Regular_25);
    lv_style_set_text_opa(&style_conf_wifi_wifi_ssid_extra_list_main_default, 255);
    lv_style_set_border_width(&style_conf_wifi_wifi_ssid_extra_list_main_default, 0);
    lv_style_set_radius(&style_conf_wifi_wifi_ssid_extra_list_main_default, 10);
    lv_style_set_bg_opa(&style_conf_wifi_wifi_ssid_extra_list_main_default, 121);
    lv_style_set_bg_color(&style_conf_wifi_wifi_ssid_extra_list_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_conf_wifi_wifi_ssid_extra_list_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_wifi_wifi_ssid), &style_conf_wifi_wifi_ssid_extra_list_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_conf_wifi_wifi_ssid_extra_list_scrollbar_default
    static lv_style_t style_conf_wifi_wifi_ssid_extra_list_scrollbar_default;
    ui_init_style(&style_conf_wifi_wifi_ssid_extra_list_scrollbar_default);

    lv_style_set_radius(&style_conf_wifi_wifi_ssid_extra_list_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_conf_wifi_wifi_ssid_extra_list_scrollbar_default, 255);
    lv_style_set_bg_color(&style_conf_wifi_wifi_ssid_extra_list_scrollbar_default, lv_color_hex(0x00ff00));
    lv_style_set_bg_grad_dir(&style_conf_wifi_wifi_ssid_extra_list_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_wifi_wifi_ssid), &style_conf_wifi_wifi_ssid_extra_list_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_wifi_wifi_pwd
    ui->conf_wifi_wifi_pwd = lv_textarea_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_wifi_pwd, 51, 156);
    lv_obj_set_size(ui->conf_wifi_wifi_pwd, 470, 54);
    lv_textarea_set_text(ui->conf_wifi_wifi_pwd, "");
    lv_textarea_set_placeholder_text(ui->conf_wifi_wifi_pwd, "");
    lv_textarea_set_password_bullet(ui->conf_wifi_wifi_pwd, "*");
    lv_textarea_set_password_mode(ui->conf_wifi_wifi_pwd, false);
    lv_textarea_set_one_line(ui->conf_wifi_wifi_pwd, true);
    lv_textarea_set_accepted_chars(ui->conf_wifi_wifi_pwd, "");
    lv_textarea_set_max_length(ui->conf_wifi_wifi_pwd, 32);
#if LV_USE_KEYBOARD
    lv_obj_add_event_cb(ui->conf_wifi_wifi_pwd, ta_event_cb, LV_EVENT_ALL, ui->g_kb_top_layer);
#endif

    //Write style for conf_wifi_wifi_pwd, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_wifi_wifi_pwd, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_wifi_wifi_pwd, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_wifi_wifi_pwd, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_wifi_wifi_pwd, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_wifi_wifi_pwd, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_wifi_wifi_pwd, 144, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_wifi_wifi_pwd, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_wifi_wifi_pwd, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_wifi_wifi_pwd, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_wifi_wifi_pwd, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_wifi_wifi_pwd, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_wifi_wifi_pwd, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_wifi_wifi_pwd, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_wifi_pwd, 10, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_wifi_wifi_pwd, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_wifi_wifi_pwd, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_wifi_wifi_pwd, lv_color_hex(0x2195f6), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_wifi_wifi_pwd, LV_GRAD_DIR_NONE, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_wifi_pwd, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_wifi_label_4
    ui->conf_wifi_label_4 = lv_label_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_label_4, 54, 230);
    lv_obj_set_size(ui->conf_wifi_label_4, 469, 72);
    lv_label_set_text(ui->conf_wifi_label_4, "请确保链接的家庭wi-fi网络为2.4G或2.4/5G混合网络");
    lv_label_set_long_mode(ui->conf_wifi_label_4, LV_LABEL_LONG_WRAP);

    //Write style for conf_wifi_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_wifi_label_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_wifi_label_4, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_wifi_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_wifi_label_4, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_wifi_label_4, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_wifi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_img_2
    ui->conf_wifi_img_2 = lv_image_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_img_2, 67, 95);
    lv_obj_set_size(ui->conf_wifi_img_2, 30, 30);
    lv_obj_add_flag(ui->conf_wifi_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_wifi_img_2, &_wifi_holder_RGB565A8_30x30);
    lv_image_set_pivot(ui->conf_wifi_img_2, 50,50);
    lv_image_set_rotation(ui->conf_wifi_img_2, 0);

    //Write style for conf_wifi_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_wifi_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_wifi_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_wifi_img_3
    ui->conf_wifi_img_3 = lv_image_create(ui->conf_wifi_cont_1);
    lv_obj_set_pos(ui->conf_wifi_img_3, 65, 166);
    lv_obj_set_size(ui->conf_wifi_img_3, 30, 30);
    lv_obj_add_flag(ui->conf_wifi_img_3, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_wifi_img_3, &_pwd_holder_RGB565A8_30x30);
    lv_image_set_pivot(ui->conf_wifi_img_3, 50,50);
    lv_image_set_rotation(ui->conf_wifi_img_3, 0);

    //Write style for conf_wifi_img_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_wifi_img_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_wifi_img_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of conf_wifi.


    //Update current screen layout.
    lv_obj_update_layout(ui->conf_wifi);

    //Init events for screen.
    events_init_conf_wifi(ui);
}
