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



void setup_scr_index(lv_ui *ui)
{
    //Write codes index
    ui->index = lv_obj_create(NULL);
    lv_obj_set_size(ui->index, 800, 480);
    lv_obj_set_scrollbar_mode(ui->index, LV_SCROLLBAR_MODE_OFF);

    //Write style for index, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->index, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->index, lv_color_hex(0x7f45df), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->index, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui->index, lv_color_hex(0xdf45c9), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui->index, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui->index, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_1
    ui->index_img_1 = lv_image_create(ui->index);
    lv_obj_set_pos(ui->index_img_1, 0, 0);
    lv_obj_set_size(ui->index_img_1, 800, 480);
    lv_obj_add_flag(ui->index_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->index_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->index_img_1, 50, 50);
    lv_image_set_rotation(ui->index_img_1, 0);

    //Write style for index_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_panel_status
    ui->index_panel_status = lv_button_create(ui->index);
    lv_obj_set_pos(ui->index_panel_status, 15, 71);
    lv_obj_set_size(ui->index_panel_status, 380, 144);
    lv_obj_add_flag(ui->index_panel_status, LV_OBJ_FLAG_HIDDEN);
    ui->index_panel_status_label = lv_label_create(ui->index_panel_status);
    lv_label_set_text(ui->index_panel_status_label, "");
    lv_label_set_long_mode(ui->index_panel_status_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->index_panel_status_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->index_panel_status, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->index_panel_status_label, LV_PCT(100));

    //Write style for index_panel_status, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->index_panel_status, 76, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->index_panel_status, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->index_panel_status, LV_GRAD_DIR_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->index_panel_status, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_panel_status, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_panel_status, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->index_panel_status, lv_color_hex(0x2a3a47),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->index_panel_status, 145, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->index_panel_status, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_x(ui->index_panel_status, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_y(ui->index_panel_status, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->index_panel_status, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->index_panel_status, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->index_panel_status, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->index_panel_status, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_cont_1
    ui->index_cont_1 = lv_obj_create(ui->index);
    lv_obj_set_pos(ui->index_cont_1, 420, 71);
    lv_obj_set_size(ui->index_cont_1, 377, 391);
    lv_obj_set_scrollbar_mode(ui->index_cont_1, LV_SCROLLBAR_MODE_AUTO);

    //Write style for index_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_cont_1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->index_cont_1, lv_color_hex(0x2195f6),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->index_cont_1, LV_BORDER_SIDE_FULL,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_cont_2
    ui->index_cont_2 = lv_obj_create(ui->index);
    lv_obj_set_pos(ui->index_cont_2, 15, 71);
    lv_obj_set_size(ui->index_cont_2, 381, 144);
    lv_obj_set_scrollbar_mode(ui->index_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for index_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_cont_2, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_cont_2, 78, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->index_cont_2, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->index_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_water_info
    ui->index_water_info = lv_label_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_water_info, 178, 58);
    lv_obj_set_size(ui->index_water_info, 186, 24);
    lv_label_set_text(ui->index_water_info, "水量：");
    lv_label_set_long_mode(ui->index_water_info, LV_LABEL_LONG_WRAP);

    //Write style for index_water_info, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->index_water_info, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->index_water_info, &lv_font_SourceHanSansSC_Regular_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->index_water_info, 235, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->index_water_info, LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_water_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_time_info
    ui->index_time_info = lv_label_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_time_info, 177, 26);
    lv_obj_set_size(ui->index_time_info, 170, 24);
    lv_label_set_text(ui->index_time_info, "时间： 10min");
    lv_label_set_long_mode(ui->index_time_info, LV_LABEL_LONG_WRAP);

    //Write style for index_time_info, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->index_time_info, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->index_time_info, &lv_font_SourceHanSansSC_Regular_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->index_time_info, 235, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->index_time_info, LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_time_info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_temp_value
    ui->index_temp_value = lv_label_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_temp_value, 20, 16);
    lv_obj_set_size(ui->index_temp_value, 154, 70);
    lv_label_set_text(ui->index_temp_value, "35℃");
    lv_label_set_long_mode(ui->index_temp_value, LV_LABEL_LONG_WRAP);

    //Write style for index_temp_value, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->index_temp_value, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->index_temp_value, &lv_font_SourceHanSansSC_Regular_70,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->index_temp_value, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->index_temp_value, LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_temp_value, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_water1
    ui->index_img_water1 = lv_image_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_img_water1, 234, 53);
    lv_obj_set_size(ui->index_img_water1, 30, 30);
    lv_obj_add_flag(ui->index_img_water1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->index_img_water1, &_water_RGB565A8_30x30);
    lv_image_set_pivot(ui->index_img_water1, 50, 50);
    lv_image_set_rotation(ui->index_img_water1, 0);

    //Write style for index_img_water1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_water1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_water1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_water2
    ui->index_img_water2 = lv_image_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_img_water2, 254, 53);
    lv_obj_set_size(ui->index_img_water2, 30, 30);
    lv_obj_add_flag(ui->index_img_water2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->index_img_water2, &_water_RGB565A8_30x30);
    lv_image_set_pivot(ui->index_img_water2, 50, 50);
    lv_image_set_rotation(ui->index_img_water2, 0);

    //Write style for index_img_water2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_water2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_water2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_water3
    ui->index_img_water3 = lv_image_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_img_water3, 273, 53);
    lv_obj_set_size(ui->index_img_water3, 30, 30);
    lv_obj_add_flag(ui->index_img_water3, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->index_img_water3, &_water_RGB565A8_30x30);
    lv_image_set_pivot(ui->index_img_water3, 50, 50);
    lv_image_set_rotation(ui->index_img_water3, 0);

    //Write style for index_img_water3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_water3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_water3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_medicine1
    ui->index_img_medicine1 = lv_image_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_img_medicine1, 220, 93);
    lv_obj_set_size(ui->index_img_medicine1, 30, 30);
    lv_obj_add_flag(ui->index_img_medicine1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->index_img_medicine1, &_medicine1_RGB565A8_30x30);
    lv_image_set_pivot(ui->index_img_medicine1, 50, 50);
    lv_image_set_rotation(ui->index_img_medicine1, 0);

    //Write style for index_img_medicine1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_medicine1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_medicine1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_medicine2
    ui->index_img_medicine2 = lv_image_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_img_medicine2, 256, 93);
    lv_obj_set_size(ui->index_img_medicine2, 30, 30);
    lv_obj_add_flag(ui->index_img_medicine2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->index_img_medicine2, &_medicine2_RGB565A8_30x30);
    lv_image_set_pivot(ui->index_img_medicine2, 50, 50);
    lv_image_set_rotation(ui->index_img_medicine2, 0);

    //Write style for index_img_medicine2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_medicine2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_medicine2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_need_liquid
    ui->index_img_need_liquid = lv_image_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_img_need_liquid, 36, 93);
    lv_obj_set_size(ui->index_img_need_liquid, 30, 30);
    lv_obj_add_flag(ui->index_img_need_liquid, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(ui->index_img_need_liquid, LV_OBJ_FLAG_HIDDEN);
    lv_image_set_src(ui->index_img_need_liquid, &_need_liquid_RGB565A8_30x30);
    lv_image_set_pivot(ui->index_img_need_liquid, 50, 50);
    lv_image_set_rotation(ui->index_img_need_liquid, 0);

    //Write style for index_img_need_liquid, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_need_liquid, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_label_need_liquid
    ui->index_label_need_liquid = lv_label_create(ui->index_cont_2);
    lv_obj_set_pos(ui->index_label_need_liquid, 69, 100);
    lv_obj_set_size(ui->index_label_need_liquid, 150, 32);
    lv_label_set_text(ui->index_label_need_liquid, "");
    lv_label_set_long_mode(ui->index_label_need_liquid, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(ui->index_label_need_liquid, LV_OBJ_FLAG_HIDDEN);

    //Write style for index_label_need_liquid, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->index_label_need_liquid, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->index_label_need_liquid, &lv_font_SourceHanSansSC_Regular_18,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->index_label_need_liquid, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->index_label_need_liquid, LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_label_need_liquid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_cont_clean
    ui->index_cont_clean = lv_obj_create(ui->index);
    lv_obj_set_pos(ui->index_cont_clean, 15, 71);
    lv_obj_set_size(ui->index_cont_clean, 381, 144);
    lv_obj_set_scrollbar_mode(ui->index_cont_clean, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->index_cont_clean, LV_OBJ_FLAG_HIDDEN);

    //Write style for index_cont_clean, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_cont_clean, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_cont_clean, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_cont_clean, 78, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->index_cont_clean, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->index_cont_clean, LV_GRAD_DIR_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_cont_clean, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_cont_clean, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_cont_clean, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_cont_clean, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_cont_clean, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_img_2
    ui->index_img_2 = lv_image_create(ui->index_cont_clean);
    lv_obj_set_pos(ui->index_img_2, 14, 23);
    lv_obj_set_size(ui->index_img_2, 100, 100);
    lv_obj_add_flag(ui->index_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->index_img_2, &_clean_RGB565A8_100x100);
    lv_image_set_pivot(ui->index_img_2, 50, 50);
    lv_image_set_rotation(ui->index_img_2, 0);

    //Write style for index_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->index_img_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->index_img_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    //Write codes index_label_1
    ui->index_label_1 = lv_label_create(ui->index_cont_clean);
    lv_obj_set_pos(ui->index_label_1, 104, 49);
    lv_obj_set_size(ui->index_label_1, 249, 49);
    lv_label_set_text(ui->index_label_1, "开启自动清洁");
    lv_label_set_long_mode(ui->index_label_1, LV_LABEL_LONG_WRAP);

    //Write style for index_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->index_label_1, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->index_label_1, &lv_font_SourceHanSansSC_Regular_40,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->index_label_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->index_label_1, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->index_label_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //The custom code of index.


    //Update current screen layout.
    lv_obj_update_layout(ui->index);

    //Init events for screen.
    events_init_index(ui);
}
