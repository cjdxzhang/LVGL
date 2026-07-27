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



void setup_scr_preparing(lv_ui *ui)
{
    //Write codes preparing
    ui->preparing = lv_obj_create(NULL);
    lv_obj_set_size(ui->preparing, 800, 480);
    lv_obj_set_scrollbar_mode(ui->preparing, LV_SCROLLBAR_MODE_OFF);

    //Write style for preparing, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_1
    ui->preparing_img_1 = lv_image_create(ui->preparing);
    lv_obj_set_pos(ui->preparing_img_1, 0, 0);
    lv_obj_set_size(ui->preparing_img_1, 800, 480);
    lv_obj_add_flag(ui->preparing_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_1, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->preparing_img_1, 50,50);
    lv_image_set_rotation(ui->preparing_img_1, 0);

    //Write style for preparing_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cont_1
    ui->preparing_cont_1 = lv_obj_create(ui->preparing);
    lv_obj_set_pos(ui->preparing_cont_1, 14, 59);
    lv_obj_set_size(ui->preparing_cont_1, 574, 403);
    lv_obj_set_scrollbar_mode(ui->preparing_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for preparing_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cont_1, 50, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cont_1, 72, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->preparing_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->preparing_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cont_cleaning
    ui->preparing_cont_cleaning = lv_obj_create(ui->preparing_cont_1);
    lv_obj_set_pos(ui->preparing_cont_cleaning, 32, 20);
    lv_obj_set_size(ui->preparing_cont_cleaning, 510, 353);
    lv_obj_set_scrollbar_mode(ui->preparing_cont_cleaning, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->preparing_cont_cleaning, LV_OBJ_FLAG_HIDDEN);

    //Write style for preparing_cont_cleaning, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cont_cleaning, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_clean_time_left
    ui->preparing_label_clean_time_left = lv_label_create(ui->preparing_cont_cleaning);
    lv_obj_set_pos(ui->preparing_label_clean_time_left, 19, 116);
    lv_obj_set_size(ui->preparing_label_clean_time_left, 276, 84);
    lv_label_set_text(ui->preparing_label_clean_time_left, "10:25");
    lv_label_set_long_mode(ui->preparing_label_clean_time_left, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_clean_time_left, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_clean_time_left, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_clean_time_left, &lv_font_montserratMedium_80, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_clean_time_left, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_clean_time_left, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_clean_time_left, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cleaning_tips
    ui->preparing_cleaning_tips = lv_label_create(ui->preparing_cont_cleaning);
    lv_obj_set_pos(ui->preparing_cleaning_tips, -24, 290);
    lv_obj_set_size(ui->preparing_cleaning_tips, 560, 56);
    lv_label_set_text(ui->preparing_cleaning_tips, "清洁已完成，暖风烘干中...");
    lv_label_set_long_mode(ui->preparing_cleaning_tips, LV_LABEL_LONG_WRAP);

    //Write style for preparing_cleaning_tips, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_cleaning_tips, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_cleaning_tips, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_cleaning_tips, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_cleaning_tips, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cleaning_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_9
    ui->preparing_label_9 = lv_label_create(ui->preparing_cont_cleaning);
    lv_obj_set_pos(ui->preparing_label_9, 308, 156);
    lv_obj_set_size(ui->preparing_label_9, 97, 39);
    lv_label_set_text(ui->preparing_label_9, "s");
    lv_label_set_long_mode(ui->preparing_label_9, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_9, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_9, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_9, &lv_font_montserratMedium_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_9, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_9, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_10
    ui->preparing_label_10 = lv_label_create(ui->preparing_cont_cleaning);
    lv_obj_set_pos(ui->preparing_label_10, 305, 118);
    lv_obj_set_size(ui->preparing_label_10, 101, 39);
    lv_label_set_text(ui->preparing_label_10, "min");
    lv_label_set_long_mode(ui->preparing_label_10, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_10, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_10, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_10, &lv_font_montserratMedium_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_10, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_3
    ui->preparing_img_3 = lv_image_create(ui->preparing_cont_cleaning);
    lv_obj_set_pos(ui->preparing_img_3, 6, -2);
    lv_obj_set_size(ui->preparing_img_3, 57, 57);
    lv_obj_add_flag(ui->preparing_img_3, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_3, &_clean_RGB565A8_57x57);
    lv_image_set_pivot(ui->preparing_img_3, 50,50);
    lv_image_set_rotation(ui->preparing_img_3, 0);

    //Write style for preparing_img_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_12
    ui->preparing_label_12 = lv_label_create(ui->preparing_cont_cleaning);
    lv_obj_set_pos(ui->preparing_label_12, 63, 11);
    lv_obj_set_size(ui->preparing_label_12, 123, 32);
    lv_label_set_text(ui->preparing_label_12, "自动清洁");
    lv_label_set_long_mode(ui->preparing_label_12, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_12, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_12, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_12, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_12, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_12, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cont_move_finished
    ui->preparing_cont_move_finished = lv_obj_create(ui->preparing_cont_1);
    lv_obj_set_pos(ui->preparing_cont_move_finished, 50, 85);
    lv_obj_set_size(ui->preparing_cont_move_finished, 510, 303);
    lv_obj_set_scrollbar_mode(ui->preparing_cont_move_finished, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->preparing_cont_move_finished, LV_OBJ_FLAG_HIDDEN);

    //Write style for preparing_cont_move_finished, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cont_move_finished, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_17
    ui->preparing_label_17 = lv_label_create(ui->preparing_cont_move_finished);
    lv_obj_set_pos(ui->preparing_label_17, -35, 75);
    lv_obj_set_size(ui->preparing_label_17, 540, 40);
    lv_label_set_text(ui->preparing_label_17, "已到达指定位置");
    lv_label_set_long_mode(ui->preparing_label_17, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_17, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_17, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_17, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_17, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_17, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_timer_tips
    ui->preparing_label_timer_tips = lv_label_create(ui->preparing_cont_move_finished);
    lv_obj_set_pos(ui->preparing_label_timer_tips, -35, 123);
    lv_obj_set_size(ui->preparing_label_timer_tips, 540, 40);
    lv_label_set_text(ui->preparing_label_timer_tips, "(5s后享受健康足疗)");
    lv_label_set_long_mode(ui->preparing_label_timer_tips, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_timer_tips, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_timer_tips, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_timer_tips, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_timer_tips, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_timer_tips, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_timer_tips, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_timer_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cont_moving
    ui->preparing_cont_moving = lv_obj_create(ui->preparing_cont_1);
    lv_obj_set_pos(ui->preparing_cont_moving, 32, 53);
    lv_obj_set_size(ui->preparing_cont_moving, 510, 303);
    lv_obj_set_scrollbar_mode(ui->preparing_cont_moving, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->preparing_cont_moving, LV_OBJ_FLAG_HIDDEN);

    //Write style for preparing_cont_moving, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cont_moving, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_2
    ui->preparing_img_2 = lv_image_create(ui->preparing_cont_moving);
    lv_obj_set_pos(ui->preparing_img_2, 56, 84);
    lv_obj_set_size(ui->preparing_img_2, 100, 100);
    lv_obj_add_flag(ui->preparing_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_2, &_warnning_RGB565A8_100x100);
    lv_image_set_pivot(ui->preparing_img_2, 50,50);
    lv_image_set_rotation(ui->preparing_img_2, 0);

    //Write style for preparing_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_11
    ui->preparing_label_11 = lv_label_create(ui->preparing_cont_moving);
    lv_obj_set_pos(ui->preparing_label_11, 183, 58);
    lv_obj_set_size(ui->preparing_label_11, 300, 174);
    lv_label_set_text(ui->preparing_label_11, "移动中\n请注意避让\n请物操作");
    lv_label_set_long_mode(ui->preparing_label_11, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_11, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_11, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_11, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_11, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_11, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cont_preparing
    ui->preparing_cont_preparing = lv_obj_create(ui->preparing_cont_1);
    lv_obj_set_pos(ui->preparing_cont_preparing, 32, 20);
    lv_obj_set_size(ui->preparing_cont_preparing, 510, 364);
    lv_obj_set_scrollbar_mode(ui->preparing_cont_preparing, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->preparing_cont_preparing, LV_OBJ_FLAG_HIDDEN);

    //Write style for preparing_cont_preparing, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cont_preparing, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_4
    ui->preparing_label_4 = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_label_4, 229, 184);
    lv_obj_set_size(ui->preparing_label_4, 43, 24);
    lv_label_set_text(ui->preparing_label_4, "水量");
    lv_label_set_long_mode(ui->preparing_label_4, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_4, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_4, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_4, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_5
    ui->preparing_label_5 = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_label_5, 373, 187);
    lv_obj_set_size(ui->preparing_label_5, 44, 26);
    lv_label_set_text(ui->preparing_label_5, "药液");
    lv_label_set_long_mode(ui->preparing_label_5, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_5, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_5, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_5, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_5, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_temprature
    ui->preparing_label_temprature = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_label_temprature, 37, 123);
    lv_obj_set_size(ui->preparing_label_temprature, 173, 91);
    lv_label_set_text(ui->preparing_label_temprature, "50℃");
    lv_label_set_long_mode(ui->preparing_label_temprature, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_temprature, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_temprature, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_temprature, &lv_font_SourceHanSansSC_Regular_80, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_temprature, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_temprature, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_temprature, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_3
    ui->preparing_label_3 = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_label_3, 225, 131);
    lv_obj_set_size(ui->preparing_label_3, 61, 21);
    lv_label_set_text(ui->preparing_label_3, "时间：");
    lv_label_set_long_mode(ui->preparing_label_3, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_3, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_3, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_3, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_3, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_13
    ui->preparing_label_13 = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_label_13, 55, 15);
    lv_obj_set_size(ui->preparing_label_13, 123, 32);
    lv_label_set_text(ui->preparing_label_13, "自动注水");
    lv_label_set_long_mode(ui->preparing_label_13, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_13, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_13, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_13, &lv_font_SourceHanSansSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_13, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_13, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_4
    ui->preparing_img_4 = lv_image_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_img_4, -2, 2);
    lv_obj_set_size(ui->preparing_img_4, 57, 57);
    lv_obj_add_flag(ui->preparing_img_4, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_4, &_water_RGB565A8_57x57);
    lv_image_set_pivot(ui->preparing_img_4, 50,50);
    lv_image_set_rotation(ui->preparing_img_4, 0);

    //Write style for preparing_img_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_timeleft
    ui->preparing_label_timeleft = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_label_timeleft, 277, 121);
    lv_obj_set_size(ui->preparing_label_timeleft, 128, 51);
    lv_label_set_text(ui->preparing_label_timeleft, "10:25");
    lv_label_set_long_mode(ui->preparing_label_timeleft, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_timeleft, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_timeleft, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_timeleft, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_timeleft, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_timeleft, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_timeleft, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_15
    ui->preparing_label_15 = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_label_15, 391, 125);
    lv_obj_set_size(ui->preparing_label_15, 61, 42);
    lv_label_set_text(ui->preparing_label_15, "min\ns");
    lv_label_set_long_mode(ui->preparing_label_15, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_15, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_15, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_15, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_15, 235, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_15, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_water3
    ui->preparing_img_water3 = lv_image_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_img_water3, 310, 181);
    lv_obj_set_size(ui->preparing_img_water3, 30, 30);
    lv_obj_add_flag(ui->preparing_img_water3, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_water3, &_water_RGB565A8_30x30);
    lv_image_set_pivot(ui->preparing_img_water3, 50,50);
    lv_image_set_rotation(ui->preparing_img_water3, 0);

    //Write style for preparing_img_water3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_water3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_water3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_water2
    ui->preparing_img_water2 = lv_image_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_img_water2, 291, 181);
    lv_obj_set_size(ui->preparing_img_water2, 30, 30);
    lv_obj_add_flag(ui->preparing_img_water2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_water2, &_water_RGB565A8_30x30);
    lv_image_set_pivot(ui->preparing_img_water2, 50,50);
    lv_image_set_rotation(ui->preparing_img_water2, 0);

    //Write style for preparing_img_water2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_water2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_water2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_water1
    ui->preparing_img_water1 = lv_image_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_img_water1, 271, 181);
    lv_obj_set_size(ui->preparing_img_water1, 30, 30);
    lv_obj_add_flag(ui->preparing_img_water1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_water1, &_water_RGB565A8_30x30);
    lv_image_set_pivot(ui->preparing_img_water1, 50,50);
    lv_image_set_rotation(ui->preparing_img_water1, 0);

    //Write style for preparing_img_water1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_water1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_water1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_preparing_tips
    ui->preparing_preparing_tips = lv_label_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_preparing_tips, -24, 281);
    lv_obj_set_size(ui->preparing_preparing_tips, 560, 73);
    lv_label_set_text(ui->preparing_preparing_tips, "注水已完成，请尽快取出，\n享受健康足疗");
    lv_label_set_long_mode(ui->preparing_preparing_tips, LV_LABEL_LONG_WRAP);

    //Write style for preparing_preparing_tips, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_preparing_tips, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_preparing_tips, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_preparing_tips, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_preparing_tips, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_preparing_tips, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_preparing_tips, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_medicine2
    ui->preparing_img_medicine2 = lv_image_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_img_medicine2, 444, 183);
    lv_obj_set_size(ui->preparing_img_medicine2, 30, 30);
    lv_obj_add_flag(ui->preparing_img_medicine2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_medicine2, &_medicine2_RGB565A8_30x30);
    lv_image_set_pivot(ui->preparing_img_medicine2, 50,50);
    lv_image_set_rotation(ui->preparing_img_medicine2, 0);

    //Write style for preparing_img_medicine2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_medicine2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_medicine2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_medicine1
    ui->preparing_img_medicine1 = lv_image_create(ui->preparing_cont_preparing);
    lv_obj_set_pos(ui->preparing_img_medicine1, 416, 182);
    lv_obj_set_size(ui->preparing_img_medicine1, 30, 30);
    lv_obj_add_flag(ui->preparing_img_medicine1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_medicine1, &_medicine1_RGB565A8_30x30);
    lv_image_set_pivot(ui->preparing_img_medicine1, 50,50);
    lv_image_set_rotation(ui->preparing_img_medicine1, 0);

    //Write style for preparing_img_medicine1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_medicine1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_medicine1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cont_3
    ui->preparing_cont_3 = lv_obj_create(ui->preparing);
    lv_obj_set_pos(ui->preparing_cont_3, 615, 59);
    lv_obj_set_size(ui->preparing_cont_3, 184, 409);
    lv_obj_set_scrollbar_mode(ui->preparing_cont_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for preparing_cont_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cont_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_dialog1
    ui->preparing_dialog1 = lv_obj_create(ui->preparing);
    lv_obj_set_pos(ui->preparing_dialog1, 0, 0);
    lv_obj_set_size(ui->preparing_dialog1, 800, 480);
    lv_obj_set_scrollbar_mode(ui->preparing_dialog1, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->preparing_dialog1, LV_OBJ_FLAG_HIDDEN);

    //Write style for preparing_dialog1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_dialog1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_dialog1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_dialog1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->preparing_dialog1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->preparing_dialog1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_dialog1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_dialog1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_dialog1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_dialog1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_dialog1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_img_5
    ui->preparing_img_5 = lv_image_create(ui->preparing_dialog1);
    lv_obj_set_pos(ui->preparing_img_5, 0, 0);
    lv_obj_set_size(ui->preparing_img_5, 800, 480);
    lv_obj_add_flag(ui->preparing_img_5, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->preparing_img_5, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->preparing_img_5, 50,50);
    lv_image_set_rotation(ui->preparing_img_5, 0);

    //Write style for preparing_img_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->preparing_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->preparing_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_cont_2
    ui->preparing_cont_2 = lv_obj_create(ui->preparing_dialog1);
    lv_obj_set_pos(ui->preparing_cont_2, 112, 96);
    lv_obj_set_size(ui->preparing_cont_2, 576, 331);
    lv_obj_set_scrollbar_mode(ui->preparing_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for preparing_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_cont_2, 30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_cont_2, 49, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->preparing_cont_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->preparing_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_btn_no
    ui->preparing_btn_no = lv_button_create(ui->preparing_cont_2);
    lv_obj_set_pos(ui->preparing_btn_no, 299, 197);
    lv_obj_set_size(ui->preparing_btn_no, 250, 108);
    ui->preparing_btn_no_label = lv_label_create(ui->preparing_btn_no);
    lv_label_set_text(ui->preparing_btn_no_label, "否");
    lv_label_set_long_mode(ui->preparing_btn_no_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->preparing_btn_no_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->preparing_btn_no, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->preparing_btn_no_label, LV_PCT(100));

    //Write style for preparing_btn_no, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->preparing_btn_no, 79, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->preparing_btn_no, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->preparing_btn_no, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->preparing_btn_no, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_btn_no, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_btn_no, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_btn_no, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_btn_no, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_btn_no, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_btn_no, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_btn_yes
    ui->preparing_btn_yes = lv_button_create(ui->preparing_cont_2);
    lv_obj_set_pos(ui->preparing_btn_yes, 26, 197);
    lv_obj_set_size(ui->preparing_btn_yes, 250, 108);
    ui->preparing_btn_yes_label = lv_label_create(ui->preparing_btn_yes);
    lv_label_set_text(ui->preparing_btn_yes_label, "是");
    lv_label_set_long_mode(ui->preparing_btn_yes_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->preparing_btn_yes_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->preparing_btn_yes, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->preparing_btn_yes_label, LV_PCT(100));

    //Write style for preparing_btn_yes, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->preparing_btn_yes, 78, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->preparing_btn_yes, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->preparing_btn_yes, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->preparing_btn_yes, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_btn_yes, 20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_btn_yes, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_btn_yes, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_btn_yes, &lv_font_SourceHanSansSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_btn_yes, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_btn_yes, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes preparing_label_6
    ui->preparing_label_6 = lv_label_create(ui->preparing_cont_2);
    lv_obj_set_pos(ui->preparing_label_6, 87, 54);
    lv_obj_set_size(ui->preparing_label_6, 421, 56);
    lv_label_set_text(ui->preparing_label_6, "是否结束当前工作？");
    lv_label_set_long_mode(ui->preparing_label_6, LV_LABEL_LONG_WRAP);

    //Write style for preparing_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->preparing_label_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_6, &lv_font_SourceHanSansSC_Regular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->preparing_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->preparing_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->preparing_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of preparing.


    //Update current screen layout.
    lv_obj_update_layout(ui->preparing);

    //Init events for screen.
    events_init_preparing(ui);
}
