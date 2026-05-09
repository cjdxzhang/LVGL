#include "setting.h"
#include "top_status.h"   /* 如果你顶部状态栏是独立文件，就保留；没有就先删这行 */
#include "setting_1.h"
#include "clk_light.h"
#include "wifi_set.h"
#include <stdint.h>

LV_FONT_DECLARE(setting_cn_1);
LV_FONT_DECLARE(setting_cn_title);
LV_IMAGE_DECLARE(q0ULJ1);
LV_FONT_DECLARE(back_pos);
LV_FONT_DECLARE(setting_mode);

static lv_obj_t * s_setting_root = NULL;
static lv_obj_t * s_clk_light_page = NULL;
static lv_obj_t * s_wifi_set_page = NULL;
static setting_back_cb_t s_back_cb = NULL;
static setting_item_cb_t s_item_cb = NULL;
static void apply_panel_style(lv_obj_t * obj, lv_coord_t radius);
static void apply_dark_btn_style(lv_obj_t * obj, lv_coord_t radius);
static void setting_item_event_cb(lv_event_t * e);
static void open_setting_1_page(void);
static void open_clk_light_page(void);
static void open_wifi_set_page(void);
static void setting_1_back_to_setting(void);
static void setting_1_go_home(void);
static void clk_light_back_to_setting(lv_event_t * e);
static void clk_light_go_home(lv_event_t * e);
static void wifi_set_back_to_setting(lv_event_t * e);
static void wifi_set_go_home(lv_event_t * e);

static void setting_back_btn_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    if(s_back_cb) {
        s_back_cb();
    }
}

static void apply_panel_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);

    /* 深色磨砂蓝：参考图二 */
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x132842), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x0E2238), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_90, 0);

    /* 边框弱一点，避免发白 */
    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x8FAECE), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_60, 0);

    /* 阴影偏深，增强厚重感 */
    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x08131F), 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_30, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t * create_setting_item(lv_obj_t * parent,
                                      lv_coord_t x, lv_coord_t y,
                                      const char * icon_txt,
                                      const char * title_txt,
                                      setting_item_id_t item_id)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 120, 120);
    lv_obj_set_pos(cont, x, y);

    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_shadow_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, setting_item_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)item_id);

    lv_obj_t * icon = lv_label_create(cont);
    lv_label_set_text(icon, icon_txt);
    lv_obj_set_style_text_font(icon, &setting_mode, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t * title = lv_label_create(cont);
    lv_label_set_text(title, title_txt);
    lv_obj_set_style_text_font(title, &setting_cn_1, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, 0);

    return cont;
}

lv_obj_t * setting_create(lv_obj_t * parent)
{
    if(s_setting_root) {
        return s_setting_root;
    }

    s_setting_root = lv_obj_create(parent);
    lv_obj_set_size(s_setting_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_radius(s_setting_root, 0, 0);
    lv_obj_set_style_border_width(s_setting_root, 0, 0);
    lv_obj_set_style_pad_all(s_setting_root, 0, 0);
    lv_obj_set_style_bg_opa(s_setting_root, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(s_setting_root, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶部状态栏：如果你已有独立函数，就调用 */
    status_bar_create(s_setting_root);

    /* 背景渐变层 */
    lv_obj_t * bg = lv_image_create(s_setting_root);
    lv_image_set_src(bg, &q0ULJ1);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_move_background(bg);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);

    /* 左侧大面板 */
    lv_obj_t * panel = lv_obj_create(s_setting_root);
    lv_obj_set_size(panel, 570, 390);
    lv_obj_set_pos(panel, 16, 56);
    apply_panel_style(panel, 36);
    lv_obj_set_style_pad_all(panel, 0, 0);

    /* 标题 */
    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "设置");
    lv_obj_set_style_text_font(title, &setting_cn_title, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    /* 分隔线 */
    lv_obj_t * line = lv_obj_create(panel);
    lv_obj_set_size(line, 480, 2);
    lv_obj_set_pos(line, 45, 58);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);
    lv_obj_set_style_bg_color(line, lv_color_hex(0xE6EEF8), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_60, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);

    /* 六个功能项
       这里图标先用占位符，你后面换自己的图标字体编码即可 */
    create_setting_item(panel,  60,  90, "\xEE\x99\xB6", "模式设置",SETTING_ITEM_MODE);
    create_setting_item(panel, 220,  90, "\xEE\xA1\xAD", "声光时钟",SETTING_ITEM_CLOCK);
    create_setting_item(panel, 380,  90, "\xEE\xA0\xBF", "WLAN设置",SETTING_ITEM_WLAN);

    create_setting_item(panel,  60, 220, "\xEE\x99\xB5", "定位设置",SETTING_ITEM_LOCATION);
    create_setting_item(panel, 220, 220, "\xEE\x98\x92", "高级设置",SETTING_ITEM_ADVANCED);
    create_setting_item(panel, 380, 220, "\xEE\x99\x81", "待定",SETTING_ITEM_TBD);

    /* 右下角返回按钮 */
    lv_obj_t * back_btn = lv_btn_create(s_setting_root);
    lv_obj_set_size(back_btn, 150, 150);
    lv_obj_set_pos(back_btn, 620, 297);
    apply_dark_btn_style(back_btn, 34);
    lv_obj_add_event_cb(back_btn, setting_back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * back_icon = lv_label_create(back_btn);
    lv_label_set_text(back_icon, "\xEE\x98\x81"); /* 这里后面换成你的返回图标编码 */
    lv_obj_set_style_text_font(back_icon, &back_pos, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_icon);

    return s_setting_root;
}

void setting_set_back_callback(setting_back_cb_t cb)
{
    s_back_cb = cb;
}

void setting_set_item_callback(setting_item_cb_t cb)
{
    s_item_cb = cb;
}

lv_obj_t * setting_get_root(void)
{
    return s_setting_root;
}

void setting_destroy(void)
{
    if(s_clk_light_page) {
        lv_obj_del(s_clk_light_page);
        s_clk_light_page = NULL;
    }

    if(s_wifi_set_page) {
        lv_obj_del(s_wifi_set_page);
        s_wifi_set_page = NULL;
    }

    setting_1_destroy();

    if(s_setting_root) {
        lv_obj_del(s_setting_root);
        s_setting_root = NULL;
    }
}

static void apply_dark_btn_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);

    lv_obj_set_style_bg_color(obj, lv_color_hex(0x102033), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x0A1827), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_90, 0);

    lv_obj_set_style_border_width(obj, 2, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x89A9C8), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_50, 0);

    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x050C15), 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_30, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void setting_item_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    setting_item_id_t item_id = (setting_item_id_t)(intptr_t)lv_event_get_user_data(e);

    if(item_id == SETTING_ITEM_MODE) {
        open_setting_1_page();
        return;
    }

    if(item_id == SETTING_ITEM_CLOCK) {
        open_clk_light_page();
        return;
    }

    if(item_id == SETTING_ITEM_WLAN) {
        open_wifi_set_page();
        return;
    }

    if(s_item_cb) {
        s_item_cb(item_id);
    }
}

