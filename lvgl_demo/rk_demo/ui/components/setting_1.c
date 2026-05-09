#include "setting_1.h"
#include "top_status.h"
#include "sleep_set.h"
#include <stdint.h>

LV_IMAGE_DECLARE(q0ULJ1);
/* 这里按你工程里现有字体来声明 */
LV_FONT_DECLARE(setting_cn1);
LV_FONT_DECLARE(setting_cn1_title);
LV_FONT_DECLARE(setting1_mode); 
LV_FONT_DECLARE(setting1_back); 
LV_FONT_DECLARE(back_pos);

static lv_obj_t * s_setting_1_root = NULL;
static setting_1_back_cb_t s_back_cb = NULL;
static setting_1_home_cb_t s_home_cb = NULL;
static lv_obj_t * g_sleep_set_page = NULL;

static void apply_panel_style(lv_obj_t * obj, lv_coord_t radius);
static void apply_side_btn_style(lv_obj_t * obj, lv_coord_t radius);
static lv_obj_t * create_mode_item(lv_obj_t * parent,
                                   lv_coord_t x, lv_coord_t y,
                                   const char * icon_txt,
                                   const char * title_txt);
static void back_btn_event_cb(lv_event_t * e);
static void home_btn_event_cb(lv_event_t * e);
static void sleep_set_back_cb(lv_event_t * e);
static void sleep_set_home_cb(lv_event_t * e);
static void sleep_item_event_cb(lv_event_t * e);

static void apply_panel_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);

    /* 磨砂蓝主卡片 */
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x173657), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x0F2741), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);

    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x8FB3D3), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_50, 0);

    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x08131F), 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_30, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void apply_side_btn_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);

    /* 右侧按钮也统一成磨砂蓝 */
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x18385B), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x102742), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);

    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x94B8D7), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_50, 0);

    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x07111C), 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_30, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t * create_mode_item(lv_obj_t * parent,
                                   lv_coord_t x, lv_coord_t y,
                                   const char * icon_txt,
                                   const char * title_txt)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 150, 125);
    lv_obj_set_pos(cont, x, y);

    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_shadow_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * icon = lv_label_create(cont);
    lv_label_set_text(icon, icon_txt);
    lv_obj_set_style_text_font(icon, &setting1_mode, 0);   /* 后续你可以替换成自己的模式页图标字体 */
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t * title = lv_label_create(cont);
    lv_label_set_text(title, title_txt);
    lv_obj_set_style_text_font(title, &setting_cn1, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, 0);

    return cont;
}

static void home_btn_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    if(s_home_cb) {
        s_home_cb();
    }
}

static void back_btn_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    if(s_back_cb) {
        s_back_cb();
    }
}

