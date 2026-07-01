#include "page_loca_end.h"
#include "top_status.h"

LV_FONT_DECLARE(loca_end_cn);
LV_FONT_DECLARE(loca_end_cn1);

static void (*s_show_page_cb)(lv_obj_t * page) = NULL;
static lv_obj_t * s_page_main = NULL;
static lv_obj_t * s_page_loca_end = NULL;
static lv_timer_t * s_auto_back_timer = NULL;

static void apply_loca_end_panel_style(lv_obj_t * obj, lv_coord_t radius);
static void loca_end_auto_back_timer_cb(lv_timer_t * timer);
static void page_loca_end_start_auto_back(void);

lv_obj_t * page_loca_end_create(lv_obj_t * parent,
                                void (*show_page_cb)(lv_obj_t * page),
                                lv_obj_t * page_main)
{
    s_show_page_cb = show_page_cb;
    s_page_main = page_main;

    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_set_size(page, 800, 480);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_10, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);

    status_bar_create(page);

    lv_obj_t * card = lv_obj_create(page);
    lv_obj_set_size(card, 560, 300);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 20);
    apply_loca_end_panel_style(card, 34);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, "已到达指定位置");
    lv_obj_set_style_text_font(title, &loca_end_cn, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -28);

    lv_obj_t * tips = lv_label_create(card);
    lv_label_set_text(tips, "5s后开启健康足疗");
    lv_obj_set_style_text_font(tips, &loca_end_cn1, 0);
    lv_obj_set_style_text_color(tips, lv_color_hex(0xD8E2F0), 0);
    lv_obj_set_style_text_opa(tips, LV_OPA_90, 0);
    lv_obj_align(tips, LV_ALIGN_CENTER, 0, 34);

    s_page_loca_end = page;
    return page;
}

void page_loca_end_show(void)
{
    if(s_page_loca_end == NULL) {
        return;
    }

    if(s_show_page_cb) {
        s_show_page_cb(s_page_loca_end);
    }

    lv_obj_clear_flag(s_page_loca_end, LV_OBJ_FLAG_HIDDEN);
    page_loca_end_start_auto_back();
}

void page_loca_end_stop_auto_back(void)
{
    if(s_auto_back_timer) {
        lv_timer_delete(s_auto_back_timer);
        s_auto_back_timer = NULL;
    }
}

static void page_loca_end_start_auto_back(void)
{
    page_loca_end_stop_auto_back();
    s_auto_back_timer = lv_timer_create(loca_end_auto_back_timer_cb, 5000, NULL);
    lv_timer_set_repeat_count(s_auto_back_timer, 1);
}

static void loca_end_auto_back_timer_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);
    s_auto_back_timer = NULL;

    if(s_page_loca_end) {
        lv_obj_add_flag(s_page_loca_end, LV_OBJ_FLAG_HIDDEN);
    }

    if(s_show_page_cb) {
        s_show_page_cb(s_page_main);
    }
}

static void apply_loca_end_panel_style(lv_obj_t * obj, lv_coord_t radius)
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
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}
