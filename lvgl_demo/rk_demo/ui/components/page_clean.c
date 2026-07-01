#include "page_clean.h"
#include <lvgl/lvgl.h>
#include "top_status.h"
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
LV_FONT_DECLARE(clean_cn_new);
LV_FONT_DECLARE(clean_cnbig_new);
LV_FONT_DECLARE(clean_finish);
LV_FONT_DECLARE(clean_finish_cn);
LV_FONT_DECLARE(clean_finish_cn1);
LV_FONT_DECLARE(clean_finish_cn2);
LV_FONT_DECLARE(clean_finish_cn3);
LV_FONT_DECLARE(clean_finish_home);

#define CLEAN_STEP_LINE_X     78
#define CLEAN_STEP_LINE_Y     258
#define CLEAN_STEP_LINE_W     430
#define CLEAN_STEP_LINE_H     3
#define CLEAN_STEP_NODE_SIZE  38
#define CLEAN_STEP_ACTIVE_DOT_SIZE 18
#define CLEAN_STEP_GAP        (CLEAN_STEP_LINE_W / (PAGE_CLEAN_STEP_COUNT - 1))
#define CLEAN_SHINE_W         30
#define CLEAN_SHINE_H         5

static void (*s_show_page_cb)(lv_obj_t * page) = NULL;
static lv_obj_t * s_page_main = NULL;
static lv_obj_t * s_page_clean_confirm = NULL;
static lv_obj_t * s_page_clean_running = NULL;
static lv_obj_t * s_page_clean_finish = NULL;
static lv_obj_t * s_clean_step_label = NULL;
static lv_obj_t * s_clean_progress_fill = NULL;
static lv_obj_t * s_clean_progress_shine = NULL;
static lv_obj_t * s_clean_step_nodes[PAGE_CLEAN_STEP_COUNT] = {NULL};
static lv_obj_t * s_clean_step_node_labels[PAGE_CLEAN_STEP_COUNT] = {NULL};
static lv_obj_t * s_clean_step_inner_dots[PAGE_CLEAN_STEP_COUNT] = {NULL};
static int s_clean_current_step = PAGE_CLEAN_STEP_DRAIN_1;
static int s_clean_shine_anim_step = -1;

static const char * s_clean_step_texts[PAGE_CLEAN_STEP_COUNT] = {
    "排水中", "喷淋中", "排水中", "喷淋中", "排水中", "烘干中"
};

static void clean_stop_btn_cb(lv_event_t * e);
static void clean_finish_back_cb(lv_event_t * e);
static void clean_confirm_cancel_cb(lv_event_t * e);
static void clean_confirm_stop_cb(lv_event_t * e);
static void apply_frosted_btn_style(lv_obj_t * obj);
static void apply_clean_panel_style(lv_obj_t * obj, int radius);
static lv_obj_t * create_clean_glow_dot(lv_obj_t * parent, lv_coord_t cx, lv_coord_t cy);
static void create_clean_step_node(lv_obj_t * parent, int index);
static void clean_shine_anim_cb(void * obj, int32_t v);
static void start_clean_shine_anim(lv_obj_t * obj, lv_coord_t start_x, lv_coord_t end_x);
static lv_coord_t clean_step_center_x(int index);
static void update_clean_shine_range(int current_step);
static void update_clean_step_ui(int current_step);

// 点击停止自动清洁，弹出确认页面
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

    lv_obj_add_event_cb(btn_cancel, clean_confirm_cancel_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_ok, clean_confirm_stop_cb, LV_EVENT_CLICKED, NULL);

    return page;
}