static void setting_1_back_to_setting(void)
{
    setting_1_destroy();

    if(s_setting_root) {
        lv_obj_clear_flag(s_setting_root, LV_OBJ_FLAG_HIDDEN);
    }
}

static void setting_1_go_home(void)
{
    setting_1_destroy();

    if(s_setting_root) {
        lv_obj_del(s_setting_root);
        s_setting_root = NULL;
    }

    if(s_back_cb) {
        s_back_cb();
    }
}

static void open_setting_1_page(void)
{
    if(s_setting_root) {
        lv_obj_add_flag(s_setting_root, LV_OBJ_FLAG_HIDDEN);
    }

    setting_1_create(lv_scr_act());
    setting_1_set_back_callback(setting_1_back_to_setting);
    setting_1_set_home_callback(setting_1_go_home);
}

static void open_clk_light_page(void)
{
    if(s_setting_root) {
        lv_obj_add_flag(s_setting_root, LV_OBJ_FLAG_HIDDEN);
    }

    if(s_clk_light_page) {
        lv_obj_del(s_clk_light_page);
        s_clk_light_page = NULL;
    }

    clk_light_set_back_cb(clk_light_back_to_setting);
    clk_light_set_home_cb(clk_light_go_home);
    s_clk_light_page = clk_light_create(lv_scr_act());
}

static void open_wifi_set_page(void)
{
    if(s_setting_root) {
        lv_obj_add_flag(s_setting_root, LV_OBJ_FLAG_HIDDEN);
    }

    if(s_wifi_set_page) {
        lv_obj_del(s_wifi_set_page);
        s_wifi_set_page = NULL;
    }

    wifi_set_set_back_cb(wifi_set_back_to_setting);
    wifi_set_set_home_cb(wifi_set_go_home);
    s_wifi_set_page = wifi_set_create(lv_scr_act());
}

static void clk_light_back_to_setting(lv_event_t * e)
{
    LV_UNUSED(e);

    if(s_clk_light_page) {
        lv_obj_del(s_clk_light_page);
        s_clk_light_page = NULL;
    }

    if(s_setting_root) {
        lv_obj_clear_flag(s_setting_root, LV_OBJ_FLAG_HIDDEN);
    }
}

static void clk_light_go_home(lv_event_t * e)
{
    LV_UNUSED(e);

    if(s_clk_light_page) {
        lv_obj_del(s_clk_light_page);
        s_clk_light_page = NULL;
    }

    if(s_setting_root) {
        lv_obj_del(s_setting_root);
        s_setting_root = NULL;
    }

    if(s_back_cb) {
        s_back_cb();
    }
}

static void wifi_set_back_to_setting(lv_event_t * e)
{
    LV_UNUSED(e);

    if(s_wifi_set_page) {
        lv_obj_del(s_wifi_set_page);
        s_wifi_set_page = NULL;
    }

    if(s_setting_root) {
        lv_obj_clear_flag(s_setting_root, LV_OBJ_FLAG_HIDDEN);
    }
}

static void wifi_set_go_home(lv_event_t * e)
{
    LV_UNUSED(e);

    if(s_wifi_set_page) {
        lv_obj_del(s_wifi_set_page);
        s_wifi_set_page = NULL;
    }

    if(s_setting_root) {
        lv_obj_del(s_setting_root);
        s_setting_root = NULL;
    }

    if(s_back_cb) {
        s_back_cb();
    }
}

