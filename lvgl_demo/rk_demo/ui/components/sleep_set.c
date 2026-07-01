#include "sleep_set.h"
#include "top_status.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

LV_IMAGE_DECLARE(q0ULJ1);
LV_FONT_DECLARE(sleep_cn);
LV_FONT_DECLARE(setting1_back);
LV_FONT_DECLARE(back_pos);

#define PANEL_X        16
#define PANEL_Y        56
#define PANEL_W        570
#define PANEL_H        390

#define VIEW_X         34
#define VIEW_Y         66
#define VIEW_W         510
#define VIEW_H         340

#define THUMB_X        8
#define THUMB_Y        96
#define THUMB_W        4
#define THUMB_H        82
#define THUMB_TRAVEL   180

static lv_event_cb_t s_back_cb = NULL;
static lv_event_cb_t s_home_cb = NULL;
static lv_obj_t * s_setting_viewport = NULL;
static lv_obj_t * s_setting_thumb = NULL;
static lv_obj_t * s_dd_water_temp = NULL;
static lv_obj_t * s_dd_water_level = NULL;
static lv_obj_t * s_dd_bath_time_main = NULL;
static lv_obj_t * s_dd_bath_time_sub = NULL;
static lv_obj_t * s_dd_medicine = NULL;
static lv_obj_t * s_dd_location = NULL;

#define SLEEP_SET_CFG_MAGIC   0x53504C50u /* SPLP */
#define SLEEP_SET_CFG_VERSION 1u
#define SLEEP_SET_CFG_PATH    "sleep_set.cfg"

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint8_t water_temp;
    uint8_t water_level;
    uint8_t bath_time;
    uint8_t medicine;
    uint8_t location;
} sleep_set_cfg_t;

typedef enum {
    SLEEP_DD_WATER_TEMP = 0,
    SLEEP_DD_WATER_LEVEL,
    SLEEP_DD_BATH_TIME_MAIN,
    SLEEP_DD_BATH_TIME_SUB,
    SLEEP_DD_MEDICINE,
    SLEEP_DD_LOCATION
} sleep_dropdown_role_t;

typedef enum {
    SLEEP_ACTION_SAVE = 0,
    SLEEP_ACTION_RESTORE
} sleep_action_id_t;

static void apply_panel_style(lv_obj_t * obj, lv_coord_t radius);
static void apply_side_btn_style(lv_obj_t * obj, lv_coord_t radius);
static void apply_input_style(lv_obj_t * obj, lv_coord_t radius);
static void apply_action_btn_style(lv_obj_t * obj);
static lv_obj_t * create_page(lv_obj_t * parent, lv_coord_t y);
static lv_obj_t * create_row_label(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, const char * text);
static lv_obj_t * create_readonly_value(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, const char * text);
static lv_obj_t * create_dropdown_value(lv_obj_t * parent,
                                        lv_coord_t x,
                                        lv_coord_t y,
                                        const char * options,
                                        uint16_t selected_id,
                                        sleep_dropdown_role_t role);
static lv_obj_t * create_action_btn(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, const char * text);
static void dropdown_event_cb(lv_event_t * e);
static void action_btn_event_cb(lv_event_t * e);
static void create_setting_pages(lv_obj_t * panel);
static void create_page_thumb(lv_obj_t * panel);
static void update_setting_thumb_pos(void);
static void setting_viewport_scroll_cb(lv_event_t * e);
static void back_btn_event_cb(lv_event_t * e);
static void home_btn_event_cb(lv_event_t * e);
static sleep_set_cfg_t sleep_set_get_default_cfg(void);
static bool sleep_set_load_cfg(sleep_set_cfg_t * cfg);
static bool sleep_set_save_cfg(const sleep_set_cfg_t * cfg);
static void sleep_set_apply_cfg_to_ui(const sleep_set_cfg_t * cfg);
static void sleep_set_read_cfg_from_ui(sleep_set_cfg_t * cfg);
static void sleep_set_sync_bath_time(lv_obj_t * changed);

void sleep_set_set_back_cb(lv_event_cb_t cb)
{
    s_back_cb = cb;
}

void sleep_set_set_home_cb(lv_event_cb_t cb)
{
    s_home_cb = cb;
}

