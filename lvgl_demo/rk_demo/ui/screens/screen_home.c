#include "screen_home.h"
#include <lvgl/lvgl.h>
#include "components/page_clean.h"
#include "components/top_status.h"
#include "components/card.h"
#include "components/btn.h"
#include "components/page_location.h"
#include "components/page_loca_end.h"
#include "components/setting.h"
#include "components/water_add.h"
#include "protocol.h"
#include "device_state.h"
#include "device_control.h"
#include "screen_home_clean_flow.h"

/******* 图标和背景 *****/
LV_IMAGE_DECLARE(q0ULJ1);
LV_FONT_DECLARE(back_btn);

#define RIGHT_VIEW_X      441
#define RIGHT_VIEW_Y      56
#define RIGHT_VIEW_W      346
#define RIGHT_VIEW_H      395

#define RIGHT_THUMB_X     792
#define RIGHT_THUMB_Y     56
#define RIGHT_THUMB_W     5
#define RIGHT_THUMB_H     82

#define START_TEMP_MIN    35
#define START_TEMP_MAX    48
#define ACTUAL_TEMP_MIN   0
#define ACTUAL_TEMP_MAX   50
#define START_TIME_MIN    10
#define START_TIME_MAX    30
#define BUCKET_TEMP_FALLBACK 35
#define BUCKET_CONFIRM_MS 3000
#define BUCKET_RESTORE_MS 5000
#define BUCKET_TIMER_STEP_MIN 5
#define BUCKET_TIMER_MAX_MIN 30

typedef struct {
    lv_obj_t * viewport;
    lv_obj_t * thumb;
} right_area_t;

typedef enum {
    HOME_VARIANT_CONNECTED = 0,
    HOME_VARIANT_DISCONNECTED,
} home_variant_t;

typedef struct {
    uint8_t temp; // 屏幕上显示的温度
    uint8_t water_liters; // 屏幕上显示的水量
    uint8_t timer_units; // 屏幕上定时点击的单位，0不定时，1-5对应10-30分钟
    uint16_t timer_minutes; // 屏幕上定时点击的分钟数
    uint8_t herb1;  // 屏幕上显示的药材1图标，0x00无药材，0x05有药材
    uint8_t herb2;  // 屏幕上显示的药材2图标，0x00无药材，0x05有药材
    bool self_clean_pending; // 是否有待发自清洁命令
} pending_start_t; // 基站界面的待发送设置

typedef struct {
    uint8_t temp;
    uint8_t massage_level;
    bool heat_on;
    bool sterilize_on;
} bucket_home_status_t; // 桶界面显示的状态，包含设备实际状态和编辑状态叠加的结果

typedef struct {
    uint8_t actual_temp;
    bool has_actual_temp;
    uint8_t target_temp;
    bool editing;
    lv_timer_t *confirm_timer;
    lv_timer_t *restore_timer;
} bucket_temp_edit_state_t; // 桶界面温度编辑状态

typedef struct {
    uint16_t actual_min;
    bool has_actual_min;
    uint16_t target_min;
    bool editing;
    lv_timer_t *restore_timer;
} bucket_timer_edit_state_t; // 桶界面定时编辑状态

static right_area_t s_connected_right;
static right_area_t s_disconnected_right;
static lv_obj_t * s_page_location_main = NULL;
static lv_obj_t * s_page_loca_end = NULL;
static lv_obj_t * s_connected_controls = NULL;
static lv_obj_t * s_disconnected_controls = NULL;
static home_variant_t s_home_variant = HOME_VARIANT_CONNECTED;
static pending_start_t s_pending_start;
static bool s_proto_inited = false;
static bucket_home_status_t s_bucket_status = {
    .temp = 38,
    .massage_level = 0,
    .heat_on = false,
    .sterilize_on = false,
};
static bucket_temp_edit_state_t s_bucket_temp_edit = {0};
static bucket_timer_edit_state_t s_bucket_timer_edit = {0};
static screen_home_clean_flow_t s_clean_flow = {0};


