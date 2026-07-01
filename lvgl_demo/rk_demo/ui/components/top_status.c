#include "top_status.h"
#include "ui_time.h"

/****WIFI状态****/
LV_FONT_DECLARE(wifi_icon_font);
/****连接状态*****/
LV_FONT_DECLARE(connect);
/****扬声器状态****/ 
LV_FONT_DECLARE(voice);
/****电池状态****/ 
LV_FONT_DECLARE(battery);
/****时间显示****/ 
LV_FONT_DECLARE(font_cn_28);

/***全局静态对象指针***/
static lv_obj_t * g_wifi_label;
static lv_obj_t * g_connect_label;
static lv_obj_t * g_voice_label;
static lv_obj_t * g_battery_label;
static lv_obj_t * g_time_label;

/***状态切换函数***/
static void status_bar_set_wifi_internal(uint8_t level);
static void status_bar_set_connect_internal(uint8_t state);
static void status_bar_set_voice_internal(uint8_t state);
static void status_bar_set_battery_internal(uint8_t level);
static void status_bar_update_time(lv_obj_t *label);
static void status_bar_time_timer_cb(lv_timer_t *timer);
static void status_bar_time_delete_cb(lv_event_t *e);

void status_bar_create(lv_obj_t * parent)
{
    /* ===== 顶部状态信息，直接浮在背景上 ===== */
    /* ===== 左上 WIFI 图标 ===== */
    g_wifi_label = lv_label_create(parent);
    lv_obj_set_style_text_font(g_wifi_label, &wifi_icon_font, 0);
    lv_obj_set_style_text_color(g_wifi_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_wifi_label, 18, 10);
    lv_obj_clear_flag(g_wifi_label, LV_OBJ_FLAG_SCROLLABLE);
    
    /* ===== 左上 连接状态图标 ===== */
    g_connect_label = lv_label_create(parent);
    lv_obj_set_style_text_font(g_connect_label, &connect, 0);
    lv_obj_set_style_text_color(g_connect_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_connect_label, 60, 12); 
    lv_obj_clear_flag(g_connect_label, LV_OBJ_FLAG_SCROLLABLE);

    /* ===== 左上 扬声器状态图标 ===== */
    g_voice_label = lv_label_create(parent);
    lv_obj_set_style_text_font(g_voice_label, &voice, 0);
    lv_obj_set_style_text_color(g_voice_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_voice_label, 100, 10); 
    lv_obj_clear_flag(g_voice_label, LV_OBJ_FLAG_SCROLLABLE);

    /* ===== 时间显示 ===== */
    g_time_label = lv_label_create(parent);
    lv_obj_set_style_text_font(g_time_label, &font_cn_28, 0);
    lv_obj_set_style_text_color(g_time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_time_label, 510, 13);
    status_bar_update_time(g_time_label);
    lv_timer_t *time_timer = lv_timer_create(status_bar_time_timer_cb, 1000, g_time_label);
    lv_obj_add_event_cb(g_time_label, status_bar_time_delete_cb, LV_EVENT_DELETE, time_timer);

    /* ===== 电池状态 ===== */
    g_battery_label = lv_label_create(parent);
    lv_obj_set_style_text_font(g_battery_label, &battery, 0);
    lv_obj_set_style_text_color(g_battery_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(g_battery_label, 742, 15);
    lv_obj_clear_flag(g_battery_label, LV_OBJ_FLAG_SCROLLABLE);

    status_bar_set_wifi_internal(0);
    status_bar_set_connect_internal(0);
    status_bar_set_voice_internal(0);
    status_bar_set_battery_internal(5);
}

void status_bar_set_wifi(uint8_t level)
{
    status_bar_set_wifi_internal(level);
}

void status_bar_set_connect(uint8_t state)
{
    status_bar_set_connect_internal(state);
}

void status_bar_set_voice(uint8_t state)
{
    status_bar_set_voice_internal(state);
}

void status_bar_set_battery(uint8_t level)
{
    status_bar_set_battery_internal(level);
}

void status_bar_set_time(const char * text)
{
    if (g_time_label == NULL) {
        return;
    }
    lv_label_set_text(g_time_label, text);
}

static void status_bar_update_time(lv_obj_t *label)
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

static void status_bar_time_timer_cb(lv_timer_t *timer)
{
    if (timer == NULL) {
        return;
    }
    status_bar_update_time((lv_obj_t *)timer->user_data);
}

static void status_bar_time_delete_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    lv_timer_t *timer = (lv_timer_t *)lv_event_get_user_data(e);

    if (timer) {
        lv_timer_del(timer);
    }
    if (target == g_time_label) {
        g_time_label = NULL;
    }
}

static void status_bar_set_wifi_internal(uint8_t level)
{
    if (g_wifi_label == NULL) {
        return;
    }

    switch (level) {
        case 0:
            lv_label_set_text(g_wifi_label, "\xEE\x98\xBD");
            break;
        case 1:
            lv_label_set_text(g_wifi_label, "\xEE\x99\xB2");
            break;
        case 2:
        default:
            lv_label_set_text(g_wifi_label, "\xEE\x9D\x83");
            break;
    }
}

static void status_bar_set_connect_internal(uint8_t state)
{
    if (g_connect_label == NULL) {
        return;
    }

    switch (state) {
        case 0:
            lv_label_set_text(g_connect_label, "\xEE\x99\x99");
            break;
        case 1:
        default:
            lv_label_set_text(g_connect_label, "\xEE\x99\x98");
            break;
    }
}

static void status_bar_set_voice_internal(uint8_t state)
{
    if (g_voice_label == NULL) {
        return;
    }

    switch (state) {
        case 0:
            lv_label_set_text(g_voice_label, "\xEE\x9B\xA4");
            break;
        case 1:
        default:
            lv_label_set_text(g_voice_label, "\xEE\x98\xB5");
            break;
    }
}

static void status_bar_set_battery_internal(uint8_t level)
{
    if (g_battery_label == NULL) {
        return;
    }

    switch (level) {
        case 0:
            lv_label_set_text(g_battery_label, "\xEE\x98\x99");
            break;
        case 1:
            lv_label_set_text(g_battery_label, "\xEE\x98\x9E");
            break;
        case 2:
            lv_label_set_text(g_battery_label, "\xEE\x98\x9C");
            break;
        case 3:
            lv_label_set_text(g_battery_label, "\xEE\x98\x9D");
            break;
        case 4:
            lv_label_set_text(g_battery_label, "\xEE\x98\x9F");
            break;
        case 5:
        default:
            lv_label_set_text(g_battery_label, "\xEE\x98\xA1");
            break;
    }
}