lv_obj_t * sleep_set_create(lv_obj_t * parent)
{
    s_setting_viewport = NULL;
    s_setting_thumb = NULL;
    s_dd_water_temp = NULL;
    s_dd_water_level = NULL;
    s_dd_bath_time_main = NULL;
    s_dd_bath_time_sub = NULL;
    s_dd_medicine = NULL;
    s_dd_location = NULL;

    lv_obj_t * root = lv_obj_create(parent);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_radius(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    status_bar_create(root);

    lv_obj_t * bg = lv_image_create(root);
    lv_image_set_src(bg, &q0ULJ1);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_move_background(bg);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * panel = lv_obj_create(root);
    lv_obj_set_size(panel, PANEL_W, PANEL_H);
    lv_obj_set_pos(panel, PANEL_X, PANEL_Y);
    apply_panel_style(panel, 28);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_set_style_clip_corner(panel, true, 0);

    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "\u52A9\u7720\u8BBE\u7F6E");
    lv_obj_set_style_text_font(title, &sleep_cn, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t * line = lv_obj_create(panel);
    lv_obj_set_size(line, 510, 2);
    lv_obj_set_pos(line, 30, 58);
    lv_obj_set_style_radius(line, 0, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_pad_all(line, 0, 0);
    lv_obj_set_style_bg_color(line, lv_color_hex(0xE6EEF8), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_70, 0);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);

    create_page_thumb(panel);
    create_setting_pages(panel);

    lv_obj_t * home_btn = lv_btn_create(root);
    lv_obj_set_size(home_btn, 150, 150);
    lv_obj_set_pos(home_btn, 620, 56);
    apply_side_btn_style(home_btn, 24);
    lv_obj_add_event_cb(home_btn, home_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * home_icon = lv_label_create(home_btn);
    lv_label_set_text(home_icon, "\xEE\x98\xAE");
    lv_obj_set_style_text_font(home_icon, &setting1_back, 0);
    lv_obj_set_style_text_color(home_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(home_icon);

    lv_obj_t * back_btn = lv_btn_create(root);
    lv_obj_set_size(back_btn, 150, 150);
    lv_obj_set_pos(back_btn, 620, 297);
    apply_side_btn_style(back_btn, 24);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * back_icon = lv_label_create(back_btn);
    lv_label_set_text(back_icon, "\xEE\x98\x81");
    lv_obj_set_style_text_font(back_icon, &back_pos, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_icon);

    return root;
}

static void create_setting_pages(lv_obj_t * panel)
{
    const lv_coord_t row1 = 30;
    const lv_coord_t row2 = 110;
    const lv_coord_t row3 = 190;
    const lv_coord_t row4 = 270;
    const lv_coord_t ctrl_ofs = -6;

    lv_obj_t * viewport = lv_obj_create(panel);
    lv_obj_set_size(viewport, VIEW_W, VIEW_H);
    lv_obj_set_pos(viewport, VIEW_X, VIEW_Y);
    lv_obj_set_style_radius(viewport, 0, 0);
    lv_obj_set_style_border_width(viewport, 0, 0);
    lv_obj_set_style_bg_opa(viewport, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(viewport, 0, 0);
    lv_obj_set_style_clip_corner(viewport, true, 0);
    lv_obj_add_flag(viewport, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(viewport, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(viewport, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(viewport, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(viewport, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_set_scroll_snap_y(viewport, LV_SCROLL_SNAP_START);
    lv_obj_add_event_cb(viewport, setting_viewport_scroll_cb, LV_EVENT_SCROLL, NULL);

    s_setting_viewport = viewport;

    lv_obj_t * page1 = create_page(viewport, 0);
    create_row_label(page1, 14, row1, "\u6A21\u5F0F\u540D\u79F0");
    create_row_label(page1, 14, row2, "\u6C34\u6E29\u8BBE\u7F6E");
    create_row_label(page1, 14, row3, "\u6C34\u4F4D\u4F4D\u7F6E");
    create_row_label(page1, 14, row4, "\u8DB3\u6D74\u65F6\u95F4");
    create_readonly_value(page1, 330, row1 + ctrl_ofs, "\u52A9\u7720");
    s_dd_water_temp = create_dropdown_value(page1, 330, row2 + ctrl_ofs, "35\u00B0\n36\u00B0\n37\u00B0\n38\u00B0\n39\u00B0\n40\u00B0", 0, SLEEP_DD_WATER_TEMP);
    s_dd_water_level = create_dropdown_value(page1, 330, row3 + ctrl_ofs, "1\u6863\n2\u6863\n3\u6863", 0, SLEEP_DD_WATER_LEVEL);
    s_dd_bath_time_main = create_dropdown_value(page1, 330, row4 + ctrl_ofs, "10min\n15min\n20min\n30min", 1, SLEEP_DD_BATH_TIME_MAIN);

    const lv_coord_t page_gap = 13;
    const lv_coord_t page2_y = row4 + 42 + page_gap;
    lv_obj_t * page2 = create_page(viewport, page2_y);
    create_row_label(page2, 14, row1, "\u8DB3\u6D74\u65F6\u95F4");
    create_row_label(page2, 14, row2, "\u836F\u6DB2\u8BBE\u7F6E");
    create_row_label(page2, 14, row3, "\u5B9A\u4F4D\u8BBE\u7F6E");
    s_dd_bath_time_sub = create_dropdown_value(page2, 330, row1 + ctrl_ofs, "10min\n15min\n20min\n30min", 1, SLEEP_DD_BATH_TIME_SUB);
    s_dd_medicine = create_dropdown_value(page2, 330, row2 + ctrl_ofs, "\u65E0\n\u6DF7\u5408\n\u836F\u6DB21\n\u836F\u6DB22", 0, SLEEP_DD_MEDICINE);
    s_dd_location = create_dropdown_value(page2, 330, row3 + ctrl_ofs, "\u5BA2\u5385\n\u5367\u5BA4\n\u4E66\u623F\n\u9633\u53F0", 0, SLEEP_DD_LOCATION);

    lv_obj_t * save_btn = create_action_btn(page2, 110, row4 - 4, "\u4FDD\u5B58");
    lv_obj_add_event_cb(save_btn, action_btn_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)SLEEP_ACTION_SAVE);
    lv_obj_t * restore_btn = create_action_btn(page2, 300, row4 - 4, "\u6062\u590D");
    lv_obj_add_event_cb(restore_btn, action_btn_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)SLEEP_ACTION_RESTORE);

    sleep_set_cfg_t cfg = sleep_set_get_default_cfg();
    (void)sleep_set_load_cfg(&cfg);
    sleep_set_apply_cfg_to_ui(&cfg);

    lv_obj_scroll_to_y(viewport, 0, LV_ANIM_OFF);
    update_setting_thumb_pos();
}

static lv_obj_t * create_page(lv_obj_t * parent, lv_coord_t y)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_set_size(page, VIEW_W, VIEW_H);
    lv_obj_set_pos(page, 0, y);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page, LV_OBJ_FLAG_SNAPPABLE);
    return page;
}

static void create_page_thumb(lv_obj_t * panel)
{
    lv_obj_t * track = lv_obj_create(panel);
    lv_obj_set_size(track, THUMB_W, THUMB_H + THUMB_TRAVEL);
    lv_obj_set_pos(track, THUMB_X, THUMB_Y);
    lv_obj_set_style_radius(track, 4, 0);
    lv_obj_set_style_border_width(track, 0, 0);
    lv_obj_set_style_bg_color(track, lv_color_hex(0xCBBBE6), 0);
    lv_obj_set_style_bg_opa(track, LV_OPA_30, 0);
    lv_obj_set_style_pad_all(track, 0, 0);
    lv_obj_clear_flag(track, LV_OBJ_FLAG_SCROLLABLE);

    s_setting_thumb = lv_obj_create(panel);
    lv_obj_set_size(s_setting_thumb, THUMB_W, THUMB_H);
    lv_obj_set_pos(s_setting_thumb, THUMB_X, THUMB_Y);
    lv_obj_set_style_radius(s_setting_thumb, 4, 0);
    lv_obj_set_style_border_width(s_setting_thumb, 0, 0);
    lv_obj_set_style_bg_color(s_setting_thumb, lv_color_hex(0xCBBBE6), 0);
    lv_obj_set_style_bg_opa(s_setting_thumb, LV_OPA_90, 0);
    lv_obj_set_style_pad_all(s_setting_thumb, 0, 0);
    lv_obj_clear_flag(s_setting_thumb, LV_OBJ_FLAG_SCROLLABLE);
}

static void update_setting_thumb_pos(void)
{
    if (s_setting_viewport == NULL || s_setting_thumb == NULL) {
        return;
    }

    lv_coord_t scroll_top = lv_obj_get_scroll_top(s_setting_viewport);
    lv_coord_t max_scroll = scroll_top + lv_obj_get_scroll_bottom(s_setting_viewport);
    lv_coord_t thumb_y = THUMB_Y;

    if (max_scroll > 0) {
        thumb_y = THUMB_Y + (lv_coord_t)((int32_t)scroll_top * THUMB_TRAVEL / max_scroll);
    }

    if (thumb_y < THUMB_Y) {
        thumb_y = THUMB_Y;
    }
    if (thumb_y > THUMB_Y + THUMB_TRAVEL) {
        thumb_y = THUMB_Y + THUMB_TRAVEL;
    }

    lv_obj_set_y(s_setting_thumb, thumb_y);
}

static void setting_viewport_scroll_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    update_setting_thumb_pos();
}

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

static void apply_input_style(lv_obj_t * obj, lv_coord_t radius)
{
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xF4F1F8), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_100, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_pad_hor(obj, 14, 0);
    lv_obj_set_style_text_font(obj, &sleep_cn, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(0x8E77A9), 0);
}

static void apply_action_btn_style(lv_obj_t * obj)
{
    apply_input_style(obj, 10);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t * create_row_label(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, const char * text)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &sleep_cn, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xEAF1FA), 0);
    lv_obj_set_pos(label, x, y);
    return label;
}

static lv_obj_t * create_readonly_value(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, const char * text)
{
    lv_obj_t * box = lv_obj_create(parent);
    lv_obj_set_size(box, 150, 42);
    lv_obj_set_pos(box, x, y);
    apply_input_style(box, 10);
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * value = lv_label_create(box);
    lv_label_set_text(value, text);
    lv_obj_set_style_text_font(value, &sleep_cn, 0);
    lv_obj_set_style_text_color(value, lv_color_hex(0x8E77A9), 0);
    lv_obj_center(value);

    return box;
}

static lv_obj_t * create_dropdown_value(lv_obj_t * parent,
                                        lv_coord_t x,
                                        lv_coord_t y,
                                        const char * options,
                                        uint16_t selected_id,
                                        sleep_dropdown_role_t role)
{
    lv_obj_t * dd = lv_dropdown_create(parent);
    lv_obj_set_size(dd, 150, 42);
    lv_obj_set_pos(dd, x, y);
    apply_input_style(dd, 10);
    lv_dropdown_set_options(dd, options);
    lv_dropdown_set_selected(dd, selected_id);
    lv_dropdown_set_symbol(dd, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_font(dd, &sleep_cn, LV_PART_MAIN);
    lv_obj_set_style_text_color(dd, lv_color_hex(0x8E77A9), LV_PART_MAIN);
    lv_obj_set_style_text_font(dd, &lv_font_montserrat_16, LV_PART_INDICATOR);
    lv_obj_add_event_cb(dd, dropdown_event_cb, LV_EVENT_ALL, (void *)(uintptr_t)role);

    return dd;
}

static void dropdown_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_VALUE_CHANGED && code != LV_EVENT_READY) {
        return;
    }

    lv_obj_t * dd = lv_event_get_target(e);
    sleep_dropdown_role_t role = (sleep_dropdown_role_t)(uintptr_t)lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED && 
         (role == SLEEP_DD_BATH_TIME_MAIN || role == SLEEP_DD_BATH_TIME_SUB)) {
        sleep_set_sync_bath_time(dd);
    }

    lv_obj_t * list = lv_dropdown_get_list(dd);
    if (list == NULL) {
        return;
    }

    lv_obj_set_style_text_font(list, &sleep_cn, LV_PART_MAIN);
    lv_obj_set_style_text_color(list, lv_color_hex(0x8E77A9), LV_PART_MAIN);
    lv_obj_set_style_text_font(list, &sleep_cn, LV_PART_SELECTED);
}

