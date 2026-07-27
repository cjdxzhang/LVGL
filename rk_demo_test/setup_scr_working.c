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



void setup_scr_working(lv_ui *ui)
{
    //Write codes working
    ui->working = lv_obj_create(NULL);
    lv_obj_set_size(ui->working, 800, 480);
    lv_obj_set_scrollbar_mode(ui->working, LV_SCROLLBAR_MODE_OFF);

    //Write style for working, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->working, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_1
    ui->working_img_1 = lv_image_create(ui->working);
    lv_obj_set_pos(ui->working_img_1, 0, 0);
    lv_obj_set_size(ui->working_img_1, 800, 480);
    lv_obj_add_flag(ui->working_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->working_img_1, 50,50);
    lv_image_set_rotation(ui->working_img_1, 0);

    //Write style for working_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_cont_playing
    ui->working_cont_playing = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_cont_playing, 15, 71);
    lv_obj_set_size(ui->working_cont_playing, 380, 144);
    lv_obj_set_scrollbar_mode(ui->working_cont_playing, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->working_cont_playing, LV_OBJ_FLAG_HIDDEN);

    //Write style for working_cont_playing, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_cont_playing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_playing, 50, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_playing, 80, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_cont_playing, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_cont_playing, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_playing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_playing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_playing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_playing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_playing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_label_time_left
    ui->working_label_time_left = lv_label_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_label_time_left, 274, 27);
    lv_obj_set_size(ui->working_label_time_left, 102, 24);
    lv_label_set_text(ui->working_label_time_left, "10min");
    lv_label_set_long_mode(ui->working_label_time_left, LV_LABEL_LONG_WRAP);

    //Write style for working_label_time_left, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_label_time_left, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_label_time_left, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_label_time_left, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_label_time_left, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_label_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_label_temperature
    ui->working_label_temperature = lv_label_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_label_temperature, 8, 29);
    lv_obj_set_size(ui->working_label_temperature, 203, 80);
    lv_label_set_text(ui->working_label_temperature, "50℃");
    lv_label_set_long_mode(ui->working_label_temperature, LV_LABEL_LONG_WRAP);

    //Write style for working_label_temperature, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_label_temperature, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_label_temperature, &lv_font_SourceHanSansSC_Regular_80, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_label_temperature, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_label_temperature, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_label_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_2
    ui->working_img_2 = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_2, 274, 56);
    lv_obj_set_size(ui->working_img_2, 34, 34);
    lv_obj_add_flag(ui->working_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_2, &_mode_foot3_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_2, 50,50);
    lv_image_set_rotation(ui->working_img_2, 0);

    //Write style for working_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_massage_intensity0
    ui->working_img_massage_intensity0 = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_massage_intensity0, 230, 56);
    lv_obj_set_size(ui->working_img_massage_intensity0, 34, 34);
    lv_obj_add_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_massage_intensity0, &_minus_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_massage_intensity0, 50,50);
    lv_image_set_rotation(ui->working_img_massage_intensity0, 900);

    //Write style for working_img_massage_intensity0, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_massage_intensity0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_massage_intensity0, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_massage_intensity1
    ui->working_img_massage_intensity1 = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_massage_intensity1, 247, 56);
    lv_obj_set_size(ui->working_img_massage_intensity1, 34, 34);
    lv_obj_add_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_massage_intensity1, &_minus_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_massage_intensity1, 50,50);
    lv_image_set_rotation(ui->working_img_massage_intensity1, 900);

    //Write style for working_img_massage_intensity1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_massage_intensity1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_massage_intensity1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_massage_intensity2
    ui->working_img_massage_intensity2 = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_massage_intensity2, 264, 56);
    lv_obj_set_size(ui->working_img_massage_intensity2, 34, 34);
    lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_massage_intensity2, &_minus_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_massage_intensity2, 50,50);
    lv_image_set_rotation(ui->working_img_massage_intensity2, 900);

    //Write style for working_img_massage_intensity2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_massage_intensity2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_massage_intensity2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_constant_temperature
    ui->working_img_constant_temperature = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_constant_temperature, 217, 96);
    lv_obj_set_size(ui->working_img_constant_temperature, 32, 27);
    lv_obj_add_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_constant_temperature, &_worm_RGB565A8_32x27);
    lv_image_set_pivot(ui->working_img_constant_temperature, 50,50);
    lv_image_set_rotation(ui->working_img_constant_temperature, 0);

    //Write style for working_img_constant_temperature, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_constant_temperature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_constant_temperature, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_sterilization
    ui->working_img_sterilization = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_sterilization, 252, 94);
    lv_obj_set_size(ui->working_img_sterilization, 34, 34);
    lv_obj_add_flag(ui->working_img_sterilization, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_sterilization, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_sterilization, &_bacteria_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_sterilization, 50,50);
    lv_image_set_rotation(ui->working_img_sterilization, 0);

    //Write style for working_img_sterilization, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_sterilization, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_sterilization, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_label_5
    ui->working_label_5 = lv_label_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_label_5, 217, 62);
    lv_obj_set_size(ui->working_label_5, 65, 24);
    lv_label_set_text(ui->working_label_5, "按摩：");
    lv_label_set_long_mode(ui->working_label_5, LV_LABEL_LONG_WRAP);

    //Write style for working_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_label_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_label_5, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_label_5, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_label_5, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_label_9
    ui->working_label_9 = lv_label_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_label_9, 217, 27);
    lv_obj_set_size(ui->working_label_9, 68, 24);
    lv_label_set_text(ui->working_label_9, "时间：");
    lv_label_set_long_mode(ui->working_label_9, LV_LABEL_LONG_WRAP);

    //Write style for working_label_9, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_label_9, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_label_9, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_label_9, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_label_9, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_cont_pause
    ui->working_cont_pause = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_cont_pause, 15, 71);
    lv_obj_set_size(ui->working_cont_pause, 380, 144);
    lv_obj_set_scrollbar_mode(ui->working_cont_pause, LV_SCROLLBAR_MODE_OFF);

    //Write style for working_cont_pause, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_cont_pause, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_pause, 50, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_pause, 80, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_cont_pause, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_cont_pause, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_pause, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_pause, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_pause, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_pause, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_pause, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_label_6
    ui->working_label_6 = lv_label_create(ui->working_cont_pause);
    lv_obj_set_pos(ui->working_label_6, 20, -11);
    lv_obj_set_size(ui->working_label_6, 342, 102);
    lv_label_set_text(ui->working_label_6, "_ _");
    lv_label_set_long_mode(ui->working_label_6, LV_LABEL_LONG_WRAP);

    //Write style for working_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_label_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_label_6, &lv_font_montserratMedium_90, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_cont_2
    ui->working_cont_2 = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_cont_2, 15, 239);
    lv_obj_set_size(ui->working_cont_2, 399, 230);
    lv_obj_set_scrollbar_mode(ui->working_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for working_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_btn_6
    ui->working_btn_6 = lv_button_create(ui->working);
    lv_obj_set_pos(ui->working_btn_6, 15, 71);
    lv_obj_set_size(ui->working_btn_6, 380, 144);
    lv_obj_add_flag(ui->working_btn_6, LV_OBJ_FLAG_HIDDEN);
    ui->working_btn_6_label = lv_label_create(ui->working_btn_6);
    lv_label_set_text(ui->working_btn_6_label, "");
    lv_label_set_long_mode(ui->working_btn_6_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->working_btn_6_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->working_btn_6, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->working_btn_6_label, LV_PCT(100));

    //Write style for working_btn_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->working_btn_6, 76, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_btn_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_btn_6, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->working_btn_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_btn_6, 28, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_btn_6, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(ui->working_btn_6, lv_color_hex(0x2a3a47), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(ui->working_btn_6, 145, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(ui->working_btn_6, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_x(ui->working_btn_6, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_y(ui->working_btn_6, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_btn_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_btn_6, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_btn_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_btn_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_cont_1
    ui->working_cont_1 = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_cont_1, 420, 71);
    lv_obj_set_size(ui->working_cont_1, 377, 391);
    lv_obj_set_scrollbar_mode(ui->working_cont_1, LV_SCROLLBAR_MODE_AUTO);

    //Write style for working_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->working_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->working_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_dialog
    ui->working_dialog = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_dialog, 0, 0);
    lv_obj_set_size(ui->working_dialog, 800, 480);
    lv_obj_set_scrollbar_mode(ui->working_dialog, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->working_dialog, LV_OBJ_FLAG_HIDDEN);

    //Write style for working_dialog, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_dialog, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->working_dialog, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->working_dialog, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->working_dialog, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_dialog, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_dialog, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_dialog, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_dialog, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_dialog, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_dialog, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_dialog, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_dialog, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_dialog, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_img_3
    ui->working_img_3 = lv_image_create(ui->working_dialog);
    lv_obj_set_pos(ui->working_img_3, 0, 0);
    lv_obj_set_size(ui->working_img_3, 800, 480);
    lv_obj_add_flag(ui->working_img_3, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_3, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->working_img_3, 50,50);
    lv_image_set_rotation(ui->working_img_3, 0);

    //Write style for working_img_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->working_img_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_cont_3
    ui->working_cont_3 = lv_obj_create(ui->working_dialog);
    lv_obj_set_pos(ui->working_cont_3, 112, 95);
    lv_obj_set_size(ui->working_cont_3, 572, 331);
    lv_obj_set_scrollbar_mode(ui->working_cont_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for working_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_3, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_3, 52, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_cont_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_cont_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_label_8
    ui->working_label_8 = lv_label_create(ui->working_cont_3);
    lv_obj_set_pos(ui->working_label_8, 13, 116);
    lv_obj_set_size(ui->working_label_8, 537, 32);
    lv_label_set_text(ui->working_label_8, "启动前请确认已取出双脚");
    lv_label_set_long_mode(ui->working_label_8, LV_LABEL_LONG_WRAP);

    //Write style for working_label_8, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_label_8, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_label_8, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_label_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_label_8, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_btn_no
    ui->working_btn_no = lv_button_create(ui->working_cont_3);
    lv_obj_set_pos(ui->working_btn_no, 299, 197);
    lv_obj_set_size(ui->working_btn_no, 250, 108);
    ui->working_btn_no_label = lv_label_create(ui->working_btn_no);
    lv_label_set_text(ui->working_btn_no_label, "否");
    lv_label_set_long_mode(ui->working_btn_no_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->working_btn_no_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->working_btn_no, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->working_btn_no_label, LV_PCT(100));

    //Write style for working_btn_no, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->working_btn_no, 51, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_btn_no, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_btn_no, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->working_btn_no, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_btn_no, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_btn_no, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_btn_no, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_btn_no, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_btn_no, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_btn_no, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_btn_yes
    ui->working_btn_yes = lv_button_create(ui->working_cont_3);
    lv_obj_set_pos(ui->working_btn_yes, 26, 197);
    lv_obj_set_size(ui->working_btn_yes, 250, 108);
    ui->working_btn_yes_label = lv_label_create(ui->working_btn_yes);
    lv_label_set_text(ui->working_btn_yes_label, "是");
    lv_label_set_long_mode(ui->working_btn_yes_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->working_btn_yes_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->working_btn_yes, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->working_btn_yes_label, LV_PCT(100));

    //Write style for working_btn_yes, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->working_btn_yes, 51, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_btn_yes, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_btn_yes, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->working_btn_yes, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_btn_yes, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_btn_yes, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_btn_yes, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_btn_yes, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_btn_yes, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_btn_yes, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes working_label_7
    ui->working_label_7 = lv_label_create(ui->working_cont_3);
    lv_obj_set_pos(ui->working_label_7, 17, 55);
    lv_obj_set_size(ui->working_label_7, 537, 40);
    lv_label_set_text(ui->working_label_7, "是否返回基站？");
    lv_label_set_long_mode(ui->working_label_7, LV_LABEL_LONG_WRAP);

    //Write style for working_label_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->working_label_7, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->working_label_7, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->working_label_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->working_label_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of working.


    //Update current screen layout.
    lv_obj_update_layout(ui->working);

    //Init events for screen.
    events_init_working(ui);
}
