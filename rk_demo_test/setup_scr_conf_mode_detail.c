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



void setup_scr_conf_mode_detail(lv_ui *ui)
{
    //Write codes conf_mode_detail
    ui->conf_mode_detail = lv_obj_create(NULL);
    lv_obj_set_size(ui->conf_mode_detail, 800, 480);
    lv_obj_set_scrollbar_mode(ui->conf_mode_detail, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_mode_detail, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_mode_detail, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_img_1
    ui->conf_mode_detail_img_1 = lv_image_create(ui->conf_mode_detail);
    lv_obj_set_pos(ui->conf_mode_detail_img_1, 0, 0);
    lv_obj_set_size(ui->conf_mode_detail_img_1, 800, 480);
    lv_obj_add_flag(ui->conf_mode_detail_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->conf_mode_detail_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->conf_mode_detail_img_1, 50,50);
    lv_image_set_rotation(ui->conf_mode_detail_img_1, 0);

    //Write style for conf_mode_detail_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->conf_mode_detail_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->conf_mode_detail_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_cont_1
    ui->conf_mode_detail_cont_1 = lv_obj_create(ui->conf_mode_detail);
    lv_obj_set_pos(ui->conf_mode_detail_cont_1, 14, 61);
    lv_obj_set_size(ui->conf_mode_detail_cont_1, 577, 403);
    lv_obj_set_scrollbar_mode(ui->conf_mode_detail_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for conf_mode_detail_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->conf_mode_detail_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->conf_mode_detail_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->conf_mode_detail_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_cont_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_cont_1, 130, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_cont_1, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->conf_mode_detail_cont_1, lv_color_hex(0x6c6c6c), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->conf_mode_detail_cont_1, 144, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->conf_mode_detail_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_x(ui->conf_mode_detail_cont_1, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_y(ui->conf_mode_detail_cont_1, 1, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_label_1
    ui->conf_mode_detail_label_1 = lv_label_create(ui->conf_mode_detail_cont_1);
    lv_obj_set_pos(ui->conf_mode_detail_label_1, 30, 14);
    lv_obj_set_size(ui->conf_mode_detail_label_1, 518, 34);
    lv_label_set_text(ui->conf_mode_detail_label_1, "助眠设置");
    lv_label_set_long_mode(ui->conf_mode_detail_label_1, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_detail_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_label_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_1, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_cont_2
    ui->conf_mode_detail_cont_2 = lv_obj_create(ui->conf_mode_detail_cont_1);
    lv_obj_set_pos(ui->conf_mode_detail_cont_2, 25, 71);
    lv_obj_set_size(ui->conf_mode_detail_cont_2, 527, 296);
    lv_obj_set_scrollbar_mode(ui->conf_mode_detail_cont_2, LV_SCROLLBAR_MODE_AUTO);

    //Write style for conf_mode_detail_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_combo_water_level
    ui->conf_mode_detail_combo_water_level = lv_dropdown_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_combo_water_level, 175, 251);
    lv_obj_set_size(ui->conf_mode_detail_combo_water_level, 310, 54);
    lv_dropdown_set_options(ui->conf_mode_detail_combo_water_level, "1档\n2档\n3档");
    lv_dropdown_set_symbol(ui->conf_mode_detail_combo_water_level, NULL);

    //Write style for conf_mode_detail_combo_water_level, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_mode_detail_combo_water_level, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_combo_water_level, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_combo_water_level, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_combo_water_level, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_combo_water_level, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_combo_water_level, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_combo_water_level, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_combo_water_level, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_combo_water_level, 121, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_combo_water_level, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_combo_water_level, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_combo_water_level, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_conf_mode_detail_combo_water_level_extra_list_selected_checked
    static lv_style_t style_conf_mode_detail_combo_water_level_extra_list_selected_checked;
    ui_init_style(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked);

    lv_style_set_border_width(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, 1);
    lv_style_set_border_opa(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, 255);
    lv_style_set_border_color(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, lv_color_hex(0x00a1b5));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_water_level_extra_list_selected_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_water_level), &style_conf_mode_detail_combo_water_level_extra_list_selected_checked, LV_PART_SELECTED|LV_STATE_CHECKED);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_water_level_extra_list_main_default
    static lv_style_t style_conf_mode_detail_combo_water_level_extra_list_main_default;
    ui_init_style(&style_conf_mode_detail_combo_water_level_extra_list_main_default);

    lv_style_set_max_height(&style_conf_mode_detail_combo_water_level_extra_list_main_default, 90);
    lv_style_set_text_color(&style_conf_mode_detail_combo_water_level_extra_list_main_default, lv_color_hex(0x937dad));
    lv_style_set_text_font(&style_conf_mode_detail_combo_water_level_extra_list_main_default, &lv_font_SourceHanSansSC_Regular_25);
    lv_style_set_text_opa(&style_conf_mode_detail_combo_water_level_extra_list_main_default, 255);
    lv_style_set_border_width(&style_conf_mode_detail_combo_water_level_extra_list_main_default, 0);
    lv_style_set_radius(&style_conf_mode_detail_combo_water_level_extra_list_main_default, 10);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_water_level_extra_list_main_default, 121);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_water_level_extra_list_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_water_level_extra_list_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_water_level), &style_conf_mode_detail_combo_water_level_extra_list_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default
    static lv_style_t style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default;
    ui_init_style(&style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default);

    lv_style_set_radius(&style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default, lv_color_hex(0x00ff00));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_water_level), &style_conf_mode_detail_combo_water_level_extra_list_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_label_2
    ui->conf_mode_detail_label_2 = lv_label_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_label_2, 33, 181);
    lv_obj_set_size(ui->conf_mode_detail_label_2, 130, 35);
    lv_label_set_text(ui->conf_mode_detail_label_2, "足浴时间");
    lv_label_set_long_mode(ui->conf_mode_detail_label_2, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_detail_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_label_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_2, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_combo_medicinal
    ui->conf_mode_detail_combo_medicinal = lv_dropdown_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_combo_medicinal, 175, 331);
    lv_obj_set_size(ui->conf_mode_detail_combo_medicinal, 310, 54);
    lv_dropdown_set_options(ui->conf_mode_detail_combo_medicinal, "无\n药液1\n药液2\n药液1+药液2");
    lv_dropdown_set_symbol(ui->conf_mode_detail_combo_medicinal, NULL);

    //Write style for conf_mode_detail_combo_medicinal, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_mode_detail_combo_medicinal, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_combo_medicinal, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_combo_medicinal, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_combo_medicinal, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_combo_medicinal, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_combo_medicinal, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_combo_medicinal, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_combo_medicinal, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_combo_medicinal, 121, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_combo_medicinal, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_combo_medicinal, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_combo_medicinal, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_conf_mode_detail_combo_medicinal_extra_list_selected_checked
    static lv_style_t style_conf_mode_detail_combo_medicinal_extra_list_selected_checked;
    ui_init_style(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked);

    lv_style_set_border_width(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, 1);
    lv_style_set_border_opa(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, 255);
    lv_style_set_border_color(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, lv_color_hex(0x00a1b5));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_medicinal), &style_conf_mode_detail_combo_medicinal_extra_list_selected_checked, LV_PART_SELECTED|LV_STATE_CHECKED);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_medicinal_extra_list_main_default
    static lv_style_t style_conf_mode_detail_combo_medicinal_extra_list_main_default;
    ui_init_style(&style_conf_mode_detail_combo_medicinal_extra_list_main_default);

    lv_style_set_max_height(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, 90);
    lv_style_set_text_color(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, lv_color_hex(0x937dad));
    lv_style_set_text_font(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, &lv_font_SourceHanSansSC_Regular_25);
    lv_style_set_text_opa(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, 255);
    lv_style_set_border_width(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, 0);
    lv_style_set_radius(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, 10);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, 121);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_medicinal_extra_list_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_medicinal), &style_conf_mode_detail_combo_medicinal_extra_list_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default
    static lv_style_t style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default;
    ui_init_style(&style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default);

    lv_style_set_radius(&style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default, lv_color_hex(0x00ff00));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_medicinal), &style_conf_mode_detail_combo_medicinal_extra_list_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_label_3
    ui->conf_mode_detail_label_3 = lv_label_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_label_3, 33, 342);
    lv_obj_set_size(ui->conf_mode_detail_label_3, 130, 35);
    lv_label_set_text(ui->conf_mode_detail_label_3, "药液设置");
    lv_label_set_long_mode(ui->conf_mode_detail_label_3, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_detail_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_label_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_3, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_label_4
    ui->conf_mode_detail_label_4 = lv_label_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_label_4, 33, 420);
    lv_obj_set_size(ui->conf_mode_detail_label_4, 130, 35);
    lv_label_set_text(ui->conf_mode_detail_label_4, "定位设置");
    lv_label_set_long_mode(ui->conf_mode_detail_label_4, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_detail_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_label_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_4, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_combo_location
    ui->conf_mode_detail_combo_location = lv_dropdown_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_combo_location, 175, 411);
    lv_obj_set_size(ui->conf_mode_detail_combo_location, 310, 54);
    lv_dropdown_set_options(ui->conf_mode_detail_combo_location, "客厅\n书房\n主卧\n小孩房");
    lv_dropdown_set_symbol(ui->conf_mode_detail_combo_location, NULL);

    //Write style for conf_mode_detail_combo_location, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_mode_detail_combo_location, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_combo_location, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_combo_location, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_combo_location, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_combo_location, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_combo_location, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_combo_location, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_combo_location, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_combo_location, 121, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_combo_location, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_combo_location, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_combo_location, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_conf_mode_detail_combo_location_extra_list_selected_checked
    static lv_style_t style_conf_mode_detail_combo_location_extra_list_selected_checked;
    ui_init_style(&style_conf_mode_detail_combo_location_extra_list_selected_checked);

    lv_style_set_border_width(&style_conf_mode_detail_combo_location_extra_list_selected_checked, 1);
    lv_style_set_border_opa(&style_conf_mode_detail_combo_location_extra_list_selected_checked, 255);
    lv_style_set_border_color(&style_conf_mode_detail_combo_location_extra_list_selected_checked, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_conf_mode_detail_combo_location_extra_list_selected_checked, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_conf_mode_detail_combo_location_extra_list_selected_checked, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_location_extra_list_selected_checked, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_location_extra_list_selected_checked, lv_color_hex(0x00a1b5));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_location_extra_list_selected_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_location), &style_conf_mode_detail_combo_location_extra_list_selected_checked, LV_PART_SELECTED|LV_STATE_CHECKED);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_location_extra_list_main_default
    static lv_style_t style_conf_mode_detail_combo_location_extra_list_main_default;
    ui_init_style(&style_conf_mode_detail_combo_location_extra_list_main_default);

    lv_style_set_max_height(&style_conf_mode_detail_combo_location_extra_list_main_default, 90);
    lv_style_set_text_color(&style_conf_mode_detail_combo_location_extra_list_main_default, lv_color_hex(0x937dad));
    lv_style_set_text_font(&style_conf_mode_detail_combo_location_extra_list_main_default, &lv_font_SourceHanSansSC_Regular_25);
    lv_style_set_text_opa(&style_conf_mode_detail_combo_location_extra_list_main_default, 255);
    lv_style_set_border_width(&style_conf_mode_detail_combo_location_extra_list_main_default, 0);
    lv_style_set_radius(&style_conf_mode_detail_combo_location_extra_list_main_default, 10);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_location_extra_list_main_default, 121);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_location_extra_list_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_location_extra_list_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_location), &style_conf_mode_detail_combo_location_extra_list_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_location_extra_list_scrollbar_default
    static lv_style_t style_conf_mode_detail_combo_location_extra_list_scrollbar_default;
    ui_init_style(&style_conf_mode_detail_combo_location_extra_list_scrollbar_default);

    lv_style_set_radius(&style_conf_mode_detail_combo_location_extra_list_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_location_extra_list_scrollbar_default, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_location_extra_list_scrollbar_default, lv_color_hex(0x00ff00));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_location_extra_list_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_location), &style_conf_mode_detail_combo_location_extra_list_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_label_5
    ui->conf_mode_detail_label_5 = lv_label_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_label_5, 33, 23);
    lv_obj_set_size(ui->conf_mode_detail_label_5, 130, 35);
    lv_label_set_text(ui->conf_mode_detail_label_5, "模式名称");
    lv_label_set_long_mode(ui->conf_mode_detail_label_5, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_detail_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_label_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_5, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_label_6
    ui->conf_mode_detail_label_6 = lv_label_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_label_6, 33, 102);
    lv_obj_set_size(ui->conf_mode_detail_label_6, 130, 35);
    lv_label_set_text(ui->conf_mode_detail_label_6, "水温设置");
    lv_label_set_long_mode(ui->conf_mode_detail_label_6, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_detail_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_label_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_6, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_label_7
    ui->conf_mode_detail_label_7 = lv_label_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_label_7, 33, 260);
    lv_obj_set_size(ui->conf_mode_detail_label_7, 130, 35);
    lv_label_set_text(ui->conf_mode_detail_label_7, "水位设置");
    lv_label_set_long_mode(ui->conf_mode_detail_label_7, LV_LABEL_LONG_WRAP);

    //Write style for conf_mode_detail_label_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_label_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_7, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_label_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_label_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_btn_resume
    ui->conf_mode_detail_btn_resume = lv_button_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_btn_resume, 285, 508);
    lv_obj_set_size(ui->conf_mode_detail_btn_resume, 165, 50);
    ui->conf_mode_detail_btn_resume_label = lv_label_create(ui->conf_mode_detail_btn_resume);
    lv_label_set_text(ui->conf_mode_detail_btn_resume_label, "恢复");
    lv_label_set_long_mode(ui->conf_mode_detail_btn_resume_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->conf_mode_detail_btn_resume_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->conf_mode_detail_btn_resume, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->conf_mode_detail_btn_resume_label, LV_PCT(100));

    //Write style for conf_mode_detail_btn_resume, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_btn_resume, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_btn_resume, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_btn_resume, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_btn_resume, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_btn_resume, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_btn_resume, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_btn_resume, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_btn_resume, &lv_font_SourceHanSansSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_btn_resume, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_btn_resume, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_btn_save
    ui->conf_mode_detail_btn_save = lv_button_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_btn_save, 64, 507);
    lv_obj_set_size(ui->conf_mode_detail_btn_save, 155, 50);
    ui->conf_mode_detail_btn_save_label = lv_label_create(ui->conf_mode_detail_btn_save);
    lv_label_set_text(ui->conf_mode_detail_btn_save_label, "保存");
    lv_label_set_long_mode(ui->conf_mode_detail_btn_save_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->conf_mode_detail_btn_save_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->conf_mode_detail_btn_save, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->conf_mode_detail_btn_save_label, LV_PCT(100));

    //Write style for conf_mode_detail_btn_save, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_btn_save, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_btn_save, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_btn_save, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_btn_save, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_btn_save, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_btn_save, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_btn_save, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_btn_save, &lv_font_SourceHanSansSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_btn_save, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_btn_save, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_combo_temperature
    ui->conf_mode_detail_combo_temperature = lv_dropdown_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_combo_temperature, 175, 91);
    lv_obj_set_size(ui->conf_mode_detail_combo_temperature, 310, 54);
    lv_dropdown_set_options(ui->conf_mode_detail_combo_temperature, "35℃\n36℃\n37℃\n38℃\n39℃\n40℃\n41℃\n42℃\n43℃\n44℃\n45℃\n46℃\n47℃\n48℃");
    lv_dropdown_set_symbol(ui->conf_mode_detail_combo_temperature, NULL);

    //Write style for conf_mode_detail_combo_temperature, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_mode_detail_combo_temperature, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_combo_temperature, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_combo_temperature, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_combo_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_combo_temperature, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_combo_temperature, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_combo_temperature, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_combo_temperature, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_combo_temperature, 121, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_combo_temperature, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_combo_temperature, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_combo_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_conf_mode_detail_combo_temperature_extra_list_selected_checked
    static lv_style_t style_conf_mode_detail_combo_temperature_extra_list_selected_checked;
    ui_init_style(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked);

    lv_style_set_border_width(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, 1);
    lv_style_set_border_opa(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, 255);
    lv_style_set_border_color(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, lv_color_hex(0x00a1b5));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_temperature_extra_list_selected_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_temperature), &style_conf_mode_detail_combo_temperature_extra_list_selected_checked, LV_PART_SELECTED|LV_STATE_CHECKED);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_temperature_extra_list_main_default
    static lv_style_t style_conf_mode_detail_combo_temperature_extra_list_main_default;
    ui_init_style(&style_conf_mode_detail_combo_temperature_extra_list_main_default);

    lv_style_set_max_height(&style_conf_mode_detail_combo_temperature_extra_list_main_default, 90);
    lv_style_set_text_color(&style_conf_mode_detail_combo_temperature_extra_list_main_default, lv_color_hex(0x937dad));
    lv_style_set_text_font(&style_conf_mode_detail_combo_temperature_extra_list_main_default, &lv_font_SourceHanSansSC_Regular_25);
    lv_style_set_text_opa(&style_conf_mode_detail_combo_temperature_extra_list_main_default, 255);
    lv_style_set_border_width(&style_conf_mode_detail_combo_temperature_extra_list_main_default, 0);
    lv_style_set_radius(&style_conf_mode_detail_combo_temperature_extra_list_main_default, 10);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_temperature_extra_list_main_default, 121);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_temperature_extra_list_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_temperature_extra_list_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_temperature), &style_conf_mode_detail_combo_temperature_extra_list_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default
    static lv_style_t style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default;
    ui_init_style(&style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default);

    lv_style_set_radius(&style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default, lv_color_hex(0x00ff00));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_temperature), &style_conf_mode_detail_combo_temperature_extra_list_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_combo_timer
    ui->conf_mode_detail_combo_timer = lv_dropdown_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_combo_timer, 175, 171);
    lv_obj_set_size(ui->conf_mode_detail_combo_timer, 310, 54);
    lv_dropdown_set_options(ui->conf_mode_detail_combo_timer, "10min\n15min\n20min\n25min\n30min");
    lv_dropdown_set_symbol(ui->conf_mode_detail_combo_timer, NULL);

    //Write style for conf_mode_detail_combo_timer, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_mode_detail_combo_timer, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_combo_timer, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_combo_timer, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_combo_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_combo_timer, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_combo_timer, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_combo_timer, 6, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_combo_timer, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_combo_timer, 121, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_combo_timer, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_combo_timer, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_combo_timer, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_CHECKED for &style_conf_mode_detail_combo_timer_extra_list_selected_checked
    static lv_style_t style_conf_mode_detail_combo_timer_extra_list_selected_checked;
    ui_init_style(&style_conf_mode_detail_combo_timer_extra_list_selected_checked);

    lv_style_set_border_width(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, 1);
    lv_style_set_border_opa(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, 255);
    lv_style_set_border_color(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, lv_color_hex(0xe1e6ee));
    lv_style_set_border_side(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, LV_BORDER_SIDE_FULL);
    lv_style_set_radius(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, lv_color_hex(0x00a1b5));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_timer_extra_list_selected_checked, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_timer), &style_conf_mode_detail_combo_timer_extra_list_selected_checked, LV_PART_SELECTED|LV_STATE_CHECKED);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_timer_extra_list_main_default
    static lv_style_t style_conf_mode_detail_combo_timer_extra_list_main_default;
    ui_init_style(&style_conf_mode_detail_combo_timer_extra_list_main_default);

    lv_style_set_max_height(&style_conf_mode_detail_combo_timer_extra_list_main_default, 90);
    lv_style_set_text_color(&style_conf_mode_detail_combo_timer_extra_list_main_default, lv_color_hex(0x937dad));
    lv_style_set_text_font(&style_conf_mode_detail_combo_timer_extra_list_main_default, &lv_font_SourceHanSansSC_Regular_25);
    lv_style_set_text_opa(&style_conf_mode_detail_combo_timer_extra_list_main_default, 255);
    lv_style_set_border_width(&style_conf_mode_detail_combo_timer_extra_list_main_default, 0);
    lv_style_set_radius(&style_conf_mode_detail_combo_timer_extra_list_main_default, 10);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_timer_extra_list_main_default, 121);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_timer_extra_list_main_default, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_timer_extra_list_main_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_timer), &style_conf_mode_detail_combo_timer_extra_list_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style state: LV_STATE_DEFAULT for &style_conf_mode_detail_combo_timer_extra_list_scrollbar_default
    static lv_style_t style_conf_mode_detail_combo_timer_extra_list_scrollbar_default;
    ui_init_style(&style_conf_mode_detail_combo_timer_extra_list_scrollbar_default);

    lv_style_set_radius(&style_conf_mode_detail_combo_timer_extra_list_scrollbar_default, 3);
    lv_style_set_bg_opa(&style_conf_mode_detail_combo_timer_extra_list_scrollbar_default, 255);
    lv_style_set_bg_color(&style_conf_mode_detail_combo_timer_extra_list_scrollbar_default, lv_color_hex(0x00ff00));
    lv_style_set_bg_grad_dir(&style_conf_mode_detail_combo_timer_extra_list_scrollbar_default, LV_GRAD_DIR_NONE);
    lv_obj_add_style(lv_dropdown_get_list(ui->conf_mode_detail_combo_timer), &style_conf_mode_detail_combo_timer_extra_list_scrollbar_default, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_text_mode_name
    ui->conf_mode_detail_text_mode_name = lv_textarea_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_text_mode_name, 175, 11);
    lv_obj_set_size(ui->conf_mode_detail_text_mode_name, 310, 54);
    lv_textarea_set_text(ui->conf_mode_detail_text_mode_name, "助眠设置");
    lv_textarea_set_placeholder_text(ui->conf_mode_detail_text_mode_name, "");
    lv_textarea_set_password_bullet(ui->conf_mode_detail_text_mode_name, "*");
    lv_textarea_set_password_mode(ui->conf_mode_detail_text_mode_name, false);
    lv_textarea_set_one_line(ui->conf_mode_detail_text_mode_name, true);
    lv_textarea_set_accepted_chars(ui->conf_mode_detail_text_mode_name, "");
    lv_textarea_set_max_length(ui->conf_mode_detail_text_mode_name, 32);