static void action_btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    sleep_action_id_t action_id = (sleep_action_id_t)(uintptr_t)lv_event_get_user_data(e);

    if (action_id == SLEEP_ACTION_SAVE) {
        sleep_set_cfg_t cfg = sleep_set_get_default_cfg();
        sleep_set_read_cfg_from_ui(&cfg);
        (void)sleep_set_save_cfg(&cfg);
        return;
    }

    if (action_id == SLEEP_ACTION_RESTORE) {
        sleep_set_cfg_t cfg = sleep_set_get_default_cfg();
        sleep_set_apply_cfg_to_ui(&cfg);
        (void)sleep_set_save_cfg(&cfg);
    }
}

static lv_obj_t * create_action_btn(lv_obj_t * parent, lv_coord_t x, lv_coord_t y, const char * text)
{
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 150, 42);
    lv_obj_set_pos(btn, x, y);
    apply_action_btn_style(btn);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &sleep_cn, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8E77A9), 0);
    lv_obj_center(label);

    return btn;
}

static void back_btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    if (s_back_cb) {
        s_back_cb(e);
    }
}

static void home_btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    if (s_home_cb) {
        s_home_cb(e);
    }
}

static sleep_set_cfg_t sleep_set_get_default_cfg(void)
{
    sleep_set_cfg_t cfg;
    cfg.magic = SLEEP_SET_CFG_MAGIC;
    cfg.version = SLEEP_SET_CFG_VERSION;
    cfg.water_temp = 0;
    cfg.water_level = 0;
    cfg.bath_time = 1;
    cfg.medicine = 0;
    cfg.location = 0;
    return cfg;
}

