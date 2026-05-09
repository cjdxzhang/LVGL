#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "card.h"

LV_FONT_DECLARE(font_cn_28);
LV_FONT_DECLARE(font_cn_24);
LV_FONT_DECLARE(temp);
LV_FONT_DECLARE(degree);
LV_FONT_DECLARE(water);
LV_FONT_DECLARE(card_clean);
LV_FONT_DECLARE(clean_cn);
LV_FONT_DECLARE(clean_big);

static lv_obj_t * g_info_card;
static lv_obj_t * g_temp_big;
static lv_obj_t * g_temp_unit;
static lv_obj_t * g_time_title;
static lv_obj_t * g_water_title;
static lv_obj_t * g_time_value;
static lv_obj_t * g_water_value;
static lv_obj_t * g_low_water_icon;
static lv_obj_t * g_low_water_text;

/* 自动清洁状态卡新增对象 */
static lv_obj_t * g_clean_icon;
static lv_obj_t * g_clean_text;

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
    if(g_temp_big)       show ? lv_obj_clear_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_temp_big, LV_OBJ_FLAG_HIDDEN);

    if(g_temp_unit)      show ? lv_obj_clear_flag(g_temp_unit, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_temp_unit, LV_OBJ_FLAG_HIDDEN);

    if(g_time_title)     show ? lv_obj_clear_flag(g_time_title, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_time_title, LV_OBJ_FLAG_HIDDEN);

    if(g_water_title)    show ? lv_obj_clear_flag(g_water_title, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_water_title, LV_OBJ_FLAG_HIDDEN);

    if(g_time_value)     show ? lv_obj_clear_flag(g_time_value, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_time_value, LV_OBJ_FLAG_HIDDEN);

    if(g_water_value)    show ? lv_obj_clear_flag(g_water_value, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_water_value, LV_OBJ_FLAG_HIDDEN);

    if(g_low_water_icon) show ? lv_obj_clear_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);

    if(g_low_water_text) show ? lv_obj_clear_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN)
                              : lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
}

static void card_show_clean_items(bool show)
{
    if(g_clean_icon) show ? lv_obj_clear_flag(g_clean_icon, LV_OBJ_FLAG_HIDDEN)
                          : lv_obj_add_flag(g_clean_icon, LV_OBJ_FLAG_HIDDEN);

    if(g_clean_text) show ? lv_obj_clear_flag(g_clean_text, LV_OBJ_FLAG_HIDDEN)
                          : lv_obj_add_flag(g_clean_text, LV_OBJ_FLAG_HIDDEN);
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
    lv_obj_set_pos(g_temp_big, 20, 15);

    /* 温度单位图标 */
    g_temp_unit = lv_label_create(info_card);
    lv_label_set_text(g_temp_unit, "\xEE\x98\xAE");
    lv_obj_set_style_text_font(g_temp_unit, &degree, 0);
    lv_obj_set_style_text_color(g_temp_unit, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_temp_unit, 150, 15);

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
    lv_obj_set_pos(g_time_value, 289, 18);

    g_water_value = lv_label_create(info_card);
    lv_label_set_text(g_water_value, "");
    lv_obj_set_style_text_font(g_water_value, &water, 0);
    lv_obj_set_style_text_color(g_water_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_letter_space(g_water_value, -8, 0);
    lv_obj_set_pos(g_water_value, 285, 52);

    g_low_water_icon = lv_label_create(info_card);
    lv_label_set_text(g_low_water_icon, "\xEE\x99\x96");   /* U+E656 */
    lv_obj_set_style_text_font(g_low_water_icon, &water, 0);
    lv_obj_set_style_text_color(g_low_water_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_low_water_icon, 51, 109);
    lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);

    g_low_water_text = lv_label_create(info_card);
    lv_label_set_text(g_low_water_text, "缺液");
    lv_obj_set_style_text_font(g_low_water_text, &font_cn_24, 0);
    lv_obj_set_style_text_color(g_low_water_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_low_water_text, 95, 115);
    lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);

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

}

void card_set_temperature(int temp_value)
{
    char buf[16];

    if(g_temp_big == NULL) return;

    snprintf(buf, sizeof(buf), "%d", temp_value);
    lv_label_set_text(g_temp_big, buf);
}

void card_set_time_min(uint16_t min)
{
    char buf[24];

    if(g_time_value == NULL) return;

    snprintf(buf, sizeof(buf), "%umin", min);
    lv_label_set_text(g_time_value, buf);
}

void card_set_water_text(const char * text)
{
    if(g_water_value == NULL || text == NULL) return;
    lv_label_set_text(g_water_value, text);
}

void card_set_status_text(const char * text)
{
    if(g_low_water_text == NULL || g_low_water_icon == NULL) return;

    if(text == NULL || text[0] == '\0') {
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
    if(g_water_value == NULL) return;

    switch(level) {
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

void card_set_low_water(bool low)
{
    if(g_low_water_icon == NULL || g_low_water_text == NULL) return;

    if(low) {
        lv_label_set_text(g_low_water_text, "缺液");
        lv_obj_clear_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(g_low_water_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_low_water_text, LV_OBJ_FLAG_HIDDEN);
    }
}

void card_set_clean_idle(void)
{
    card_show_clean_items(false);
    card_show_default_items(true);

    card_set_temperature(45);
    card_set_time_min(10);
    card_set_water_level(3);
    card_set_low_water(true);
}

void card_set_clean_running(void)
{
    card_show_default_items(false);
    card_show_clean_items(true);

    if(g_clean_text) {
        lv_label_set_text(g_clean_text, "自动清洁中...");
    }
}

void card_set_clean_pending(void)
{
    card_show_default_items(false);
    card_show_clean_items(true);

    if(g_clean_text) {
        lv_label_set_text(g_clean_text, "进入自清洁");
    }
}