/* 静态函数声明 */
static void create_home_background(lv_obj_t *parent);
static void create_home_overlay(lv_obj_t *parent);
static void create_right_function_area(lv_obj_t *parent, right_area_t *area, bool disconnected);
static void right_area_scroll_cb(lv_event_t *e);
static void update_right_thumb_pos(right_area_t *area);
static void home_btn_event_handler(btn_event_id_t id);
static void hide_all_pages(void);
static void show_page(lv_obj_t * page);
static bool home_main_visible(void);
static lv_obj_t * create_func_page(lv_obj_t * parent, const char * title);
static void func_page_back_btn_cb(lv_event_t * e);
static void add_back_button(lv_obj_t * page);
static void setting_back_to_home_cb(void);
static void ensure_proto_init(void);
static void home_apply_variant(home_variant_t variant, bool force_home);
static void reset_right_area(right_area_t *area);
static void pending_set_sleep(void);
static void pending_set_relax(void);
static void pending_set_warm(void);
static void pending_refresh_card(void);
static uint8_t pending_water_icon_level(void);
static void pending_cancel_self_clean(void);
static void pending_temp_step(int delta);
static void bucket_temp_step(int delta);
static void pending_timer_next(void);
static void pending_water_next(void);
static void bucket_refresh_card(void);
static void refresh_card_runtime_status(const device_state_t *state);
static void handle_device_state_changed(const device_state_t *state, void *user_data);
static void update_clean_flow_from_state(const device_state_t *state);
static void apply_clean_flow_step(page_clean_step_t step);
static bool bucket_temp_valid(int temp);
static uint8_t bucket_temp_clamp(int temp);
static uint8_t bucket_temp_base_value(void);
static void bucket_temp_set_display(uint8_t temp);
static void bucket_temp_start_edit(uint8_t temp);
static void bucket_temp_clear_edit(void);
static void bucket_temp_confirm_timer_cb(lv_timer_t *timer);
static void bucket_temp_restore_timer_cb(lv_timer_t *timer);
static void bucket_temp_reset_confirm_timer(void);
static void bucket_temp_reset_restore_timer(void);
static uint16_t bucket_timer_units_to_min(uint8_t units);
static uint8_t bucket_timer_min_to_units(uint16_t min);
static void bucket_timer_restore_timer_cb(lv_timer_t *timer);
static void bucket_timer_reset_restore_timer(void);
static void bucket_timer_clear_edit(void);
static void bucket_timer_next_step(void);

/* 右侧功能区的每一页都提前创建好，后续根据需要显示和隐藏 */
static lv_obj_t * page_main = NULL;
static lv_obj_t * page_clean_confirm = NULL;
static lv_obj_t * page_clean_running = NULL;
static lv_obj_t * page_clean_finish  = NULL;
static lv_obj_t * page_water_running = NULL;
static lv_obj_t * page_water_confirm = NULL;

void screen_home_create(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* 清屏，避免以后重复进入首页时对象叠加 */
    lv_obj_clean(scr);

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    create_home_background(scr);
    create_home_overlay(scr);
}

static void create_home_background(lv_obj_t *parent)
{
    lv_obj_t *bg = lv_image_create(parent);
    lv_image_set_src(bg, &q0ULJ1);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
}