static bool sleep_set_load_cfg(sleep_set_cfg_t * cfg)
{
    FILE * fp = fopen(SLEEP_SET_CFG_PATH, "rb");
    if (fp == NULL) {
        return false;
    }

    sleep_set_cfg_t file_cfg;
    size_t nread = fread(&file_cfg, sizeof(file_cfg), 1, fp);
    fclose(fp);
    if (nread != 1) {
        return false;
    }

    if (file_cfg.magic != SLEEP_SET_CFG_MAGIC || file_cfg.version != SLEEP_SET_CFG_VERSION) {
        return false;
    }

    *cfg = file_cfg;
    return true;
}

static bool sleep_set_save_cfg(const sleep_set_cfg_t * cfg)
{
    FILE * fp = fopen(SLEEP_SET_CFG_PATH, "wb");
    if (fp == NULL) {
        return false;
    }

    size_t nwritten = fwrite(cfg, sizeof(*cfg), 1, fp);
    fclose(fp);
    return nwritten == 1;
}

static uint16_t sleep_set_limit_dropdown_value(lv_obj_t * dd, uint8_t value)
{
    uint16_t option_cnt = lv_dropdown_get_option_count(dd);
    if (option_cnt == 0) {
        return 0;
    }

    if (value >= option_cnt) {
        return (uint16_t)(option_cnt - 1);
    }

    return value;
}

