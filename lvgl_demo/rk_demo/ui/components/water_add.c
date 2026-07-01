#include "water_add.h"
#include <stdio.h>
#include "components/top_status.h"

LV_FONT_DECLARE(clean_cn);
LV_FONT_DECLARE(confirm_1);
LV_FONT_DECLARE(st_ps);
LV_FONT_DECLARE(adwater_cf1);
LV_FONT_DECLARE(adwater_cf2);
LV_FONT_DECLARE(add_water);
LV_FONT_DECLARE(adwater_cn_s);
LV_FONT_DECLARE(adwater_num_s);
LV_FONT_DECLARE(adwater_ic1);
LV_FONT_DECLARE(temp);
LV_FONT_DECLARE(degree);
LV_FONT_DECLARE(water);
LV_FONT_DECLARE(footbath_liquid);

static void (*s_show_page_cb)(lv_obj_t * page) = NULL;
static lv_obj_t * s_page_main = NULL;
static lv_obj_t * s_page_water_running = NULL;
static lv_obj_t * s_page_water_confirm = NULL;
static lv_obj_t * s_temp_value = NULL;
static lv_obj_t * s_time_value = NULL;
static lv_obj_t * s_water_icon = NULL;
static lv_obj_t * s_medicine_icon1 = NULL;
static lv_obj_t * s_medicine_icon2 = NULL;

static void apply_frosted_style(lv_obj_t * obj, lv_coord_t radius);
static void water_stop_btn_cb(lv_event_t * e);
static void water_confirm_cancel_cb(lv_event_t * e);
static void water_confirm_ok_cb(lv_event_t * e);

