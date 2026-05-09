#include "wifi_set.h"

#include "top_status.h"

LV_IMAGE_DECLARE(q0ULJ1);
LV_FONT_DECLARE(back_pos);
LV_FONT_DECLARE(setting1_back);
LV_FONT_DECLARE(wifi_icon_font);
LV_FONT_DECLARE(lv_font_source_han_sans_sc_16_cjk);

static lv_obj_t * s_root = NULL;
static lv_obj_t * s_ssid_ta = NULL;
static lv_obj_t * s_pwd_ta = NULL;
static lv_obj_t * s_keyboard = NULL;
static lv_obj_t * s_hint_label = NULL;
static lv_obj_t * s_connect_btn_label = NULL;
static bool s_connected = false;
static lv_event_cb_t s_back_cb = NULL;
static lv_event_cb_t s_home_cb = NULL;

static void apply_panel_style(lv_obj_t * obj, lv_coord_t radius);
static void apply_side_btn_style(lv_obj_t * obj, lv_coord_t radius);
static void apply_input_row_style(lv_obj_t * obj, lv_coord_t radius);
static void set_keyboard_target(lv_obj_t * ta);
static void hide_keyboard(void);
static void update_connect_view(void);
static void home_btn_event_cb(lv_event_t * e);
static void back_btn_event_cb(lv_event_t * e);
static void input_event_cb(lv_event_t * e);
static void keyboard_event_cb(lv_event_t * e);
static void connect_btn_event_cb(lv_event_t * e);

static void apply_panel_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);
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

static void apply_input_row_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_90, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_left(obj, 12, 0);
    lv_obj_set_style_pad_right(obj, 12, 0);
    lv_obj_set_style_pad_top(obj, 0, 0);
    lv_obj_set_style_pad_bottom(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void set_keyboard_target(lv_obj_t * ta)
{
    if(s_keyboard == NULL || ta == NULL) {
        return;
    }

    lv_keyboard_set_textarea(s_keyboard, ta);
    lv_obj_clear_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void hide_keyboard(void)
{
    if(s_keyboard == NULL) {
        return;
    }

    lv_keyboard_set_textarea(s_keyboard, NULL);
    lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void update_connect_view(void)
{
    if(s_hint_label == NULL || s_connect_btn_label == NULL) {
        return;
    }

    if(s_connected) {
        lv_label_set_text(s_hint_label, "已连接");
        lv_label_set_text(s_connect_btn_label, "断开连接");
    }
    else {
        lv_label_set_text(s_hint_label, "请确保连接的家庭wifi网络为2.4G\n或2.4/5G混合网络");
        lv_label_set_text(s_connect_btn_label, "下一步");
    }
}

static void home_btn_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    hide_keyboard();
    if(s_home_cb) {
        s_home_cb(e);
    }
}

static void back_btn_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    hide_keyboard();
    if(s_back_cb) {
        s_back_cb(e);
    }
}

static void input_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        set_keyboard_target(ta);
    }
}

static void keyboard_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        hide_keyboard();
    }
}

static void connect_btn_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    hide_keyboard();
    s_connected = !s_connected;
    update_connect_view();
}

