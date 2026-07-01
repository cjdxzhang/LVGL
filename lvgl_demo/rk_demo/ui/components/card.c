#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "card.h"

LV_FONT_DECLARE(font_cn_28);
LV_FONT_DECLARE(font_cn_28_3);
LV_FONT_DECLARE(font_cn_24);
LV_FONT_DECLARE(temp);
LV_FONT_DECLARE(degree);
LV_FONT_DECLARE(water);
LV_FONT_DECLARE(card_clean);
LV_FONT_DECLARE(clean_cn);
LV_FONT_DECLARE(clean_big);
LV_FONT_DECLARE(footbath_liquid);
LV_FONT_DECLARE(mode3);
LV_FONT_DECLARE(heat_on);
LV_FONT_DECLARE(massage);
LV_FONT_DECLARE(sterilize);

static lv_obj_t * g_info_card;
static lv_obj_t * g_temp_big;
static lv_obj_t * g_temp_unit;
static lv_obj_t * g_time_title;
static lv_obj_t * g_water_title;
static lv_obj_t * g_time_value;
static lv_obj_t * g_water_value;
static lv_obj_t * g_massage_title;
static lv_obj_t * g_massage_level_value;
static lv_obj_t * g_low_water_icon;
static lv_obj_t * g_low_water_text;
static lv_obj_t * g_herb1_icon;
static lv_obj_t * g_herb2_icon;
static lv_obj_t * g_location_icon;
static lv_obj_t * g_status_heat_icon;
static lv_obj_t * g_status_massage_icon;
static lv_obj_t * g_status_sterilize_icon;

/* 自动清洁状态卡新增对象 */
static lv_obj_t * g_clean_icon;
static lv_obj_t * g_clean_text;
static lv_timer_t * g_temp_blink_timer;
static uint8_t g_temp_blink_toggle_count;

static void temp_blink_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (g_temp_big == NULL) {
        if (g_temp_blink_timer) {
            lv_timer_del(g_temp_blink_timer);
            g_temp_blink_timer = NULL;
        }
        return;
    }

    if (lv_obj_has_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);
    }

    g_temp_blink_toggle_count++;
    if (g_temp_blink_toggle_count >= 4) {
        lv_obj_clear_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);
        lv_timer_del(g_temp_blink_timer);
        g_temp_blink_timer = NULL;
    }
}

static void card_style_shadow(lv_obj_t * obj)//设置卡片阴影
{
    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x050B16), 0);
}

static void card_style_frosted_base(lv_obj_t * obj, lv_coord_t radius)//设置卡片毛玻璃底色
{
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0xe1e6eb), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_50, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x102033), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x1A3550), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    card_style_shadow(obj);
}

