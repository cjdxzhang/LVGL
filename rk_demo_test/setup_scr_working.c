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
#include "custom_imgbtn.h"
#include "system_manager.h"
#include "ui_label.h"

/*
 * working                              工作页根对象
 * working_img_background               工作页背景图
 * working_cont_playing                  左上运行状态框
 * working_label_temperature             当前温度数值
 * working_label_time_title              “时间”标题
 * working_label_time_left               剩余倒计时时间
 * working_label_massage_title           “按摩”标题
 * working_img_massage                   按摩状态图标
 * working_img_massage_intensity0        按摩强度第一档图标
 * working_img_massage_intensity1        按摩强度第二档图标
 * working_img_massage_intensity2        按摩强度第三档图标
 * working_img_constant_temperature      恒温状态图标
 * working_img_sterilization             除菌状态图标
 * working_cont_controls                 左下待机、加温和减温控制区
 * working_cont_function_buttons         右侧功能按钮区
 * working_dialog                       回仓确认弹窗遮罩
 * working_img_dialog_background         回仓确认弹窗背景图
 * working_cont_return_dialog            回仓确认弹窗内容区
 * working_label_return_title            回仓确认标题
 * working_label_return_warning          回仓前安全提示
 * working_btn_yes                       回仓确认“是”按钮
 * working_btn_yes_label                 回仓确认“是”按钮文字
 * working_btn_no                        回仓确认“否”按钮
 * working_btn_no_label                  回仓确认“否”按钮文字
 * btn_constant_temp                     恒温按钮
 * btn_massage                           按摩按钮
 * btn_sterilization                     除菌按钮
 * btn_timer                             定时按钮
 * btn_auto_return                       自动回仓按钮
 * btn_setting                           设置按钮
 * btn_position                          定位按钮
 * btn_pause                             长按待机息屏按钮
 * btn_plus                              加温按钮
 * btn_minus                             减温按钮
 */