lv_obj_t * wifi_set_create(lv_obj_t * parent)
{
    if(s_root) {
        lv_obj_clear_flag(s_root, LV_OBJ_FLAG_HIDDEN);
        return s_root;
    }

    s_connected = false;

    s_root = lv_obj_create(parent);
    lv_obj_set_size(s_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_radius(s_root, 0, 0);
    lv_obj_set_style_border_width(s_root, 0, 0);
    lv_obj_set_style_pad_all(s_root, 0, 0);
    lv_obj_set_style_bg_opa(s_root, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(s_root, LV_OBJ_FLAG_SCROLLABLE);

    status_bar_create(s_root);

    lv_obj_t * bg = lv_image_create(s_root);
    lv_image_set_src(bg, &q0ULJ1);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_move_background(bg);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * panel = lv_obj_create(s_root);
    lv_obj_set_size(panel, 570, 390);
    lv_obj_set_pos(panel, 16, 56);
    apply_panel_style(panel, 28);
    lv_obj_set_style_pad_all(panel, 0, 0);

    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "wifi设置");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_source_han_sans_sc_16_cjk, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    lv_obj_t * line = lv_obj_create(panel);
    lv_obj_set_size(line, 470, 2);
    lv_obj_set_pos(line, 45, 58);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);
    lv_obj_set_style_bg_color(line, lv_color_hex(0xE6EEF8), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_70, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * ssid_row = lv_obj_create(panel);
    lv_obj_set_size(ssid_row, 380, 42);
    lv_obj_set_pos(ssid_row, 55, 90);
    apply_input_row_style(ssid_row, 10);

    lv_obj_t * ssid_icon = lv_label_create(ssid_row);
    lv_label_set_text(ssid_icon, "\xEE\x98\xBD");
    lv_obj_set_style_text_font(ssid_icon, &wifi_icon_font, 0);
    lv_obj_set_style_text_color(ssid_icon, lv_color_hex(0x8990B2), 0);
    lv_obj_align(ssid_icon, LV_ALIGN_LEFT_MID, 0, 0);

    s_ssid_ta = lv_textarea_create(ssid_row);
    lv_obj_set_size(s_ssid_ta, 330, 32);
    lv_obj_align(s_ssid_ta, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_textarea_set_one_line(s_ssid_ta, true);
    lv_textarea_set_placeholder_text(s_ssid_ta, "网络名称");
    lv_textarea_set_max_length(s_ssid_ta, 32);
    lv_obj_set_style_bg_opa(s_ssid_ta, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_ssid_ta, 0, 0);
    lv_obj_set_style_outline_width(s_ssid_ta, 0, 0);
    lv_obj_set_style_text_color(s_ssid_ta, lv_color_hex(0x6E7092), 0);
    lv_obj_set_style_text_font(s_ssid_ta, &lv_font_montserrat_20, 0);
    lv_obj_set_style_pad_all(s_ssid_ta, 0, 0);
    lv_obj_add_event_cb(s_ssid_ta, input_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(s_ssid_ta, input_event_cb, LV_EVENT_FOCUSED, NULL);

    lv_obj_t * pwd_row = lv_obj_create(panel);
    lv_obj_set_size(pwd_row, 380, 42);
    lv_obj_set_pos(pwd_row, 55, 145);
    apply_input_row_style(pwd_row, 10);

    lv_obj_t * pwd_icon = lv_label_create(pwd_row);
    lv_label_set_text(pwd_icon, LV_SYMBOL_EYE_CLOSE);
    lv_obj_set_style_text_font(pwd_icon, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(pwd_icon, lv_color_hex(0x8990B2), 0);
    lv_obj_align(pwd_icon, LV_ALIGN_LEFT_MID, 0, 0);

    s_pwd_ta = lv_textarea_create(pwd_row);
    lv_obj_set_size(s_pwd_ta, 330, 32);
    lv_obj_align(s_pwd_ta, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_textarea_set_one_line(s_pwd_ta, true);
    lv_textarea_set_password_mode(s_pwd_ta, true);
    lv_textarea_set_placeholder_text(s_pwd_ta, "请输入wifi秘密");
    lv_textarea_set_max_length(s_pwd_ta, 64);
    lv_obj_set_style_bg_opa(s_pwd_ta, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_pwd_ta, 0, 0);
    lv_obj_set_style_outline_width(s_pwd_ta, 0, 0);
    lv_obj_set_style_text_color(s_pwd_ta, lv_color_hex(0x6E7092), 0);
    lv_obj_set_style_text_font(s_pwd_ta, &lv_font_montserrat_20, 0);
    lv_obj_set_style_pad_all(s_pwd_ta, 0, 0);
    lv_obj_add_event_cb(s_pwd_ta, input_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(s_pwd_ta, input_event_cb, LV_EVENT_FOCUSED, NULL);

    s_hint_label = lv_label_create(panel);
    lv_obj_set_width(s_hint_label, 380);
    lv_obj_set_pos(s_hint_label, 55, 198);
    lv_obj_set_style_text_color(s_hint_label, lv_color_hex(0xE7EAF6), 0);
    lv_obj_set_style_text_font(s_hint_label, &lv_font_source_han_sans_sc_16_cjk, 0);

    lv_obj_t * connect_btn = lv_btn_create(panel);
    lv_obj_set_size(connect_btn, 150, 42);
    lv_obj_set_pos(connect_btn, 55, 292);
    lv_obj_set_style_radius(connect_btn, 10, 0);
    lv_obj_set_style_bg_color(connect_btn, lv_color_hex(0xE9EAF2), 0);
    lv_obj_set_style_bg_opa(connect_btn, LV_OPA_90, 0);
    lv_obj_set_style_border_width(connect_btn, 0, 0);
    lv_obj_add_event_cb(connect_btn, connect_btn_event_cb, LV_EVENT_CLICKED, NULL);

    s_connect_btn_label = lv_label_create(connect_btn);
    lv_obj_set_style_text_color(s_connect_btn_label, lv_color_hex(0x6A6492), 0);
    lv_obj_set_style_text_font(s_connect_btn_label, &lv_font_source_han_sans_sc_16_cjk, 0);
    lv_obj_center(s_connect_btn_label);

    update_connect_view();

    lv_obj_t * home_btn = lv_btn_create(s_root);
    lv_obj_set_size(home_btn, 150, 150);
    lv_obj_set_pos(home_btn, 620, 56);
    apply_side_btn_style(home_btn, 24);
    lv_obj_add_event_cb(home_btn, home_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * home_icon = lv_label_create(home_btn);
    lv_label_set_text(home_icon, "\xEE\x98\xAE");
    lv_obj_set_style_text_font(home_icon, &setting1_back, 0);
    lv_obj_set_style_text_color(home_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(home_icon);

    lv_obj_t * back_btn = lv_btn_create(s_root);
    lv_obj_set_size(back_btn, 150, 150);
    lv_obj_set_pos(back_btn, 620, 297);
    apply_side_btn_style(back_btn, 24);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * back_icon = lv_label_create(back_btn);
    lv_label_set_text(back_icon, "\xEE\x98\x81");
    lv_obj_set_style_text_font(back_icon, &back_pos, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_icon);

    s_keyboard = lv_keyboard_create(s_root);
    lv_obj_set_size(s_keyboard, 800, 210);
    lv_obj_set_pos(s_keyboard, 0, 270);
    lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_mode(s_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_add_event_cb(s_keyboard, keyboard_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(s_keyboard, keyboard_event_cb, LV_EVENT_CANCEL, NULL);

    return s_root;
}

void wifi_set_set_back_cb(lv_event_cb_t cb)
{
    s_back_cb = cb;
}

void wifi_set_set_home_cb(lv_event_cb_t cb)
{
    s_home_cb = cb;
}

lv_obj_t * wifi_set_get_root(void)
{
    return s_root;
}

void wifi_set_destroy(void)
{
    if(s_root) {
        lv_obj_del(s_root);
        s_root = NULL;
    }

    s_ssid_ta = NULL;
    s_pwd_ta = NULL;
    s_keyboard = NULL;
    s_hint_label = NULL;
    s_connect_btn_label = NULL;
    s_connected = false;
}