static void card_show_default_items(bool show)
{
    if (g_temp_big) {
        if (show) {
            lv_obj_clear_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_temp_unit) {
        if (show) {
            lv_obj_clear_flag(g_temp_unit, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_temp_unit, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_time_title) {
        if (show) {
            lv_obj_clear_flag(g_time_title, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_time_title, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_water_title) {
        if (show) {
            lv_obj_clear_flag(g_water_title, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_water_title, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_time_value) {
        if (show) {
            lv_obj_clear_flag(g_time_value, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_time_value, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_water_value) {
        if (show) {
            lv_obj_clear_flag(g_water_value, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_water_value, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_massage_title) {
        lv_obj_add_flag(g_massage_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_massage_level_value) {
        lv_obj_add_flag(g_massage_level_value, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_low_water_icon) {
        if (show) {
            lv_obj_clear_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_low_water_text) {
        if (show) {
            lv_obj_clear_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_location_icon) {
        if (show) {
            lv_obj_clear_flag(g_location_icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_location_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_herb1_icon && !show) {
        lv_obj_add_flag(g_herb1_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_herb2_icon && !show) {
        lv_obj_add_flag(g_herb2_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (!show) {
        if (g_status_heat_icon) {
            lv_obj_add_flag(g_status_heat_icon, LV_OBJ_FLAG_HIDDEN);
        }
        if (g_status_massage_icon) {
            lv_obj_add_flag(g_status_massage_icon, LV_OBJ_FLAG_HIDDEN);
        }
        if (g_status_sterilize_icon) {
            lv_obj_add_flag(g_status_sterilize_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void card_show_clean_items(bool show)
{
    if (g_clean_icon) {
        if (show) {
            lv_obj_clear_flag(g_clean_icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_clean_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_clean_text) {
        if (show) {
            lv_obj_clear_flag(g_clean_text, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_clean_text, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static lv_obj_t * card_create_small_icon(lv_obj_t * parent,
                                         const lv_font_t * font,
                                         const char * text)
{
    lv_obj_t * icon = lv_label_create(parent);
    lv_label_set_text(icon, text);
    lv_obj_set_style_text_font(icon, font, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
    return icon;
}

static void card_place_icon_ex(lv_obj_t * icon, lv_coord_t x, lv_coord_t y,
                               int scale, lv_coord_t pivot_x,
                               lv_coord_t pivot_y)
{
    if (icon == NULL) {
        return;
    }
    // 图标设置的位置，x y轴坐标
    lv_obj_set_pos(icon, x, y);
    // scale表示图标的缩放，256表示不缩放，128表示缩小一半，512表示放大一倍
    lv_obj_set_style_transform_scale(icon, scale, 0);
    // pivot_x和pivot_y表示图标缩放的中心点坐标，通常设置为图标的中心点坐标
    lv_obj_set_style_transform_pivot_x(icon, pivot_x, 0);
    lv_obj_set_style_transform_pivot_y(icon, pivot_y, 0);
}

static void card_place_icon(lv_obj_t * icon, lv_coord_t x, lv_coord_t y,
                            int scale)
{
    card_place_icon_ex(icon, x, y, scale, 50, 50);
}

static void card_restore_base_status_icon_layout(void)
{
    if (g_time_title) {
        lv_obj_set_pos(g_time_title, 190, 18);
        lv_obj_set_style_transform_scale(g_time_title, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_time_title, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_time_title, 0, 0);
    }

    if (g_time_value) {
        lv_obj_set_pos(g_time_value, 266, 21);
        lv_obj_set_style_transform_scale(g_time_value, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_time_value, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_time_value, 0, 0);
    }

    if (g_water_title) {
        lv_obj_set_pos(g_water_title, 190, 64);
        lv_obj_set_style_transform_scale(g_water_title, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_water_title, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_water_title, 0, 0);
    }

    if (g_water_value) {
        lv_obj_set_pos(g_water_value, 260, 60);
        lv_obj_set_style_transform_scale(g_water_value, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_water_value, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_water_value, 0, 0);
    }

    card_place_icon(g_herb1_icon, 162, 80, 100);
    card_place_icon(g_herb2_icon, 204, 80, 100);
    card_place_icon(g_location_icon, 246, 80, 100);
}

static void card_apply_bucket_status_icon_layout(void)
{
    if (g_time_title) {
        lv_obj_set_pos(g_time_title, 190, 18);
        lv_obj_set_style_transform_scale(g_time_title, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_time_title, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_time_title, 0, 0);
    }

    if (g_time_value) {
        lv_obj_set_pos(g_time_value, 266, 21);
        lv_obj_set_style_transform_scale(g_time_value, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_time_value, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_time_value, 0, 0);
    }

    if (g_massage_title) {
        lv_obj_set_pos(g_massage_title, 190, 64);
        lv_obj_set_style_transform_scale(g_massage_title, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_massage_title, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_massage_title, 0, 0);
    }

    card_place_icon_ex(g_status_massage_icon, 270, 60, 100, 0, 0);

    if (g_massage_level_value) {
        lv_obj_set_pos(g_massage_level_value, 310, 64);
        lv_obj_set_style_transform_scale(g_massage_level_value, 300, 0);
        lv_obj_set_style_transform_pivot_x(g_massage_level_value, 0, 0);
        lv_obj_set_style_transform_pivot_y(g_massage_level_value, 0, 0);
    }

    card_place_icon(g_status_heat_icon, 162, 80, 100);
    card_place_icon(g_status_sterilize_icon, 204, 80, 100);
}

void card_create_info(lv_obj_t * parent)//创建信息卡
{
    g_info_card = lv_obj_create(parent);
    lv_obj_t * info_card = g_info_card;
    lv_obj_set_size(info_card, 400, 170);
    lv_obj_set_pos(info_card, 15, 50);

    card_style_frosted_base(info_card, 50);
    lv_obj_set_style_pad_all(info_card, 0, 0);
    lv_obj_clear_flag(info_card, LV_OBJ_FLAG_SCROLLABLE);

    /* 温度大数字 */
    g_temp_big = lv_label_create(info_card);
    lv_label_set_text(g_temp_big, "--");
    lv_obj_set_style_text_font(g_temp_big, &temp, 0);
    lv_obj_set_style_text_color(g_temp_big, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_transform_scale(g_temp_big, 238, 0);
    lv_obj_set_style_transform_pivot_x(g_temp_big, 0, 0);
    lv_obj_set_style_transform_pivot_y(g_temp_big, 0, 0);
    lv_obj_set_pos(g_temp_big, 18, 12);

    /* 温度单位图标 */
    g_temp_unit = lv_label_create(info_card);
    lv_label_set_text(g_temp_unit, "\xEE\x98\xAE");
    lv_obj_set_style_text_font(g_temp_unit, &degree, 0);
    lv_obj_set_style_text_color(g_temp_unit, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_transform_scale(g_temp_unit, 238, 0);
    lv_obj_set_style_transform_pivot_x(g_temp_unit, 0, 0);
    lv_obj_set_style_transform_pivot_y(g_temp_unit, 0, 0);
    lv_obj_set_pos(g_temp_unit, 140, 12);

    /* 固定标题 */
    g_time_title = lv_label_create(info_card);
    lv_label_set_text(g_time_title, "时间:");
    lv_obj_set_style_text_font(g_time_title, &font_cn_28, 0);
    lv_obj_set_style_text_color(g_time_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_time_title, 230, 15);

    g_water_title = lv_label_create(info_card);
    lv_label_set_text(g_water_title, "水量:");
    lv_obj_set_style_text_font(g_water_title, &font_cn_28, 0);
    lv_obj_set_style_text_color(g_water_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_water_title, 230, 55);

    /* 动态值 */
    g_time_value = lv_label_create(info_card);
    lv_label_set_text(g_time_value, "--");
    lv_obj_set_style_text_font(g_time_value, &font_cn_24, 0);
    lv_obj_set_style_text_color(g_time_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_time_value, 300, 18);

    g_water_value = lv_label_create(info_card);
    lv_label_set_text(g_water_value, "");
    lv_obj_set_style_text_font(g_water_value, &water, 0);
    lv_obj_set_style_text_color(g_water_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(g_water_value, -8, 0);
    lv_obj_set_pos(g_water_value, 285, 52);

    g_massage_title = lv_label_create(info_card);
    lv_label_set_text(g_massage_title, "按摩:");
    lv_obj_set_style_text_font(g_massage_title, &font_cn_28_3, 0);
    lv_obj_set_style_text_color(g_massage_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_massage_title, 230, 55);
    lv_obj_add_flag(g_massage_title, LV_OBJ_FLAG_HIDDEN);

    // 按摩后面的数值
    g_massage_level_value = lv_label_create(info_card);
    lv_label_set_text(g_massage_level_value, "--");
    lv_obj_set_style_text_font(g_massage_level_value, &font_cn_24, 0);
    lv_obj_set_style_text_color(g_massage_level_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_massage_level_value, 340, 58);
    lv_obj_add_flag(g_massage_level_value, LV_OBJ_FLAG_HIDDEN);

    // 缺液图标
    g_low_water_icon = lv_label_create(info_card);
    lv_label_set_text(g_low_water_icon, "\xEE\x99\x96");   /* U+E656 */
    lv_obj_set_style_text_font(g_low_water_icon, &water, 0);
    lv_obj_set_style_text_color(g_low_water_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_low_water_icon, 51, 109);
    lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);

    // 缺液文字
    g_low_water_text = lv_label_create(info_card);
    lv_label_set_text(g_low_water_text, "缺液");
    lv_obj_set_style_text_font(g_low_water_text, &font_cn_24, 0);
    lv_obj_set_style_text_color(g_low_water_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_low_water_text, 95, 115);
    lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);

    g_herb1_icon = card_create_small_icon(info_card, &footbath_liquid,
                                          "\xEE\x98\x85");
    g_herb2_icon = card_create_small_icon(info_card, &footbath_liquid,
                                          "\xEE\x98\x86");
    g_location_icon = card_create_small_icon(info_card, &mode3,
                                             "\xEE\x98\xA9");
    lv_obj_clear_flag(g_location_icon, LV_OBJ_FLAG_HIDDEN);

    g_status_heat_icon = card_create_small_icon(info_card, &heat_on,
                                                "\xEE\x9B\xB7");
    g_status_massage_icon = card_create_small_icon(info_card, &massage,
                                                   "\xEE\x98\x8A");
    g_status_sterilize_icon = card_create_small_icon(info_card, &sterilize,
                                                     "\xEE\x9A\x8F");

    g_clean_icon = lv_label_create(info_card);
    lv_label_set_text(g_clean_icon, "\xEE\x99\xBE");   /* 和运行页同一个清洁图标 */
    lv_obj_set_style_text_font(g_clean_icon, &card_clean, 0);
    lv_obj_set_style_text_color(g_clean_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_clean_icon, 30, 41);
    lv_obj_add_flag(g_clean_icon, LV_OBJ_FLAG_HIDDEN);

    g_clean_text = lv_label_create(info_card);
    lv_label_set_text(g_clean_text, "自动清洁中...");
    lv_obj_set_style_text_font(g_clean_text, &clean_big, 0);
    lv_obj_set_style_text_color(g_clean_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(g_clean_text, LV_ALIGN_LEFT_MID, 120, 0);
    //lv_obj_set_pos(g_clean_text, 130, 69);
    lv_obj_add_flag(g_clean_text, LV_OBJ_FLAG_HIDDEN);

    card_restore_base_status_icon_layout();
}

void card_set_temperature(int temp_value)
{
    char buf[16];

    if (g_temp_big == NULL) {
        return;
    }

    snprintf(buf, sizeof(buf), "%d", temp_value);
    lv_label_set_text(g_temp_big, buf);
}

void card_set_time_min(uint16_t min)
{
    char buf[24];

    if (g_time_value == NULL) {
        return;
    }

    snprintf(buf, sizeof(buf), "%umin", min);
    lv_label_set_text(g_time_value, buf);
}

void card_set_water_text(const char * text)
{
    if (g_water_value == NULL || text == NULL) {
        return;
    }
    lv_label_set_text(g_water_value, text);
}

void card_set_status_text(const char * text)
{
    if (g_low_water_text == NULL || g_low_water_icon == NULL) {
        return;
    }

    if (text == NULL || text[0] == '\0') {
        lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_label_set_text(g_low_water_text, text);
    lv_obj_clear_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
}

void card_set_water_level(uint8_t level)
{
    if (g_water_value == NULL) {
        return;
    }

    lv_obj_set_style_text_font(g_water_value, &water, 0);
    lv_obj_set_style_text_letter_space(g_water_value, -8, 0);
    lv_obj_set_style_transform_scale(g_water_value, 256, 0);
    lv_obj_set_style_transform_pivot_x(g_water_value, 0, 0);
    lv_obj_set_style_transform_pivot_y(g_water_value, 0, 0);

    switch (level) {
        case 1:
            lv_label_set_text(g_water_value, "\xEE\xA0\xBC");
            break;
        case 2:
            lv_label_set_text(g_water_value, "\xEE\xA0\xBC\xEE\xA0\xBC");
            break;
        case 3:
        default:
            lv_label_set_text(g_water_value, "\xEE\xA0\xBC\xEE\xA0\xBC\xEE\xA0\xBC");
            break;
    }
}

void card_set_water_liters(uint8_t liters)
{
    char buf[16];

    if (g_water_value == NULL) {
        return;
    }

    snprintf(buf, sizeof(buf), "%uL", liters);
    lv_obj_set_style_text_font(g_water_value, &font_cn_24, 0);
    lv_obj_set_style_text_letter_space(g_water_value, 0, 0);
    lv_obj_set_style_transform_scale(g_water_value, 256, 0);
    lv_label_set_text(g_water_value, buf);
}

void card_set_low_water(bool low)
{
    if (g_low_water_icon == NULL || g_low_water_text == NULL) {
        return;
    }

    if (low) {
        lv_label_set_text(g_low_water_text, "缺液");
        lv_obj_clear_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void card_set_herb_icons(uint8_t herb1, uint8_t herb2)
{
    card_restore_base_status_icon_layout();

    if (g_low_water_text) {
        lv_obj_set_pos(g_low_water_text, 95, 115);
    }

    if (g_herb1_icon) {
        herb1 ? lv_obj_clear_flag(g_herb1_icon, LV_OBJ_FLAG_HIDDEN)
                : lv_obj_add_flag(g_herb1_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_herb2_icon) {
        herb2 ? lv_obj_clear_flag(g_herb2_icon, LV_OBJ_FLAG_HIDDEN)
                : lv_obj_add_flag(g_herb2_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_location_icon) {
        lv_obj_clear_flag(g_location_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_status_heat_icon) {
        lv_obj_add_flag(g_status_heat_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_status_massage_icon) {
        lv_obj_add_flag(g_status_massage_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_status_sterilize_icon) {
        lv_obj_add_flag(g_status_sterilize_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_massage_title) {
        lv_obj_add_flag(g_massage_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_massage_level_value) {
        lv_obj_add_flag(g_massage_level_value, LV_OBJ_FLAG_HIDDEN);
    }
}

void card_set_runtime_status(uint8_t massage_level, uint8_t heat,
                             uint8_t sterilize_on)
{
    char buf[16];

    card_apply_bucket_status_icon_layout();

    if (g_water_title) {
        lv_obj_add_flag(g_water_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_water_value) {
        lv_obj_add_flag(g_water_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_low_water_icon) {
        lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_low_water_text) {
        lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_herb1_icon) {
        lv_obj_add_flag(g_herb1_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_herb2_icon) {
        lv_obj_add_flag(g_herb2_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_location_icon) {
        lv_obj_add_flag(g_location_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_massage_title) {
        lv_obj_clear_flag(g_massage_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_massage_level_value) {
        snprintf(buf, sizeof(buf), "%u", massage_level);
        lv_label_set_text(g_massage_level_value, buf);
        lv_obj_clear_flag(g_massage_level_value, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_status_heat_icon) {
        heat ? lv_obj_clear_flag(g_status_heat_icon, LV_OBJ_FLAG_HIDDEN)
             : lv_obj_add_flag(g_status_heat_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_status_massage_icon) {
        massage_level ? lv_obj_clear_flag(g_status_massage_icon, LV_OBJ_FLAG_HIDDEN)
                        : lv_obj_add_flag(g_status_massage_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_status_sterilize_icon) {
        sterilize_on ? lv_obj_clear_flag(g_status_sterilize_icon, LV_OBJ_FLAG_HIDDEN)
                     : lv_obj_add_flag(g_status_sterilize_icon, LV_OBJ_FLAG_HIDDEN);
    }

}

void card_clear_bucket_status(void)
{
    card_show_clean_items(false);
    card_apply_bucket_status_icon_layout();

    if (g_temp_big) {
        lv_label_set_text(g_temp_big, "--");
        lv_obj_clear_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_temp_unit) {
        lv_obj_clear_flag(g_temp_unit, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_time_title) {
        lv_obj_clear_flag(g_time_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_time_value) {
        lv_obj_add_flag(g_time_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_massage_title) {
        lv_obj_clear_flag(g_massage_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_massage_level_value) {
        lv_obj_add_flag(g_massage_level_value, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_status_massage_icon) {
        lv_obj_add_flag(g_status_massage_icon, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_status_heat_icon) {
        lv_obj_add_flag(g_status_heat_icon, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_status_sterilize_icon) {
        lv_obj_add_flag(g_status_sterilize_icon, LV_OBJ_FLAG_HIDDEN);
    }
}

void card_exit_clean(void)
{
    card_show_clean_items(false);
    card_show_default_items(true);
}

void card_set_clean_running(void)
{
    card_show_default_items(false);
    card_show_clean_items(true);

    if (g_clean_text) {
        lv_label_set_text(g_clean_text, "自动清洁中...");
    }
}

void card_set_clean_pending(void)
{
    card_show_default_items(false);
    card_show_clean_items(true);

    if (g_clean_text) {
        lv_label_set_text(g_clean_text, "进入自清洁");
    }
}

void card_blink_temperature_confirm(void)
{
    if (g_temp_big == NULL) {
        return;
    }

    if (g_temp_blink_timer) {
        lv_timer_del(g_temp_blink_timer);
        g_temp_blink_timer = NULL;
    }

    lv_obj_clear_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);
    g_temp_blink_toggle_count = 0;
    g_temp_blink_timer = lv_timer_create(temp_blink_timer_cb, 220, NULL);
    if (g_temp_blink_timer) {
        lv_timer_set_repeat_count(g_temp_blink_timer, -1);
    }
}
