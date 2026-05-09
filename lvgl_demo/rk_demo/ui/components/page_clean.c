#include "page_clean.h"
#include <lvgl/lvgl.h>
#include "top_status.h"
#include "card.h"
#include <stdbool.h>

extern void screen_home_set_clean_pending(bool pending);

LV_FONT_DECLARE(back_btn);
LV_FONT_DECLARE(clean_ic);
LV_FONT_DECLARE(st_ps);
LV_FONT_DECLARE(clean_cn);
LV_FONT_DECLARE(clean_cn_big);
LV_FONT_DECLARE(time_big);
LV_FONT_DECLARE(confirm_1);
LV_FONT_DECLARE(confirm_2);

static void (*s_show_page_cb)(lv_obj_t * page) = NULL;
static lv_obj_t * s_page_main = NULL;
static lv_obj_t * s_page_clean_confirm = NULL;
static lv_obj_t * s_page_clean_running = NULL;
static lv_obj_t * s_page_clean_finish = NULL;
static lv_obj_t * s_clean_time_label = NULL;

static void clean_stop_btn_cb(lv_event_t * e);
static void clean_finish_back_cb(lv_event_t * e);
static void clean_confirm_btn_ok_cb(lv_event_t * e);
static void clean_confirm_ok_btn_cb(lv_event_t * e);
static void apply_frosted_btn_style(lv_obj_t * obj);

static lv_obj_t * create_clean_confirm_page(lv_obj_t * parent)
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
    lv_obj_clear_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_radius(dialog, 28, 0);
    lv_obj_set_style_border_width(dialog, 1, 0);
    lv_obj_set_style_border_color(dialog, lv_color_hex(0xe1e6eb), 0);
    lv_obj_set_style_border_opa(dialog, LV_OPA_60, 0);

    lv_obj_set_style_bg_color(dialog, lv_color_hex(0x102033), 0);
    lv_obj_set_style_bg_grad_color(dialog, lv_color_hex(0x1A3550), 0);
    lv_obj_set_style_bg_grad_dir(dialog, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(dialog, LV_OPA_90, 0);

    lv_obj_set_style_shadow_width(dialog, 24, 0);
    lv_obj_set_style_shadow_opa(dialog, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(dialog, 0, 0);
    lv_obj_set_style_shadow_ofs_x(dialog, 0, 0);
    lv_obj_set_style_shadow_ofs_y(dialog, 8, 0);
    lv_obj_set_style_shadow_color(dialog, lv_color_hex(0x050B16), 0);

    lv_obj_set_style_pad_all(dialog, 0, 0);

    lv_obj_t * title = lv_label_create(dialog);
    lv_label_set_text(title, "停止自动清洁");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &confirm_2, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 38);

    lv_obj_t * tips = lv_label_create(dialog);
    lv_label_set_text(tips, "确认后将停止自动清洁流程");
    lv_obj_set_style_text_font(tips, &confirm_1, 0);
    lv_obj_set_style_text_color(tips, lv_color_hex(0xD8E2F0), 0);
    lv_obj_align(tips, LV_ALIGN_TOP_MID, 0, 105);

    lv_obj_t * btn_cancel = lv_btn_create(dialog);
    lv_obj_set_size(btn_cancel, 150, 56);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_MID, -95, -28);
    lv_obj_clear_flag(btn_cancel, LV_OBJ_FLAG_SCROLLABLE);
    apply_frosted_btn_style(btn_cancel);

    lv_obj_t * start_label = lv_label_create(btn_cancel);
    lv_label_set_text(start_label, "取消");
    lv_obj_set_style_text_font(start_label, &confirm_1, 0);
    lv_obj_center(start_label);

    lv_obj_t * btn_ok = lv_btn_create(dialog);
    lv_obj_set_size(btn_ok, 150, 56);
    lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_MID, 95, -28);
    lv_obj_clear_flag(btn_ok, LV_OBJ_FLAG_SCROLLABLE);
    apply_frosted_btn_style(btn_ok);

    lv_obj_t * cancel_label = lv_label_create(btn_ok);
    lv_label_set_text(cancel_label, "确认");
    lv_obj_set_style_text_font(cancel_label, &confirm_1, 0);
    lv_obj_center(cancel_label);

    lv_obj_add_event_cb(btn_cancel, clean_confirm_btn_ok_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_ok, clean_confirm_ok_btn_cb, LV_EVENT_CLICKED, NULL);

    return page;
}

