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



void setup_scr_screen_img_list(lv_ui *ui)
{
    //Write codes screen_img_list
    ui->screen_img_list = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_img_list, 800, 480);
    lv_obj_set_scrollbar_mode(ui->screen_img_list, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_img_list, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_img_list, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_1
    ui->screen_img_list_img_1 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_1, 25, 36);
    lv_obj_set_size(ui->screen_img_list_img_1, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_1, &_home_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_1, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_1, 0);

    //Write style for screen_img_list_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_2
    ui->screen_img_list_img_2 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_2, 128, 50);
    lv_obj_set_size(ui->screen_img_list_img_2, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_2, &_return_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_2, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_2, 0);

    //Write style for screen_img_list_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_3
    ui->screen_img_list_img_3 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_3, 263, 50);
    lv_obj_set_size(ui->screen_img_list_img_3, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_3, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_3, &_add_one_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_3, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_3, 0);

    //Write style for screen_img_list_img_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_5
    ui->screen_img_list_img_5 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_5, 250, 140);
    lv_obj_set_size(ui->screen_img_list_img_5, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_5, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_5, &_medicine1_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_5, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_5, 0);

    //Write style for screen_img_list_img_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_6
    ui->screen_img_list_img_6 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_6, 366, 136);
    lv_obj_set_size(ui->screen_img_list_img_6, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_6, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_6, &_massage_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_6, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_6, 0);

    //Write style for screen_img_list_img_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_7
    ui->screen_img_list_img_7 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_7, 451, 136);
    lv_obj_set_size(ui->screen_img_list_img_7, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_7, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_7, &_location_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_7, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_7, 0);

    //Write style for screen_img_list_img_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_8
    ui->screen_img_list_img_8 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_8, 552, 150);
    lv_obj_set_size(ui->screen_img_list_img_8, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_8, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_8, &_local1_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_8, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_8, 0);

    //Write style for screen_img_list_img_8, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_9
    ui->screen_img_list_img_9 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_9, 664, 147);
    lv_obj_set_size(ui->screen_img_list_img_9, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_9, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_9, &_local_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_9, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_9, 0);

    //Write style for screen_img_list_img_9, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_9, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_10
    ui->screen_img_list_img_10 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_10, 621, 29);
    lv_obj_set_size(ui->screen_img_list_img_10, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_10, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_10, &_clean_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_10, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_10, 0);

    //Write style for screen_img_list_img_10, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_11
    ui->screen_img_list_img_11 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_11, 507, 33);
    lv_obj_set_size(ui->screen_img_list_img_11, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_11, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_11, &_bacteria_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_11, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_11, 0);

    //Write style for screen_img_list_img_11, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_12
    ui->screen_img_list_img_12 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_12, 380, 36);
    lv_obj_set_size(ui->screen_img_list_img_12, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_12, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_12, &_auto_back_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_12, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_12, 0);

    //Write style for screen_img_list_img_12, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_12, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_13
    ui->screen_img_list_img_13 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_13, 431, 246);
    lv_obj_set_size(ui->screen_img_list_img_13, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_13, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_13, &_mode_foot2_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_13, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_13, 0);

    //Write style for screen_img_list_img_13, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_13, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_14
    ui->screen_img_list_img_14 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_14, 543, 250);
    lv_obj_set_size(ui->screen_img_list_img_14, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_14, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_14, &_mode_foot1_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_14, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_14, 0);

    //Write style for screen_img_list_img_14, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_14, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_15
    ui->screen_img_list_img_15 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_15, 659, 256);
    lv_obj_set_size(ui->screen_img_list_img_15, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_15, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_15, &_mode_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_15, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_15, 0);

    //Write style for screen_img_list_img_15, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_15, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_16
    ui->screen_img_list_img_16 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_16, -93, 485);
    lv_obj_set_size(ui->screen_img_list_img_16, 50, 50);
    lv_obj_add_flag(ui->screen_img_list_img_16, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_16, &_minus_RGB565A8_50x50);
    lv_image_set_pivot(ui->screen_img_list_img_16, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_16, 0);

    //Write style for screen_img_list_img_16, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_17
    ui->screen_img_list_img_17 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_17, 145, 147);
    lv_obj_set_size(ui->screen_img_list_img_17, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_17, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_17, &_medicine2_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_17, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_17, 0);

    //Write style for screen_img_list_img_17, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_18
    ui->screen_img_list_img_18 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_18, 171, 240);
    lv_obj_set_size(ui->screen_img_list_img_18, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_18, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_18, &_mode_relax1_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_18, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_18, 0);

    //Write style for screen_img_list_img_18, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_19
    ui->screen_img_list_img_19 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_19, 263, 246);
    lv_obj_set_size(ui->screen_img_list_img_19, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_19, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_19, &_mode_foot4_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_19, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_19, 0);

    //Write style for screen_img_list_img_19, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_20
    ui->screen_img_list_img_20 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_20, 75, 236);
    lv_obj_set_size(ui->screen_img_list_img_20, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_20, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_20, &_mode_relax2_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_20, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_20, 0);

    //Write style for screen_img_list_img_20, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_21
    ui->screen_img_list_img_21 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_21, 335, 250);
    lv_obj_set_size(ui->screen_img_list_img_21, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_21, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_21, &_mode_foot3_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_21, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_21, 0);

    //Write style for screen_img_list_img_21, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_21, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_22
    ui->screen_img_list_img_22 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_22, 424, 364);
    lv_obj_set_size(ui->screen_img_list_img_22, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_22, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_22, &_mode_sleep3_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_22, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_22, 0);

    //Write style for screen_img_list_img_22, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_22, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_23
    ui->screen_img_list_img_23 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_23, 507, 346);
    lv_obj_set_size(ui->screen_img_list_img_23, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_23, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_23, &_mode_sleep2_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_23, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_23, 0);

    //Write style for screen_img_list_img_23, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_23, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_24
    ui->screen_img_list_img_24 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_24, 586, 356);
    lv_obj_set_size(ui->screen_img_list_img_24, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_24, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_24, &_mode_sleep1_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_24, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_24, 0);

    //Write style for screen_img_list_img_24, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_24, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_25
    ui->screen_img_list_img_25 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_25, 679, 350);
    lv_obj_set_size(ui->screen_img_list_img_25, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_25, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_25, &_mode_relax4_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_25, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_25, 0);

    //Write style for screen_img_list_img_25, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_25, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_26
    ui->screen_img_list_img_26 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_26, 7, 221);
    lv_obj_set_size(ui->screen_img_list_img_26, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_26, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_26, &_mode_relax3_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_26, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_26, 0);

    //Write style for screen_img_list_img_26, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_26, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_27
    ui->screen_img_list_img_27 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_27, 600, 474);
    lv_obj_set_size(ui->screen_img_list_img_27, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_27, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_27, &_setting_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_27, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_27, 0);

    //Write style for screen_img_list_img_27, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_27, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_28
    ui->screen_img_list_img_28 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_28, -34, 488);
    lv_obj_set_size(ui->screen_img_list_img_28, 50, 50);
    lv_obj_add_flag(ui->screen_img_list_img_28, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_28, &_plus_RGB565A8_50x50);
    lv_image_set_pivot(ui->screen_img_list_img_28, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_28, 0);

    //Write style for screen_img_list_img_28, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_28, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_29
    ui->screen_img_list_img_29 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_29, 138, 317);
    lv_obj_set_size(ui->screen_img_list_img_29, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_29, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_29, &_play_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_29, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_29, 0);

    //Write style for screen_img_list_img_29, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_29, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_30
    ui->screen_img_list_img_30 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_30, 228, 346);
    lv_obj_set_size(ui->screen_img_list_img_30, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_30, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_30, &_need_liquid_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_30, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_30, 0);

    //Write style for screen_img_list_img_30, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_30, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_31
    ui->screen_img_list_img_31 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_31, 290, 353);
    lv_obj_set_size(ui->screen_img_list_img_31, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_31, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_31, &_mode_worm4_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_31, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_31, 0);

    //Write style for screen_img_list_img_31, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_31, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_33
    ui->screen_img_list_img_33 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_33, 363, 370);
    lv_obj_set_size(ui->screen_img_list_img_33, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_33, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_33, &_mode_worm3_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_33, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_33, 0);

    //Write style for screen_img_list_img_33, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_33, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_33, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_37
    ui->screen_img_list_img_37 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_37, 217, 499);
    lv_obj_set_size(ui->screen_img_list_img_37, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_37, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_37, &_wifi_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_37, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_37, 0);

    //Write style for screen_img_list_img_37, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_37, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_37, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_38
    ui->screen_img_list_img_38 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_38, 275, 488);
    lv_obj_set_size(ui->screen_img_list_img_38, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_38, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_38, &_water_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_38, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_38, 0);

    //Write style for screen_img_list_img_38, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_38, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_38, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_39
    ui->screen_img_list_img_39 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_39, 357, 474);
    lv_obj_set_size(ui->screen_img_list_img_39, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_39, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_39, &_warnning_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_39, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_39, 0);

    //Write style for screen_img_list_img_39, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_39, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_39, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_40
    ui->screen_img_list_img_40 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_40, -93, 538);
    lv_obj_set_size(ui->screen_img_list_img_40, 50, 50);
    lv_obj_add_flag(ui->screen_img_list_img_40, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_40, &_timer_RGB565A8_50x50);
    lv_image_set_pivot(ui->screen_img_list_img_40, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_40, 0);

    //Write style for screen_img_list_img_40, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_40, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_40, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_41
    ui->screen_img_list_img_41 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_41, 528, 485);
    lv_obj_set_size(ui->screen_img_list_img_41, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_41, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_41, &_stop_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_41, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_41, 0);

    //Write style for screen_img_list_img_41, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_41, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_41, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_42
    ui->screen_img_list_img_42 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_42, -119, 147);
    lv_obj_set_size(ui->screen_img_list_img_42, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_42, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_42, &_timer_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_42, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_42, 0);

    //Write style for screen_img_list_img_42, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_42, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_42, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_43
    ui->screen_img_list_img_43 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_43, -43, 535);
    lv_obj_set_size(ui->screen_img_list_img_43, 50, 50);
    lv_obj_add_flag(ui->screen_img_list_img_43, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_43, &_water_RGB565A8_50x50);
    lv_image_set_pivot(ui->screen_img_list_img_43, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_43, 0);

    //Write style for screen_img_list_img_43, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_43, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_43, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_44
    ui->screen_img_list_img_44 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_44, -206, 480);
    lv_obj_set_size(ui->screen_img_list_img_44, 50, 50);
    lv_obj_add_flag(ui->screen_img_list_img_44, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_44, &_play_RGB565A8_50x50);
    lv_image_set_pivot(ui->screen_img_list_img_44, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_44, 0);

    //Write style for screen_img_list_img_44, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_44, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_44, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_45
    ui->screen_img_list_img_45 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_45, -214, 530);
    lv_obj_set_size(ui->screen_img_list_img_45, 50, 50);
    lv_obj_add_flag(ui->screen_img_list_img_45, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_45, &_return_RGB565A8_50x50);
    lv_image_set_pivot(ui->screen_img_list_img_45, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_45, 0);

    //Write style for screen_img_list_img_45, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_45, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_45, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_46
    ui->screen_img_list_img_46 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_46, 55, 488);
    lv_obj_set_size(ui->screen_img_list_img_46, 50, 50);
    lv_obj_add_flag(ui->screen_img_list_img_46, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_46, &_stop_RGB565A8_50x50);
    lv_image_set_pivot(ui->screen_img_list_img_46, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_46, 0);

    //Write style for screen_img_list_img_46, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_46, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_46, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_47
    ui->screen_img_list_img_47 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_47, -172, 269);
    lv_obj_set_size(ui->screen_img_list_img_47, 100, 100);
    lv_obj_add_flag(ui->screen_img_list_img_47, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_47, &_btn_worm_RGB565A8_100x100);
    lv_image_set_pivot(ui->screen_img_list_img_47, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_47, 0);

    //Write style for screen_img_list_img_47, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_47, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_47, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_48
    ui->screen_img_list_img_48 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_48, 107, 510);
    lv_obj_set_size(ui->screen_img_list_img_48, 70, 70);
    lv_obj_add_flag(ui->screen_img_list_img_48, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_48, &_stop_RGB565A8_70x70);
    lv_image_set_pivot(ui->screen_img_list_img_48, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_48, 0);

    //Write style for screen_img_list_img_48, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_48, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_48, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_49
    ui->screen_img_list_img_49 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_49, 55, 557);
    lv_obj_set_size(ui->screen_img_list_img_49, 70, 70);
    lv_obj_add_flag(ui->screen_img_list_img_49, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_49, &_play_RGB565A8_70x70);
    lv_image_set_pivot(ui->screen_img_list_img_49, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_49, 0);

    //Write style for screen_img_list_img_49, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_49, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_49, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_img_list_img_50
    ui->screen_img_list_img_50 = lv_image_create(ui->screen_img_list);
    lv_obj_set_pos(ui->screen_img_list_img_50, 145, 580);
    lv_obj_set_size(ui->screen_img_list_img_50, 70, 70);
    lv_obj_add_flag(ui->screen_img_list_img_50, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_img_list_img_50, &_return_RGB565A8_70x70);
    lv_image_set_pivot(ui->screen_img_list_img_50, 50,50);
    lv_image_set_rotation(ui->screen_img_list_img_50, 0);

    //Write style for screen_img_list_img_50, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_img_list_img_50, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_img_list_img_50, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_img_list.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_img_list);

}
