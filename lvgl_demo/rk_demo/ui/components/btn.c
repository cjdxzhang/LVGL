#include "btn.h"
#include <stdint.h>

LV_FONT_DECLARE(font_cn_28);
LV_FONT_DECLARE(font_cn_28_2);
LV_FONT_DECLARE(control);
LV_FONT_DECLARE(control1);
LV_FONT_DECLARE(mode);
LV_FONT_DECLARE(mode1);
LV_FONT_DECLARE(mode2);
LV_FONT_DECLARE(mode3);

static btn_event_cb_t s_btn_event_cb = NULL;

static void btn_click_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    if(s_btn_event_cb == NULL) {
        return;
    }

    btn_event_id_t id = (btn_event_id_t)(intptr_t)lv_event_get_user_data(e);
    s_btn_event_cb(id);
}

void btn_set_event_callback(btn_event_cb_t cb)
{
    s_btn_event_cb = cb;
}

static void btn_style_shadow(lv_obj_t * obj)
{
    lv_obj_set_style_shadow_width(obj, 18, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_20, 0);
    lv_obj_set_style_shadow_spread(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 6, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x050B16), 0);
}

static void btn_style_frosted_base(lv_obj_t * obj, lv_coord_t radius)
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
    btn_style_shadow(obj);
}

static lv_obj_t * create_mode_btn(lv_obj_t * parent, int w, int h,
                                  const lv_font_t * icon_font,
                                  const lv_font_t * title_font,
                                  const char * icon_txt,
                                  const char * title_txt)
{
    lv_obj_t * card = lv_btn_create(parent);
    lv_obj_set_size(card, w, h);

    btn_style_frosted_base(card, 50);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * icon = lv_label_create(card);
    lv_label_set_text(icon, icon_txt);
    lv_obj_set_style_text_font(icon, icon_font, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, title_txt);
    lv_obj_set_style_text_font(title, title_font, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -16);

    return card;
}

static lv_obj_t * create_small_ctrl_btn(lv_obj_t * parent, int x, int y, const char * txt)
{
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 94, 84);
    lv_obj_set_pos(btn, x, y);

    btn_style_frosted_base(btn, 35);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_font(label, &control, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    return btn;
}