static lv_obj_t *working_create_power_button(lv_obj_t *parent,
        lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *button;
    lv_obj_t *symbol;

    if (parent == NULL)
    {
        return NULL;
    }

    button = lv_obj_create(parent);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, width, height);
    lv_obj_remove_flag(button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(button, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(button, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(button, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(button, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_PRESSED);

    symbol = lv_label_create(button);
    lv_label_set_text(symbol, LV_SYMBOL_POWER);
    lv_obj_set_style_text_font(symbol, &lv_font_montserratMedium_80, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(symbol, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(symbol);

    return button;
}

void WorkPageInit(lv_ui *ui)
{
    size_t index;
    int x_offset;
    int y_offset;
    const int button_width = 170;
    const int button_height = 178;
    const int horizontal_spacing = 24;
    const int vertical_spacing = 22;
    lv_obj_t *btn_constant_temp;
    lv_obj_t *btn_massage;
    lv_obj_t *btn_sterilization;
    lv_obj_t *btn_timer;
    lv_obj_t *btn_auto_return;
    lv_obj_t *btn_setting;
    lv_obj_t *btn_position;
    lv_obj_t *btn_pause;
    lv_obj_t *btn_plus;
    lv_obj_t *btn_minus;
    const ui_label_fixed_config_t time_left_config =
    {
        .x = 280,
        .y = 35,
        .width = 90,
        .height = 32,
        .style = {
            .font = &lv_font_SourceHanSansSC_Regular_25,
            .color = lv_color_hex(0xffffff),
            .long_mode = LV_LABEL_LONG_WRAP,
            .text_align = LV_TEXT_ALIGN_LEFT,
        },
    };
    const ui_label_fixed_config_t temperature_config =
    {
        .x = 8,
        .y = 56,
        .width = 203,
        .height = 80,
        .style = {
            .font = &lv_font_SourceHanSansSC_Regular_80,
            .color = lv_color_hex(0xffffff),
            .long_mode = LV_LABEL_LONG_WRAP,
            .text_align = LV_TEXT_ALIGN_RIGHT,
        },
    };
    const ui_label_fixed_config_t massage_title_config =
    {
        .x = 217,
        .y = 84,
        .width = 63,
        .height = 32,
        .style = {
            .font = &lv_font_SourceHanSansSC_Regular_25,
            .color = lv_color_hex(0xffffff),
            .long_mode = LV_LABEL_LONG_WRAP,
            .text_align = LV_TEXT_ALIGN_LEFT,
        },
    };
    const ui_label_fixed_config_t time_title_config =
    {
        .x = 217,
        .y = 35,
        .width = 63,
        .height = 32,
        .style = {
            .font = &lv_font_SourceHanSansSC_Regular_25,
            .color = lv_color_hex(0xffffff),
            .long_mode = LV_LABEL_LONG_WRAP,
            .text_align = LV_TEXT_ALIGN_LEFT,
        },
    };
    const ui_label_fixed_config_t return_title_config =
    {
        .x = 17,
        .y = 55,
        .width = 537,
        .height = 40,
        .style = {
            .font = &lv_font_SourceHanSansSC_Regular_40,
            .color = lv_color_hex(0xffffff),
            .long_mode = LV_LABEL_LONG_WRAP,
            .text_align = LV_TEXT_ALIGN_CENTER,
        },
    };
    const ui_label_fixed_config_t return_warning_config =
    {
        .x = 13,
        .y = 116,
        .width = 537,
        .height = 32,
        .style = {
            .font = &lv_font_SourceHanSansSC_Regular_25,
            .color = lv_color_hex(0xffffff),
            .long_mode = LV_LABEL_LONG_WRAP,
            .text_align = LV_TEXT_ALIGN_CENTER,
        },
    };
    const ui_label_button_config_t return_button_label_config =
    {
        .width = LV_PCT(100),
        .align = LV_ALIGN_CENTER,
        .x_offset = 0,
        .y_offset = 0,
        .style = {
            .font = &lv_font_SourceHanSansSC_Regular_20,
            .color = lv_color_hex(0xffffff),
            .long_mode = LV_LABEL_LONG_WRAP,
            .text_align = LV_TEXT_ALIGN_CENTER,
        },
    };

    // 工作页根对象
    ui->working = lv_obj_create(NULL);
    lv_obj_set_size(ui->working, 800, 480);
    lv_obj_set_scrollbar_mode(ui->working, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(ui->working, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 工作页背景图
    ui->working_img_background = lv_image_create(ui->working);
    lv_obj_set_pos(ui->working_img_background, 0, 0);
    lv_obj_set_size(ui->working_img_background, 800, 480);
    lv_obj_add_flag(ui->working_img_background, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_background, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->working_img_background, 50, 50);
    lv_image_set_rotation(ui->working_img_background, 0);
    lv_obj_set_style_image_recolor_opa(ui->working_img_background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_background, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 左上运行状态框
    ui->working_cont_playing = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_cont_playing, 15, 71);
    lv_obj_set_size(ui->working_cont_playing, 380, 192);
    lv_obj_set_scrollbar_mode(ui->working_cont_playing, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->working_cont_playing, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_border_width(ui->working_cont_playing, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_playing, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_playing, 80, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_cont_playing, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_cont_playing, LV_GRAD_DIR_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_playing, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_playing, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_playing, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_playing, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_playing, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 剩余倒计时时间
    ui->working_label_time_left = ui_label_create_common(
                                      ui->working_cont_playing, "10min", &time_left_config);
    if (ui->working_label_time_left != NULL)
    {
        lv_obj_set_style_text_opa(ui->working_label_time_left, 235,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // 当前温度数值
    ui->working_label_temperature = ui_label_create_common(
                                        ui->working_cont_playing, "50℃", &temperature_config);

    // 按摩状态图标
    ui->working_img_massage = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_massage, 280, 75);
    lv_obj_add_flag(ui->working_img_massage, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_massage, &_mode_foot3_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_massage, 17, 17);
    lv_image_set_scale(ui->working_img_massage, 341);
    lv_image_set_rotation(ui->working_img_massage, 0);
    lv_obj_set_style_image_recolor_opa(ui->working_img_massage, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_massage, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 按摩强度第一档图标
    ui->working_img_massage_intensity0 = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_massage_intensity0, 300, 75);
    lv_obj_add_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_massage_intensity0, &_minus_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_massage_intensity0, 17, 17);
    lv_image_set_scale(ui->working_img_massage_intensity0, 341);
    lv_image_set_rotation(ui->working_img_massage_intensity0, 900);
    lv_obj_set_style_image_recolor_opa(ui->working_img_massage_intensity0, 0,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_massage_intensity0, 255,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    // 按摩强度第二档图标
    ui->working_img_massage_intensity1 = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_massage_intensity1, 315, 75);
    lv_obj_add_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_massage_intensity1, &_minus_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_massage_intensity1, 17, 17);
    lv_image_set_scale(ui->working_img_massage_intensity1, 341);
    lv_image_set_rotation(ui->working_img_massage_intensity1, 900);
    lv_obj_set_style_image_recolor_opa(ui->working_img_massage_intensity1, 0,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_massage_intensity1, 255,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    // 按摩强度第三档图标
    ui->working_img_massage_intensity2 = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_massage_intensity2, 330, 75);
    lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_massage_intensity2, &_minus_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_massage_intensity2, 17, 17);
    lv_image_set_scale(ui->working_img_massage_intensity2, 341);
    lv_image_set_rotation(ui->working_img_massage_intensity2, 900);
    lv_obj_set_style_image_recolor_opa(ui->working_img_massage_intensity2, 0,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_massage_intensity2, 255,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    // 恒温状态图标
    ui->working_img_constant_temperature = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_constant_temperature, 217, 130);
    lv_obj_add_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_constant_temperature, &_worm_RGB565A8_32x27);
    lv_image_set_pivot(ui->working_img_constant_temperature, 16, 13);
    lv_image_set_scale(ui->working_img_constant_temperature, 341);
    lv_image_set_rotation(ui->working_img_constant_temperature, 0);
    lv_obj_set_style_image_recolor_opa(ui->working_img_constant_temperature, 0,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_constant_temperature, 255,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    // 除菌状态图标
    ui->working_img_sterilization = lv_image_create(ui->working_cont_playing);
    lv_obj_set_pos(ui->working_img_sterilization, 260, 130);
    lv_obj_add_flag(ui->working_img_sterilization, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_img_sterilization, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_sterilization, &_bacteria_RGB565A8_34x34);
    lv_image_set_pivot(ui->working_img_sterilization, 17, 17);
    lv_image_set_scale(ui->working_img_sterilization, 341);
    lv_image_set_rotation(ui->working_img_sterilization, 0);
    lv_obj_set_style_image_recolor_opa(ui->working_img_sterilization, 0,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_sterilization, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // “按摩”标题
    ui->working_label_massage_title = ui_label_create_common(
                                          ui->working_cont_playing, "按摩:", &massage_title_config);
    if (ui->working_label_massage_title != NULL)
    {
        lv_obj_set_style_text_opa(ui->working_label_massage_title, 235,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // “时间”标题
    ui->working_label_time_title = ui_label_create_common(
                                       ui->working_cont_playing, "时间:", &time_title_config);

    // 左下基础控制区
    ui->working_cont_controls = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_cont_controls, 15, 287);
    lv_obj_set_size(ui->working_cont_controls, 399, 182);
    lv_obj_set_scrollbar_mode(ui->working_cont_controls, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_controls, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 右侧功能按钮区
    ui->working_cont_function_buttons = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_cont_function_buttons, 420, 71);
    lv_obj_set_size(ui->working_cont_function_buttons, 377, 391);
    lv_obj_set_scrollbar_mode(ui->working_cont_function_buttons, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_border_width(ui->working_cont_function_buttons, 2,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->working_cont_function_buttons, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->working_cont_function_buttons, lv_color_hex(0x2195f6),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->working_cont_function_buttons, LV_BORDER_SIDE_FULL,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_function_buttons, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_function_buttons, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_function_buttons, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_function_buttons, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_function_buttons, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_function_buttons, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_function_buttons, 0,
                                  LV_PART_MAIN | LV_STATE_DEFAULT);

    // 回仓确认弹窗遮罩
    ui->working_dialog = lv_obj_create(ui->working);
    lv_obj_set_pos(ui->working_dialog, 0, 0);
    lv_obj_set_size(ui->working_dialog, 800, 480);
    lv_obj_set_scrollbar_mode(ui->working_dialog, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ui->working_dialog, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_border_width(ui->working_dialog, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->working_dialog, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->working_dialog, lv_color_hex(0x2195f6),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->working_dialog, LV_BORDER_SIDE_FULL,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_dialog, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_dialog, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_dialog, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 回仓确认弹窗背景图
    ui->working_img_dialog_background = lv_image_create(ui->working_dialog);
    lv_obj_set_pos(ui->working_img_dialog_background, 0, 0);
    lv_obj_set_size(ui->working_img_dialog_background, 800, 480);
    lv_obj_add_flag(ui->working_img_dialog_background, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->working_img_dialog_background, &_bg_RGB565A8_800x480);
    lv_image_set_pivot(ui->working_img_dialog_background, 50, 50);
    lv_image_set_rotation(ui->working_img_dialog_background, 0);
    lv_obj_set_style_image_recolor_opa(ui->working_img_dialog_background, 0,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->working_img_dialog_background, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 回仓确认弹窗内容区
    ui->working_cont_return_dialog = lv_obj_create(ui->working_dialog);
    lv_obj_set_pos(ui->working_cont_return_dialog, 112, 95);
    lv_obj_set_size(ui->working_cont_return_dialog, 572, 331);
    lv_obj_set_scrollbar_mode(ui->working_cont_return_dialog, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(ui->working_cont_return_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_cont_return_dialog, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_cont_return_dialog, 52, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_cont_return_dialog, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->working_cont_return_dialog, LV_GRAD_DIR_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->working_cont_return_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->working_cont_return_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->working_cont_return_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->working_cont_return_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_cont_return_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 回仓确认标题
    ui->working_label_return_title = ui_label_create_common(
                                         ui->working_cont_return_dialog, "是否返回基站？", &return_title_config);

    // 回仓前安全提示
    ui->working_label_return_warning = ui_label_create_common(
                                           ui->working_cont_return_dialog, "启动前请确认已取出双脚",
                                           &return_warning_config);

    // 回仓确认“是”按钮
    ui->working_btn_yes = lv_button_create(ui->working_cont_return_dialog);
    lv_obj_set_pos(ui->working_btn_yes, 26, 197);
    lv_obj_set_size(ui->working_btn_yes, 250, 108);
    lv_obj_set_style_pad_all(ui->working_btn_yes, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_btn_yes, 51, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_btn_yes, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->working_btn_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_btn_yes, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_btn_yes, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    //回仓确认“是”按钮文字
    ui->working_btn_yes_label = ui_label_create_button(
                                    ui->working_btn_yes, "是", &return_button_label_config);

    // 回仓确认“否”按钮
    ui->working_btn_no = lv_button_create(ui->working_cont_return_dialog);
    lv_obj_set_pos(ui->working_btn_no, 299, 197);
    lv_obj_set_size(ui->working_btn_no, 250, 108);
    lv_obj_set_style_pad_all(ui->working_btn_no, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->working_btn_no, 51, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->working_btn_no, lv_color_hex(0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->working_btn_no, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->working_btn_no, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->working_btn_no, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 回仓确认“否”按钮文字
    ui->working_btn_no_label = ui_label_create_button(
                                   ui->working_btn_no, "否", &return_button_label_config);

    index = 0;
    x_offset = (int)(index % 2) * (button_width + horizontal_spacing);
    y_offset = (int)(index / 2) * (button_height + vertical_spacing);
    btn_constant_temp = imgbtn_create(ui->working_cont_function_buttons,
                                      &_btn_worm_RGB565A8_100x100, "恒温",
                                      x_offset, y_offset, button_width, button_height);
    lv_obj_add_event_cb(btn_constant_temp, working_constant_temperature_toggle, LV_EVENT_ALL, ui);

    index++;
    x_offset = (int)(index % 2) * (button_width + horizontal_spacing);
    y_offset = (int)(index / 2) * (button_height + vertical_spacing);
    btn_massage = imgbtn_create(ui->working_cont_function_buttons,
                                &_massage_RGB565A8_100x100, "按摩",
                                x_offset, y_offset, button_width, button_height);
    lv_obj_add_event_cb(btn_massage, working_massage_intensity_clicked, LV_EVENT_ALL, ui);

    index++;
    x_offset = (int)(index % 2) * (button_width + horizontal_spacing);
    y_offset = (int)(index / 2) * (button_height + vertical_spacing);
    btn_sterilization = imgbtn_create(ui->working_cont_function_buttons,
                                      &_bacteria_RGB565A8_100x100, "除菌",
                                      x_offset, y_offset, button_width, button_height);
    lv_obj_add_event_cb(btn_sterilization, working_sterilization_toggle, LV_EVENT_ALL, ui);

    index++;
    x_offset = (int)(index % 2) * (button_width + horizontal_spacing);
    y_offset = (int)(index / 2) * (button_height + vertical_spacing);
    btn_timer = imgbtn_create(ui->working_cont_function_buttons,
                              &_timer_RGB565A8_100x100, "定时",
                              x_offset, y_offset, button_width, button_height);
    lv_obj_add_event_cb(btn_timer, working_timer_clicked, LV_EVENT_ALL, ui);

    index++;
    x_offset = (int)(index % 2) * (button_width + horizontal_spacing);
    y_offset = (int)(index / 2) * (button_height + vertical_spacing);
    btn_auto_return = imgbtn_create(ui->working_cont_function_buttons,
                                    &_auto_back_RGB565A8_100x100, "自动回仓",
                                    x_offset, y_offset, button_width, button_height);
    lv_obj_add_event_cb(btn_auto_return, working_show_return_dialog, LV_EVENT_ALL, ui);

    index++;
    x_offset = (int)(index % 2) * (button_width + horizontal_spacing);
    y_offset = (int)(index / 2) * (button_height + vertical_spacing);
    btn_setting = imgbtn_create(ui->working_cont_function_buttons,
                                &_setting_RGB565A8_100x100, "设置",
                                x_offset, y_offset, button_width, button_height);
    lv_obj_add_event_cb(btn_setting, working_to_setting, LV_EVENT_ALL, ui);

    index++;
    x_offset = (int)(index % 2) * (button_width + horizontal_spacing);
    y_offset = (int)(index / 2) * (button_height + vertical_spacing);
    btn_position = imgbtn_create(ui->working_cont_function_buttons,
                                 &_location_RGB565A8_100x100, "定位",
                                 x_offset, y_offset, button_width, button_height);
    lv_obj_add_event_cb(btn_position, working_to_location, LV_EVENT_ALL, ui);

    btn_pause = working_create_power_button(ui->working_cont_controls, 0, 0, 146, 163);
    lv_obj_add_event_cb(btn_pause, working_pause_clicked, LV_EVENT_ALL, ui);

    btn_plus = imgbtn_create(ui->working_cont_controls,
                             &_plus_RGB565A8_50x50, "", 171, 0, 92, 163);
    lv_obj_add_event_cb(btn_plus, working_plus_clicked, LV_EVENT_ALL, ui);
    btn_minus = imgbtn_create(ui->working_cont_controls,
                              &_minus_RGB565A8_50x50, "", 287, 0, 92, 163);
    lv_obj_add_event_cb(btn_minus, working_minus_clicked, LV_EVENT_ALL, ui);

    lv_obj_set_style_shadow_width(ui->working_cont_playing, 8, 0);
    lv_obj_set_style_shadow_ofs_x(ui->working_cont_playing, 3, 0);
    lv_obj_set_style_shadow_ofs_y(ui->working_cont_playing, 3, 0);
    lv_obj_set_style_shadow_color(ui->working_cont_playing, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(ui->working_cont_playing, LV_OPA_40, 0);

    //Update current screen layout.
    lv_obj_update_layout(ui->working);

    //Init events for screen.
    events_init_working(ui);
}
