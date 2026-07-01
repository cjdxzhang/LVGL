#include "page_location.h"
#include <stdio.h>
#include <stdint.h>
#include "top_status.h"
#include "gateway_client.h"

LV_FONT_DECLARE(clean_cn);
LV_FONT_DECLARE(back_btn);
LV_FONT_DECLARE(location);
LV_FONT_DECLARE(position_cn);
LV_FONT_DECLARE(start_pos_stop);
LV_FONT_DECLARE(back_pos);
LV_FONT_DECLARE(alarm_pos);
LV_FONT_DECLARE(st_ps);
LV_FONT_DECLARE(moving_cn);
LV_FONT_DECLARE(mov_sta_cn);
LV_FONT_DECLARE(mov_sta_cn_1);

typedef enum {
    LOCATION_LIVING_ROOM = 0,
    LOCATION_BEDROOM,
    LOCATION_STUDY,
    LOCATION_BALCONY,
    LOCATION_COUNT
} location_id_t;

static void (*s_show_page_cb)(lv_obj_t * page) = NULL;
static lv_obj_t * s_page_main = NULL;
static lv_obj_t * s_page_location_main = NULL;
static lv_obj_t * s_card_icon = NULL;
static lv_obj_t * s_label_target = NULL;
static lv_obj_t * s_page_location_moving = NULL;
static lv_obj_t * s_page_location_pause_confirm = NULL;

static lv_obj_t * s_btn_location[LOCATION_COUNT] = {0};
static location_id_t s_selected_location = LOCATION_LIVING_ROOM;
static lv_obj_t * create_location_moving_page(lv_obj_t * parent);
static void emergency_stop_btn_event_cb(lv_event_t * e);
static void page_location_switch(lv_obj_t * from, lv_obj_t * to);
static lv_obj_t * create_location_pause_confirm_page(lv_obj_t * parent);
static void moving_pause_yes_event_cb(lv_event_t * e);
static void moving_pause_no_event_cb(lv_event_t * e);

static const char * s_location_names[LOCATION_COUNT] = {
    "客厅",
    "卧室",
    "书房",
    "阳台"
};

static const gateway_location_id_t s_gateway_location_ids[LOCATION_COUNT] = {
    GATEWAY_LOCATION_LIVING_ROOM,
    GATEWAY_LOCATION_BEDROOM,
    GATEWAY_LOCATION_STUDY,
    GATEWAY_LOCATION_BALCONY
};

static const char * get_location_icon(location_id_t id)
{
    switch (id) {
        case LOCATION_LIVING_ROOM:
            return "\xEE\x99\x8C";  /* U+E601 */
        case LOCATION_BEDROOM:
            return "\xEE\x98\xA1";  /* U+E606 */
        case LOCATION_STUDY:
            return "\xEE\x98\x86";  /* U+E621 */
        case LOCATION_BALCONY:
            return "\xEE\x98\x81";  /* U+E64C */
        default:
            return "\xEE\x99\x8C";
    }
}

static void apply_glass_btn_style(lv_obj_t * obj)
{
    lv_obj_set_style_radius(obj, 30, 0);

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

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void update_location_card(location_id_t id)
{
    static char buf[32];
    lv_snprintf(buf, sizeof(buf), "移动至%s", s_location_names[id]);

    if (s_label_target) {
        lv_label_set_text(s_label_target, buf);
    }

    if (s_card_icon) {
        lv_label_set_text(s_card_icon, get_location_icon(id));
    }
}

static void update_location_btn_state(location_id_t selected)
{
    for(int i = 0; i < LOCATION_COUNT; i++)
    {
        if (s_btn_location[i] == NULL) {
            continue;
        }

        if (i == selected) {
            lv_obj_set_style_bg_opa(s_btn_location[i], LV_OPA_50, 0);
            lv_obj_set_style_border_width(s_btn_location[i], 2, 0);
            lv_obj_set_style_border_color(s_btn_location[i], lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_border_opa(s_btn_location[i], LV_OPA_70, 0);
        } else {
            lv_obj_set_style_bg_opa(s_btn_location[i], LV_OPA_30, 0);
            lv_obj_set_style_border_width(s_btn_location[i], 1, 0);
            lv_obj_set_style_border_color(s_btn_location[i], lv_color_hex(0xE6ECF5), 0);
            lv_obj_set_style_border_opa(s_btn_location[i], LV_OPA_40, 0);
        }
    }
}

static void location_btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    location_id_t id = (location_id_t)(uintptr_t)lv_event_get_user_data(e);
    s_selected_location = id;

    update_location_card(s_selected_location);
    update_location_btn_state(s_selected_location);
}

static void start_move_btn_event_cb(lv_event_t * e)
{
    int ret;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    LV_LOG_USER("start move to %s", s_location_names[s_selected_location]);
    ret = gateway_client_go_location(s_gateway_location_ids[s_selected_location]);
    if (ret != GATEWAY_CLIENT_OK) {
        LV_LOG_WARN("[LOCATION] send go location failed: id=%d ret=%d",
                    (int)s_selected_location, ret);
        return;
    }

    page_location_switch(s_page_location_main, s_page_location_moving);
}

static void back_btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    if (s_show_page_cb) {
        s_show_page_cb(s_page_main);
    }
}