// 清洁运行中页面
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

    lv_obj_t * card = lv_obj_create(page);
    lv_obj_set_size(card, 580, 390);
    lv_obj_set_pos(card, 24, 72);
    apply_clean_panel_style(card, 34);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * clean_icon = lv_label_create(card);
    lv_label_set_text(clean_icon, "\xEE\x99\xBE");
    lv_obj_set_style_text_font(clean_icon, &clean_ic, 0);
    lv_obj_set_style_text_color(clean_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(clean_icon, 38, 16);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, "自动清洁");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &clean_cn, 0);
    lv_obj_set_pos(title, 128, 41);

    lv_obj_t * progress_box = lv_obj_create(card);
    lv_obj_set_size(progress_box, 136, 42);
    lv_obj_set_pos(progress_box, 222, 78);
    lv_obj_set_style_radius(progress_box, 12, 0);
    lv_obj_set_style_bg_color(progress_box, lv_color_hex(0x102E52), 0);
    lv_obj_set_style_bg_opa(progress_box, LV_OPA_40, 0);
    lv_obj_set_style_border_width(progress_box, 1, 0);
    lv_obj_set_style_border_color(progress_box, lv_color_hex(0x6CCFFF), 0);
    lv_obj_set_style_border_opa(progress_box, LV_OPA_70, 0);
    lv_obj_set_style_shadow_width(progress_box, 8, 0);
    lv_obj_set_style_shadow_color(progress_box, lv_color_hex(0x42BFFF), 0);
    lv_obj_set_style_shadow_opa(progress_box, LV_OPA_40, 0);
    lv_obj_set_style_pad_all(progress_box, 0, 0);
    lv_obj_clear_flag(progress_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * progress_title = lv_label_create(progress_box);
    lv_label_set_text(progress_title, "流程进度");
    lv_obj_set_style_text_color(progress_title, lv_color_hex(0xDDEBFA), 0);
    lv_obj_set_style_text_font(progress_title, &clean_cn_new, 0);
    lv_obj_center(progress_title);

    s_clean_step_label = lv_label_create(card);
    lv_label_set_text(s_clean_step_label, s_clean_step_texts[PAGE_CLEAN_STEP_DRAIN_1]);
    lv_obj_set_style_text_color(s_clean_step_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_clean_step_label, &clean_cnbig_new, 0);
    lv_obj_align(s_clean_step_label, LV_ALIGN_TOP_MID, 0, 154);

    lv_obj_t * line_bg = lv_obj_create(card);
    lv_obj_set_size(line_bg, CLEAN_STEP_LINE_W, CLEAN_STEP_LINE_H);
    lv_obj_set_pos(line_bg, CLEAN_STEP_LINE_X, CLEAN_STEP_LINE_Y);
    lv_obj_set_style_radius(line_bg, 3, 0);
    lv_obj_set_style_border_width(line_bg, 0, 0);
    lv_obj_set_style_bg_color(line_bg, lv_color_hex(0x9FB4CC), 0);
    lv_obj_set_style_bg_opa(line_bg, LV_OPA_60, 0);
    lv_obj_set_style_pad_all(line_bg, 0, 0);
    lv_obj_clear_flag(line_bg, LV_OBJ_FLAG_SCROLLABLE);

    s_clean_progress_fill = lv_obj_create(card);
    lv_obj_set_size(s_clean_progress_fill, 0, CLEAN_STEP_LINE_H);
    lv_obj_set_pos(s_clean_progress_fill, CLEAN_STEP_LINE_X, CLEAN_STEP_LINE_Y);
    lv_obj_set_style_radius(s_clean_progress_fill, 3, 0);
    lv_obj_set_style_border_width(s_clean_progress_fill, 0, 0);
    lv_obj_set_style_bg_color(s_clean_progress_fill, lv_color_hex(0x3B9EFF), 0);
    lv_obj_set_style_bg_grad_color(s_clean_progress_fill, lv_color_hex(0x75D9FF), 0);
    lv_obj_set_style_bg_grad_dir(s_clean_progress_fill, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(s_clean_progress_fill, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(s_clean_progress_fill, 8, 0);
    lv_obj_set_style_shadow_color(s_clean_progress_fill, lv_color_hex(0x2AA5FF), 0);
    lv_obj_set_style_shadow_opa(s_clean_progress_fill, LV_OPA_70, 0);
    lv_obj_set_style_pad_all(s_clean_progress_fill, 0, 0);
    lv_obj_clear_flag(s_clean_progress_fill, LV_OBJ_FLAG_SCROLLABLE);

    s_clean_progress_shine = lv_obj_create(card);
    lv_obj_set_size(s_clean_progress_shine, CLEAN_SHINE_W, CLEAN_SHINE_H);
    lv_obj_set_pos(s_clean_progress_shine, CLEAN_STEP_LINE_X, CLEAN_STEP_LINE_Y - 1);
    lv_obj_set_style_radius(s_clean_progress_shine, 4, 0);
    lv_obj_set_style_border_width(s_clean_progress_shine, 0, 0);
    lv_obj_set_style_bg_color(s_clean_progress_shine, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_grad_color(s_clean_progress_shine, lv_color_hex(0x65D8FF), 0);
    lv_obj_set_style_bg_grad_dir(s_clean_progress_shine, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(s_clean_progress_shine, LV_OPA_80, 0);
    lv_obj_set_style_shadow_width(s_clean_progress_shine, 16, 0);
    lv_obj_set_style_shadow_color(s_clean_progress_shine, lv_color_hex(0x62D9FF), 0);
    lv_obj_set_style_shadow_opa(s_clean_progress_shine, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(s_clean_progress_shine, 0, 0);
    lv_obj_clear_flag(s_clean_progress_shine, LV_OBJ_FLAG_SCROLLABLE);

    for (int i = 0; i < PAGE_CLEAN_STEP_COUNT; i++) {
        create_clean_step_node(card, i);
    }

    lv_obj_t * tips = lv_label_create(card);
    lv_label_set_text(tips, "清洁进行中");
    lv_obj_set_style_text_color(tips, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(tips, &clean_cn_new, 0);
    lv_obj_set_style_text_opa(tips, LV_OPA_80, 0);
    lv_obj_align(tips, LV_ALIGN_BOTTOM_MID, 7, -22);

    create_clean_glow_dot(card, 112, 354);
    create_clean_glow_dot(card, 152, 354);
    create_clean_glow_dot(card, 192, 354);
    create_clean_glow_dot(card, 390, 354);
    create_clean_glow_dot(card, 430, 354);
    create_clean_glow_dot(card, 470, 354);

    lv_obj_t * stop_box = lv_obj_create(page);
    lv_obj_set_size(stop_box, 140, 140);
    lv_obj_set_pos(stop_box, 630, 316);
    apply_clean_panel_style(stop_box, 30);
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
    lv_label_set_text(stop_icon, "\xEE\x99\x9D");
    lv_obj_set_style_text_color(stop_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stop_icon, &st_ps, 0);
    lv_obj_center(stop_icon);

    lv_obj_t * stop_text = lv_label_create(stop_box);
    lv_label_set_text(stop_text, "停止");
    lv_obj_set_style_text_color(stop_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(stop_text, &clean_cn, 0);
    lv_obj_align(stop_text, LV_ALIGN_BOTTOM_MID, 0, -18);

    lv_obj_add_event_cb(stop_btn, clean_stop_btn_cb, LV_EVENT_CLICKED, NULL);
    update_clean_step_ui(PAGE_CLEAN_STEP_DRAIN_1);

    return page;
}

// 清洁完成页面，显示清洁完成，提供返回按钮
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

    {
        lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);

        lv_obj_t * finish_panel = lv_obj_create(page);
        lv_obj_set_size(finish_panel, 580, 360);
        lv_obj_set_pos(finish_panel, 24, 70);
        apply_clean_panel_style(finish_panel, 34);
        lv_obj_set_style_pad_all(finish_panel, 0, 0);
        lv_obj_clear_flag(finish_panel, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t * clean_icon = lv_label_create(finish_panel);
        lv_label_set_text(clean_icon, "\xEE\x99\xBE");
        lv_obj_set_style_text_font(clean_icon, &clean_ic, 0);
        lv_obj_set_style_text_color(clean_icon, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(clean_icon, 43, 27);

        lv_obj_t * panel_title = lv_label_create(finish_panel);
        lv_label_set_text(panel_title, "\xE8\x87\xAA\xE5\x8A\xA8\xE6\xB8\x85\xE6\xB4\x81");
        lv_obj_set_style_text_color(panel_title, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(panel_title, &clean_cn, 0);
        lv_obj_set_pos(panel_title, 133, 53);

        lv_obj_t * finish_icon_wrap = lv_obj_create(finish_panel);
        lv_obj_set_size(finish_icon_wrap, 150, 150);
        lv_obj_align(finish_icon_wrap, LV_ALIGN_TOP_MID, 0, 90);
        lv_obj_set_style_radius(finish_icon_wrap, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(finish_icon_wrap, 3, 0);
        lv_obj_set_style_border_color(finish_icon_wrap, lv_color_hex(0xD8F4FF), 0);
        lv_obj_set_style_border_opa(finish_icon_wrap, LV_OPA_90, 0);
        lv_obj_set_style_bg_color(finish_icon_wrap, lv_color_hex(0x1C4774), 0);
        lv_obj_set_style_bg_opa(finish_icon_wrap, LV_OPA_40, 0);
        lv_obj_set_style_shadow_width(finish_icon_wrap, 26, 0);
        lv_obj_set_style_shadow_color(finish_icon_wrap, lv_color_hex(0x60D6FF), 0);
        lv_obj_set_style_shadow_opa(finish_icon_wrap, LV_OPA_70, 0);
        lv_obj_set_style_pad_all(finish_icon_wrap, 0, 0);
        lv_obj_clear_flag(finish_icon_wrap, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(finish_icon_wrap, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t * finish_icon = lv_label_create(finish_icon_wrap);
        lv_label_set_text(finish_icon, "\xEE\x99\xAA");
        lv_obj_set_style_text_font(finish_icon, &clean_finish, 0);
        lv_obj_set_style_text_color(finish_icon, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(finish_icon);

        lv_obj_t * finish_title = lv_label_create(finish_panel);
        lv_label_set_text(finish_title, "\xE8\x87\xAA\xE6\xB8\x85\xE6\xB4\x81\xE5\xB7\xB2\xE5\xAE\x8C\xE6\x88\x90");
        lv_obj_set_style_text_color(finish_title, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(finish_title, &clean_finish_cn, 0);
        lv_obj_align(finish_title, LV_ALIGN_TOP_MID, 0, 258);

        lv_obj_t * finish_tips = lv_label_create(finish_panel);
        lv_label_set_text(finish_tips, "\xE5\x96\xB7\xE6\xB7\x8B\xE6\xB8\x85\xE6\xB4\x81\xE5\xAE\x8C\xE6\x88\x90  \xC2\xB7  \xE6\x9A\x96\xE9\xA3\x8E\xE7\x83\x98\xE5\xB9\xB2\xE5\xAE\x8C\xE6\x88\x90");
        lv_obj_set_style_text_color(finish_tips, lv_color_hex(0xD8E2F0), 0);
        lv_obj_set_style_text_opa(finish_tips, LV_OPA_90, 0);
        lv_obj_set_style_text_font(finish_tips, &clean_finish_cn3, 0);
        lv_obj_align(finish_tips, LV_ALIGN_BOTTOM_MID, 0, -20);

        lv_obj_t * back_btn = lv_obj_create(page);
        lv_obj_set_size(back_btn, 150, 150);
        lv_obj_set_pos(back_btn, 622, 280);
        apply_clean_panel_style(back_btn, 30);
        lv_obj_set_style_pad_all(back_btn, 0, 0);
        lv_obj_clear_flag(back_btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t * home_icon = lv_label_create(back_btn);
        lv_label_set_text(home_icon, "\xEE\x98\xAE");
        lv_obj_set_style_text_color(home_icon, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(home_icon, &clean_finish_home, 0);
        lv_obj_align(home_icon, LV_ALIGN_TOP_MID, 0, 10);

        lv_obj_t * back_label = lv_label_create(back_btn);
        lv_label_set_text(back_label, "\xE8\xBF\x94\xE5\x9B\x9E\xE9\xA6\x96\xE9\xA1\xB5");
        lv_obj_set_style_text_color(back_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(back_label, &clean_finish_cn1, 0);
        lv_obj_align(back_label, LV_ALIGN_TOP_MID, 0, 79);

        lv_obj_t * auto_label = lv_label_create(back_btn);
        lv_label_set_text(auto_label, "2min\xE5\x90\x8E\xE8\x87\xAA\xE5\x8A\xA8\xE8\xBF\x94\xE5\x9B\x9E");
        lv_obj_set_style_text_color(auto_label, lv_color_hex(0xD8E2F0), 0);
        lv_obj_set_style_text_opa(auto_label, LV_OPA_80, 0);
        lv_obj_set_style_text_font(auto_label, &clean_finish_cn2, 0);
        lv_obj_align(auto_label, LV_ALIGN_BOTTOM_MID, 0, -10);

        lv_obj_add_event_cb(back_btn, clean_finish_back_cb, LV_EVENT_CLICKED, NULL);

        return page;
    }

    return page;
}

// 清洁页面显示函数，创建三个页面并返回页面对象，外部通过 show_page_cb 切换显示
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

// 停止自动清洁按钮回调，显示确认页面
static void clean_stop_btn_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_show_page_cb) {
        s_show_page_cb(s_page_clean_confirm);
    }
}

// 清洁完成页面返回按钮回调，返回主页
static void clean_finish_back_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    screen_home_set_clean_pending(false);

    if (s_show_page_cb) {
        s_show_page_cb(s_page_main);
    }
}

void page_clean_set_step(page_clean_step_t step)
{
    update_clean_step_ui(step);
}

// 确认取消停止自动清洁，返回运行页面
static void clean_confirm_cancel_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }
    if (s_show_page_cb) {
        s_show_page_cb(s_page_clean_running);
    }
}

// 确认停止自动清洁，返回主页
static void clean_confirm_stop_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    screen_home_set_clean_pending(false);

    if (s_show_page_cb) {
        s_show_page_cb(s_page_main);
    }
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

static void apply_clean_panel_style(lv_obj_t * obj, int radius)
{
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0xDCEBFA), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_50, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x10223A), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x07172B), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);
    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x050B16), 0);
}

static lv_obj_t * create_clean_glow_dot(lv_obj_t * parent, lv_coord_t cx, lv_coord_t cy)
{
    lv_obj_t * wrap = lv_obj_create(parent);
    lv_obj_set_size(wrap, 18, 18);
    lv_obj_set_pos(wrap, cx - 9, cy - 9);
    lv_obj_set_style_bg_opa(wrap, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(wrap, 0, 0);
    lv_obj_set_style_pad_all(wrap, 0, 0);
    lv_obj_set_style_shadow_width(wrap, 0, 0);
    lv_obj_clear_flag(wrap, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(wrap, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * glow = lv_obj_create(wrap);
    lv_obj_set_size(glow, 13, 13);
    lv_obj_center(glow);
    lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(glow, lv_color_hex(0x66D9FF), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_30, 0);
    lv_obj_set_style_border_width(glow, 0, 0);
    lv_obj_set_style_shadow_width(glow, 10, 0);
    lv_obj_set_style_shadow_spread(glow, 1, 0);
    lv_obj_set_style_shadow_color(glow, lv_color_hex(0x4CCFFF), 0);
    lv_obj_set_style_shadow_opa(glow, LV_OPA_90, 0);
    lv_obj_set_style_pad_all(glow, 0, 0);
    lv_obj_clear_flag(glow, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(glow, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * ring = lv_obj_create(wrap);
    lv_obj_set_size(ring, 12, 12);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ring, 1, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0xA8EEFF), 0);
    lv_obj_set_style_border_opa(ring, LV_OPA_100, 0);
    lv_obj_set_style_shadow_width(ring, 0, 0);
    lv_obj_set_style_pad_all(ring, 0, 0);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * core = lv_obj_create(wrap);
    lv_obj_set_size(core, 6, 6);
    lv_obj_center(core);
    lv_obj_set_style_radius(core, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(core, lv_color_hex(0x7BE6FF), 0);
    lv_obj_set_style_bg_opa(core, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(core, 0, 0);
    lv_obj_set_style_shadow_width(core, 0, 0);
    lv_obj_set_style_pad_all(core, 0, 0);
    lv_obj_clear_flag(core, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(core, LV_OBJ_FLAG_CLICKABLE);

    return wrap;
}

static void create_clean_step_node(lv_obj_t * parent, int index)
{
    lv_coord_t center_x = CLEAN_STEP_LINE_X + index * CLEAN_STEP_GAP;
    lv_obj_t * node = lv_obj_create(parent);
    lv_obj_set_size(node, CLEAN_STEP_NODE_SIZE, CLEAN_STEP_NODE_SIZE);
    lv_obj_set_pos(node, center_x - CLEAN_STEP_NODE_SIZE / 2,
                   CLEAN_STEP_LINE_Y - CLEAN_STEP_NODE_SIZE / 2 + CLEAN_STEP_LINE_H / 2);
    lv_obj_set_style_radius(node, CLEAN_STEP_NODE_SIZE / 2, 0);
    lv_obj_set_style_border_width(node, 2, 0);
    lv_obj_set_style_border_color(node, lv_color_hex(0xC5D8EC), 0);
    lv_obj_set_style_border_opa(node, LV_OPA_70, 0);
    lv_obj_set_style_bg_color(node, lv_color_hex(0x16263A), 0);
    lv_obj_set_style_bg_opa(node, LV_OPA_90, 0);
    lv_obj_set_style_pad_all(node, 0, 0);
    lv_obj_clear_flag(node, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * label = lv_label_create(node);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    lv_obj_t * inner_dot = lv_obj_create(node);
    lv_obj_set_size(inner_dot, CLEAN_STEP_ACTIVE_DOT_SIZE, CLEAN_STEP_ACTIVE_DOT_SIZE);
    lv_obj_set_style_radius(inner_dot, CLEAN_STEP_ACTIVE_DOT_SIZE / 2, 0);
    lv_obj_set_style_border_width(inner_dot, 2, 0);
    lv_obj_set_style_border_color(inner_dot, lv_color_hex(0xC6F6FF), 0);
    lv_obj_set_style_bg_color(inner_dot, lv_color_hex(0x64E6FF), 0);
    lv_obj_set_style_bg_opa(inner_dot, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(inner_dot, 10, 0);
    lv_obj_set_style_shadow_color(inner_dot, lv_color_hex(0x61D9FF), 0);
    lv_obj_set_style_shadow_opa(inner_dot, LV_OPA_70, 0);
    lv_obj_set_style_pad_all(inner_dot, 0, 0);
    lv_obj_clear_flag(inner_dot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(inner_dot);
    lv_obj_add_flag(inner_dot, LV_OBJ_FLAG_HIDDEN);

    s_clean_step_nodes[index] = node;
    s_clean_step_node_labels[index] = label;
    s_clean_step_inner_dots[index] = inner_dot;
}

static void clean_shine_anim_cb(void * obj, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)v);
}

static void start_clean_shine_anim(lv_obj_t * obj, lv_coord_t start_x, lv_coord_t end_x)
{
    lv_anim_del(obj, clean_shine_anim_cb);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start_x, end_x);
    lv_anim_set_duration(&a, 2200);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_exec_cb(&a, clean_shine_anim_cb);
    lv_anim_start(&a);
}

static lv_coord_t clean_step_center_x(int index)
{
    return CLEAN_STEP_LINE_X + index * CLEAN_STEP_GAP;
}

static void update_clean_shine_range(int current_step)
{
    if(s_clean_progress_shine == NULL) return;

    if(current_step <= PAGE_CLEAN_STEP_DRAIN_1) {
        lv_anim_del(s_clean_progress_shine, clean_shine_anim_cb);
        lv_obj_add_flag(s_clean_progress_shine, LV_OBJ_FLAG_HIDDEN);
        s_clean_shine_anim_step = current_step;
        return;
    }

    if(s_clean_shine_anim_step == current_step) {
        return;
    }

    s_clean_shine_anim_step = current_step;
    lv_obj_clear_flag(s_clean_progress_shine, LV_OBJ_FLAG_HIDDEN);

    lv_coord_t start_x = clean_step_center_x(PAGE_CLEAN_STEP_DRAIN_1) - CLEAN_SHINE_W / 2;
    lv_coord_t end_x = clean_step_center_x(current_step) - CLEAN_SHINE_W / 2;
    if(end_x <= start_x) end_x = start_x;

    lv_obj_set_x(s_clean_progress_shine, start_x);
    lv_obj_set_y(s_clean_progress_shine, CLEAN_STEP_LINE_Y - 1);
    start_clean_shine_anim(s_clean_progress_shine, start_x, end_x);
}

static void update_clean_step_ui(int current_step)
{
    if(current_step < PAGE_CLEAN_STEP_DRAIN_1) current_step = PAGE_CLEAN_STEP_DRAIN_1;
    if(current_step >= PAGE_CLEAN_STEP_COUNT) current_step = PAGE_CLEAN_STEP_COUNT - 1;

    s_clean_current_step = current_step;

    if (s_clean_step_label) {
        lv_label_set_text(s_clean_step_label, s_clean_step_texts[current_step]);
    }

    if (s_clean_progress_fill) {
        lv_obj_set_width(s_clean_progress_fill, current_step * CLEAN_STEP_GAP);
    }
    update_clean_shine_range(current_step);

    for (int i = 0; i < PAGE_CLEAN_STEP_COUNT; i++) {
        lv_obj_t * node = s_clean_step_nodes[i];
        lv_obj_t * label = s_clean_step_node_labels[i];
        lv_obj_t * inner_dot = s_clean_step_inner_dots[i];
        char num_buf[4];

        if (node == NULL || label == NULL) {
            continue;
        }
        if (inner_dot) {
            lv_obj_add_flag(inner_dot, LV_OBJ_FLAG_HIDDEN);
        }

        if (i < current_step) {
            lv_obj_set_style_border_color(node, lv_color_hex(0x74D6FF), 0);
            lv_obj_set_style_border_opa(node, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(node, lv_color_hex(0x12305A), 0);
            lv_obj_set_style_bg_opa(node, LV_OPA_COVER, 0);
            lv_obj_set_style_shadow_width(node, 14, 0);
            lv_obj_set_style_shadow_color(node, lv_color_hex(0x4DBDFF), 0);
            lv_obj_set_style_shadow_opa(node, LV_OPA_70, 0);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
            lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
            lv_label_set_text(label, LV_SYMBOL_OK);
        } else if (i == current_step) {
            lv_obj_set_style_border_color(node, lv_color_hex(0x68DDFF), 0);
            lv_obj_set_style_border_opa(node, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(node, lv_color_hex(0x2177B9), 0);
            lv_obj_set_style_bg_opa(node, LV_OPA_COVER, 0);
            lv_obj_set_style_shadow_width(node, 26, 0);
            lv_obj_set_style_shadow_color(node, lv_color_hex(0x2BC8FF), 0);
            lv_obj_set_style_shadow_opa(node, LV_OPA_90, 0);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_22, 0);
            lv_label_set_text(label, "");
            if (inner_dot) {
                lv_obj_clear_flag(inner_dot, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(inner_dot);
            }
        } else {
            lv_obj_set_style_border_color(node, lv_color_hex(0xB8C8DA), 0);
            lv_obj_set_style_border_opa(node, LV_OPA_70, 0);
            lv_obj_set_style_bg_color(node, lv_color_hex(0x142339), 0);
            lv_obj_set_style_bg_opa(node, LV_OPA_90, 0);
            lv_obj_set_style_shadow_width(node, 0, 0);
            lv_obj_set_style_shadow_opa(node, LV_OPA_TRANSP, 0);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
            lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
            lv_snprintf(num_buf, sizeof(num_buf), "%d", i + 1);
            lv_label_set_text(label, num_buf);
        }
    }

    for(int i = 0; i < PAGE_CLEAN_STEP_COUNT; i++) {
        if(s_clean_step_nodes[i]) {
            lv_obj_move_foreground(s_clean_step_nodes[i]);
        }
    }
}