void btn_create_left_group(lv_obj_t * parent)
{
    /* ===== 左下大播放按钮 ===== */
    lv_obj_t * play_btn = lv_btn_create(parent);
    lv_obj_set_size(play_btn, 135, 201);
    lv_obj_set_pos(play_btn, 15, 245);

    btn_style_frosted_base(play_btn, 42);
    lv_obj_clear_flag(play_btn, LV_OBJ_FLAG_SCROLLABLE);

   lv_obj_t * play_label = lv_label_create(play_btn);
    lv_label_set_text(play_label, "\xEE\x98\x96");   /* U+E616 */
    lv_obj_set_style_text_font(play_label, &control1, 0);
    lv_obj_set_style_text_color(play_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(play_label, LV_ALIGN_CENTER, 3, 0);
    lv_obj_add_event_cb(play_btn, btn_click_event_cb, LV_EVENT_CLICKED,
                   (void *)(intptr_t)BTN_EVENT_START);

    /* ===== 中间四个控制按钮 ===== */
    lv_obj_t * btn_plus  = create_small_ctrl_btn(parent, 175, 232, "\xEE\x98\x80");  /* U+E600 */
    lv_obj_t * btn_minus = create_small_ctrl_btn(parent, 310, 232, "\xEE\x98\xAB");  /* U+E62B */
    lv_obj_t * btn_time  = create_small_ctrl_btn(parent, 175, 352, "\xEE\x99\xA0");  /* U+E660 */
    lv_obj_t * btn_water = create_small_ctrl_btn(parent, 310, 352, "\xEE\x9A\xAF");  /* U+E6AF */

    lv_obj_set_size(btn_plus, 100, 100);
    lv_obj_set_size(btn_minus, 100, 100);
    lv_obj_set_size(btn_time, 100, 100);
    lv_obj_set_size(btn_water, 100, 100);
}

void btn_create_right_page1(lv_obj_t * parent)
{
    /* ===== 右侧四功能按钮：第一页 ===== */
    lv_obj_t * card_sleep = create_mode_btn(parent, 160, 160, &mode, &font_cn_28, "\xEE\x9B\x8F", "助眠");
    lv_obj_set_pos(card_sleep, 0, 0);
    lv_obj_add_event_cb(card_sleep, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_MODE_SLEEP);

    lv_obj_t * card_relax = lv_btn_create(parent);
    lv_obj_set_size(card_relax, 160, 160);
    lv_obj_set_pos(card_relax, 186, 0);
    lv_obj_add_event_cb(card_relax, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_MODE_RELAX);

    btn_style_frosted_base(card_relax, 50);
    lv_obj_set_style_pad_all(card_relax, 0, 0);
    lv_obj_clear_flag(card_relax, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * relax_icon = lv_label_create(card_relax);
    lv_label_set_text(relax_icon, "\xEE\x98\x80");
    lv_obj_set_style_text_font(relax_icon, &mode1, 0);
    lv_obj_set_style_text_color(relax_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(relax_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(relax_icon, LV_ALIGN_TOP_MID, 0, 45);

    lv_obj_t * relax_title = lv_label_create(card_relax);
    lv_label_set_text(relax_title, "放松");
    lv_obj_set_style_text_font(relax_title, &font_cn_28, 0);
    lv_obj_set_style_text_color(relax_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(relax_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(relax_title, LV_ALIGN_BOTTOM_MID, 0, -16);

    lv_obj_t * card_warm = lv_btn_create(parent);
    lv_obj_set_size(card_warm, 160, 160);
    lv_obj_set_pos(card_warm, 0, 205);

    btn_style_frosted_base(card_warm, 50);
    lv_obj_set_style_pad_all(card_warm, 0, 0);
    lv_obj_clear_flag(card_warm, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * warm_icon = lv_label_create(card_warm);
    lv_label_set_text(warm_icon, "\xEE\xA3\x86");
    lv_obj_set_style_text_font(warm_icon, &mode2, 0);
    lv_obj_set_style_text_color(warm_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(warm_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(warm_icon, LV_ALIGN_TOP_MID, 0, 25);

    lv_obj_t * warm_title = lv_label_create(card_warm);
    lv_label_set_text(warm_title, "驱寒");
    lv_obj_set_style_text_font(warm_title, &font_cn_28, 0);
    lv_obj_set_style_text_color(warm_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(warm_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(warm_title, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(card_warm, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_MODE_WARM);

    lv_obj_t * card_clean = lv_btn_create(parent);
    lv_obj_set_size(card_clean, 160, 160);
    lv_obj_set_pos(card_clean, 186, 205);

    btn_style_frosted_base(card_clean, 50);
    lv_obj_set_style_pad_all(card_clean, 0, 0);
    lv_obj_clear_flag(card_clean, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * clean_icon = lv_label_create(card_clean);
    lv_label_set_text(clean_icon, "\xEE\x99\xBE");
    lv_obj_set_style_text_font(clean_icon, &mode, 0);
    lv_obj_set_style_text_color(clean_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(clean_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(clean_icon, LV_ALIGN_TOP_MID, 0, 25);

    lv_obj_t * clean_title = lv_label_create(card_clean);
    lv_label_set_text(clean_title, "自清洁");
    lv_obj_set_style_text_font(clean_title, &font_cn_28, 0);
    lv_obj_set_style_text_color(clean_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(clean_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(clean_title, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(card_clean, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_MODE_CLEAN);
}

void btn_create_right_page2(lv_obj_t * parent)
{
    /* 第二页四个功能按钮 */

    lv_obj_t * card_bath1 = create_mode_btn(parent, 160, 160, &mode3, &font_cn_28_2, "\xEE\x98\x85", "药浴1");
    lv_obj_set_pos(card_bath1, 0, 0);
    lv_obj_add_event_cb(card_bath1, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_BATH1);

    lv_obj_t * card_bath2 = lv_btn_create(parent);
    lv_obj_set_size(card_bath2, 160, 160);
    lv_obj_set_pos(card_bath2, 186, 0);

    btn_style_frosted_base(card_bath2, 50);
    lv_obj_set_style_pad_all(card_bath2, 0, 0);
    lv_obj_clear_flag(card_bath2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bath2_icon = lv_label_create(card_bath2);
    lv_label_set_text(bath2_icon, "\xEE\x98\x85");
    lv_obj_set_style_text_font(bath2_icon, &mode3, 0);
    lv_obj_align(bath2_icon, LV_ALIGN_TOP_MID, 0, 18);
    lv_obj_set_style_text_color(bath2_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(bath2_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(bath2_icon, LV_ALIGN_TOP_MID, 0, 25);

    lv_obj_t * bath2_title = lv_label_create(card_bath2);
    lv_label_set_text(bath2_title, "药浴2");
    lv_obj_set_style_text_font(bath2_title, &font_cn_28_2, 0);
    lv_obj_set_style_text_color(bath2_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(bath2_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(bath2_title, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(card_bath2, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_BATH2);

    lv_obj_t * card_locate = lv_btn_create(parent);
    lv_obj_set_size(card_locate, 160, 160);
    lv_obj_set_pos(card_locate, 0, 205);

    btn_style_frosted_base(card_locate, 50);
    lv_obj_set_style_pad_all(card_locate, 0, 0);
    lv_obj_clear_flag(card_locate, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * locate_icon = lv_label_create(card_locate);
    lv_label_set_text(locate_icon, "\xEE\x98\xA9");
    lv_obj_set_style_text_font(locate_icon, &mode3, 0);
    lv_obj_align(locate_icon, LV_ALIGN_TOP_MID, 0, 28);
    lv_obj_set_style_text_color(locate_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(locate_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(locate_icon, LV_ALIGN_TOP_MID, 0, 25);

    lv_obj_t * locate_title = lv_label_create(card_locate);
    lv_label_set_text(locate_title, "定位");
    lv_obj_set_style_text_font(locate_title, &font_cn_28_2, 0);
    lv_obj_set_style_text_color(locate_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(locate_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(locate_title, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(card_locate, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_LOCATE);

    lv_obj_t * card_setting = lv_btn_create(parent);
    lv_obj_set_size(card_setting, 160, 160);
    lv_obj_set_pos(card_setting, 186, 205);

    btn_style_frosted_base(card_setting, 50);
    lv_obj_set_style_pad_all(card_setting, 0, 0);
    lv_obj_clear_flag(card_setting, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * setting_icon = lv_label_create(card_setting);
    lv_label_set_text(setting_icon, "\xEE\x98\x9B");
    lv_obj_set_style_text_font(setting_icon, &mode3, 0);
    lv_obj_align(setting_icon, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_set_style_text_color(setting_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(setting_icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(setting_icon, LV_ALIGN_TOP_MID, 0, 25);

    lv_obj_t * setting_title = lv_label_create(card_setting);
    lv_label_set_text(setting_title, "设置");
    lv_obj_set_style_text_font(setting_title, &font_cn_28_2, 0);
    lv_obj_set_style_text_color(setting_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(setting_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(setting_title, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_add_event_cb(card_setting, btn_click_event_cb, LV_EVENT_CLICKED,
                       (void *)(intptr_t)BTN_EVENT_SETTING);
}