static lv_obj_t * create_water_running_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_set_size(page, 800, 480);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);

    status_bar_create(page);

    lv_obj_t * card = lv_obj_create(page);
    lv_obj_set_size(card, 580, 390);
    lv_obj_set_pos(card, 25, 69);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    apply_frosted_style(card, 34);

    lv_obj_t * icon = lv_label_create(card);
    lv_label_set_text(icon, "\xEE\x9A\xAF");
    lv_obj_set_style_text_font(icon, &add_water, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(icon, 26, 18);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, "自动注水");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &adwater_cn_s, 0);
    lv_obj_set_pos(title, 95, 27);

    s_temp_value = lv_label_create(card);
    lv_label_set_text(s_temp_value, "45");
    lv_obj_set_style_text_color(s_temp_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_temp_value, &temp, 0);
    lv_obj_set_pos(s_temp_value, 20, 140);

    lv_obj_t * temp_unit = lv_label_create(card);
    lv_label_set_text(temp_unit, "\xEE\x98\xAE");
    lv_obj_set_style_text_color(temp_unit, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(temp_unit, &degree, 0);
    lv_obj_set_pos(temp_unit, 150, 140);

    lv_obj_t * time_title = lv_label_create(card);
    lv_label_set_text(time_title, "设定时间：");
    lv_obj_set_style_text_color(time_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(time_title, &adwater_cn_s, 0);
    lv_obj_set_pos(time_title, 245, 140);

    s_time_value = lv_label_create(card);
    lv_label_set_text(s_time_value, "10:00");
    lv_obj_set_style_text_color(s_time_value, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_time_value, &adwater_num_s, 0);
    lv_obj_set_pos(s_time_value, 380, 140);

    lv_obj_t * water_title = lv_label_create(card);
    lv_label_set_text(water_title, "水量：");
    lv_obj_set_style_text_color(water_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(water_title, &adwater_cn_s, 0);
    lv_obj_set_pos(water_title, 245, 198);

    s_water_icon = lv_label_create(card);
    lv_label_set_text(s_water_icon, "\xEE\xA0\xBC\xEE\xA0\xBC\xEE\xA0\xBC");
    lv_obj_set_style_text_color(s_water_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_water_icon, &water, 0);
    lv_obj_set_style_text_letter_space(s_water_icon, -8, 0);
    lv_obj_set_pos(s_water_icon, 310, 198);

    lv_obj_t * medicine_title = lv_label_create(card);
    lv_label_set_text(medicine_title, "药液：");
    lv_obj_set_style_text_color(medicine_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(medicine_title, &adwater_cn_s, 0);
    lv_obj_set_pos(medicine_title, 420, 198);

    s_medicine_icon1 = lv_label_create(card);
    lv_label_set_text(s_medicine_icon1, "\xEE\x98\x85");
    lv_obj_set_style_text_color(s_medicine_icon1, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_medicine_icon1, &footbath_liquid, 0);
    lv_obj_set_style_text_letter_space(s_medicine_icon1, -4, 0);
    lv_obj_set_style_transform_scale(s_medicine_icon1, 90, 0);
    lv_obj_set_style_transform_pivot_x(s_medicine_icon1, 0, 0);
    lv_obj_set_style_transform_pivot_y(s_medicine_icon1, 0, 0);
    lv_obj_set_pos(s_medicine_icon1, 494, 199);

    s_medicine_icon2 = lv_label_create(card);
    lv_label_set_text(s_medicine_icon2, "\xEE\x98\x86");
    lv_obj_set_style_text_color(s_medicine_icon2, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_medicine_icon2, &footbath_liquid, 0);
    lv_obj_set_style_text_letter_space(s_medicine_icon2, -4, 0);
    lv_obj_set_style_transform_scale(s_medicine_icon2, 90, 0);
    lv_obj_set_style_transform_pivot_x(s_medicine_icon2, 0, 0);
    lv_obj_set_style_transform_pivot_y(s_medicine_icon2, 0, 0);
    lv_obj_set_pos(s_medicine_icon2, 526, 199);

    lv_obj_t * status = lv_label_create(card);
    lv_label_set_text(status, "自动注水中...");
    lv_obj_set_style_text_color(status, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(status, &adwater_cn_s, 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -30);

    lv_obj_t * stop_box = lv_obj_create(page);
    lv_obj_set_size(stop_box, 140, 140);
    lv_obj_set_pos(stop_box, 630, 316);
    lv_obj_set_style_pad_all(stop_box, 0, 0);
    lv_obj_clear_flag(stop_box, LV_OBJ_FLAG_SCROLLABLE);
    apply_frosted_style(stop_box, 30);

    lv_obj_t * stop_btn = lv_btn_create(stop_box);
    lv_obj_set_size(stop_btn, 68, 68);
    lv_obj_align(stop_btn, LV_ALIGN_TOP_MID, 0, 18);
    lv_obj_clear_flag(stop_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(stop_btn, 34, 0);
    lv_obj_set_style_bg_opa(stop_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(stop_btn, 0, 0);
    lv_obj_set_style_shadow_width(stop_btn, 0, 0);
    lv_obj_set_style_outline_width(stop_btn, 0, 0);
    lv_obj_set_style_pad_all(stop_btn, 0, 0);
    lv_obj_add_event_cb(stop_btn, water_stop_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * stop_icon = lv_label_create(stop_btn);
    lv_label_set_text(stop_icon, "\xEE\x99\x9D");
    lv_obj_set_style_text_color(stop_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stop_icon, &st_ps, 0);
    lv_obj_center(stop_icon);

    lv_obj_t * stop_text = lv_label_create(stop_box);
    lv_label_set_text(stop_text, "停止");
    lv_obj_set_style_text_color(stop_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stop_text, &clean_cn, 0);
    lv_obj_align(stop_text, LV_ALIGN_BOTTOM_MID, 0, -18);

    return page;
}

static lv_obj_t * create_water_confirm_page(lv_obj_t * parent)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_set_size(page, 800, 480);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x081018), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);

    status_bar_create(page);

    lv_obj_t * dialog = lv_obj_create(page);
    lv_obj_set_size(dialog, 520, 250);
    lv_obj_align(dialog, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(dialog, 0, 0);
    lv_obj_clear_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
    apply_frosted_style(dialog, 28);

    lv_obj_t * title = lv_label_create(dialog);
    lv_label_set_text(title, "停止自动注水");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &adwater_cf1, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 38);

    lv_obj_t * tips = lv_label_create(dialog);
    lv_label_set_text(tips, "确认后将停止自动注水流程");
    lv_obj_set_style_text_font(tips, &adwater_cf2, 0);
    lv_obj_set_style_text_color(tips, lv_color_hex(0xD8E2F0), 0);
    lv_obj_align(tips, LV_ALIGN_TOP_MID, 0, 105);

    lv_obj_t * btn_cancel = lv_btn_create(dialog);
    lv_obj_set_size(btn_cancel, 150, 56);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_MID, -95, -28);
    lv_obj_clear_flag(btn_cancel, LV_OBJ_FLAG_SCROLLABLE);
    apply_frosted_style(btn_cancel, 20);
    lv_obj_add_event_cb(btn_cancel, water_confirm_cancel_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, "取消");
    lv_obj_set_style_text_font(label_cancel, &confirm_1, 0);
    lv_obj_center(label_cancel);

    lv_obj_t * btn_ok = lv_btn_create(dialog);
    lv_obj_set_size(btn_ok, 150, 56);
    lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_MID, 95, -28);
    lv_obj_clear_flag(btn_ok, LV_OBJ_FLAG_SCROLLABLE);
    apply_frosted_style(btn_ok, 20);
    lv_obj_add_event_cb(btn_ok, water_confirm_ok_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label_ok = lv_label_create(btn_ok);
    lv_label_set_text(label_ok, "确认");
    lv_obj_set_style_text_font(label_ok, &confirm_1, 0);
    lv_obj_center(label_ok);

    return page;
}

page_water_add_set_t page_water_add_create(lv_obj_t * parent,
                                           void (*show_page_cb)(lv_obj_t * page),
                                           lv_obj_t * page_main)
{
    page_water_add_set_t set = {0};
    s_show_page_cb = show_page_cb;
    s_page_main = page_main;
    s_page_water_running = create_water_running_page(parent);
    s_page_water_confirm = create_water_confirm_page(parent);
    set.page_running = s_page_water_running;
    set.page_confirm = s_page_water_confirm;
    return set;
}

void page_water_add_set_info(uint8_t temp,
                             uint16_t timer_minutes,
                             uint8_t water_level,
                             uint8_t herb1,
                             uint8_t herb2)
{
    char buf[16];

    if(s_temp_value) {
        snprintf(buf, sizeof(buf), "%u", temp);
        lv_label_set_text(s_temp_value, buf);
    }

    if(s_time_value) {
        snprintf(buf, sizeof(buf), "%u:00", timer_minutes);
        lv_label_set_text(s_time_value, buf);
    }

    if(s_water_icon) {
        switch(water_level) {
        case 1:
            lv_label_set_text(s_water_icon, "\xEE\xA0\xBC");
            break;
        case 2:
            lv_label_set_text(s_water_icon, "\xEE\xA0\xBC\xEE\xA0\xBC");
            break;
        case 3:
        default:
            lv_label_set_text(s_water_icon, "\xEE\xA0\xBC\xEE\xA0\xBC\xEE\xA0\xBC");
            break;
        }
    }

    if(s_medicine_icon1) {
        herb1 ? lv_obj_clear_flag(s_medicine_icon1, LV_OBJ_FLAG_HIDDEN)
              : lv_obj_add_flag(s_medicine_icon1, LV_OBJ_FLAG_HIDDEN);
    }

    if(s_medicine_icon2) {
        herb2 ? lv_obj_clear_flag(s_medicine_icon2, LV_OBJ_FLAG_HIDDEN)
              : lv_obj_add_flag(s_medicine_icon2, LV_OBJ_FLAG_HIDDEN);
    }
}

static void water_stop_btn_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if(s_show_page_cb) {
        s_show_page_cb(s_page_water_confirm);
    }
}

static void water_confirm_cancel_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if(s_show_page_cb) {
        s_show_page_cb(s_page_water_running);
    }
}

static void water_confirm_ok_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if(s_show_page_cb) {
        s_show_page_cb(s_page_main);
    }
}

static void apply_frosted_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0xe1e6eb), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_50, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x102033), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x1A3550), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);
    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x050B16), 0);
}