lv_obj_t * setting_1_create(lv_obj_t * parent)
{
    if(s_setting_1_root) {
    lv_obj_clear_flag(s_setting_1_root, LV_OBJ_FLAG_HIDDEN);
    return s_setting_1_root;
    }

    s_setting_1_root = lv_obj_create(parent);
    lv_obj_set_size(s_setting_1_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_radius(s_setting_1_root, 0, 0);
    lv_obj_set_style_border_width(s_setting_1_root, 0, 0);
    lv_obj_set_style_pad_all(s_setting_1_root, 0, 0);
    lv_obj_set_style_bg_opa(s_setting_1_root, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(s_setting_1_root, LV_OBJ_FLAG_SCROLLABLE);

    status_bar_create(s_setting_1_root);

    lv_obj_t * bg = lv_image_create(s_setting_1_root);
    lv_image_set_src(bg, &q0ULJ1);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_move_background(bg);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);

    /* 左侧主面板 */
    lv_obj_t * panel = lv_obj_create(s_setting_1_root);
    lv_obj_set_size(panel, 570, 390);
    lv_obj_set_pos(panel, 16, 56);
    apply_panel_style(panel, 28);
    lv_obj_set_style_pad_all(panel, 0, 0);

    /* 标题 */
    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "模式设置");
    lv_obj_set_style_text_font(title, &setting_cn1_title, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* 分隔线 */
    lv_obj_t * line = lv_obj_create(panel);
    lv_obj_set_size(line, 470, 2);
    lv_obj_set_pos(line, 45, 58);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);
    lv_obj_set_style_bg_color(line, lv_color_hex(0xE6EEF8), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_70, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);

    /*
     * 下面 6 个图标先用占位编码
     * 你后面可以直接把 home 页现成的图标编码替换进来
     */
    lv_obj_t * sleep_item = create_mode_item(panel,  60,  90, "\xEE\x98\x90", "助眠");
    lv_obj_add_event_cb(sleep_item, sleep_item_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(sleep_item, LV_OBJ_FLAG_CLICKABLE);
    create_mode_item(panel, 220,  90, "\xEE\x9E\x96", "放松");
    create_mode_item(panel, 380,  90, "\xEE\x9E\xA4", "驱寒");

    create_mode_item(panel,  60, 220, "\xEE\x98\xAA", "爸爸的jio");
    create_mode_item(panel, 220, 220, "\xEE\x9E\xA5", "新增");

    /* 右侧主页按钮 */
    lv_obj_t * home_btn = lv_btn_create(s_setting_1_root);
    lv_obj_set_size(home_btn, 150, 150);
    lv_obj_set_pos(home_btn, 620, 56);
    apply_side_btn_style(home_btn, 24);
    lv_obj_add_event_cb(home_btn, home_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * home_icon = lv_label_create(home_btn);
    lv_label_set_text(home_icon, "\xEE\x98\xAE");
    lv_obj_set_style_text_font(home_icon, &setting1_back, 0);
    lv_obj_set_style_text_color(home_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(home_icon);

    /* 右侧返回按钮 */
    lv_obj_t * back_btn = lv_btn_create(s_setting_1_root);
    lv_obj_set_size(back_btn, 150, 150);
    lv_obj_set_pos(back_btn, 620, 297);
    apply_side_btn_style(back_btn, 24);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * back_icon = lv_label_create(back_btn);
    lv_label_set_text(back_icon, "\xEE\x98\x81");
    lv_obj_set_style_text_font(back_icon, &back_pos, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_icon);

    return s_setting_1_root;
}

void setting_1_set_back_callback(setting_1_back_cb_t cb)
{
    s_back_cb = cb;
}

void setting_1_set_home_callback(setting_1_home_cb_t cb)
{
    s_home_cb = cb;
}

lv_obj_t * setting_1_get_root(void)
{
    return s_setting_1_root;
}

void setting_1_destroy(void)
{
    if(g_sleep_set_page) {
        lv_obj_del(g_sleep_set_page);
        g_sleep_set_page = NULL;
    }

    if(s_setting_1_root) {
        lv_obj_del(s_setting_1_root);
        s_setting_1_root = NULL;
    }
}

static void sleep_set_back_cb(lv_event_t * e)
{
    LV_UNUSED(e);

    if(g_sleep_set_page) {
        lv_obj_del(g_sleep_set_page);
        g_sleep_set_page = NULL;
    }

    if(s_setting_1_root) {
        lv_obj_clear_flag(s_setting_1_root, LV_OBJ_FLAG_HIDDEN);
    }
}

static void sleep_set_home_cb(lv_event_t * e)
{
    LV_UNUSED(e);

    if(g_sleep_set_page) {
        lv_obj_del(g_sleep_set_page);
        g_sleep_set_page = NULL;
    }

    if(s_setting_1_root) {
        lv_obj_add_flag(s_setting_1_root, LV_OBJ_FLAG_HIDDEN);
    }

    if(s_home_cb) {
        s_home_cb();
    }
}

static void sleep_item_event_cb(lv_event_t * e)
{
    LV_UNUSED(e);

    if(s_setting_1_root) {
        lv_obj_add_flag(s_setting_1_root, LV_OBJ_FLAG_HIDDEN);
    }

    if(g_sleep_set_page) {
        lv_obj_del(g_sleep_set_page);
        g_sleep_set_page = NULL;
    }

    sleep_set_set_back_cb(sleep_set_back_cb);
    sleep_set_set_home_cb(sleep_set_home_cb);

    g_sleep_set_page = sleep_set_create(lv_scr_act());
}