static lv_obj_t * create_clean_running_page(lv_obj_t * parent)
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

    /* 左侧主信息卡 */
    lv_obj_t * card = lv_obj_create(page);
    lv_obj_set_size(card, 580, 390);
    lv_obj_set_pos(card, 25, 69);
    lv_obj_set_style_radius(card, 34, 0);

    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xe1e6eb), 0);
    lv_obj_set_style_border_opa(card, LV_OPA_50, 0);

    lv_obj_set_style_bg_color(card, lv_color_hex(0x102033), 0);
    lv_obj_set_style_bg_grad_color(card, lv_color_hex(0x1A3550), 0);
    lv_obj_set_style_bg_grad_dir(card, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_80, 0);

    lv_obj_set_style_shadow_width(card, 18, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(card, 0, 0);
    lv_obj_set_style_shadow_ofs_x(card, 0, 0);
    lv_obj_set_style_shadow_ofs_y(card, 6, 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0x050B16), 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * clean_icon = lv_label_create(card);
    lv_label_set_text(clean_icon, "\xEE\x99\xBE");   /* U+E67E */
    lv_obj_set_style_text_font(clean_icon, &clean_ic, 0);
    lv_obj_set_style_text_color(clean_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(clean_icon, 26, 18);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, "自动清洁");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &clean_cn, 0);
    lv_obj_set_pos(title, 120,44);

    s_clean_time_label = lv_label_create(card);
    lv_label_set_text(s_clean_time_label, "10:00"); /* 默认显示，进入页面后会被外部真实倒计时覆盖 */
    lv_obj_set_style_text_color(s_clean_time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_clean_time_label, &clean_cn_big, 0);
    lv_obj_set_pos(s_clean_time_label, 20, 150);

    lv_obj_t * min_label = lv_label_create(card);
    lv_label_set_text(min_label, "min");
    lv_obj_set_style_text_color(min_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(min_label, &time_big, 0);
    lv_obj_set_pos(min_label, 420, 153);

    lv_obj_t * sec_label = lv_label_create(card);
    lv_label_set_text(sec_label, "s");
    lv_obj_set_style_text_color(sec_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(sec_label, &time_big, 0);
    lv_obj_set_pos(sec_label, 420, 218);

    lv_obj_t * tips = lv_label_create(card);
    lv_label_set_text(tips, "正在清洁中...");
    lv_obj_set_style_text_color(tips, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(tips, &clean_cn, 0);
    lv_obj_align(tips, LV_ALIGN_BOTTOM_MID, 0, -17);

    /* 右侧停止按钮 */
    lv_obj_t * stop_box = lv_obj_create(page);
    lv_obj_set_size(stop_box, 140, 140);
    lv_obj_set_pos(stop_box, 630, 316);
    lv_obj_set_style_radius(stop_box, 30, 0);

    lv_obj_set_style_border_width(stop_box, 1, 0);
    lv_obj_set_style_border_color(stop_box, lv_color_hex(0xe1e6eb), 0);
    lv_obj_set_style_border_opa(stop_box, LV_OPA_50, 0);

    lv_obj_set_style_bg_color(stop_box, lv_color_hex(0x102033), 0);
    lv_obj_set_style_bg_grad_color(stop_box, lv_color_hex(0x1A3550), 0);
    lv_obj_set_style_bg_grad_dir(stop_box, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(stop_box, LV_OPA_80, 0);

    lv_obj_set_style_shadow_width(stop_box, 18, 0);
    lv_obj_set_style_shadow_opa(stop_box, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(stop_box, 0, 0);
    lv_obj_set_style_shadow_ofs_x(stop_box, 0, 0);
    lv_obj_set_style_shadow_ofs_y(stop_box, 6, 0);
    lv_obj_set_style_shadow_color(stop_box, lv_color_hex(0x050B16), 0);
    lv_obj_set_style_pad_all(stop_box, 0, 0);
    lv_obj_clear_flag(stop_box, LV_OBJ_FLAG_SCROLLABLE);

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

    lv_obj_t * stop_icon = lv_label_create(stop_btn);
    lv_label_set_text(stop_icon, "\xEE\x99\x9D");   /* U+E65D */
    lv_obj_set_style_text_color(stop_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stop_icon, &st_ps, 0);
    lv_obj_center(stop_icon);

    lv_obj_t * stop_text = lv_label_create(stop_box);
    lv_label_set_text(stop_text, "停止");
    lv_obj_set_style_text_color(stop_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stop_text, &clean_cn, 0);
    lv_obj_align(stop_text, LV_ALIGN_BOTTOM_MID, 0, -18);

    lv_obj_add_event_cb(stop_btn, clean_stop_btn_cb, LV_EVENT_CLICKED, NULL);

    return page;
}

static lv_obj_t * create_clean_finish_page(lv_obj_t * parent)
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

    lv_obj_t * title = lv_label_create(page);
    lv_label_set_text(title, "自动清洁完成");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t * tips = lv_label_create(page);
    lv_label_set_text(tips, "暖风烘干完成");
    lv_obj_set_style_text_color(tips, lv_color_hex(0xD8E2F0), 0);
    lv_obj_align(tips, LV_ALIGN_CENTER, 0, 10);

    lv_obj_t * back_btn = lv_btn_create(page);
    lv_obj_set_size(back_btn, 180, 58);
    lv_obj_align(back_btn, LV_ALIGN_CENTER, 0, 95);
    lv_obj_clear_flag(back_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "返回主页");
    lv_obj_center(back_label);

    lv_obj_add_event_cb(back_btn, clean_finish_back_cb, LV_EVENT_CLICKED, NULL);

    return page;
}

page_clean_set_t page_clean_create(lv_obj_t * parent,
                                   void (*show_page_cb)(lv_obj_t * page),
                                   lv_obj_t * page_main)
{
    page_clean_set_t set = {0};

    s_show_page_cb = show_page_cb;
    s_page_main = page_main;

    s_page_clean_confirm = create_clean_confirm_page(parent);
    s_page_clean_running = create_clean_running_page(parent);
    s_page_clean_finish  = create_clean_finish_page(parent);

    set.page_confirm = s_page_clean_confirm;
    set.page_running = s_page_clean_running;
    set.page_finish  = s_page_clean_finish;

    return set;
}

static void clean_stop_btn_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if(s_show_page_cb) s_show_page_cb(s_page_clean_confirm);
}

static void clean_finish_back_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    card_set_clean_idle();
    screen_home_set_clean_pending(false);

    if(s_show_page_cb) s_show_page_cb(s_page_main);
}

void page_clean_set_countdown(int min, int sec)
{
    static char buf[16];

    if(min < 0) min = 0;
    if(sec < 0) sec = 0;

    if(sec >= 60) {
        min += sec / 60;
        sec %= 60;
    }

    if(s_clean_time_label) {
        lv_snprintf(buf, sizeof(buf), "%02d:%02d", min, sec);
        lv_label_set_text(s_clean_time_label, buf);
    }
}

void page_clean_set_countdown_seconds(int total_sec)
{
    if(total_sec < 0) total_sec = 0;
    page_clean_set_countdown(total_sec / 60, total_sec % 60);
}

static void clean_confirm_btn_ok_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if(s_show_page_cb) s_show_page_cb(s_page_clean_running);
}

static void clean_confirm_ok_btn_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    card_set_clean_idle();
    screen_home_set_clean_pending(false);

    if(s_show_page_cb) s_show_page_cb(s_page_main);
}

static void apply_frosted_btn_style(lv_obj_t * obj)
{
    lv_obj_set_style_radius(obj, 20, 0);

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