static lv_obj_t * create_location_btn(lv_obj_t * parent, const char * text, int x, int y, location_id_t id)
{
    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 168, 168);
    lv_obj_set_pos(btn, x, y);
    apply_glass_btn_style(btn);

    lv_obj_add_event_cb(btn, location_btn_event_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)id);

    lv_obj_t * icon = lv_label_create(btn);
    lv_label_set_text(icon, get_location_icon(id));
    lv_obj_set_style_text_font(icon, &location, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 35);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &position_cn, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -15);

    s_btn_location[id] = btn;
    return btn;
}

static lv_obj_t * create_base_page_with_status_bar(lv_obj_t * parent)
{
    // 1. 创建基础页面
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_set_size(page, 800, 480);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0); // 保持透明背景，让状态栏看起来是浮在上面的
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);
    // 2. 在此页面上创建状态栏
    status_bar_create(page);

    return page;
}

page_location_set_t page_location_create(lv_obj_t * parent,
                                         void (*show_page_cb)(lv_obj_t * page),
                                         lv_obj_t * page_main)
                                         {
    page_location_set_t set = {0};

    s_show_page_cb = show_page_cb;
    s_page_main = page_main;    

    s_page_location_main = create_base_page_with_status_bar(parent);

    /* 左上目标卡片 */
    lv_obj_t * card = lv_obj_create(s_page_location_main);
    lv_obj_set_size(card, 380, 180);
    lv_obj_set_pos(card, 20, 40);
    apply_glass_btn_style(card);

    s_card_icon = lv_label_create(card);
    lv_label_set_text(s_card_icon, get_location_icon(s_selected_location));
    lv_obj_set_style_text_font(s_card_icon, &location, 0);
    lv_obj_set_style_text_color(s_card_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(s_card_icon, LV_ALIGN_LEFT_MID, 50, 0);

    s_label_target = lv_label_create(card);
    lv_obj_set_style_text_font(s_label_target, &position_cn, 0);
    lv_obj_set_style_text_color(s_label_target, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(s_label_target, LV_ALIGN_LEFT_MID, 130, 0);

    /* 右侧四个位置按钮 */
    create_location_btn(s_page_location_main, "客厅", 430, 40,  LOCATION_LIVING_ROOM);
    create_location_btn(s_page_location_main, "卧室", 610, 40,  LOCATION_BEDROOM);
    create_location_btn(s_page_location_main, "书房", 430, 248, LOCATION_STUDY);
    create_location_btn(s_page_location_main, "阳台", 610, 248, LOCATION_BALCONY);

    /* 左下启动按钮 */
    lv_obj_t * btn_start = lv_btn_create(s_page_location_main);
    lv_obj_set_size(btn_start, 150, 185);
    lv_obj_set_pos(btn_start, 45, 240);
    apply_glass_btn_style(btn_start);
    lv_obj_add_event_cb(btn_start, start_move_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label_start = lv_label_create(btn_start);
    lv_label_set_text(label_start, "\xEE\x9A\xA4");   /* U+E6A4 */
    lv_obj_set_style_text_font(label_start, &start_pos_stop, 0);
    lv_obj_set_style_text_color(label_start, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_start, LV_ALIGN_CENTER, 0, 0);

    /* 左下返回按钮 */
    lv_obj_t * btn_back = lv_btn_create(s_page_location_main);
    lv_obj_set_size(btn_back, 150, 185);
    lv_obj_set_pos(btn_back, 214, 240);
    apply_glass_btn_style(btn_back);
    lv_obj_add_event_cb(btn_back, back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "\xEE\x98\x81");
    lv_obj_set_style_text_font(label_back, &back_pos, 0);
    lv_obj_set_style_text_color(label_back, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label_back);
    
    s_page_location_moving = create_location_moving_page(parent);
    s_page_location_pause_confirm = create_location_pause_confirm_page(parent);
    update_location_card(s_selected_location);
    update_location_btn_state(s_selected_location);

    set.page_location_main = s_page_location_main;
    return set;
}

static void emergency_stop_btn_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    /* 这里后续可接暂停确认页
         现在先回到定位选择页，方便你先验证流程 */
    page_location_switch(s_page_location_moving, s_page_location_pause_confirm);
}

static lv_obj_t * create_location_moving_page(lv_obj_t * parent)
{
    lv_obj_t * page = create_base_page_with_status_bar(parent);
    // 注意：这里不再设置背景色和不透明度，因为基础页面已经是透明的。
    // 如果您希望移动页面有特定的背景，可以给page再加一个子对象作为背景层，或者直接修改基础页面的背景。
    // 为了简单起见，我们可以直接设置page的背景：
    lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_10, 0);
    //lv_obj_t * page = lv_obj_create(parent);
    //lv_obj_set_size(page, 800, 480);
    //lv_obj_set_pos(page, 0, 0);
    //lv_obj_set_style_radius(page, 0, 0);
    //lv_obj_set_style_border_width(page, 0, 0);
    //lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    //lv_obj_set_style_bg_opa(page, LV_OPA_10, 0);
    //lv_obj_set_style_pad_all(page, 0, 0);
    //lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);

    /* 左侧大提示卡片 */
    lv_obj_t * card = lv_obj_create(page);

    lv_obj_set_size(card, 525, 390);
    lv_obj_set_pos(card, 25, 60);
    apply_glass_btn_style(card);

    /* 警告图标 */
    lv_obj_t * warn_icon = lv_label_create(card);
    lv_label_set_text(warn_icon, "\xEE\x98\xA9");   /* U+E629 */
    lv_obj_set_style_text_font(warn_icon, &alarm_pos, 0);
    lv_obj_set_style_text_color(warn_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(warn_icon, LV_ALIGN_LEFT_MID, 45, 0);

    /* 提示文字 */
    lv_obj_t * label_tip = lv_label_create(card);
    lv_label_set_text(label_tip, "移动中\n请注意避让\n请勿操作");
    lv_obj_set_style_text_font(label_tip, &moving_cn, 0);
    lv_obj_set_style_text_color(label_tip, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_line_space(label_tip, 14, 0);
    lv_obj_align(label_tip, LV_ALIGN_LEFT_MID, 210, 0);

    /* 右下暂停按钮 */
    lv_obj_t * btn_stop = lv_btn_create(page);
    lv_obj_set_size(btn_stop, 175, 175);
    lv_obj_set_pos(btn_stop, 590, 275);
    apply_glass_btn_style(btn_stop);
    lv_obj_add_event_cb(btn_stop, emergency_stop_btn_event_cb, LV_EVENT_CLICKED, NULL);

    /* 暂停图标 */
    lv_obj_t * stop_icon = lv_label_create(btn_stop);
    lv_label_set_text(stop_icon, "\xEE\x99\x9D");   /* U+E65D */
    lv_obj_set_style_text_font(stop_icon, &st_ps, 0);
    lv_obj_set_style_text_color(stop_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(stop_icon, LV_ALIGN_TOP_MID, 0, 28);

    /* 按钮文字 */

    lv_obj_t * stop_text = lv_label_create(btn_stop);
    lv_label_set_text(stop_text, "紧急停止");
    lv_obj_set_style_text_font(stop_text, &position_cn, 0);
    lv_obj_set_style_text_color(stop_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(stop_text, LV_ALIGN_BOTTOM_MID, 0, -18);

    return page;
}

static void apply_back_icon_style(lv_obj_t * icon)
{
    lv_obj_set_style_text_font(icon, &back_btn, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
}

static void page_location_switch(lv_obj_t * from, lv_obj_t * to)
{
    if (from) {
        lv_obj_add_flag(from, LV_OBJ_FLAG_HIDDEN);
    }
    if (to) {
        lv_obj_clear_flag(to, LV_OBJ_FLAG_HIDDEN);
    }
}

void page_location_hide_all(void)
{
    if (s_page_location_main) {
        lv_obj_add_flag(s_page_location_main, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_page_location_moving) {
        lv_obj_add_flag(s_page_location_moving, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_page_location_pause_confirm) {
        lv_obj_add_flag(s_page_location_pause_confirm, LV_OBJ_FLAG_HIDDEN);
    }
}

static void moving_pause_yes_event_cb(lv_event_t * e)
{
    int ret;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    ret = gateway_client_clear_navigation();
    if (ret != GATEWAY_CLIENT_OK) {
        LV_LOG_WARN("[LOCATION] clear navigation failed: ret=%d", ret);
    }

    /* 确认停止：先回定位选择页 */
    page_location_switch(s_page_location_pause_confirm, s_page_location_main);
}

static void moving_pause_no_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    /* 取消停止：回到正在移动页 */
    page_location_switch(s_page_location_pause_confirm, s_page_location_moving);
}

static lv_obj_t * create_location_pause_confirm_page(lv_obj_t * parent)
{
    //lv_obj_t * page = lv_obj_create(parent);
    //lv_obj_set_size(page, 800, 480);
    //lv_obj_set_pos(page, 0, 0);
    //lv_obj_set_style_radius(page, 0, 0);
    //lv_obj_set_style_border_width(page, 0, 0);
    //lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    //lv_obj_set_style_bg_opa(page, LV_OPA_60, 0);
    //lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * page = create_base_page_with_status_bar(parent);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_60, 0);

    /* 中间确认大卡片 */
    lv_obj_t * card = lv_obj_create(page);
    lv_obj_set_size(card, 540, 250);
    lv_obj_set_pos(card, 130, 95);
    apply_glass_btn_style(card);

    /* 标题文字 */
    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, "是否停止移动");
    lv_obj_set_style_text_font(title, &mov_sta_cn, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

    /* “是”按钮 */
    lv_obj_t * btn_yes = lv_btn_create(card);
    lv_obj_set_size(btn_yes, 150, 90);
    lv_obj_set_pos(btn_yes, 70, 130);
    apply_glass_btn_style(btn_yes);
    lv_obj_add_event_cb(btn_yes, moving_pause_yes_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * yes_text = lv_label_create(btn_yes);
    lv_label_set_text(yes_text, "是");
    lv_obj_set_style_text_font(yes_text, &mov_sta_cn_1, 0);
    lv_obj_set_style_text_color(yes_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(yes_text);

    /* “否”按钮 */
    lv_obj_t * btn_no = lv_btn_create(card);
    lv_obj_set_size(btn_no, 150, 90);
    lv_obj_set_pos(btn_no, 300, 130);
    apply_glass_btn_style(btn_no);
    lv_obj_add_event_cb(btn_no, moving_pause_no_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * no_text = lv_label_create(btn_no);
    lv_label_set_text(no_text, "否");
    lv_obj_set_style_text_font(no_text, &mov_sta_cn_1, 0);
    lv_obj_set_style_text_color(no_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(no_text, LV_ALIGN_CENTER, 0, 0);

    return page;
}

static void apply_pause_confirm_btn_style(lv_obj_t * obj)
{
    lv_obj_set_style_radius(obj, 20, 0);

    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0xE9E1EA), 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_40, 0);

    lv_obj_set_style_bg_color(obj, lv_color_hex(0xBCA6BA), 0);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xA892A8), 0);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_90, 0);

    lv_obj_set_style_shadow_width(obj, 14, 0);
    lv_obj_set_style_shadow_opa(obj, LV_OPA_10, 0);
    lv_obj_set_style_shadow_spread(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_x(obj, 0, 0);
    lv_obj_set_style_shadow_ofs_y(obj, 5, 0);
    lv_obj_set_style_shadow_color(obj, lv_color_hex(0x4A334F), 0);

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}