#if LV_USE_KEYBOARD
    lv_obj_add_event_cb(ui->conf_mode_detail_text_mode_name, ta_event_cb, LV_EVENT_ALL, ui->g_kb_top_layer);
#endif

    //Write style for conf_mode_detail_text_mode_name, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->conf_mode_detail_text_mode_name, lv_color_hex(0x937dad), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_text_mode_name, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_text_mode_name, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->conf_mode_detail_text_mode_name, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_text_mode_name, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_text_mode_name, 144, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_text_mode_name, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_text_mode_name, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_text_mode_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_text_mode_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->conf_mode_detail_text_mode_name, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->conf_mode_detail_text_mode_name, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->conf_mode_detail_text_mode_name, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_text_mode_name, 10, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for conf_mode_detail_text_mode_name, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_text_mode_name, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_text_mode_name, lv_color_hex(0x2195f6), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_text_mode_name, LV_GRAD_DIR_NONE, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_text_mode_name, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_btn_delete
    ui->conf_mode_detail_btn_delete = lv_button_create(ui->conf_mode_detail_cont_2);
    lv_obj_set_pos(ui->conf_mode_detail_btn_delete, 285, 508);
    lv_obj_set_size(ui->conf_mode_detail_btn_delete, 165, 50);
    ui->conf_mode_detail_btn_delete_label = lv_label_create(ui->conf_mode_detail_btn_delete);
    lv_label_set_text(ui->conf_mode_detail_btn_delete_label, "删除");
    lv_label_set_long_mode(ui->conf_mode_detail_btn_delete_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->conf_mode_detail_btn_delete_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->conf_mode_detail_btn_delete, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->conf_mode_detail_btn_delete_label, LV_PCT(100));

    //Write style for conf_mode_detail_btn_delete, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->conf_mode_detail_btn_delete, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->conf_mode_detail_btn_delete, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->conf_mode_detail_btn_delete, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->conf_mode_detail_btn_delete, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->conf_mode_detail_btn_delete, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->conf_mode_detail_btn_delete, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->conf_mode_detail_btn_delete, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_btn_delete, &lv_font_SourceHanSansSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->conf_mode_detail_btn_delete, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->conf_mode_detail_btn_delete, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes conf_mode_detail_line_1
    ui->conf_mode_detail_line_1 = lv_line_create(ui->conf_mode_detail);
    lv_obj_set_pos(ui->conf_mode_detail_line_1, 42, 123);
    lv_obj_set_size(ui->conf_mode_detail_line_1, 527, 1);
    static lv_point_precise_t conf_mode_detail_line_1[] = {{0, 0},{527, 0},{0, 0}};
    lv_line_set_points(ui->conf_mode_detail_line_1, conf_mode_detail_line_1, 3);

    //Write style for conf_mode_detail_line_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->conf_mode_detail_line_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->conf_mode_detail_line_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->conf_mode_detail_line_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->conf_mode_detail_line_1, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of conf_mode_detail.


    //Update current screen layout.
    lv_obj_update_layout(ui->conf_mode_detail);

    //Init events for screen.
    events_init_conf_mode_detail(ui);
}