static void sleep_set_apply_cfg_to_ui(const sleep_set_cfg_t * cfg)
{
    if (s_dd_water_temp) {
        lv_dropdown_set_selected(s_dd_water_temp, sleep_set_limit_dropdown_value(s_dd_water_temp, cfg->water_temp));
    }
    if (s_dd_water_level) {
        lv_dropdown_set_selected(s_dd_water_level, sleep_set_limit_dropdown_value(s_dd_water_level, cfg->water_level));
    }
    if (s_dd_bath_time_main) {
        lv_dropdown_set_selected(s_dd_bath_time_main, sleep_set_limit_dropdown_value(s_dd_bath_time_main, cfg->bath_time));
    }
    if (s_dd_bath_time_sub) {
        lv_dropdown_set_selected(s_dd_bath_time_sub, sleep_set_limit_dropdown_value(s_dd_bath_time_sub, cfg->bath_time));
    }
    if (s_dd_medicine) {
        lv_dropdown_set_selected(s_dd_medicine, sleep_set_limit_dropdown_value(s_dd_medicine, cfg->medicine));
    }
    if (s_dd_location) {
        lv_dropdown_set_selected(s_dd_location, sleep_set_limit_dropdown_value(s_dd_location, cfg->location));
    }
}

static void sleep_set_read_cfg_from_ui(sleep_set_cfg_t * cfg)
{
    cfg->magic = SLEEP_SET_CFG_MAGIC;
    cfg->version = SLEEP_SET_CFG_VERSION;
    if (s_dd_water_temp) {
        cfg->water_temp = (uint8_t)lv_dropdown_get_selected(s_dd_water_temp);
    }
    if (s_dd_water_level) {
        cfg->water_level = (uint8_t)lv_dropdown_get_selected(s_dd_water_level);
    }
    if (s_dd_bath_time_main) {
        cfg->bath_time = (uint8_t)lv_dropdown_get_selected(s_dd_bath_time_main);
    }
    if (s_dd_medicine) {
        cfg->medicine = (uint8_t)lv_dropdown_get_selected(s_dd_medicine);
    }
    if (s_dd_location) {
        cfg->location = (uint8_t)lv_dropdown_get_selected(s_dd_location);
    }
}

static void sleep_set_sync_bath_time(lv_obj_t * changed)
{
    uint16_t selected = lv_dropdown_get_selected(changed);

    if (changed == s_dd_bath_time_main && s_dd_bath_time_sub) {
        lv_dropdown_set_selected(s_dd_bath_time_sub, selected);
    } else if (changed == s_dd_bath_time_sub && s_dd_bath_time_main) {
        lv_dropdown_set_selected(s_dd_bath_time_main, selected);
    }
}
