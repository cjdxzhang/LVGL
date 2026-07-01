#include "clk_light.h"
#include "top_status.h"
#include "ui_time.h"

LV_IMAGE_DECLARE(q0ULJ1);
LV_FONT_DECLARE(sleep_cn);
LV_FONT_DECLARE(setting1_back);
LV_FONT_DECLARE(back_pos);

#define PANEL_X 16
#define PANEL_Y 56
#define PANEL_W 570
#define PANEL_H 390

static lv_event_cb_t s_back_cb = NULL;
static lv_event_cb_t s_home_cb = NULL;

static void apply_panel_style(lv_obj_t * obj);
static void apply_side_btn_style(lv_obj_t * obj);
static lv_obj_t * create_switch_row(lv_obj_t * parent, lv_coord_t y, const char * text, bool checked);
static void update_time_label(lv_obj_t * label);
static void time_timer_cb(lv_timer_t * timer);
static void time_label_delete_cb(lv_event_t * e);
static void back_btn_event_cb(lv_event_t * e);
static void home_btn_event_cb(lv_event_t * e);

void clk_light_set_back_cb(lv_event_cb_t cb)
{
    s_back_cb = cb;
}

void clk_light_set_home_cb(lv_event_cb_t cb)
{
    s_home_cb = cb;
}

lv_obj_t * clk_light_create(lv_obj_t * parent)
{
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
    apply_panel_style(panel);
    lv_obj_set_style_pad_all(panel, 0, 0);

    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "\u58F0\u5149\u65F6\u949F");
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

    create_switch_row(panel, 92, "\u57FA\u7840\u6C1B\u56F4\u706F", true);
    create_switch_row(panel, 160, "\u8BED\u97F3\u64AD\u62A5", false);

    lv_obj_t * row3 = lv_obj_create(panel);
    lv_obj_set_size(row3, 430, 42);
    lv_obj_set_pos(row3, 58, 230);
    lv_obj_set_style_radius(row3, 10, 0);
    lv_obj_set_style_bg_color(row3, lv_color_hex(0xF4F1F8), 0);
    lv_obj_set_style_bg_opa(row3, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(row3, 0, 0);
    lv_obj_set_style_pad_hor(row3, 14, 0);
    lv_obj_set_style_pad_ver(row3, 0, 0);
    lv_obj_set_style_shadow_width(row3, 0, 0);
    lv_obj_clear_flag(row3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * label = lv_label_create(row3);
    lv_label_set_text(label, "\u65F6\u95F4\u540C\u6B65");
    lv_obj_set_style_text_font(label, &sleep_cn, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8E77A9), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * time_txt = lv_label_create(row3);
    lv_obj_set_style_text_font(time_txt, &sleep_cn, 0);
    lv_obj_set_style_text_color(time_txt, lv_color_hex(0x8E77A9), 0);
    lv_obj_align(time_txt, LV_ALIGN_RIGHT_MID, -40, 0);
    update_time_label(time_txt);
    lv_timer_t * time_timer = lv_timer_create(time_timer_cb, 1000, time_txt);
    lv_obj_add_event_cb(time_txt, time_label_delete_cb, LV_EVENT_DELETE, time_timer);

    lv_obj_t * sync_icon = lv_label_create(row3);
    lv_label_set_text(sync_icon, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_font(sync_icon, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(sync_icon, lv_color_hex(0x8E77A9), 0);
    lv_obj_align(sync_icon, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t * home_btn = lv_btn_create(root);
    lv_obj_set_size(home_btn, 150, 150);
    lv_obj_set_pos(home_btn, 620, 56);
    apply_side_btn_style(home_btn);
    lv_obj_add_event_cb(home_btn, home_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * home_icon = lv_label_create(home_btn);
    lv_label_set_text(home_icon, "\xEE\x98\xAE");
    lv_obj_set_style_text_font(home_icon, &setting1_back, 0);
    lv_obj_set_style_text_color(home_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(home_icon);

    lv_obj_t * back_btn = lv_btn_create(root);
    lv_obj_set_size(back_btn, 150, 150);
    lv_obj_set_pos(back_btn, 620, 297);
    apply_side_btn_style(back_btn);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * back_icon = lv_label_create(back_btn);
    lv_label_set_text(back_icon, "\xEE\x98\x81");
    lv_obj_set_style_text_font(back_icon, &back_pos, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_icon);

    return root;
}

static lv_obj_t * create_switch_row(lv_obj_t * parent, lv_coord_t y, const char * text, bool checked)
{
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_set_size(row, 430, 42);
    lv_obj_set_pos(row, 58, y);
    lv_obj_set_style_radius(row, 10, 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0xF4F1F8), 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_hor(row, 14, 0);
    lv_obj_set_style_pad_ver(row, 0, 0);
    lv_obj_set_style_shadow_width(row, 0, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * label = lv_label_create(row);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &sleep_cn, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8E77A9), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t * sw = lv_switch_create(row);
    if (checked) {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
    lv_obj_set_size(sw, 52, 28);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xC9BCD8), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x8E77A9), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, 0, 0);

    return row;
}

static void update_time_label(lv_obj_t * label)
{
    char text[UI_TIME_TEXT_LEN];

    if (label == NULL) {
        return;
    }
    if (!ui_time_format_now(text, sizeof(text))) {
        return;
    }
    lv_label_set_text(label, text);
}

static void time_timer_cb(lv_timer_t * timer)
{
    if (timer == NULL) {
        return;
    }
    update_time_label((lv_obj_t *)timer->user_data);
}

static void time_label_delete_cb(lv_event_t * e)
{
    lv_timer_t * timer = (lv_timer_t *)lv_event_get_user_data(e);

    if (timer == NULL) {
        return;
    }
    lv_timer_del(timer);
}

static void apply_panel_style(lv_obj_t * obj)
{
    lv_obj_set_style_radius(obj, 28, 0);
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

static void apply_side_btn_style(lv_obj_t * obj)
{
    lv_obj_set_style_radius(obj, 24, 0);
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