static void create_home_overlay(lv_obj_t *parent)
{
    lv_obj_t *panel = lv_obj_create(parent);
    page_main = panel;
    lv_obj_set_size(panel, 800, 480);
    lv_obj_set_pos(panel, 0, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶部状态栏 */
    status_bar_create(panel);

    /* 左上信息卡 */
    card_create_info(panel);

    pending_set_sleep();

    // 连接状态相关的控件分成两套，放在两个对象里，方便根据连接状态切换显示和隐藏
    // 基站界面的控件
    s_connected_controls = lv_obj_create(panel);
    lv_obj_set_size(s_connected_controls, 800, 480);
    lv_obj_set_pos(s_connected_controls, 0, 0);
    lv_obj_set_style_bg_opa(s_connected_controls, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_connected_controls, 0, 0);
    lv_obj_set_style_pad_all(s_connected_controls, 0, 0);
    lv_obj_clear_flag(s_connected_controls, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_connected_controls, LV_OBJ_FLAG_CLICKABLE);

    // 桶体界面的控件
    s_disconnected_controls = lv_obj_create(panel);
    lv_obj_set_size(s_disconnected_controls, 800, 480);
    lv_obj_set_pos(s_disconnected_controls, 0, 0);
    lv_obj_set_style_bg_opa(s_disconnected_controls, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_disconnected_controls, 0, 0);
    lv_obj_set_style_pad_all(s_disconnected_controls, 0, 0);
    lv_obj_clear_flag(s_disconnected_controls, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_disconnected_controls, LV_OBJ_FLAG_CLICKABLE);

    btn_create_left_group(s_connected_controls);
    create_right_function_area(s_connected_controls, &s_connected_right, false);

    btn_create_disconnected_left_group(s_disconnected_controls);
    create_right_function_area(s_disconnected_controls, &s_disconnected_right, true);
    home_apply_variant(HOME_VARIANT_CONNECTED, false);

    // 创建自清洁页面
    page_clean_set_t clean_set = page_clean_create(parent, show_page, page_main);
    page_clean_confirm = clean_set.page_confirm;
    page_clean_running = clean_set.page_running;
    page_clean_finish  = clean_set.page_finish;

    // 创建定位页面
    page_location_set_t location_set = page_location_create(parent, show_page, page_main);
    s_page_location_main = location_set.page_location_main;
    s_page_loca_end = page_loca_end_create(parent, show_page, page_main);

    // 创建加水页面
    page_water_add_set_t water_set = page_water_add_create(parent, show_page, page_main);
    page_water_running = water_set.page_running;
    page_water_confirm = water_set.page_confirm;

    ensure_proto_init();
    // 注册状态变化回调
    device_state_register_listener(handle_device_state_changed, NULL);
    show_page(page_main);
}

static void create_right_function_area(lv_obj_t *parent, right_area_t *area, bool disconnected)
{
    /* 右侧可视区域：真正可滚动的是它 */
    lv_obj_t * viewport = lv_obj_create(parent);
    lv_obj_set_size(viewport, RIGHT_VIEW_W, RIGHT_VIEW_H);
    lv_obj_set_pos(viewport, RIGHT_VIEW_X, RIGHT_VIEW_Y);
    lv_obj_set_style_radius(viewport, 0, 0);
    lv_obj_set_style_border_width(viewport, 0, 0);
    lv_obj_set_style_bg_opa(viewport, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(viewport, 0, 0);
    lv_obj_set_style_clip_corner(viewport, true, 0);

    /* 打开纵向滚动，关闭默认滚动条显示 */
    lv_obj_add_flag(viewport, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(viewport, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(viewport, LV_SCROLLBAR_MODE_OFF);

    /* 关闭弹性和惯性，避免超过边界 */
    lv_obj_clear_flag(viewport, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(viewport, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    /* 启用整页吸附 */
    lv_obj_set_scroll_snap_y(viewport, LV_SCROLL_SNAP_START);

    /* 绑定滚动事件，用来更新右侧滑块 */
    lv_obj_add_event_cb(viewport, right_area_scroll_cb, LV_EVENT_SCROLL, area);

    area->viewport = viewport;

    /* ===== 第 1 页 ===== */
    lv_obj_t * page1 = lv_obj_create(viewport);
    lv_obj_set_size(page1, RIGHT_VIEW_W, RIGHT_VIEW_H);
    lv_obj_set_pos(page1, 0, 0);
    lv_obj_set_style_radius(page1, 0, 0);
    lv_obj_set_style_border_width(page1, 0, 0);
    lv_obj_set_style_bg_opa(page1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page1, 0, 0);
    lv_obj_clear_flag(page1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page1, LV_OBJ_FLAG_SNAPPABLE);

    if (disconnected) {
        btn_create_disconnected_right_page1(page1);
    } else {
        btn_create_right_page1(page1);
    }

    /* ===== 第 2 页 ===== */
    lv_obj_t * page2 = lv_obj_create(viewport);
    lv_obj_set_size(page2, RIGHT_VIEW_W, RIGHT_VIEW_H);
    lv_obj_set_pos(page2, 0, RIGHT_VIEW_H);
    lv_obj_set_style_radius(page2, 0, 0);
    lv_obj_set_style_border_width(page2, 0, 0);
    lv_obj_set_style_bg_opa(page2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page2, 0, 0);
    lv_obj_clear_flag(page2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page2, LV_OBJ_FLAG_SNAPPABLE);

    if (disconnected) {
        btn_create_disconnected_right_page2(page2);
    } else {
        btn_create_right_page2(page2);
    }

    btn_set_event_callback(home_btn_event_handler);
    /* ===== 单条滑块 ===== */
    lv_obj_t * thumb = lv_obj_create(parent);
    lv_obj_set_size(thumb, RIGHT_THUMB_W, RIGHT_THUMB_H);
    lv_obj_set_pos(thumb, RIGHT_THUMB_X, RIGHT_THUMB_Y);
    lv_obj_set_style_radius(thumb, 4, 0);
    lv_obj_set_style_border_width(thumb, 0, 0);
    lv_obj_set_style_bg_color(thumb, lv_color_hex(0xCBBBE6), 0);
    lv_obj_set_style_bg_opa(thumb, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(thumb, 0, 0);
    lv_obj_clear_flag(thumb, LV_OBJ_FLAG_SCROLLABLE);

    area->thumb = thumb;

    /* 初始显示第一页 */
    lv_obj_scroll_to_y(viewport, 0, LV_ANIM_OFF);
    update_right_thumb_pos(area);
}

// 滚动事件回调，根据滚动位置更新右侧滑块位置
static void right_area_scroll_cb(lv_event_t *e)
{
    right_area_t *area = (right_area_t *)lv_event_get_user_data(e);
    update_right_thumb_pos(area);
}

// 根据滚动位置计算滑块位置，并更新滑块位置
static void update_right_thumb_pos(right_area_t *area)
{
    if (area == NULL || area->viewport == NULL || area->thumb == NULL) {
        return;
    }

    /* 当前已经滚过去的距离 */
    lv_coord_t scroll_top = lv_obj_get_scroll_top(area->viewport);

    /* 最大可滚动距离 = 上面被卷走 + 下面还没显示 */
    lv_coord_t max_scroll = scroll_top + lv_obj_get_scroll_bottom(area->viewport);

    /* 滑块可移动的总行程 */
    lv_coord_t travel = RIGHT_VIEW_H - RIGHT_THUMB_H;

    lv_coord_t thumb_y = RIGHT_THUMB_Y;

    if (max_scroll > 0) {
        thumb_y = RIGHT_THUMB_Y + (lv_coord_t)((int32_t)scroll_top * travel / max_scroll);
    }

    /* 边界保护 */
    if (thumb_y < RIGHT_THUMB_Y) {
        thumb_y = RIGHT_THUMB_Y;
    }
    if (thumb_y > RIGHT_THUMB_Y + travel) {
        thumb_y = RIGHT_THUMB_Y + travel;
    }

    lv_obj_set_pos(area->thumb, RIGHT_THUMB_X, thumb_y);
}

static void ensure_proto_init(void)
{
    if (!s_proto_inited) {
        protocol_init();
        protocol_handler_init();
        s_proto_inited = true;
        LV_LOG_USER("[HOME] protocol initialized");
    }
}

// 重置右侧功能区的滚动位置和滑块位置，切换连接状态时调用
static void reset_right_area(right_area_t *area)
{
    if (area == NULL || area->viewport == NULL) {
        return;
    }

    lv_obj_scroll_to_y(area->viewport, 0, LV_ANIM_OFF);
    update_right_thumb_pos(area);
}

// 根据连接状态切换显示不同的功能区，并根据需要刷新显示内容
static void home_apply_variant(home_variant_t variant, bool force_home)
{
    bool variant_changed = s_home_variant != variant;

    if (s_connected_controls == NULL || s_disconnected_controls == NULL) {
        return;
    }

    s_home_variant = variant;
    if (variant == HOME_VARIANT_DISCONNECTED) {
        lv_obj_add_flag(s_connected_controls, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_disconnected_controls, LV_OBJ_FLAG_HIDDEN);
        if (variant_changed) {
            reset_right_area(&s_disconnected_right);
            bucket_temp_clear_edit();
            bucket_timer_clear_edit();
            bucket_refresh_card();
            s_bucket_status.heat_on = true; // 离线状态默认加热开启，显示温度
        }
    } else {
        lv_obj_clear_flag(s_connected_controls, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_disconnected_controls, LV_OBJ_FLAG_HIDDEN);
        if (variant_changed) {
            reset_right_area(&s_connected_right);
        }
        pending_refresh_card();
    }

    if (force_home) {
        show_page(page_main);
    }
}

// 设备状态变化回调，更新界面显示
static void handle_device_state_changed(const device_state_t *state, void *user_data)
{
    LV_UNUSED(user_data);

    if (state == NULL || state->comm_timeout) {
        return;
    }

    LV_LOG_USER("[HOME] state: link_status=0x%02X timeout=%d bucket_fault=0x%X base_fault=0x%X",
                state->frame.link_status,
                state->comm_timeout,
                state->bucket_faults,
                state->base_faults);

    if (state->frame.link_status == 0x00) {
        screen_home_clean_flow_stop(&s_clean_flow);
        home_apply_variant(HOME_VARIANT_DISCONNECTED, true);
        refresh_card_runtime_status(state);
    } else if (state->frame.link_status == 0x01) {
        // 切换成基站页面后，只有在之前是离线状态才刷新卡片显示，避免在基站页面时每次收到状态更新都刷新卡片导致界面闪烁
        home_apply_variant(HOME_VARIANT_CONNECTED,
                           s_home_variant == HOME_VARIANT_DISCONNECTED);
        update_clean_flow_from_state(state);
    }
}

static void update_clean_flow_from_state(const device_state_t *state)
{
    page_clean_step_t step;

    if (state == NULL) {
        return;
    }

    if (screen_home_clean_flow_update(&s_clean_flow,
                                      state->frame.base_main_status,
                                      &step)) {
        apply_clean_flow_step(step);
    }
}

static void apply_clean_flow_step(page_clean_step_t step)
{
    if (step == PAGE_CLEAN_STEP_FINISH) {
        if (page_clean_finish) {
            show_page(page_clean_finish);
        }
        return;
    }

    if (step < PAGE_CLEAN_STEP_COUNT) {
        page_clean_set_step(step);
    }
}

// 基站页面刷新左上角卡片，显示设定的数据和状态
static void pending_refresh_card(void)
{
    if (s_pending_start.self_clean_pending) {
        // 左小小卡片显示进入自清洁
        card_set_clean_pending();
        return;
    }

    card_exit_clean();
    card_set_temperature(s_pending_start.temp);
    card_set_time_min(s_pending_start.timer_minutes);
    card_set_water_level(pending_water_icon_level());
    card_set_low_water(false);
    card_set_herb_icons(s_pending_start.herb1, s_pending_start.herb2);
}

// 收到MCU消息后更新桶端界面显示状态
static void refresh_card_runtime_status(const device_state_t *state)
{
    if (state == NULL) {
        return;
    }

    if (bucket_temp_valid(state->frame.bucket_temp)) {
        s_bucket_temp_edit.actual_temp = state->frame.bucket_temp;
        s_bucket_temp_edit.has_actual_temp = true;
    }
    s_bucket_status.massage_level = state->frame.bucket_massage;
    s_bucket_status.sterilize_on = state->frame.bucket_uv > 0;
    if (state->frame.bucket_timer <= 6) {
        s_bucket_timer_edit.actual_min = bucket_timer_units_to_min(state->frame.bucket_timer);
        s_bucket_timer_edit.has_actual_min = true;
    }

    // 不在编辑状态且有实际温度时更新显示实际温度，保持编辑状态时不更新温度，避免界面温度闪来闪去
    if (!s_bucket_temp_edit.editing && s_bucket_temp_edit.has_actual_temp) {
        s_bucket_status.temp = s_bucket_temp_edit.actual_temp;
    }

    bucket_refresh_card();
}

//  切换到桶页面刷新左上角卡片
static void bucket_refresh_card(void)
{
    uint16_t timer_min = 0;

    if (s_bucket_timer_edit.editing) {
        timer_min = s_bucket_timer_edit.target_min;
    } else if (s_bucket_timer_edit.has_actual_min) {
        timer_min = s_bucket_timer_edit.actual_min;
    } else {
        timer_min = s_pending_start.timer_minutes;
    }

    card_exit_clean();
    card_set_temperature(s_bucket_status.temp);
    card_set_time_min(timer_min);
    card_set_runtime_status(s_bucket_status.massage_level,
                            s_bucket_status.heat_on,
                            s_bucket_status.sterilize_on);
}

// 显示水量图标的级别，0-3对应不同的水位图标
static uint8_t pending_water_icon_level(void)
{
    if (s_pending_start.water_liters <= 6) {
        return 1;
    }
    if (s_pending_start.water_liters <= 8) {
        return 2;
    }
    return 3;
}

// 判断温度是否在0-50合理范围内
static bool bucket_temp_valid(int temp)
{
    return temp >= ACTUAL_TEMP_MIN && temp <= ACTUAL_TEMP_MAX;
}

// 控制温度范围
static uint8_t bucket_temp_clamp(int temp)
{
    if (temp < ACTUAL_TEMP_MIN) {
        temp = ACTUAL_TEMP_MIN;
    }
    if (temp > ACTUAL_TEMP_MAX) {
        temp = ACTUAL_TEMP_MAX;
    }
    return (uint8_t)temp;
}

// 决定第一次调温时的基准温度，MCU上传实际温度最优先，当前UI保存的温度第二，默认35°C最后
static uint8_t bucket_temp_base_value(void)
{
    if (s_bucket_temp_edit.has_actual_temp && bucket_temp_valid(s_bucket_temp_edit.actual_temp)) {
        return s_bucket_temp_edit.actual_temp;
    }
    if (bucket_temp_valid(s_bucket_status.temp)) {
        return s_bucket_status.temp;
    }
    return BUCKET_TEMP_FALLBACK;
}

// 单位转换：MCU协议里定时是以5分钟为单位的，这里转换成分钟显示在界面上
static uint16_t bucket_timer_units_to_min(uint8_t units)
{
    if (units > 6) {
        units = 6;
    }
    return (uint16_t)units * BUCKET_TIMER_STEP_MIN;
}

// 单位转换：分钟数切换成单位数
static uint8_t bucket_timer_min_to_units(uint16_t min)
{
    if (min > BUCKET_TIMER_MAX_MIN) {
        min = BUCKET_TIMER_MAX_MIN;
    }
    return (uint8_t)(min / BUCKET_TIMER_STEP_MIN);
}

// 温度显示
static void bucket_temp_set_display(uint8_t temp)
{
    s_bucket_status.temp = temp;
    bucket_refresh_card();
}

// 桶端温度编辑后的确认提示定时器回调
static void bucket_temp_confirm_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    //定时器已触发了，清空定时器句柄
    s_bucket_temp_edit.confirm_timer = NULL;
    if (s_home_variant != HOME_VARIANT_DISCONNECTED) {
        return;
    }

    if (s_bucket_temp_edit.editing) {
        card_blink_temperature_confirm();
    }
}

// 设定一个3s的定时器，如果前一个还未触发，就重置它，保证在最后一次调温后3s内如果没有继续调温，就触发确认提示
static void bucket_temp_reset_confirm_timer(void)
{
    if (s_bucket_temp_edit.confirm_timer) {
        lv_timer_del(s_bucket_temp_edit.confirm_timer);
        s_bucket_temp_edit.confirm_timer = NULL;
    }

    s_bucket_temp_edit.confirm_timer = lv_timer_create(bucket_temp_confirm_timer_cb,
                                                       BUCKET_CONFIRM_MS,
                                                       NULL);
    // 设置3s后只触发一次bucket_temp_confirm_timer_cb
    if (s_bucket_temp_edit.confirm_timer) {
        lv_timer_set_repeat_count(s_bucket_temp_edit.confirm_timer, 1);
    }
}

// 5s后恢复显示实际温度，退出编辑状态
static void bucket_temp_restore_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    s_bucket_temp_edit.restore_timer = NULL;
    if (s_home_variant != HOME_VARIANT_DISCONNECTED) {
        return;
    }

    s_bucket_temp_edit.editing = false;
    if (s_bucket_temp_edit.has_actual_temp) {
        bucket_temp_set_display(s_bucket_temp_edit.actual_temp);
    }
}

// 设定一个5s的定时器，如果前一个还未触发，就重置它，保证在最后一次调温后5s内如果没有继续调温，就恢复显示实际温度，退出编辑状态
static void bucket_temp_reset_restore_timer(void)
{
    if (s_bucket_temp_edit.restore_timer) {
        lv_timer_del(s_bucket_temp_edit.restore_timer);
        s_bucket_temp_edit.restore_timer = NULL;
    }

    s_bucket_temp_edit.restore_timer = lv_timer_create(bucket_temp_restore_timer_cb,
                                                       BUCKET_RESTORE_MS,
                                                       NULL);
    if (s_bucket_temp_edit.restore_timer) {
        lv_timer_set_repeat_count(s_bucket_temp_edit.restore_timer, 1);
    }
}

// 时间设定的定时器，5s后退出编辑
static void bucket_timer_restore_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    s_bucket_timer_edit.restore_timer = NULL;
    if (s_home_variant != HOME_VARIANT_DISCONNECTED) {
        return;
    }

    s_bucket_timer_edit.editing = false;
    bucket_refresh_card();
}

static void bucket_timer_reset_restore_timer(void)
{
    if (s_bucket_timer_edit.restore_timer) {
        lv_timer_del(s_bucket_timer_edit.restore_timer);
        s_bucket_timer_edit.restore_timer = NULL;
    }

    s_bucket_timer_edit.restore_timer = lv_timer_create(bucket_timer_restore_timer_cb,
                                                        BUCKET_RESTORE_MS,
                                                        NULL);
    if (s_bucket_timer_edit.restore_timer) {
        lv_timer_set_repeat_count(s_bucket_timer_edit.restore_timer, 1);
    }
}

// 开始编辑温度，显示编辑的温度值，启动确认和恢复计时器
static void bucket_temp_start_edit(uint8_t temp)
{
    s_bucket_temp_edit.target_temp = temp;
    s_bucket_temp_edit.editing = true;

    bucket_temp_set_display(temp);
    bucket_temp_reset_confirm_timer();
    bucket_temp_reset_restore_timer();
}

// 退出编辑后，卡片立刻恢复显示MCU上报的实际温度
static void bucket_temp_clear_edit(void)
{
    s_bucket_temp_edit.editing = false;

    if (s_bucket_temp_edit.confirm_timer) {
        lv_timer_del(s_bucket_temp_edit.confirm_timer);
        s_bucket_temp_edit.confirm_timer = NULL;
    }
    if (s_bucket_temp_edit.restore_timer) {
        lv_timer_del(s_bucket_temp_edit.restore_timer);
        s_bucket_temp_edit.restore_timer = NULL;
    }

    if (s_bucket_temp_edit.has_actual_temp) {
        s_bucket_status.temp = s_bucket_temp_edit.actual_temp;
    }
}

// 推出定时编辑状态
static void bucket_timer_clear_edit(void)
{
    s_bucket_timer_edit.editing = false;

    if (s_bucket_timer_edit.restore_timer) {
        lv_timer_del(s_bucket_timer_edit.restore_timer);
        s_bucket_timer_edit.restore_timer = NULL;
    }
}

// 取消自清洁待发送状态
static void pending_cancel_self_clean(void)
{
    s_pending_start.self_clean_pending = false;
}

// 基站界面+ - 温度显示
static void pending_temp_step(int delta)
{
    int temp = s_pending_start.temp + delta;

    if (temp < START_TEMP_MIN) {
        temp = START_TEMP_MIN;
    }
    if (temp > START_TEMP_MAX) {
        temp = START_TEMP_MAX;
    }

    pending_cancel_self_clean();
    s_pending_start.temp = (uint8_t)temp;
    pending_refresh_card();
}

// + - 号编辑并显示温度
static void bucket_temp_step(int delta)
{
    uint8_t base = 0;
    uint8_t target = 0;

    if (s_bucket_temp_edit.editing) {
        base = s_bucket_temp_edit.target_temp;
    } else {
        base = bucket_temp_base_value();
    }

    target = bucket_temp_clamp((int)base + delta);
    bucket_temp_start_edit(target);

    if (s_bucket_status.heat_on) {
        ensure_proto_init();
        device_control_bucket_heat_on(target);
    }
}

// 桶体界面定时编辑，分钟数每次增加5分钟，超过30分钟后回到0分钟，界面显示以分钟为单位，发送给MCU时转换成单位数
static void bucket_timer_next_step(void)
{
    uint16_t next = BUCKET_TIMER_STEP_MIN;

    if (s_bucket_timer_edit.editing) {
        if (s_bucket_timer_edit.target_min >= BUCKET_TIMER_MAX_MIN) {
            next = 0;
        } else {
            next = s_bucket_timer_edit.target_min + BUCKET_TIMER_STEP_MIN;
        }
    }

    s_bucket_timer_edit.target_min = next;
    s_bucket_timer_edit.editing = true;
    bucket_refresh_card();
    bucket_timer_reset_restore_timer();

    ensure_proto_init();
    device_control_bucket_timer(bucket_timer_min_to_units(next));
}

// 基站界面定时编辑
static void pending_timer_next(void)
{
    if (s_pending_start.timer_minutes >= START_TIME_MAX) {
        s_pending_start.timer_minutes = START_TIME_MIN;
    } else {
        s_pending_start.timer_minutes += 5;
    }

    s_pending_start.timer_units =
        (uint8_t)((s_pending_start.timer_minutes - START_TIME_MIN) / 5 + 1);

    pending_cancel_self_clean();
    pending_refresh_card();
}

// 水量挡位控制
static void pending_water_next(void)
{
    if (s_pending_start.water_liters <= 6) {
        s_pending_start.water_liters = 8;
    } else if (s_pending_start.water_liters <= 8) {
        s_pending_start.water_liters = 12;
    } else {
        s_pending_start.water_liters = 6;
    }

    pending_cancel_self_clean();
    pending_refresh_card();
}

static void pending_set_sleep(void)
{
    s_pending_start.temp = 0x26;
    s_pending_start.water_liters = 0x06;
    s_pending_start.timer_units = 0x03;
    s_pending_start.timer_minutes = 20;
    s_pending_start.herb1 = 0x00;
    s_pending_start.herb2 = 0x00;
    s_pending_start.self_clean_pending = false;
    pending_refresh_card();
}

static void pending_set_relax(void)
{
    s_pending_start.temp = 0x28;
    s_pending_start.water_liters = 0x08;
    s_pending_start.timer_units = 0x04;
    s_pending_start.timer_minutes = 25;
    s_pending_start.herb1 = 0x05;
    s_pending_start.herb2 = 0x00;
    s_pending_start.self_clean_pending = false;
    pending_refresh_card();
}

static void pending_set_warm(void)
{
    s_pending_start.temp = 0x2A;
    s_pending_start.water_liters = 0x0C;
    s_pending_start.timer_units = 0x05;
    s_pending_start.timer_minutes = 30;
    s_pending_start.herb1 = 0x00;
    s_pending_start.herb2 = 0x05;
    s_pending_start.self_clean_pending = false;
    pending_refresh_card();
}

static void home_btn_event_handler(btn_event_id_t id)
{
    if (!home_main_visible()) {
        LV_LOG_USER("[HOME] ignore button event while home page hidden: id=%d", id);
        return;
    }

    switch (id) {
        case BTN_EVENT_MODE_SLEEP:
            ensure_proto_init();
            pending_set_sleep();
            LV_LOG_USER("[HOME] sleep mode cached");
            break;

        case BTN_EVENT_MODE_RELAX:
            pending_set_relax();
            LV_LOG_USER("[HOME] relax mode cached");
            break;

        case BTN_EVENT_MODE_WARM:
            pending_set_warm();
            LV_LOG_USER("[HOME] warm mode cached");
            break;

        case BTN_EVENT_MODE_CLEAN:
            s_pending_start.self_clean_pending = true;
            pending_refresh_card();
            break;

        case BTN_EVENT_START:
            if (s_home_variant == HOME_VARIANT_DISCONNECTED) {
                s_bucket_status.temp = 0;
                s_bucket_status.massage_level = 0;
                s_bucket_status.heat_on = false;
                s_bucket_status.sterilize_on = false;
                card_clear_bucket_status();
                LV_LOG_USER("[HOME] bucket confirm cleared status without sending");
                break;
            }

            ensure_proto_init();
            // 是否启动自清洁
            if (s_pending_start.self_clean_pending) {
                device_control_base_self_clean(5, 3, 3);
                screen_home_clean_flow_start(&s_clean_flow);
                page_clean_set_step(PAGE_CLEAN_STEP_DRAIN_1);
                show_page(page_clean_running);
            } else {
                device_control_base_start_cached(s_pending_start.water_liters,
                                                 s_pending_start.temp,
                                                 s_pending_start.herb1,
                                                 s_pending_start.herb2,
                                                 s_pending_start.timer_units);

                page_water_add_set_info(s_pending_start.temp,
                                        s_pending_start.timer_minutes,
                                        pending_water_icon_level(),
                                        s_pending_start.herb1,
                                        s_pending_start.herb2);
            // 切换到加水页面，显示加水动画                        
                show_page(page_water_running);
            }
            break;

        case BTN_EVENT_BATH1:
            s_pending_start.herb1 = s_pending_start.herb1 == 0x05 ? 0x00 : 0x05;
            pending_cancel_self_clean();
            pending_refresh_card();
            break;

        case BTN_EVENT_BATH2:
            s_pending_start.herb2 = s_pending_start.herb2 == 0x05 ? 0x00 : 0x05;
            pending_cancel_self_clean();
            pending_refresh_card();
            break;

        case BTN_EVENT_TEMP_UP:
            if (s_home_variant == HOME_VARIANT_DISCONNECTED) {
                bucket_temp_step(1);
            } else {
                pending_temp_step(1);
            }
            break;

        case BTN_EVENT_TEMP_DOWN:
            if (s_home_variant == HOME_VARIANT_DISCONNECTED) {
                bucket_temp_step(-1);
            } else {
                pending_temp_step(-1);
            }
            break;

        case BTN_EVENT_TIMER_NEXT:
            pending_timer_next();
            break;

        case BTN_EVENT_WATER_NEXT:
            pending_water_next();
            break;

        case BTN_EVENT_LOCATE:
            pending_cancel_self_clean();
            if (s_home_variant == HOME_VARIANT_DISCONNECTED) {
                bucket_refresh_card();
            } else {
                pending_refresh_card();
            }
            show_page(s_page_location_main);
            break;

        case BTN_EVENT_SETTING:
            pending_cancel_self_clean();
            if (s_home_variant == HOME_VARIANT_DISCONNECTED) {
                bucket_refresh_card();
            } else {
                pending_refresh_card();
            }
            setting_set_back_callback(setting_back_to_home_cb);
            show_page(setting_create(lv_scr_act()));
            break;

        case BTN_EVENT_DISCONNECTED_HEAT:
            ensure_proto_init();
            if (s_bucket_status.heat_on) {
                s_bucket_status.heat_on = false;
                device_control_bucket_heat_off();
                bucket_temp_clear_edit();
            } else {
                uint8_t target = s_bucket_temp_edit.editing ?
                                 s_bucket_temp_edit.target_temp :
                                 bucket_temp_base_value();
                s_bucket_status.heat_on = true;
                device_control_bucket_heat_on(target);
                bucket_temp_clear_edit();
            }
            bucket_refresh_card();
            break;

        case BTN_EVENT_DISCONNECTED_MASSAGE:
            ensure_proto_init();
            s_bucket_status.massage_level++;
            if (s_bucket_status.massage_level > 3) {
                s_bucket_status.massage_level = 0;
            }
            bucket_refresh_card();
            device_control_bucket_massage(s_bucket_status.massage_level);
            break;

        case BTN_EVENT_DISCONNECTED_STERILIZE:
            ensure_proto_init();
            s_bucket_status.sterilize_on = !s_bucket_status.sterilize_on;
            bucket_refresh_card();
            device_control_bucket_uv(s_bucket_status.sterilize_on ? 1 : 0);
            break;

        case BTN_EVENT_DISCONNECTED_TIMER:
            bucket_timer_next_step();
            break;

        case BTN_EVENT_DISCONNECTED_AUTO_WAREHOUSE:
            ensure_proto_init();
            device_control_bucket_auto_warehouse();
            break;

        default:
            break;
    }
}

/* 隐藏所有页面 */
static void hide_all_pages(void)
{
    page_loca_end_stop_auto_back();
    if (page_main) {
        lv_obj_add_flag(page_main, LV_OBJ_FLAG_HIDDEN);
    }
    if (page_clean_confirm) {
        lv_obj_add_flag(page_clean_confirm, LV_OBJ_FLAG_HIDDEN);
    }
    if (page_clean_running) {
        lv_obj_add_flag(page_clean_running, LV_OBJ_FLAG_HIDDEN);
    }
    if (page_clean_finish) {
        lv_obj_add_flag(page_clean_finish, LV_OBJ_FLAG_HIDDEN);
    }
    if (page_water_running) {
        lv_obj_add_flag(page_water_running, LV_OBJ_FLAG_HIDDEN);
    }
    if (page_water_confirm) {
        lv_obj_add_flag(page_water_confirm, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_page_location_main) {
        lv_obj_add_flag(s_page_location_main, LV_OBJ_FLAG_HIDDEN);
    }
    page_location_hide_all();
    if (s_page_loca_end) {
        lv_obj_add_flag(s_page_loca_end, LV_OBJ_FLAG_HIDDEN);
    }
    if (setting_get_root()) {
        lv_obj_add_flag(setting_get_root(), LV_OBJ_FLAG_HIDDEN);
    }
}

static void show_page(lv_obj_t * page)
{
    hide_all_pages();
    if (page) {
        lv_obj_clear_flag(page, LV_OBJ_FLAG_HIDDEN);
    }
}

static bool home_main_visible(void)
{
    if (page_main == NULL) {
        return false;
    }

    return !lv_obj_has_flag(page_main, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t * create_func_page(lv_obj_t * parent, const char * title)
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

    lv_obj_t * label = lv_label_create(page);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    lv_obj_center(label);

    add_back_button(page);

    return page;
}

static void func_page_back_btn_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    show_page(page_main);
}

static void add_back_button(lv_obj_t * page)
{
    lv_obj_t * back_icon = lv_label_create(page);
    lv_label_set_text(back_icon, "\xEE\x98\x81");
    lv_obj_set_style_text_font(back_icon, &back_btn, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(back_icon, 20, 20);

    lv_obj_add_flag(back_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(back_icon, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(back_icon, func_page_back_btn_cb, LV_EVENT_CLICKED, NULL);
}

void screen_home_set_clean_pending(bool pending)
{
    s_pending_start.self_clean_pending = pending;
    if (!pending) {
        screen_home_clean_flow_stop(&s_clean_flow);
    }
    if (s_home_variant == HOME_VARIANT_DISCONNECTED) {
        bucket_refresh_card();
    } else {
        pending_refresh_card();
    } 
}

void screen_home_show_loca_end(void)
{
    if (s_page_loca_end == NULL) {
        return;
    }

    page_loca_end_show();
}

static void setting_back_to_home_cb(void)
{
    show_page(page_main);
}
