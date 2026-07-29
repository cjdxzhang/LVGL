#include "system_manager.h"
#include "serial.h"
#include "custom_imgbtn.h"
#include "custom.h"
#include "widgets_init.h"
#include "app_manager.h"
#include "app_log.h"
#include "lv_port_init.h"
#include "backlight_control.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#define SYSTEM_MANAGER_LOG_USER(...) APP_LOG_USER("SYSTEM_MANAGER", __VA_ARGS__)
#define SYSTEM_MANAGER_LOG_WARN(...) APP_LOG_WARN("SYSTEM_MANAGER", __VA_ARGS__)
#define SYSTEM_MANAGER_LOG_ERROR(...) APP_LOG_ERROR("SYSTEM_MANAGER", __VA_ARGS__)

#define MODE_SETTINGS_DIR "./temp"
#define MODE_SETTINGS_PATH "./temp/mode_settings.bin"
#define MODE_SETTINGS_MAGIC 0x4d4f4445u
#define MODE_SETTINGS_VERSION 2u
#define LOCATION_SETTINGS_DIR "./temp"
#define LOCATION_SETTINGS_PATH "./temp/location_settings.bin"
#define LOCATION_SETTINGS_MAGIC 0x4c4f4341u
#define LOCATION_SETTINGS_VERSION 1u
#define CONF_MODE_GRID_COLS 3
#define CONF_MODE_BTN_WIDTH 160
#define CONF_MODE_BTN_HEIGHT 150
#define CONF_MODE_GRID_X_GAP 15
#define CONF_MODE_GRID_Y_GAP 15
#define CONF_MODE_GRID_X_START 10
#define CONF_MODE_GRID_Y_START 10
#define CONF_LOCATION_INFO_Y 4
#define CONF_LOCATION_INFO_HEIGHT 63
#define CONF_LOCATION_ROW_Y_START 82
#define CONF_LOCATION_ROW_HEIGHT 38
#define CONF_LOCATION_ROW_GAP 18
#define CONF_LOCATION_LABEL_X 0
#define CONF_LOCATION_LABEL_Y_OFFSET 3
#define CONF_LOCATION_LABEL_WIDTH 92
#define CONF_LOCATION_LABEL_HEIGHT 32
#define CONF_LOCATION_NAME_X 104
#define CONF_LOCATION_NAME_WIDTH 128
#define CONF_LOCATION_DROPDOWN_X 237
#define CONF_LOCATION_DROPDOWN_WIDTH 206
#define CONF_LOCATION_FIELD_HEIGHT 38
#define TEMP_MIN 35u
#define TEMP_MAX 48u
#define BUCKET_TEMP_FALLBACK TEMP_MIN
#define BUCKET_TEMP_CONFIRM_MS 3000
#define BUCKET_TEMP_RESTORE_MS 5000
#define BUCKET_TEMP_BLINK_MS 400
#define BUCKET_TEMP_BLINK_TOGGLE_COUNT 4
#define LINK_STATUS_DETACHED 0x00u
#define LINK_STATUS_DOCKED 0x01u
#define UI_ROTATION_DETACHED 0
#define UI_ROTATION_DOCKED 180
#define BUCKET_AUTO_TIMER_DELAY_MS 1000u
#define BUCKET_COUNTDOWN_REFRESH_MS 250u
#define BUCKET_TIMER_OPTION_COUNT 5u
#define BUCKET_TIMER_CONFIRM_MS 3000u
#define BUCKET_TIMER_BLINK_MS 400u
#define BUCKET_TIMER_BLINK_TOGGLE_COUNT 4u
#define BUCKET_STANDBY_HOLD_MS 3000u
#define BUCKET_STANDBY_REFRESH_MS 30u
#define BUCKET_STANDBY_DOUBLE_TAP_MS 600u
#define BUCKET_STANDBY_ARC_START_ANGLE 0
#define BUCKET_STANDBY_ARC_COMPLETE_ANGLE 359

#define CONF_LOCATION_MODE_OPTIONS_LEN ((SYSTEM_MODE_NAME_LEN + 1) * SYSTEM_MODE_MAX_COUNT)

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t count;
    system_mode_t modes[SYSTEM_MODE_MAX_COUNT];
} system_mode_store_t;

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t count;
    system_location_t locations[SYSTEM_LOCATION_MAX_COUNT];
} system_location_store_t;

// 用于存储需要动态更新的控件句柄（以便在定时器中修改）
typedef struct
{
    lv_obj_t *arc;
    lv_obj_t *label_percent;
    lv_obj_t *label_status;
} cleaning_ui_t;

typedef struct
{
    int16_t actual_temp;
    bool has_actual_temp;
    int16_t target_temp;
    bool editing;
    lv_timer_t *confirm_timer;
    lv_timer_t *restore_timer;
    lv_timer_t *blink_timer;
    lv_obj_t *display_label;
    uint8_t blink_count;
    bool blink_visible;
} bucket_temp_edit_state_t;

typedef struct
{
    uint32_t remaining_seconds;
    uint64_t deadline_ms;
    bool active;
    bool has_value;
    bool expiry_handled;
    bool editing;
    size_t button_option_index;
    uint8_t blink_count;
    bool blink_visible;
    lv_timer_t *refresh_timer;
    lv_timer_t *delayed_start_timer;
    lv_timer_t *confirm_timer;
    lv_timer_t *blink_timer;
    lv_obj_t *display_label;
} bucket_countdown_state_t;

typedef struct
{
    bool pressing;
    bool submitted;
    bool sleeping;
    uint8_t wake_tap_count;
    uint64_t last_wake_tap_ms;
    uint64_t pressed_at_ms;
    lv_timer_t *progress_timer;
    lv_obj_t *overlay;
    lv_obj_t *prompt_panel;
    lv_obj_t *prompt_label;
    lv_obj_t *progress_arc;
} bucket_standby_state_t;

static cleaning_ui_t cleaning_ui;
static bucket_temp_edit_state_t g_bucket_temp_edit = {0};
static bucket_countdown_state_t g_bucket_countdown = {0};
static bucket_standby_state_t g_bucket_standby = {0};
static int16_t g_applied_link_status = -1;

/* 全局变量定义（在头文件中用 extern 声明，在此处给出唯一定义） */
int16_t last_link_status =
    LINK_STATUS_DOCKED; /* 开机默认位于基站，收到有效帧后再更新 */
bool has_mcu_status_report =
    false; /* 是否已收到有效 MCU 状态帧，避免默认 LINK_STATUS 误触发页面切换 */
int16_t temp_set = 35;        /* 温度设置值，单位 °C */
uint32_t timer_set = 600;    /* 定时设置值，单位 秒 */
uint32_t remaining_seconds = 0; /* 桶体本地倒计时剩余秒数 */
int16_t current_temp = 0;     /* 当前温度，单位 °C */
int16_t current_water_level = 0; /* 当前水位，范围 0-3 */
int water_level = 0;   /* 目标水位挡位，范围 0-3 */
bool use_drug1 = false;        /* 是否使用药物1 */
bool use_drug2 = false;      /* 是否使用药物2 */
bool is_self_cleaning = false; /* 是否正在自清洁 */

/* 串口协议帧解析出的桶/底座状态 */
uint8_t bucket_main_status = 0;

/**

主状态值    主状态名称 子状态说明
0   上电中   持续时间S
1   自检中   持续时间S
2   关机  持续时间S
3   待机  持续时间S
4   自动上水中 剩余时间MIN
5   注水完成保温中   剩余时间MIN
6   注水完成等待排水(不保温) 剩余时间MIN
7   清洁喷淋中(热水+清洁液)   剩余时间MIN
8   清洁排水1中    剩余时间MIN
9   清水喷淋中 剩余时间MIN
10  清洁排水2中    剩余时间MIN
11  清洁热风烘干中   剩余时间MIN
12  强制排水中 剩余时间MIN
13  单独清洁喷淋中(热水+清洁液) 剩余时间MIN
14  单独清水喷淋中   剩余时间MIN
15  单独热风烘干中   剩余时间MIN
 */
uint8_t base_main_status = 0;

/* 工作时属性 */
bool constant_temperature = false; /* 恒温 */
bool sterilization = false; /* 除菌 */
int massage_intensity = 0; /* 按摩强度 */

/**定位属性 */
int current_location = 0; /* 0--基站 1，2,3,4,5.....当前定位索引 */
int going_to_location = 0; /* 目标定位索引 */

bool _is_index_page_initialized = false;
bool _is_setting_page_initialized = false;
bool _is_location_page_initialized = false;
bool _is_working_page_initialized = false;
bool _is_conf_mode_page_initialized = false;
bool _is_conf_location_page_initialized = false;
bool _is_conf_mode_detail_page_initialized = false;
bool _is_preparing_page_initialized = false;

/**全局变量，记录是否正在排水 */
bool g_is_draining = false;

static system_mode_t g_mode_settings[SYSTEM_MODE_MAX_COUNT];
system_location_t g_location_settings[SYSTEM_LOCATION_MAX_COUNT];
static size_t g_mode_count = 0;
size_t g_location_count = 0;
int current_selected_mode_index = -1;
int current_selected_location_index = 0;
static lv_obj_t *g_conf_mode_cont_2 = NULL;

static uint32_t g_preparing_countdown_remaining = 0;
static lv_obj_t *g_working_pause_btn = NULL;
static lv_obj_t *g_working_play_btn = NULL;
static lv_obj_t *g_preparing_btn_location = NULL;
static lv_timer_t *g_preparing_page_timer = NULL;
static lv_timer_t *g_working_return_dialog_timer = NULL;
static lv_timer_t *g_working_timer = NULL;
lv_timer_t *g_auto_drain_timer = NULL;
static int32_t g_working_return_dialog_countdown = 0;

/* 等待基站连接的重试计数 */
#define WAIT_LINK_MAX_RETRY 5
static int g_wait_link_retry_count = 0;

/* ========== 机器人导航相关 ========== */
static float g_nav_target_x = 0.0f;       /* 导航目标X坐标 */
static float g_nav_target_y = 0.0f;       /* 导航目标Y坐标 */
static float g_nav_target_z = 0.0f;       /* 导航目标Z朝向 */
static int   g_nav_timeout_count = 0;     /* 导航超时计数（秒） */
#define NAV_TIMEOUT_SEC   120              /* 导航最大等待秒数 */
#define NAV_ARRIVED_TOL   0.5f             /* 到达判定距离阈值（米） */

static lv_timer_t *g_nav_check_timer = NULL; /* 导航状态轮询定时器 */

static int persist_mode_settings(void);

static void build_mode_dropdown_options(char *options, size_t options_size);
static void conf_location_name_changed(lv_event_t *e);
static void conf_location_default_mode_changed(lv_event_t *e);
static void conf_location_apply_label_style(lv_obj_t *label);
static void conf_location_apply_textarea_style(lv_ui *ui, lv_obj_t *textarea);
static void conf_location_apply_dropdown_style(lv_obj_t *dropdown);
static void conf_mode_detail_keyboard_cleanup(lv_ui *ui);
static void conf_mode_detail_screen_lifecycle_cb(lv_event_t *e);
static lv_obj_t *conf_location_create_info_label(lv_obj_t *parent);
static void display_preparing_settings(lv_ui *ui);
static void refresh_auto_water_status(lv_timer_t *timer);
static void update_self_cleaning_progress(lv_ui *ui);
static void preparing_screen_lifecycle_cb(lv_event_t *e);
static void working_screen_lifecycle_cb(lv_event_t *e);
static bool bucket_temp_valid(int16_t temp);
static int16_t bucket_temp_clamp(int temp);
static int16_t bucket_temp_base_value(void);
static void bucket_temp_set_label(lv_obj_t *label, int16_t temp);
static void bucket_temp_show_actual(lv_obj_t *label);
static void bucket_temp_start_edit(lv_obj_t *label, int16_t temp);
static void bucket_temp_clear_edit(lv_obj_t *label);
static void bucket_temp_step(lv_ui *ui, int delta);
static void bucket_temp_reset_confirm_timer(void);
static void bucket_temp_reset_restore_timer(void);
static void bucket_temp_cancel_blink(void);
static void bucket_temp_confirm_timer_cb(lv_timer_t *timer);
static void bucket_temp_restore_timer_cb(lv_timer_t *timer);
static void bucket_temp_blink_timer_cb(lv_timer_t *timer);
static void bucket_timer_set_label(lv_obj_t *label, uint32_t seconds);
static void bucket_timer_step(lv_ui *ui);
static void bucket_countdown_start(uint32_t seconds);
static void bucket_countdown_refresh(void);
static void bucket_countdown_reset_confirm_timer(void);
static void bucket_countdown_cancel_edit(void);
static void bucket_auto_start_cancel(void);
static void bucket_auto_start_detached(void);
static void bucket_working_sync_function_icons(lv_ui *ui);
static void bucket_standby_cancel_press(void);
static void bucket_standby_page_cleanup(void);

static bool working_return_dialog_obj_is_valid(const lv_obj_t *obj)
{
    return obj != NULL && lv_obj_is_valid(obj);
}

static void working_return_dialog_reset_no_label(void)
{
    if (working_return_dialog_obj_is_valid(guider_ui.working_btn_no_label))
    {
        lv_label_set_text(guider_ui.working_btn_no_label, "否");
    }
}

static void working_return_dialog_cleanup(void)
{
    if (g_working_return_dialog_timer != NULL)
    {
        lv_timer_delete(g_working_return_dialog_timer);
        g_working_return_dialog_timer = NULL;
    }

    g_working_return_dialog_countdown = 0;
    working_return_dialog_reset_no_label();
}

static void working_return_dialog_button_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    working_return_dialog_cleanup();
}

static void working_return_dialog_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (!working_return_dialog_obj_is_valid(guider_ui.working_dialog) ||
            !working_return_dialog_obj_is_valid(guider_ui.working_btn_no))
    {
        working_return_dialog_cleanup();
        return;
    }

    g_working_return_dialog_countdown--;

    if (g_working_return_dialog_countdown <= 0)
    {
        working_return_dialog_cleanup();
        lv_obj_send_event(guider_ui.working_btn_no, LV_EVENT_CLICKED, NULL);
        return;
    }

    if (working_return_dialog_obj_is_valid(guider_ui.working_btn_no_label))
    {
        lv_label_set_text_fmt(guider_ui.working_btn_no_label,
                              "否\n(%lds后自动)",
                              (long)g_working_return_dialog_countdown);
    }
}

static void working_return_dialog_start_countdown(void)
{
    working_return_dialog_cleanup();

    if (!working_return_dialog_obj_is_valid(guider_ui.working_btn_yes) ||
            !working_return_dialog_obj_is_valid(guider_ui.working_btn_no) ||
            !working_return_dialog_obj_is_valid(guider_ui.working_btn_no_label))
    {
        return;
    }

    lv_obj_set_style_text_font(guider_ui.working_btn_yes,
                               custom_get_compact_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(guider_ui.working_btn_no,
                               custom_get_compact_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(guider_ui.working_btn_no_label,
                               custom_get_compact_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_remove_event_cb(guider_ui.working_btn_yes, working_return_dialog_button_event_cb);
    lv_obj_remove_event_cb(guider_ui.working_btn_no, working_return_dialog_button_event_cb);
    lv_obj_add_event_cb(guider_ui.working_btn_yes, working_return_dialog_button_event_cb,
                        LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(guider_ui.working_btn_no, working_return_dialog_button_event_cb,
                        LV_EVENT_CLICKED, NULL);

    g_working_return_dialog_countdown = 5;
    lv_label_set_text_fmt(guider_ui.working_btn_no_label,
                          "否\n(%lds后自动)",
                          (long)g_working_return_dialog_countdown);
    g_working_return_dialog_timer = lv_timer_create(working_return_dialog_timer_cb, 1000, NULL);
}

/* 前向声明 */
static void preparing_move_finished(lv_timer_t *timer);

/* 自动排水定时器回调包装（lv_timer_cb_t 需要 lv_timer_t* 参数） */
static void auto_drain_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    serial_base_force_drain();
}

/* ========== 导航轮询定时器回调 ========== */
static void preparing_nav_check_timer_cb(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }


    /* 主动发送 t=42；协议线程只更新 app_manager 中的线程安全快照。 */
    (void)app_manager_robot_request_status();

    /* LVGL 线程只读取状态拷贝，不由协议回调直接操作界面。 */
    robot_chassis_status_t state = {0};
    if (app_manager_robot_get_status(&state) != ROBOT_CHASSIS_OK ||
            (state.valid_fields & ROBOT_CHASSIS_STATUS_VALID_POSE) == 0U)
    {
        goto timeout_check;
    }

    /* 计算当前位置与目标点的欧几里得距离 */
    float dx = state.pose.x - g_nav_target_x;
    float dy = state.pose.y - g_nav_target_y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (dist <= NAV_ARRIVED_TOL && last_link_status == 0)
    {
        /* 判定是否到达目的地 并且机器人与基站断开连接*/
        SYSTEM_MANAGER_LOG_USER("到达导航目的地: target=(%.2f, %.2f)",
                                g_nav_target_x, g_nav_target_y);
        if (g_nav_check_timer != NULL)
        {
            lv_timer_delete(g_nav_check_timer);
            g_nav_check_timer = NULL;
        }
        g_nav_timeout_count = 0;
        preparing_move_finished(timer);
        return;
    }

timeout_check:
    /* 超时检查 */
    g_nav_timeout_count++;
    if (g_nav_timeout_count >= NAV_TIMEOUT_SEC)
    {
        SYSTEM_MANAGER_LOG_WARN("导航超时，取消导航: timeout=%ds", NAV_TIMEOUT_SEC);
        if (g_nav_check_timer != NULL)
        {
            lv_timer_delete(g_nav_check_timer);
            g_nav_check_timer = NULL;
        }
        g_nav_timeout_count = 0;
        /* 超时后显示提示，回到首页 */
        show_message_dialog(ui->preparing, "导航超时，请检查机器人状态");
        navigate_to_screen(UI_SCREEN_INDEX);
    }
}

void preparing_page_cleanup(void)
{
    /* 清理导航轮询定时器 */
    if (g_nav_check_timer != NULL)
    {
        lv_timer_delete(g_nav_check_timer);
        g_nav_check_timer = NULL;
    }
    g_nav_timeout_count = 0;
    g_wait_link_retry_count = 0;

    if (g_preparing_page_timer != NULL)
    {
        lv_timer_delete(g_preparing_page_timer);
        g_preparing_page_timer = NULL;
    }

    if (g_preparing_btn_location != NULL)
    {
        if (lv_obj_is_valid(g_preparing_btn_location))
        {
            lv_obj_del(g_preparing_btn_location);
        }
        g_preparing_btn_location = NULL;
    }

    /* stop_dialog_cleanup 内部已做对象有效性检查，可直接调用 */
    stop_dialog_cleanup();
}
/**暂停工作，根据当前定位，返回到前一个页面 */
void preparing_page_stop(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    if (current_location == 0 && last_link_status == 0x01)
    {
        //判定位置处于基站位置，并且与基站已经连接
        preparing_to_index(e);
    }
    else
    {
        preparing_moving_to_location(e);
    }
}
static void preparing_screen_lifecycle_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_UNLOAD_START || code == LV_EVENT_DELETE)
    {
        preparing_page_cleanup();
    }
}

static void working_screen_lifecycle_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_UNLOAD_START || code == LV_EVENT_DELETE)
    {
        bucket_standby_page_cleanup();
        bucket_temp_clear_edit(NULL);
        g_bucket_countdown.display_label = NULL;
        if (g_working_timer != NULL)
        {
            lv_timer_del(g_working_timer);
            g_working_timer = NULL;
        }
        working_return_dialog_cleanup();
    }
}

static bool bucket_temp_valid(int16_t temp)
{
    return temp >= TEMP_MIN && temp <= TEMP_MAX;
}

// 将温度值限制在 TEMP_MIN 和 TEMP_MAX 范围内，并返回限制后的值
static int16_t bucket_temp_clamp(int temp)
{
    if (temp < TEMP_MIN)
    {
        temp = TEMP_MIN;
    }
    if (temp > TEMP_MAX)
    {
        temp = TEMP_MAX;
    }
    return (int16_t)temp;
}

static int16_t bucket_temp_base_value(void)
{
    // 正在编辑时，使用上次编辑的目标温度 target_temp
    if (g_bucket_temp_edit.editing && bucket_temp_valid(g_bucket_temp_edit.target_temp))
    {
        return g_bucket_temp_edit.target_temp;
    }
    // 否则使用 MCU 最近上传的有效实际温度
    if (g_bucket_temp_edit.has_actual_temp && bucket_temp_valid(g_bucket_temp_edit.actual_temp))
    {
        return g_bucket_temp_edit.actual_temp;
    }
    // 否则使用全局实际温度 current_temp
    if (bucket_temp_valid(current_temp))
    {
        return current_temp;
    }
    return BUCKET_TEMP_FALLBACK;
}

static void bucket_temp_set_label(lv_obj_t *label, int16_t temp)
{
    char temp_str[16];

    if (label == NULL || !lv_obj_is_valid(label))
    {
        return;
    }

    snprintf(temp_str, sizeof(temp_str), "%d℃", temp);
    lv_label_set_text(label, temp_str);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void bucket_temp_cancel_blink(void)
{
    if (g_bucket_temp_edit.blink_timer != NULL)
    {
        lv_timer_del(g_bucket_temp_edit.blink_timer);
        g_bucket_temp_edit.blink_timer = NULL;
    }
    g_bucket_temp_edit.blink_count = 0;
    g_bucket_temp_edit.blink_visible = true;
}

static void bucket_temp_show_actual(lv_obj_t *label)
{
    bucket_temp_cancel_blink();

    if (label == NULL)
    {
        label = g_bucket_temp_edit.display_label;
    }
    if (g_bucket_temp_edit.has_actual_temp)
    {
        bucket_temp_set_label(label, g_bucket_temp_edit.actual_temp);
    }
    else if (bucket_temp_valid(current_temp))
    {
        bucket_temp_set_label(label, current_temp);
    }
    else
    {
        bucket_temp_set_label(label, BUCKET_TEMP_FALLBACK);
    }
}

static void bucket_temp_confirm_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    g_bucket_temp_edit.confirm_timer = NULL;
    if (!g_bucket_temp_edit.editing ||
            g_bucket_temp_edit.display_label == NULL ||
            !lv_obj_is_valid(g_bucket_temp_edit.display_label))
    {
        return;
    }

    bucket_temp_cancel_blink();
    g_bucket_temp_edit.blink_count = 0;
    g_bucket_temp_edit.blink_visible = true;
    g_bucket_temp_edit.blink_timer = lv_timer_create(bucket_temp_blink_timer_cb,
                                     BUCKET_TEMP_BLINK_MS,
                                     NULL);
    if (g_bucket_temp_edit.blink_timer != NULL)
    {
        lv_timer_set_repeat_count(g_bucket_temp_edit.blink_timer,
                                  BUCKET_TEMP_BLINK_TOGGLE_COUNT);
    }
}

static void bucket_temp_restore_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    g_bucket_temp_edit.restore_timer = NULL;
    g_bucket_temp_edit.editing = false;
    bucket_temp_show_actual(g_bucket_temp_edit.display_label);
}

static void bucket_temp_blink_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *label = g_bucket_temp_edit.display_label;

    LV_UNUSED(timer);

    if (label == NULL || !lv_obj_is_valid(label))
    {
        g_bucket_temp_edit.blink_timer = NULL;
        return;
    }

    g_bucket_temp_edit.blink_visible = !g_bucket_temp_edit.blink_visible;
    g_bucket_temp_edit.blink_count++;
    if (g_bucket_temp_edit.blink_visible)
    {
        bucket_temp_set_label(label, g_bucket_temp_edit.target_temp);
    }
    else
    {
        lv_label_set_text(label, "  ℃");
    }

    if (g_bucket_temp_edit.blink_count >= BUCKET_TEMP_BLINK_TOGGLE_COUNT)
    {
        g_bucket_temp_edit.blink_timer = NULL;
        bucket_temp_set_label(label, g_bucket_temp_edit.target_temp);
    }
}

static void bucket_temp_reset_confirm_timer(void)
{
    if (g_bucket_temp_edit.confirm_timer != NULL)
    {
        lv_timer_del(g_bucket_temp_edit.confirm_timer);
        g_bucket_temp_edit.confirm_timer = NULL;
    }

    g_bucket_temp_edit.confirm_timer = lv_timer_create(bucket_temp_confirm_timer_cb,
                                       BUCKET_TEMP_CONFIRM_MS,
                                       NULL);
    if (g_bucket_temp_edit.confirm_timer != NULL)
    {
        lv_timer_set_repeat_count(g_bucket_temp_edit.confirm_timer, 1);
    }
}

static void bucket_temp_reset_restore_timer(void)
{
    if (g_bucket_temp_edit.restore_timer != NULL)
    {
        lv_timer_del(g_bucket_temp_edit.restore_timer);
        g_bucket_temp_edit.restore_timer = NULL;
    }

    g_bucket_temp_edit.restore_timer = lv_timer_create(bucket_temp_restore_timer_cb,
                                       BUCKET_TEMP_RESTORE_MS,
                                       NULL);
    if (g_bucket_temp_edit.restore_timer != NULL)
    {
        lv_timer_set_repeat_count(g_bucket_temp_edit.restore_timer, 1);
    }
}

static void bucket_temp_start_edit(lv_obj_t *label, int16_t temp)
{
    bucket_temp_cancel_blink();

    g_bucket_temp_edit.target_temp = temp;
    g_bucket_temp_edit.editing = true;
    g_bucket_temp_edit.display_label = label;
    temp_set = g_bucket_temp_edit.target_temp;

    bucket_temp_set_label(label, g_bucket_temp_edit.target_temp);
    bucket_temp_reset_confirm_timer();
    bucket_temp_reset_restore_timer();
}

static void bucket_temp_clear_edit(lv_obj_t *label)
{
    g_bucket_temp_edit.editing = false;
    bucket_temp_cancel_blink();

    if (g_bucket_temp_edit.confirm_timer != NULL)
    {
        lv_timer_del(g_bucket_temp_edit.confirm_timer);
        g_bucket_temp_edit.confirm_timer = NULL;
    }
    if (g_bucket_temp_edit.restore_timer != NULL)
    {
        lv_timer_del(g_bucket_temp_edit.restore_timer);
        g_bucket_temp_edit.restore_timer = NULL;
    }

    if (label != NULL)
    {
        g_bucket_temp_edit.display_label = label;
    }
    bucket_temp_show_actual(g_bucket_temp_edit.display_label);
}

static void bucket_temp_step(lv_ui *ui, int delta)
{
    int16_t target;

    if (ui == NULL || ui->working_label_temperature == NULL)
    {
        return;
    }
    // 根据当前编辑状态和delta，计算目标温度
    target = bucket_temp_clamp((int)bucket_temp_base_value() + delta);
    bucket_temp_start_edit(ui->working_label_temperature, target);

    // 恒温才下发恒温命令
    if (constant_temperature)
    {
        serial_bucket_heat();
    }
}

static void bucket_timer_set_label(lv_obj_t *label, uint32_t seconds)
{
    char time_str[24];
    uint32_t min = seconds / 60u;
    uint32_t sec = seconds % 60u;

    if (label == NULL || !lv_obj_is_valid(label))
    {
        return;
    }

    snprintf(time_str, sizeof(time_str), "%u:%02u",
             (unsigned int)min, (unsigned int)sec);
    lv_label_set_text(label, time_str);
}

static uint64_t bucket_countdown_monotonic_ms(void)
{
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
    {
        return 0u;
    }
    return ((uint64_t)now.tv_sec * 1000u) + ((uint64_t)now.tv_nsec / 1000000u);
}

static void bucket_countdown_update_label(void)
{
    if (g_bucket_countdown.display_label == NULL ||
            !lv_obj_is_valid(g_bucket_countdown.display_label))
    {
        g_bucket_countdown.display_label = NULL;
        return;
    }

    bucket_timer_set_label(g_bucket_countdown.display_label,
                           g_bucket_countdown.has_value ? remaining_seconds : timer_set);
}

static void bucket_countdown_set_label_visible(bool visible)
{
    lv_obj_t *label = g_bucket_countdown.display_label;

    g_bucket_countdown.blink_visible = visible;
    if (label == NULL || !lv_obj_is_valid(label))
    {
        return;
    }

    lv_obj_set_style_text_opa(label, visible ? LV_OPA_COVER : LV_OPA_TRANSP,
                              LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void bucket_countdown_cancel_blink(void)
{
    if (g_bucket_countdown.blink_timer != NULL)
    {
        lv_timer_del(g_bucket_countdown.blink_timer);
        g_bucket_countdown.blink_timer = NULL;
    }
    g_bucket_countdown.blink_count = 0u;
    bucket_countdown_set_label_visible(true);
}

static void bucket_countdown_cancel_edit(void)
{
    if (g_bucket_countdown.confirm_timer != NULL)
    {
        lv_timer_del(g_bucket_countdown.confirm_timer);
        g_bucket_countdown.confirm_timer = NULL;
    }
    bucket_countdown_cancel_blink();
    g_bucket_countdown.editing = false;
    g_bucket_countdown.button_option_index = 0u;
}

static void bucket_countdown_blink_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    bucket_countdown_set_label_visible(!g_bucket_countdown.blink_visible);
    g_bucket_countdown.blink_count++;
    if (g_bucket_countdown.blink_count < BUCKET_TIMER_BLINK_TOGGLE_COUNT)
    {
        return;
    }

    g_bucket_countdown.blink_timer = NULL;
    bucket_countdown_set_label_visible(true);
    g_bucket_countdown.editing = false;
    g_bucket_countdown.button_option_index = 0u;
    bucket_countdown_start(timer_set);
    SYSTEM_MANAGER_LOG_USER("桶体定时确认完成，开始倒计时: %us",
                            (unsigned int)timer_set);
}

static void bucket_countdown_confirm_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    g_bucket_countdown.confirm_timer = NULL;

    if (!g_bucket_countdown.editing)
    {
        return;
    }

    bucket_countdown_cancel_blink();
    g_bucket_countdown.blink_timer = lv_timer_create(bucket_countdown_blink_timer_cb,
                                     BUCKET_TIMER_BLINK_MS,
                                     NULL);
    if (g_bucket_countdown.blink_timer != NULL)
    {
        lv_timer_set_repeat_count(g_bucket_countdown.blink_timer,
                                  BUCKET_TIMER_BLINK_TOGGLE_COUNT);
        return;
    }

    g_bucket_countdown.editing = false;
    g_bucket_countdown.button_option_index = 0u;
    bucket_countdown_start(timer_set);
}

static void bucket_countdown_reset_confirm_timer(void)
{
    if (g_bucket_countdown.confirm_timer != NULL)
    {
        lv_timer_del(g_bucket_countdown.confirm_timer);
        g_bucket_countdown.confirm_timer = NULL;
    }
    bucket_countdown_cancel_blink();

    g_bucket_countdown.confirm_timer = lv_timer_create(bucket_countdown_confirm_timer_cb,
                                       BUCKET_TIMER_CONFIRM_MS,
                                       NULL);
    if (g_bucket_countdown.confirm_timer != NULL)
    {
        lv_timer_set_repeat_count(g_bucket_countdown.confirm_timer, 1);
        return;
    }

    SYSTEM_MANAGER_LOG_ERROR("创建桶体定时确认 timer 失败，直接启动倒计时");
    g_bucket_countdown.editing = false;
    g_bucket_countdown.button_option_index = 0u;
    bucket_countdown_start(timer_set);
}

static void bucket_working_sync_function_icons(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }

    if (ui->working_img_constant_temperature != NULL &&
            lv_obj_is_valid(ui->working_img_constant_temperature))
    {
        if (constant_temperature)
        {
            lv_obj_remove_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (ui->working_img_sterilization != NULL &&
            lv_obj_is_valid(ui->working_img_sterilization))
    {
        if (sterilization)
        {
            lv_obj_remove_flag(ui->working_img_sterilization, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(ui->working_img_sterilization, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (ui->working_img_massage_intensity0 != NULL &&
            lv_obj_is_valid(ui->working_img_massage_intensity0))
    {
        if (massage_intensity >= 1)
        {
            lv_obj_remove_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (ui->working_img_massage_intensity1 != NULL &&
            lv_obj_is_valid(ui->working_img_massage_intensity1))
    {
        if (massage_intensity >= 2)
        {
            lv_obj_remove_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (ui->working_img_massage_intensity2 != NULL &&
            lv_obj_is_valid(ui->working_img_massage_intensity2))
    {
        if (massage_intensity >= 3)
        {
            lv_obj_remove_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void bucket_working_clear_function_state(void)
{
    constant_temperature = false;
    sterilization = false;
    massage_intensity = 0;
    bucket_working_sync_function_icons(&guider_ui);
}

static void bucket_standby_remove_overlay(bool async_delete)
{
    lv_obj_t *overlay = g_bucket_standby.overlay;

    g_bucket_standby.overlay = NULL;
    g_bucket_standby.prompt_panel = NULL;
    g_bucket_standby.prompt_label = NULL;
    g_bucket_standby.progress_arc = NULL;
    if (overlay == NULL || !lv_obj_is_valid(overlay))
    {
        return;
    }

    if (async_delete)
    {
        lv_obj_delete_async(overlay);
    }
    else
    {
        lv_obj_delete(overlay);
    }
}

static void bucket_standby_finish_wake(bool async_delete)
{
    g_bucket_standby.pressing = false;
    g_bucket_standby.submitted = false;
    g_bucket_standby.sleeping = false;
    g_bucket_standby.wake_tap_count = 0u;
    g_bucket_standby.last_wake_tap_ms = 0u;
    g_bucket_standby.pressed_at_ms = 0u;
    bucket_standby_remove_overlay(async_delete);
}

static void bucket_standby_overlay_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    uint64_t now_ms;

    if (code != LV_EVENT_CLICKED || !g_bucket_standby.sleeping)
    {
        return;
    }

    now_ms = bucket_countdown_monotonic_ms();
    if (now_ms == 0u || g_bucket_standby.last_wake_tap_ms == 0u ||
            now_ms - g_bucket_standby.last_wake_tap_ms > BUCKET_STANDBY_DOUBLE_TAP_MS)
    {
        g_bucket_standby.wake_tap_count = 1u;
        g_bucket_standby.last_wake_tap_ms = now_ms;
        SYSTEM_MANAGER_LOG_USER("检测到待机唤醒第一次点击");
        return;
    }

    g_bucket_standby.wake_tap_count++;
    if (g_bucket_standby.wake_tap_count < 2u)
    {
        return;
    }
    if (!backlight_control_wake())
    {
        SYSTEM_MANAGER_LOG_ERROR("恢复屏幕背光失败，保留待机双击遮罩");
        g_bucket_standby.wake_tap_count = 0u;
        g_bucket_standby.last_wake_tap_ms = 0u;
        return;
    }

    bucket_temp_show_actual(guider_ui.working_label_temperature);
    bucket_countdown_update_label();
    bucket_working_sync_function_icons(&guider_ui);
    SYSTEM_MANAGER_LOG_USER("检测到待机双击，恢复屏幕背光和页面操作");
    bucket_standby_finish_wake(true);
}

static bool bucket_standby_create_overlay(void)
{
    lv_obj_t *overlay;
    lv_obj_t *panel;
    lv_obj_t *label;
    lv_obj_t *bold_label;
    lv_obj_t *arc;

    if (g_bucket_standby.overlay != NULL && lv_obj_is_valid(g_bucket_standby.overlay))
    {
        return true;
    }

    overlay = lv_obj_create(lv_layer_top());
    if (overlay == NULL)
    {
        return false;
    }
    lv_obj_remove_style_all(overlay);
    lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_30, LV_PART_MAIN);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(overlay, bucket_standby_overlay_event_cb, LV_EVENT_ALL, NULL);

    panel = lv_obj_create(overlay);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, 340, 70);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_set_style_radius(panel, 28, LV_PART_MAIN);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x111111), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_border_opa(panel, LV_OPA_60, LV_PART_MAIN);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    bold_label = lv_label_create(panel);
    lv_label_set_text(bold_label, "继续长按将进入待机");
    lv_obj_set_style_text_font(bold_label, custom_get_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bold_label, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(bold_label, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(bold_label, LV_ALIGN_CENTER, 1, 1);

    label = lv_label_create(panel);
    lv_label_set_text(label, "继续长按将进入待机");
    lv_obj_set_style_text_font(label, custom_get_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(label, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    arc = lv_arc_create(panel);
    lv_obj_set_size(arc, 42, 42);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(arc, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xffffff),
                               LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_angles(arc, BUCKET_STANDBY_ARC_START_ANGLE, 30);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_pos(arc, 6, 14);

    g_bucket_standby.overlay = overlay;
    g_bucket_standby.prompt_panel = panel;
    g_bucket_standby.prompt_label = label;
    g_bucket_standby.progress_arc = arc;
    return true;
}

static void bucket_countdown_stop_for_standby(void)
{
    bucket_auto_start_cancel();
    bucket_countdown_cancel_edit();
    if (g_bucket_countdown.refresh_timer != NULL)
    {
        lv_timer_delete(g_bucket_countdown.refresh_timer);
        g_bucket_countdown.refresh_timer = NULL;
    }

    remaining_seconds = 0u;
    g_bucket_countdown.remaining_seconds = 0u;
    g_bucket_countdown.deadline_ms = 0u;
    g_bucket_countdown.active = false;
    g_bucket_countdown.has_value = true;
    g_bucket_countdown.expiry_handled = true;
    g_bucket_countdown.editing = false;
    g_bucket_countdown.button_option_index = 0u;
    bucket_countdown_update_label();
}

static void bucket_standby_commit(void)
{
    if (g_bucket_standby.submitted)
    {
        return;
    }
    g_bucket_standby.submitted = true;
    g_bucket_standby.pressing = false;

    bucket_temp_clear_edit(guider_ui.working_label_temperature);
    bucket_countdown_stop_for_standby();
    bucket_working_clear_function_state();
    SYSTEM_MANAGER_LOG_USER("暂停键长按满3秒，下发 A7 并进入息屏待机");
    serial_bucket_stop_all();

    if (g_bucket_standby.progress_arc != NULL)
    {
        lv_arc_set_angles(g_bucket_standby.progress_arc,
                          BUCKET_STANDBY_ARC_START_ANGLE,
                          BUCKET_STANDBY_ARC_COMPLETE_ANGLE);
        lv_refr_now(NULL);
    }
    if (g_bucket_standby.prompt_panel != NULL)
    {
        lv_obj_add_flag(g_bucket_standby.prompt_panel, LV_OBJ_FLAG_HIDDEN);
    }

    if (!backlight_control_sleep())
    {
        SYSTEM_MANAGER_LOG_ERROR("关闭屏幕背光失败，A7和桶体状态清理已完成");
        bucket_standby_finish_wake(false);
        return;
    }
    g_bucket_standby.sleeping = true;
}

static void bucket_standby_progress_timer_cb(lv_timer_t *timer)
{
    uint64_t now_ms;
    uint64_t elapsed_ms;
    int32_t end_angle;

    LV_UNUSED(timer);
    if (!g_bucket_standby.pressing || g_bucket_standby.submitted)
    {
        return;
    }

    now_ms = bucket_countdown_monotonic_ms();
    if (now_ms == 0u || now_ms < g_bucket_standby.pressed_at_ms)
    {
        bucket_standby_cancel_press();
        return;
    }
    elapsed_ms = now_ms - g_bucket_standby.pressed_at_ms;
    if (elapsed_ms >= BUCKET_STANDBY_HOLD_MS)
    {
        if (g_bucket_standby.progress_arc != NULL &&
                lv_obj_is_valid(g_bucket_standby.progress_arc))
        {
            lv_arc_set_angles(g_bucket_standby.progress_arc,
                              BUCKET_STANDBY_ARC_START_ANGLE,
                              BUCKET_STANDBY_ARC_COMPLETE_ANGLE);
        }
        g_bucket_standby.progress_timer = NULL;
        lv_timer_delete(timer);
        bucket_standby_commit();
        return;
    }

    end_angle = 30 + (int32_t)((elapsed_ms *
                                (BUCKET_STANDBY_ARC_COMPLETE_ANGLE - 30)) /
                               BUCKET_STANDBY_HOLD_MS);
    if (g_bucket_standby.progress_arc != NULL &&
            lv_obj_is_valid(g_bucket_standby.progress_arc))
    {
        lv_arc_set_angles(g_bucket_standby.progress_arc,
                          BUCKET_STANDBY_ARC_START_ANGLE,
                          end_angle);
    }
}

static void bucket_standby_cancel_press(void)
{
    if (g_bucket_standby.progress_timer != NULL)
    {
        lv_timer_delete(g_bucket_standby.progress_timer);
        g_bucket_standby.progress_timer = NULL;
    }
    g_bucket_standby.pressing = false;
    g_bucket_standby.pressed_at_ms = 0u;
    if (!g_bucket_standby.submitted && !g_bucket_standby.sleeping)
    {
        bucket_standby_remove_overlay(false);
    }
}

static void bucket_standby_start_press(void)
{
    uint64_t now_ms;

    if (g_bucket_standby.sleeping)
    {
        return;
    }
    bucket_standby_cancel_press();
    if (!bucket_standby_create_overlay())
    {
        SYSTEM_MANAGER_LOG_ERROR("创建桶体长按待机提示层失败");
        return;
    }

    now_ms = bucket_countdown_monotonic_ms();
    if (now_ms == 0u)
    {
        SYSTEM_MANAGER_LOG_ERROR("读取单调时间失败，取消长按待机");
        bucket_standby_remove_overlay(false);
        return;
    }
    g_bucket_standby.pressing = true;
    g_bucket_standby.submitted = false;
    g_bucket_standby.pressed_at_ms = now_ms;
    g_bucket_standby.progress_timer = lv_timer_create(bucket_standby_progress_timer_cb,
                                      BUCKET_STANDBY_REFRESH_MS,
                                      NULL);
    if (g_bucket_standby.progress_timer == NULL)
    {
        SYSTEM_MANAGER_LOG_ERROR("创建桶体长按待机进度 timer 失败");
        bucket_standby_cancel_press();
    }
}

static void bucket_standby_page_cleanup(void)
{
    if (g_bucket_standby.sleeping)
    {
        return;
    }
    bucket_standby_cancel_press();
}

static void bucket_countdown_expire(void)
{
    g_bucket_countdown.active = false;
    remaining_seconds = 0u;
    g_bucket_countdown.remaining_seconds = 0u;
    bucket_countdown_update_label();

    if (g_bucket_countdown.expiry_handled)
    {
        return;
    }
    g_bucket_countdown.expiry_handled = true;
    bucket_working_clear_function_state();
    SYSTEM_MANAGER_LOG_USER("桶体定时到期，下发 A7 关闭所有器件");
    serial_bucket_stop_all();
}

static void bucket_countdown_refresh(void)
{
    uint64_t now_ms;
    uint64_t remaining_ms;

    if (!g_bucket_countdown.active)
    {
        bucket_countdown_update_label();
        return;
    }

    now_ms = bucket_countdown_monotonic_ms();
    if (now_ms == 0u || now_ms >= g_bucket_countdown.deadline_ms)
    {
        bucket_countdown_expire();
        return;
    }

    remaining_ms = g_bucket_countdown.deadline_ms - now_ms;
    remaining_seconds = (uint32_t)((remaining_ms + 999u) / 1000u);
    g_bucket_countdown.remaining_seconds = remaining_seconds;
    bucket_countdown_update_label();
}

static void bucket_countdown_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    bucket_countdown_refresh();
}

static void bucket_countdown_ensure_timer(void)
{
    if (g_bucket_countdown.refresh_timer != NULL)
    {
        return;
    }

    g_bucket_countdown.refresh_timer = lv_timer_create(bucket_countdown_timer_cb,
                                       BUCKET_COUNTDOWN_REFRESH_MS,
                                       NULL);
}

static void bucket_countdown_start(uint32_t seconds)
{
    uint64_t now_ms = bucket_countdown_monotonic_ms();

    remaining_seconds = seconds;
    g_bucket_countdown.remaining_seconds = seconds;
    g_bucket_countdown.has_value = true;
    g_bucket_countdown.expiry_handled = false;
    g_bucket_countdown.editing = false;
    g_bucket_countdown.button_option_index = 0u;
    g_bucket_countdown.active = seconds > 0u;
    g_bucket_countdown.deadline_ms = now_ms + ((uint64_t)seconds * 1000u);
    bucket_countdown_ensure_timer();
    bucket_countdown_update_label();

    if (seconds == 0u)
    {
        bucket_countdown_expire();
    }
}

static void bucket_auto_start_delay_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    g_bucket_countdown.delayed_start_timer = NULL;

    if (g_applied_link_status != LINK_STATUS_DETACHED)
    {
        SYSTEM_MANAGER_LOG_WARN("离站延时 A6 已取消：当前 LINK_STATUS=0x%02X",
                                (unsigned int)g_applied_link_status);
        return;
    }

    bucket_countdown_start(timer_set);
    serial_bucket_timer();
    SYSTEM_MANAGER_LOG_USER("离站后下发 A6 并启动倒计时: %us",
                            (unsigned int)timer_set);
}

static void bucket_auto_start_cancel(void)
{
    if (g_bucket_countdown.delayed_start_timer != NULL)
    {
        lv_timer_del(g_bucket_countdown.delayed_start_timer);
        g_bucket_countdown.delayed_start_timer = NULL;
    }
}

static void bucket_auto_start_detached(void)
{
    bucket_auto_start_cancel();
    bucket_countdown_cancel_edit();
    g_bucket_countdown.button_option_index = 0u;
    remaining_seconds = timer_set;
    g_bucket_countdown.remaining_seconds = timer_set;
    g_bucket_countdown.has_value = true;
    g_bucket_countdown.active = false;
    g_bucket_countdown.expiry_handled = false;
    bucket_countdown_update_label();

    constant_temperature = true;
    serial_bucket_heat();
    SYSTEM_MANAGER_LOG_USER("离站立即下发 A2: target_temp=%d", temp_set);

    g_bucket_countdown.delayed_start_timer = lv_timer_create(bucket_auto_start_delay_cb,
            BUCKET_AUTO_TIMER_DELAY_MS,
            NULL);
    if (g_bucket_countdown.delayed_start_timer != NULL)
    {
        lv_timer_set_repeat_count(g_bucket_countdown.delayed_start_timer, 1);
    }
    else
    {
        SYSTEM_MANAGER_LOG_ERROR("创建离站延时 A6 timer 失败");
    }
}

static void bucket_timer_step(lv_ui *ui)
{
    static const uint32_t timer_options[BUCKET_TIMER_OPTION_COUNT] =
    {
        600u, 900u, 1200u, 1500u, 1800u
    };

    if (ui == NULL || ui->working_label_time_left == NULL)
    {
        return;
    }

    /* 用户主动设定优先，首次进入编辑固定从10分钟开始。 */
    bucket_auto_start_cancel();
    if (!g_bucket_countdown.editing)
    {
        g_bucket_countdown.button_option_index = 0u;
    }
    bucket_countdown_cancel_blink();
    timer_set = timer_options[g_bucket_countdown.button_option_index];
    g_bucket_countdown.button_option_index =
        (g_bucket_countdown.button_option_index + 1u) % BUCKET_TIMER_OPTION_COUNT;
    remaining_seconds = timer_set;
    g_bucket_countdown.remaining_seconds = timer_set;
    g_bucket_countdown.has_value = true;
    g_bucket_countdown.active = false;
    g_bucket_countdown.expiry_handled = false;
    g_bucket_countdown.editing = true;
    bucket_countdown_update_label();
    bucket_countdown_reset_confirm_timer();
    serial_bucket_timer();
    SYSTEM_MANAGER_LOG_USER("桶体定时按钮下发 A6，等待确认: %us",
                            (unsigned int)timer_set);
}

void bucket_temp_update_actual_from_mcu(int16_t temp)
{
    if (!bucket_temp_valid(temp))
    {
        return;
    }

    current_temp = temp;
    g_bucket_temp_edit.actual_temp = temp;
    g_bucket_temp_edit.has_actual_temp = true;
}

static void conf_mode_detail_keyboard_cleanup(lv_ui *ui)
{
#if LV_USE_KEYBOARD
    LV_UNUSED(ui);
    keyboard_manager_hide();
#else
    LV_UNUSED(ui);
#endif
}

static void conf_mode_detail_screen_lifecycle_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_UNLOAD_START || code == LV_EVENT_DELETE)
    {
        conf_mode_detail_keyboard_cleanup((lv_ui *)lv_event_get_user_data(e));
    }
}

static void init_default_locations(void)
{
    memset(g_location_settings, 0, sizeof(g_location_settings));

    snprintf(g_location_settings[0].name, sizeof(g_location_settings[0].name), "客厅");
    g_location_settings[0].index = 1;
    g_location_settings[0].default_mode_index = 0;
    g_location_settings[0].coord.x = 60;
    g_location_settings[0].coord.y = 40;

    snprintf(g_location_settings[1].name, sizeof(g_location_settings[1].name), "卧室");
    g_location_settings[1].index = 2;
    g_location_settings[1].default_mode_index = 1;
    g_location_settings[1].coord.x = 280;
    g_location_settings[1].coord.y = 40;

    snprintf(g_location_settings[2].name, sizeof(g_location_settings[2].name), "书房");
    g_location_settings[2].index = 3;
    g_location_settings[2].default_mode_index = 2;
    g_location_settings[2].coord.x = 60;
    g_location_settings[2].coord.y = 260;

    snprintf(g_location_settings[3].name, sizeof(g_location_settings[3].name), "阳台");
    g_location_settings[3].index = 4;
    g_location_settings[3].default_mode_index = 0;
    g_location_settings[3].coord.x = 280;
    g_location_settings[3].coord.y = 260;

    g_location_count = 4;
}

void working_bind_play_pause_buttons(lv_obj_t *pause_btn, lv_obj_t *play_btn)
{
    g_working_pause_btn = pause_btn;
    g_working_play_btn = play_btn;
}

static lv_obj_t *working_create_power_button(lv_obj_t *parent,
        lv_coord_t x,
        lv_coord_t y,
        lv_coord_t w,
        lv_coord_t h)
{
    lv_obj_t *button;
    lv_obj_t *symbol;

    if (parent == NULL)
    {
        return NULL;
    }

    button = lv_obj_create(parent);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, w, h);
    lv_obj_remove_flag(button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(button, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(button, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(button, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(button, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(button, lv_color_hex(0xe0e0e0), LV_PART_MAIN | LV_STATE_PRESSED);

    symbol = lv_label_create(button);
    lv_label_set_text(symbol, LV_SYMBOL_POWER);
    lv_obj_set_style_text_font(symbol, &lv_font_montserratMedium_80,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(symbol, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(symbol, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(symbol);

    return button;
}

static int get_mode_index_from_ptr(const system_mode_t *mode)
{
    if (mode == NULL || mode < g_mode_settings || mode >= (g_mode_settings + g_mode_count))
    {
        return -1;
    }

    return (int)(mode - g_mode_settings);
}

static void update_mode_from_detail_ui(system_mode_t *mode)
{
    const char *mode_name;
    uint16_t medicinal_sel;

    if (mode == NULL)
    {
        return;
    }

    mode_name = lv_textarea_get_text(guider_ui.conf_mode_detail_text_mode_name);
    if (mode_name != NULL && mode_name[0] != '\0')
    {
        snprintf(mode->name, sizeof(mode->name), "%s", mode_name);
    }

    mode->temperature = (int16_t)(35 + lv_dropdown_get_selected(
                                      guider_ui.conf_mode_detail_combo_temperature));
    mode->time_sec = (uint32_t)(600 + lv_dropdown_get_selected(guider_ui.conf_mode_detail_combo_timer) *
                                300);
    mode->water_level = (uint8_t)lv_dropdown_get_selected(guider_ui.conf_mode_detail_combo_water_level);
    medicinal_sel = (uint16_t)lv_dropdown_get_selected(guider_ui.conf_mode_detail_combo_medicinal);
    mode->use_drug1 = ((medicinal_sel & 1) == 1);
    mode->use_drug2 = ((medicinal_sel & 2) == 2);
    mode->position = (int)lv_dropdown_get_selected(guider_ui.conf_mode_detail_combo_location);
}

static void apply_dropdown_list_font(lv_obj_t *dropdown)
{
    lv_obj_t *list;
    const lv_font_t *dynamic_font = custom_get_dynamic_text_font();

    if (dropdown == NULL)
    {
        return;
    }

    list = lv_dropdown_get_list(dropdown);
    if (list == NULL)
    {
        return;
    }

    lv_obj_set_style_text_font(list, dynamic_font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(list, dynamic_font, LV_PART_SELECTED | LV_STATE_DEFAULT);
}

static void build_mode_dropdown_options(char *options, size_t options_size)
{
    size_t index;
    size_t offset = 0;

    if (options == NULL || options_size == 0u)
    {
        return;
    }

    options[0] = '\0';

    for (index = 0; index < g_mode_count; ++index)
    {
        const system_mode_t *mode = &g_mode_settings[index];
        int written = snprintf(options + offset,
                               options_size - offset,
                               "%s%s",
                               index == 0 ? "" : "\n",
                               mode->name);

        if (written < 0 || (size_t)written >= (options_size - offset))
        {
            break;
        }

        offset += (size_t)written;
    }

    if (offset == 0u)
    {
        snprintf(options, options_size, "未配置模式");
    }
}

static void conf_location_name_changed(lv_event_t *e)
{
    system_location_t *location;
    const char *name;

    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
    {
        return;
    }

    location = (system_location_t *)lv_event_get_user_data(e);
    if (location == NULL)
    {
        return;
    }

    name = lv_textarea_get_text(lv_event_get_target(e));
    if (name == NULL)
    {
        return;
    }

    snprintf(location->name, sizeof(location->name), "%s", name);
}

static void conf_location_default_mode_changed(lv_event_t *e)
{
    system_location_t *location;

    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
    {
        return;
    }

    location = (system_location_t *)lv_event_get_user_data(e);
    if (location == NULL)
    {
        return;
    }

    location->default_mode_index = (int)lv_dropdown_get_selected(lv_event_get_target(e));
}

static void conf_location_apply_label_style(lv_obj_t *label)
{
    if (label == NULL)
    {
        return;
    }

    lv_obj_set_style_border_width(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, custom_get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void conf_location_apply_textarea_style(lv_ui *ui, lv_obj_t *textarea)
{
    if (textarea == NULL)
    {
        return;
    }

    lv_textarea_set_placeholder_text(textarea, "");
    lv_textarea_set_password_bullet(textarea, "*");
    lv_textarea_set_password_mode(textarea, false);
    lv_textarea_set_one_line(textarea, true);
    lv_textarea_set_accepted_chars(textarea, "");
    lv_textarea_set_max_length(textarea, SYSTEM_LOCATION_NAME_LEN - 1);
#if LV_USE_KEYBOARD
    if (ui != NULL)
    {
        lv_obj_add_event_cb(textarea, ta_event_cb, LV_EVENT_ALL, ui->g_kb_top_layer);
    }
#endif

    lv_obj_set_style_text_color(textarea, lv_color_hex(0x937dad), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(textarea, custom_get_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(textarea, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(textarea, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(textarea, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(textarea, 144, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(textarea, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(textarea, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(textarea, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(textarea, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(textarea, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(textarea, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(textarea, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(textarea, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_opa(textarea, 255, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(textarea, lv_color_hex(0x2195f6), LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(textarea, LV_GRAD_DIR_NONE, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(textarea, 0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
}

static void conf_location_apply_dropdown_style(lv_obj_t *dropdown)
{
    lv_obj_t *list;
    const lv_font_t *dynamic_font = custom_get_dynamic_text_font();

    if (dropdown == NULL)
    {
        return;
    }

    lv_dropdown_set_symbol(dropdown, NULL);
    lv_obj_set_style_text_color(dropdown, lv_color_hex(0x937dad), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(dropdown, dynamic_font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(dropdown, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(dropdown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(dropdown, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(dropdown, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(dropdown, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(dropdown, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dropdown, 121, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(dropdown, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(dropdown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    list = lv_dropdown_get_list(dropdown);
    if (list == NULL)
    {
        return;
    }

    lv_obj_set_style_max_height(list, 90, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(list, lv_color_hex(0x937dad), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(list, dynamic_font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(list, dynamic_font, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(list, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(list, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(list, 121, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(list, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(list, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_border_width(list, 1, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(list, 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(list, lv_color_hex(0xe1e6ee), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_border_side(list, LV_BORDER_SIDE_FULL, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_radius(list, 3, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(list, 255, LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x00a1b5), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(list, LV_GRAD_DIR_NONE, LV_PART_SELECTED | LV_STATE_CHECKED);

    lv_obj_set_style_radius(list, 3, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(list, 255, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x00ff00), LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(list, LV_GRAD_DIR_NONE, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
}

static lv_obj_t *conf_location_create_info_label(lv_obj_t *parent)
{
    lv_obj_t *label;

    if (parent == NULL)
    {
        return NULL;
    }

    label = lv_label_create(parent);
    lv_obj_set_pos(label, 1, CONF_LOCATION_INFO_Y);
    lv_obj_set_size(label, 469, CONF_LOCATION_INFO_HEIGHT);
    lv_label_set_text(label,
                      "定点位置设定请移至\n移动端【地图界面】进行操作设置");
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    lv_obj_set_style_border_width(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, custom_get_dynamic_text_font(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(label, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    return label;
}

static uint16_t get_temperature_dropdown_index(int16_t temperature)
{
    if (temperature <= 35)
    {
        return 0;
    }
    if (temperature >= 48)
    {
        return 13;
    }
    return (uint16_t)(temperature - 35);
}

static uint16_t get_timer_dropdown_index(uint32_t time_sec)
{
    static const uint32_t timer_options[] = {600, 900, 1200, 1500, 1800};
    uint16_t index;

    for (index = 0; index < (sizeof(timer_options) / sizeof(timer_options[0])); ++index)
    {
        if (time_sec <= timer_options[index])
        {
            return index;
        }
    }

    return (uint16_t)((sizeof(timer_options) / sizeof(timer_options[0])) - 1u);
}

static uint16_t get_medicinal_dropdown_index(const system_mode_t *mode)
{
    int sel_index = 0;
    if (mode == NULL)
    {
        sel_index = 0;
    }

    if (mode->use_drug2)
    {
        sel_index += 2;
    }

    if (mode->use_drug1)
    {
        sel_index += 1;
    }

    return sel_index;
}

static const void *get_mode_icon_src(int icon_id)
{
    static const void *const icon_table[] =
    {
        &_mode_sleep1_RGB565A8_100x100,
        &_mode_sleep2_RGB565A8_100x100,
        &_mode_sleep3_RGB565A8_100x100,
        &_mode_relax1_RGB565A8_100x100,
        &_mode_relax2_RGB565A8_100x100,
        &_mode_relax3_RGB565A8_100x100,
        &_mode_relax4_RGB565A8_100x100,
        &_mode_foot1_RGB565A8_100x100,
        &_mode_foot2_RGB565A8_100x100,
        &_mode_foot3_RGB565A8_100x100,
        &_mode_foot4_RGB565A8_100x100,
        &_mode_worm3_RGB565A8_100x100,
        &_mode_worm4_RGB565A8_100x100,
        &_mode_RGB565A8_100x100,
    };

    if (icon_id < 0 || (size_t)icon_id >= (sizeof(icon_table) / sizeof(icon_table[0])))
    {
        return &_mode_RGB565A8_100x100;
    }

    return icon_table[icon_id];
}

static void mode_grid_btn_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        const system_mode_t *mode = (const system_mode_t *)lv_event_get_user_data(e);
        int index = get_mode_index_from_ptr(mode);

        if (index < 0)
        {
            return;
        }

        current_selected_mode_index = index;
        conf_mode_to_conf_mode_detail(e);
    }
}

static void add_mode_btn_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        system_mode_t mode;
        size_t index;

        if (g_mode_count >= SYSTEM_MODE_MAX_COUNT)
        {
            return;
        }

        memset(&mode, 0, sizeof(mode));
        index = g_mode_count;

        snprintf(mode.name, sizeof(mode.name), "新模式%u", (unsigned)(index + 1));
        mode.icon_id = (int)(index % 14u);
        mode.time_sec = timer_set > 0 ? timer_set : 600;
        mode.water_level = (uint8_t)water_level;
        mode.temperature = temp_set;
        mode.position = 0;

        g_mode_settings[index] = mode;
        g_mode_count++;

        if (persist_mode_settings() == 0)
        {
            conf_mode_init_listall(&guider_ui);
        }
        else
        {
            g_mode_count--;
            memset(&g_mode_settings[index], 0, sizeof(g_mode_settings[index]));
        }
    }
}


static int ensure_mode_settings_dir(void)
{
    struct stat st;

    if (stat(MODE_SETTINGS_DIR, &st) == 0)
    {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }

    if (mkdir(MODE_SETTINGS_DIR, 0775) == 0 || errno == EEXIST)
    {
        return 0;
    }

    return -1;
}

static int ensure_location_settings_dir(void)
{
    struct stat st;

    if (stat(LOCATION_SETTINGS_DIR, &st) == 0)
    {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }

    if (mkdir(LOCATION_SETTINGS_DIR, 0775) == 0 || errno == EEXIST)
    {
        return 0;
    }

    return -1;
}

static int persist_mode_settings(void)
{
    FILE *fp;
    system_mode_store_t store;

    memset(&store, 0, sizeof(store));
    store.magic = MODE_SETTINGS_MAGIC;
    store.version = MODE_SETTINGS_VERSION;
    store.count = (uint32_t)g_mode_count;
    if (g_mode_count > 0)
    {
        memcpy(store.modes, g_mode_settings, g_mode_count * sizeof(system_mode_t));
    }

    if (ensure_mode_settings_dir() != 0)
    {
        return -1;
    }

    fp = fopen(MODE_SETTINGS_PATH, "wb");
    if (fp == NULL)
    {
        return -1;
    }

    if (fwrite(&store, sizeof(store), 1, fp) != 1)
    {
        fclose(fp);
        return -1;
    }

    return fclose(fp) == 0 ? 0 : -1;
}

static int persist_location_settings(void)
{
    FILE *fp;
    system_location_store_t store;

    memset(&store, 0, sizeof(store));
    store.magic = LOCATION_SETTINGS_MAGIC;
    store.version = LOCATION_SETTINGS_VERSION;
    store.count = (uint32_t)g_location_count;
    if (g_location_count > 0)
    {
        memcpy(store.locations, g_location_settings, g_location_count * sizeof(system_location_t));
    }

    if (ensure_location_settings_dir() != 0)
    {
        return -1;
    }

    fp = fopen(LOCATION_SETTINGS_PATH, "wb");
    if (fp == NULL)
    {
        return -1;
    }

    if (fwrite(&store, sizeof(store), 1, fp) != 1)
    {
        fclose(fp);
        return -1;
    }

    return fclose(fp) == 0 ? 0 : -1;
}

static void init_default_modes(void)
{
    memset(g_mode_settings, 0, sizeof(g_mode_settings));

    snprintf(g_mode_settings[0].name, sizeof(g_mode_settings[0].name), "助眠");
    g_mode_settings[0].icon_id = 0;
    g_mode_settings[0].time_sec = 900;
    g_mode_settings[0].water_level = 0;
    g_mode_settings[0].temperature = 37;
    g_mode_settings[0].use_drug1 = true;
    g_mode_settings[0].use_drug2 = false;
    g_mode_settings[0].position = 1;

    snprintf(g_mode_settings[1].name, sizeof(g_mode_settings[1].name), "放松");
    g_mode_settings[1].icon_id = 3;
    g_mode_settings[1].time_sec = 1200;
    g_mode_settings[1].water_level = 1;
    g_mode_settings[1].temperature = 39;
    g_mode_settings[1].use_drug1 = false;
    g_mode_settings[1].use_drug2 = true;
    g_mode_settings[1].position = 2;

    snprintf(g_mode_settings[2].name, sizeof(g_mode_settings[2].name), "驱寒");
    g_mode_settings[2].icon_id = 11;
    g_mode_settings[2].time_sec = 1500;
    g_mode_settings[2].water_level = 2;
    g_mode_settings[2].temperature = 41;
    g_mode_settings[2].use_drug1 = true;
    g_mode_settings[2].use_drug2 = true;
    g_mode_settings[2].position = 3;

    g_mode_count = 3;
}

static int load_mode_settings(void)
{
    FILE *fp;
    system_mode_store_t store;

    fp = fopen(MODE_SETTINGS_PATH, "rb");
    if (fp == NULL)
    {
        init_default_modes();
        return persist_mode_settings();
    }

    if (fread(&store, sizeof(store), 1, fp) != 1)
    {
        fclose(fp);
        init_default_modes();
        return persist_mode_settings();
    }

    fclose(fp);

    if (store.magic != MODE_SETTINGS_MAGIC ||
            store.version != MODE_SETTINGS_VERSION ||
            store.count == 0 ||
            store.count > SYSTEM_MODE_MAX_COUNT)
    {
        init_default_modes();
        return persist_mode_settings();
    }

    memcpy(g_mode_settings, store.modes, store.count * sizeof(system_mode_t));
    g_mode_count = store.count;
    return 0;
}

static int load_location_settings(void)
{
    FILE *fp;
    system_location_store_t store;

    fp = fopen(LOCATION_SETTINGS_PATH, "rb");
    if (fp == NULL)
    {
        init_default_locations();
        return persist_location_settings();
    }

    if (fread(&store, sizeof(store), 1, fp) != 1)
    {
        fclose(fp);
        init_default_locations();
        return persist_location_settings();
    }

    fclose(fp);

    if (store.magic != LOCATION_SETTINGS_MAGIC ||
            store.version != LOCATION_SETTINGS_VERSION ||
            store.count == 0 ||
            store.count > SYSTEM_LOCATION_MAX_COUNT)
    {
        init_default_locations();
        return persist_location_settings();
    }

    memcpy(g_location_settings, store.locations, store.count * sizeof(system_location_t));
    g_location_count = store.count;
    return 0;
}

void location_grid_btn_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        int index = (int)(uintptr_t)lv_event_get_user_data(e);
        if (index < 0 || index >= (int)g_location_count)
        {
            return;
        }

        const system_location_t *location = &g_location_settings[index];

        current_selected_location_index = index;
        char tips[64];
        snprintf(tips, sizeof(tips), "移动至%s", location->name);
        going_to_location = location->index;
        if (guider_ui.location_label_moving_tips != NULL)
        {
            lv_obj_set_style_text_font(guider_ui.location_label_moving_tips,
                                       custom_get_dynamic_text_font(),
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(guider_ui.location_label_moving_tips, tips);
        }
        //location_to_location_detail(e);
    }
}

void apply_mode_to_runtime(lv_event_t *e)
{
    if (e == NULL)
    {
        return;
    }
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }


    int mode_index = (int)(uintptr_t)lv_event_get_user_data(e);
    if (mode_index >= g_mode_count)
    {
        return;
    }

    const system_mode_t *mode = &g_mode_settings[mode_index];
    if (mode == NULL)
    {
        return;
    }

    //显示cont_2,隐藏cont_clean
    is_self_cleaning = false;
    lv_obj_remove_flag(guider_ui.index_cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(guider_ui.index_cont_clean, LV_OBJ_FLAG_HIDDEN);

    temp_set = mode->temperature;
    timer_set = mode->time_sec;
    water_level = mode->water_level;
    use_drug1 = mode->use_drug1;
    use_drug2 = mode->use_drug2;

    current_temp = 0;

    //修改temp_value的显示值
    update_temp_set(&guider_ui);

    //修改time_info的显示值，用min做单位
    update_timer_set(&guider_ui);

    apply_water_level_ui(water_level);
    apply_use_drug_ui(use_drug1, use_drug2);
}

void index_page_init(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }

    //刷新温度显示
    update_temp_set(ui);

    // 清空控件容器，避免重复添加
    if (ui->index_cont_1 != NULL)
    {
        lv_obj_clean(ui->index_cont_1);
    }

    //根据当前模式设置，载入模式按钮的图标和名称
    size_t index;
    //调用imgbtn_create添加模式按钮时，
    int x_offset = 0;
    int y_offset = 0;
    int w = 170;
    int h = 178;
    int w_span = 24;
    int h_span = 22;
    for (index = 0; index < g_mode_count; ++index)
    {
        const system_mode_t *mode = &g_mode_settings[index];
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn = imgbtn_create(ui->index_cont_1, get_mode_icon_src(mode->icon_id), mode->name,
                                      x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn, apply_mode_to_runtime, LV_EVENT_ALL, (void *)index);
    }
    //自清洁
    x_offset = (int)(index % 2) * (w + w_span);
    y_offset = (int)(index / 2) * (h + h_span);
    lv_obj_t *btn_clean = imgbtn_create(ui->index_cont_1, &_clean_RGB565A8_100x100, "自清洁",
                                        x_offset, y_offset, w, h);
    lv_obj_add_event_cb(btn_clean, action_clean_clicked, LV_EVENT_ALL, ui);
    //药浴1 action_use_drug1_toggle(ui);
    index++;
    x_offset = (int)(index % 2) * (w + w_span);
    y_offset = (int)(index / 2) * (h + h_span);
    lv_obj_t *btn_drug1 = imgbtn_create(ui->index_cont_1, &_medicine1_RGB565A8_100x100, "药浴1",
                                        x_offset, y_offset, w, h);
    lv_obj_add_event_cb(btn_drug1, action_use_drug1_toggle, LV_EVENT_ALL, ui);
    //药浴2
    index++;
    x_offset = (int)(index % 2) * (w + w_span);
    y_offset = (int)(index / 2) * (h + h_span);
    lv_obj_t *btn_drug2 = imgbtn_create(ui->index_cont_1, &_medicine2_RGB565A8_100x100, "药浴2",
                                        x_offset, y_offset, w, h);
    lv_obj_add_event_cb(btn_drug2, action_use_drug2_toggle, LV_EVENT_ALL, ui);

    //定位
    index++;
    x_offset = (int)(index % 2) * (w + w_span);
    y_offset = (int)(index / 2) * (h + h_span);
    lv_obj_t *btn_position = imgbtn_create(ui->index_cont_1, &_location_RGB565A8_100x100, "定位",
                                           x_offset, y_offset, w, h);
    lv_obj_add_event_cb(btn_position, index_to_location, LV_EVENT_ALL, ui);
    //设置
    index++;
    x_offset = (int)(index % 2) * (w + w_span);
    y_offset = (int)(index / 2) * (h + h_span);
    lv_obj_t *btn_setting = imgbtn_create(ui->index_cont_1, &_setting_RGB565A8_100x100, "设置",
                                          x_offset, y_offset, w, h);
    lv_obj_add_event_cb(btn_setting, index_to_setting, LV_EVENT_ALL, ui);
}

void location_page_init(lv_ui *ui)
{
    size_t index;
    int x_offset = 0;
    int y_offset = 0;
    int w = 170;
    int h = 178;
    int w_span = 15;
    int h_span = 46;

    if (ui == NULL || ui->location_cont_2 == NULL)
    {
        return;
    }

    lv_obj_clean(ui->location_cont_2);


    for (index = 1; index < g_location_count; ++index)
    {
        const system_location_t *location = &g_location_settings[index];

        x_offset = (int)((index - 1) % 2) * (w + w_span);
        y_offset = (int)((index - 1) / 2) * (h + h_span);
        lv_obj_t *btn = imgbtn_create(ui->location_cont_2,
                                      &_local_RGB565A8_100x100,
                                      location->name,
                                      x_offset,
                                      y_offset,
                                      w,
                                      h);
        lv_obj_add_event_cb(btn, location_grid_btn_clicked, LV_EVENT_ALL, (void *)index);
    }
}

void conf_mode_init_listall(lv_ui *ui)
{
    size_t index;

    if (ui == NULL || ui->conf_mode_cont_1 == NULL)
    {
        return;
    }

    if (g_conf_mode_cont_2 != NULL && lv_obj_is_valid(g_conf_mode_cont_2))
    {
        lv_obj_del(g_conf_mode_cont_2);
        g_conf_mode_cont_2 = NULL;
    }

    g_conf_mode_cont_2 = lv_obj_create(ui->conf_mode_cont_1);
    lv_obj_set_pos(g_conf_mode_cont_2, 18, 60);
    lv_obj_set_size(g_conf_mode_cont_2, 540, 325);
    lv_obj_set_scrollbar_mode(g_conf_mode_cont_2, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(g_conf_mode_cont_2, LV_DIR_VER);
    lv_obj_set_style_bg_opa(g_conf_mode_cont_2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_conf_mode_cont_2, 0, 0);
    lv_obj_set_style_shadow_width(g_conf_mode_cont_2, 0, 0);
    lv_obj_set_style_pad_top(g_conf_mode_cont_2, 0, 0);
    lv_obj_set_style_pad_bottom(g_conf_mode_cont_2, 0, 0);
    lv_obj_set_style_pad_left(g_conf_mode_cont_2, 0, 0);
    lv_obj_set_style_pad_right(g_conf_mode_cont_2, 0, 0);

    for (index = 0; index < g_mode_count; ++index)
    {
        const system_mode_t *mode = &g_mode_settings[index];
        lv_coord_t col = (lv_coord_t)(index % CONF_MODE_GRID_COLS);
        lv_coord_t row = (lv_coord_t)(index / CONF_MODE_GRID_COLS);
        lv_coord_t x = (lv_coord_t)(CONF_MODE_GRID_X_START + col * (CONF_MODE_BTN_WIDTH +
                                    CONF_MODE_GRID_X_GAP));
        lv_coord_t y = (lv_coord_t)(CONF_MODE_GRID_Y_START + row * (CONF_MODE_BTN_HEIGHT +
                                    CONF_MODE_GRID_Y_GAP));
        lv_obj_t *btn = imgbtn_create_without_bg(g_conf_mode_cont_2,
                        get_mode_icon_src(mode->icon_id),
                        mode->name,
                        x,
                        y,
                        CONF_MODE_BTN_WIDTH,
                        CONF_MODE_BTN_HEIGHT);
        LV_UNUSED(ui);
        lv_obj_add_event_cb(btn, mode_grid_btn_clicked, LV_EVENT_ALL, (void *)mode);
    }

    {
        size_t add_index = g_mode_count;
        lv_coord_t col = (lv_coord_t)(add_index % CONF_MODE_GRID_COLS);
        lv_coord_t row = (lv_coord_t)(add_index / CONF_MODE_GRID_COLS);
        lv_coord_t x = (lv_coord_t)(CONF_MODE_GRID_X_START + col * (CONF_MODE_BTN_WIDTH +
                                    CONF_MODE_GRID_X_GAP));
        lv_coord_t y = (lv_coord_t)(CONF_MODE_GRID_Y_START + row * (CONF_MODE_BTN_HEIGHT +
                                    CONF_MODE_GRID_Y_GAP));
        lv_obj_t *btn = imgbtn_create_without_bg(g_conf_mode_cont_2,
                        &_add_one_RGB565A8_100x100,
                        "新增",
                        x,
                        y,
                        CONF_MODE_BTN_WIDTH,
                        CONF_MODE_BTN_HEIGHT);

        lv_obj_add_event_cb(btn, add_mode_btn_clicked, LV_EVENT_ALL, NULL);
    }
}

void conf_mode_detail_init(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }
    if (!_is_conf_mode_detail_page_initialized)
    {
        if (ui->conf_mode_detail != NULL)
        {
            lv_obj_add_event_cb(ui->conf_mode_detail,
                                conf_mode_detail_screen_lifecycle_cb,
                                LV_EVENT_SCREEN_UNLOAD_START,
                                ui);
            lv_obj_add_event_cb(ui->conf_mode_detail,
                                conf_mode_detail_screen_lifecycle_cb,
                                LV_EVENT_DELETE,
                                ui);
        }

        apply_dropdown_list_font(ui->conf_mode_detail_combo_temperature);
        apply_dropdown_list_font(ui->conf_mode_detail_combo_timer);
        apply_dropdown_list_font(ui->conf_mode_detail_combo_water_level);
        apply_dropdown_list_font(ui->conf_mode_detail_combo_medicinal);
        apply_dropdown_list_font(ui->conf_mode_detail_combo_location);
    }


    if (current_selected_mode_index < 0 || (size_t)current_selected_mode_index >= g_mode_count)
    {
        return;
    }

    const system_mode_t *mode = &g_mode_settings[current_selected_mode_index];
    if (mode == NULL)
    {
        return;
    }

    //conf_mode_detail_init小于等于2,则显示恢复按钮，隐藏删除按钮；大于2则显示删除按钮，隐藏恢复按钮
    if (current_selected_mode_index <= 2)
    {
        lv_obj_clear_flag(ui->conf_mode_detail_btn_resume, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->conf_mode_detail_btn_delete, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(ui->conf_mode_detail_btn_delete, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->conf_mode_detail_btn_resume, LV_OBJ_FLAG_HIDDEN);
    }
    //根据mode的设置，更新编辑界面的显示值
    lv_obj_t *text_mode_name = ui->conf_mode_detail_text_mode_name;
    char title[64];

    lv_obj_set_style_text_font(text_mode_name, custom_get_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->conf_mode_detail_label_1, custom_get_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_textarea_set_text(text_mode_name, mode->name);
    snprintf(title, sizeof(title), "%s设置", mode->name);
    lv_label_set_text(ui->conf_mode_detail_label_1, title);
    lv_dropdown_set_selected(ui->conf_mode_detail_combo_temperature,
                             get_temperature_dropdown_index(mode->temperature));
    lv_dropdown_set_selected(ui->conf_mode_detail_combo_timer,
                             get_timer_dropdown_index(mode->time_sec));
    lv_dropdown_set_selected(ui->conf_mode_detail_combo_water_level,
                             mode->water_level > 2u ? 2u : mode->water_level);
    lv_dropdown_set_selected(ui->conf_mode_detail_combo_medicinal,
                             get_medicinal_dropdown_index(mode));
    lv_dropdown_set_selected(ui->conf_mode_detail_combo_location,
                             mode->position < 0 ? 0 : (mode->position > 3 ? 3 : (uint16_t)mode->position));
}

void conf_location_init(lv_ui *ui)
{
    size_t index;
    char mode_options[CONF_LOCATION_MODE_OPTIONS_LEN];

    if (ui == NULL)
    {
        return;
    }

    if (ui->conf_location_cont_2 == NULL)
    {
        return;
    }

    if (ui->conf_location_label_7 != NULL && lv_obj_is_valid(ui->conf_location_label_7))
    {
        lv_obj_del(ui->conf_location_label_7);
    }

    lv_obj_clean(ui->conf_location_cont_2);

    ui->conf_location_label_6 = conf_location_create_info_label(ui->conf_location_cont_2);
    ui->conf_location_label_7 = NULL;
    ui->conf_location_ta_1 = NULL;
    ui->conf_location_ddlist_1 = NULL;

    build_mode_dropdown_options(mode_options, sizeof(mode_options));

    for (index = 0; index < g_location_count; ++index)
    {
        system_location_t *location = &g_location_settings[index];
        lv_coord_t row_y = (lv_coord_t)(CONF_LOCATION_ROW_Y_START + index * (CONF_LOCATION_ROW_HEIGHT +
                                        CONF_LOCATION_ROW_GAP));
        lv_obj_t *label;
        lv_obj_t *textarea;
        lv_obj_t *dropdown;
        char label_text[32];
        int selected_mode_index = location->default_mode_index;

        label = lv_label_create(ui->conf_location_cont_2);
        lv_obj_set_pos(label, CONF_LOCATION_LABEL_X, (lv_coord_t)(row_y + CONF_LOCATION_LABEL_Y_OFFSET));
        lv_obj_set_size(label, CONF_LOCATION_LABEL_WIDTH, CONF_LOCATION_LABEL_HEIGHT);
        snprintf(label_text, sizeof(label_text), "定位%d", location->index + 1);
        lv_label_set_text(label, label_text);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        conf_location_apply_label_style(label);

        textarea = lv_textarea_create(ui->conf_location_cont_2);
        lv_obj_set_pos(textarea, CONF_LOCATION_NAME_X, row_y);
        lv_obj_set_size(textarea, CONF_LOCATION_NAME_WIDTH, CONF_LOCATION_FIELD_HEIGHT);
        lv_textarea_set_text(textarea, location->name);
        conf_location_apply_textarea_style(ui, textarea);
        lv_obj_add_event_cb(textarea, conf_location_name_changed, LV_EVENT_VALUE_CHANGED, location);

        dropdown = lv_dropdown_create(ui->conf_location_cont_2);
        lv_obj_set_pos(dropdown, CONF_LOCATION_DROPDOWN_X, row_y);
        lv_obj_set_size(dropdown, CONF_LOCATION_DROPDOWN_WIDTH, CONF_LOCATION_FIELD_HEIGHT);
        lv_dropdown_set_options(dropdown, mode_options);
        if (selected_mode_index < 0 || (size_t)selected_mode_index >= g_mode_count)
        {
            selected_mode_index = 0;
        }
        lv_dropdown_set_selected(dropdown, (uint16_t)selected_mode_index);
        conf_location_apply_dropdown_style(dropdown);
        lv_obj_add_event_cb(dropdown, conf_location_default_mode_changed, LV_EVENT_VALUE_CHANGED, location);

        if (index == 0u)
        {
            ui->conf_location_label_7 = label;
            ui->conf_location_ta_1 = textarea;
            ui->conf_location_ddlist_1 = dropdown;
        }
    }
}

void preparing_move_finished_countdown(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }

    if (g_preparing_countdown_remaining > 0)
    {
        g_preparing_countdown_remaining--;
    }

    if (g_preparing_countdown_remaining > 0)
    {
        //更新倒计时显示
        char buf[32];
        snprintf(buf, sizeof(buf), "(%us后享受健康足疗)", (unsigned)g_preparing_countdown_remaining);
        lv_label_set_text(ui->preparing_label_timer_tips, buf);
    }
    else
    {
        //清理已完成任务的倒计时定时器
        g_preparing_page_timer = NULL;
        //跳转到working页面
        navigate_to_screen(UI_SCREEN_WORKING);
        //ui_load_scr_animation(&guider_ui, &guider_ui.working, guider_ui.preparing_del, &guider_ui.preparing_del, setup_scr_working, LV_SCR_LOAD_ANIM_NONE, 200, 0, false, false);
    }
}

/* 等待基站连接定时器回调：每秒检查一次last_link_status，最多等5秒 */
static void preparing_wait_link_timer_cb(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }

    /* 已连接上基站，执行跳转 */
    if (last_link_status == 1)
    {
        lv_timer_delete(timer);
        g_wait_link_retry_count = 0;
        /* 跳转到index页面 */
        navigate_to_screen(UI_SCREEN_INDEX);
        /* 1小时内没有点首页任何按钮的话，自动排水 */
        g_auto_drain_timer = lv_timer_create_basic();
        lv_timer_set_period(g_auto_drain_timer, 36000);
        lv_timer_set_repeat_count(g_auto_drain_timer, 1);
        lv_timer_set_cb(g_auto_drain_timer, auto_drain_timer_cb);
        return;
    }

    /* 最多等5秒，超时后也执行跳转 */
    g_wait_link_retry_count++;
    if (g_wait_link_retry_count >= WAIT_LINK_MAX_RETRY)
    {
        lv_timer_delete(timer);
        g_wait_link_retry_count = 0;
        navigate_to_screen(UI_SCREEN_INDEX);
        g_auto_drain_timer = lv_timer_create_basic();
        lv_timer_set_period(g_auto_drain_timer, 36000);
        lv_timer_set_repeat_count(g_auto_drain_timer, 1);
        lv_timer_set_cb(g_auto_drain_timer, auto_drain_timer_cb);
        return;
    }
}

void preparing_move_finished(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }

    //显示cont_move_finished
    lv_obj_clear_flag(ui->preparing_cont_move_finished, LV_OBJ_FLAG_HIDDEN);
    //隐藏  cont_moving
    lv_obj_add_flag(ui->preparing_cont_moving, LV_OBJ_FLAG_HIDDEN);
    current_location = going_to_location;
    //先判断current_location对应的定位名称，如果是0,则直接跳转到index页面，否则显示倒计时，倒计时结束后跳转到working页面
    if (current_location == 0)
    {
        if (last_link_status == 1)
        {
            //已经连接上基站，直接跳转到index页面
            navigate_to_screen(UI_SCREEN_INDEX);
            //1小时内，没有点首页任何按钮的话，自动排水
            g_auto_drain_timer = lv_timer_create_basic();
            lv_timer_set_period(g_auto_drain_timer, 36000); // 1小时
            lv_timer_set_repeat_count(g_auto_drain_timer, 1);
            lv_timer_set_cb(g_auto_drain_timer, auto_drain_timer_cb);
            return;
        }
        else
        {
            //机器回到基站但尚未连接上，等待最多5秒让连接建立
            lv_timer_t *wait_timer = lv_timer_create(preparing_wait_link_timer_cb, 1000, ui);
            lv_timer_set_repeat_count(wait_timer, 5);
            return;
        }
    }
    else
    {
        //这里开启5秒倒计时，每秒刷新一次label_timer_tips，5秒后跳转到working页面
        g_preparing_countdown_remaining = 5;

        /* 创建独立的倒计时定时器（不重用传入的timer，避免与导航定时器混淆） */
        if (g_preparing_page_timer != NULL)
        {
            lv_timer_delete(g_preparing_page_timer);
        }
        g_preparing_page_timer = lv_timer_create(preparing_move_finished_countdown, 1000, ui);
        lv_timer_set_repeat_count(g_preparing_page_timer, 5);
    }
}

/** 自动注水阶段每秒刷新 MCU 上报的基站状态提示。 */
static void refresh_auto_water_status(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);

    if (ui == NULL || ui->preparing_preparing_tips == NULL)
    {
        return;
    }

    /* 准备页参数保持启动时的设定值，这里只刷新运行状态提示。 */
    if (base_main_status == 4)
    {
        lv_label_set_text(ui->preparing_preparing_tips, "自动上水中");
    }
    else if (base_main_status == 5)
    {
        lv_label_set_text(ui->preparing_preparing_tips,
                          "注水已完成，请尽快取出，\n享受健康足疗");
    }
    else if (base_main_status == 3)
    {
        lv_label_set_text(ui->preparing_preparing_tips, "待机");
    }
}

void preparing_refresh_flat2(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }

    //模拟自清洁进度逐渐增加，直到完成
    update_self_cleaning_progress(ui);
}

void preparing_page_init(lv_ui *ui)
{
    preparing_page_cleanup();
    lv_obj_add_flag(ui->preparing_cont_preparing, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->preparing_cont_cleaning, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->preparing_cont_moving, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->preparing_cont_move_finished, LV_OBJ_FLAG_HIDDEN);
    if (!_is_preparing_page_initialized)
    {
        if (ui != NULL && ui->preparing != NULL)
        {
            lv_obj_add_event_cb(ui->preparing, preparing_screen_lifecycle_cb, LV_EVENT_SCREEN_UNLOAD_START,
                                NULL);
            lv_obj_add_event_cb(ui->preparing, preparing_screen_lifecycle_cb, LV_EVENT_DELETE, NULL);

            //这里初始化cont_cleaning页面

            // 3. 创建“自动清洁”标题
            // lv_obj_t *title = lv_label_create(ui->preparing_cont_cleaning);
            // lv_label_set_text(title, "自动清洁");
            // lv_obj_set_style_text_color(title, lv_color_hex(0x333333), 0);
            // lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0); // 可根据需要调整字体
            // lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

            // // 4. 创建圆形进度条 (lv_arc)
            // lv_obj_t *arc = lv_arc_create(ui->preparing_cont_cleaning);
            // lv_arc_set_range(arc, 0, 100);
            // lv_arc_set_value(arc, 0);
            // lv_arc_set_bg_angles(arc, 0, 360);          // 全圆环
            // lv_obj_set_size(arc, 150, 150);
            // lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);

            // // 定制圆弧样式（主色、背景色、线宽）
            // lv_obj_set_style_arc_color(arc, lv_color_hex(0x2196F3), LV_PART_INDICATOR); // 蓝色
            // lv_obj_set_style_arc_width(arc, 8, LV_PART_INDICATOR);
            // lv_obj_set_style_arc_color(arc, lv_color_hex(0xE0E0E0), LV_PART_MAIN);      // 灰色背景
            // lv_obj_set_style_arc_width(arc, 8, LV_PART_MAIN);
            // // 圆角端点（可选）
            // lv_arc_set_rounded(arc, true);

            // // 5. 创建中心状态文字 (“排水中”)
            // lv_obj_t *label_status = lv_label_create(ui->preparing_cont_cleaning);
            // lv_label_set_text(label_status, "排水中");
            // lv_obj_set_style_text_color(label_status, lv_color_hex(0x666666), 0);
            // lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
            // lv_obj_align_to(label_status, arc, LV_ALIGN_CENTER, 0, -15);

            // // 6. 创建中心百分比数字 (“00”)
            // lv_obj_t *label_percent = lv_label_create(ui->preparing_cont_cleaning);
            // lv_label_set_text(label_percent, "00");
            // lv_obj_set_style_text_color(label_percent, lv_color_hex(0x2196F3), 0);
            // lv_obj_set_style_text_font(label_percent, &lv_font_montserrat_22, 0); // 大号数字
            // lv_obj_align_to(label_percent, arc, LV_ALIGN_CENTER, 0, 20);

            // // 7. 创建“流程进度”标签（位于圆弧下方）
            // lv_obj_t *label_process = lv_label_create(ui->preparing_cont_cleaning);
            // lv_label_set_text(label_process, "流程进度");
            // lv_obj_set_style_text_color(label_process, lv_color_hex(0x999999), 0);
            // lv_obj_set_style_text_font(label_process, &lv_font_montserrat_12, 0);
            // lv_obj_align_to(label_process, arc, LV_ALIGN_BOTTOM_MID, 0, 25);

            // // 8. 创建“停止”按钮
            // lv_obj_t *btn = lv_btn_create(ui->preparing_cont_cleaning);
            // lv_obj_set_size(btn, 80, 35);
            // lv_obj_set_style_radius(btn, 20, 0);
            // lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF5722), 0); // 橙色
            // lv_obj_set_style_shadow_width(btn, 4, 0);
            // lv_obj_set_style_shadow_color(btn, lv_color_hex(0xCCCCCC), 0);
            // lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -15);

            // lv_obj_t *btn_label = lv_label_create(btn);
            // lv_label_set_text(btn_label, "停止");
            // lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), 0);
            // lv_obj_center(btn_label);

            // // 9. 为按钮添加点击事件（简单示例：重置进度）
            // lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            //     lv_obj_t *arc = lv_event_get_user_data(e);
            //     lv_arc_set_value(arc, 0);
            //     // 同时更新文字
            //     lv_label_set_text(lv_obj_get_parent(arc), "00"); // 这里简化，实际上需找到对应标签
            //     // 建议使用全局结构体来管理，下面会给出完整示例
            // }, LV_EVENT_CLICKED, arc);

            // // 10. 保存控件引用到静态结构体（用于定时器更新）
            // cleaning_ui.arc = arc;
            // cleaning_ui.label_percent = label_percent;
            // cleaning_ui.label_status = label_status;
        }
        //_is_preparing_page_initialized = true;
    }


    //根据to_preparing_flat的参数，显示当前模式的准备状态
    //1--注水中，2--自清洁中，3--移动中,4--返回基站中
    if (to_preparing_flat == 1)
    {
        //添加定位按钮并定义事件
        g_preparing_btn_location = imgbtn_create(ui->preparing_cont_3, &_local_RGB565A8_100x100, "定位",
                                   0, 3, 170, 187);
        lv_obj_add_event_cb(g_preparing_btn_location, preparing_to_location, LV_EVENT_ALL, ui);
        //显示cont_preparing
        lv_obj_clear_flag(ui->preparing_cont_preparing, LV_OBJ_FLAG_HIDDEN);
        display_preparing_settings(ui);
        serial_base_water();

        /* 参数固定显示下发值，定时器只刷新 MCU 上报的注水状态提示。 */
        g_preparing_page_timer = lv_timer_create_basic();
        lv_timer_set_period(g_preparing_page_timer, 1000);
        lv_timer_set_repeat_count(g_preparing_page_timer, -1);
        lv_timer_set_user_data(g_preparing_page_timer, ui);
        lv_timer_set_cb(g_preparing_page_timer, refresh_auto_water_status);
    }
    else if (to_preparing_flat == 2)
    {
        //显示cont_self_cleaning
        lv_obj_clear_flag(ui->preparing_cont_cleaning, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->preparing_label_clean_time_left, LV_OBJ_FLAG_HIDDEN);

        /* 将 preparing_cleaning_tips 的字体替换为自定义全量 30px CJK 字库，
         * 避免 GUI Guider 生成的子集字体(仅65个汉字)缺失运行时汉字而显示 □ */
        if (ui->preparing_cleaning_tips != NULL)
        {
            /* 调试：先用已验证可用的 25px 全量字体测试渲染是否正常 */
            lv_obj_set_style_text_font(ui->preparing_cleaning_tips,
                                       &lv_font_SourceHanSansSC_Regular_25,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        g_preparing_page_timer = lv_timer_create_basic();
        lv_timer_set_period(g_preparing_page_timer, 1000);
        lv_timer_set_repeat_count(g_preparing_page_timer, -1);
        lv_timer_set_user_data(g_preparing_page_timer, ui);
        lv_timer_set_cb(g_preparing_page_timer, preparing_refresh_flat2);
        //先做一次排水
        serial_base_force_drain();

    }
    else if (to_preparing_flat == 3 || to_preparing_flat == 4)
    {
        //显示cont_moving
        lv_obj_clear_flag(ui->preparing_cont_moving, LV_OBJ_FLAG_HIDDEN);

        /* ---- 机器人底盘导航逻辑 ---- */

        robot_station_t target_station = {0};
        int result;

        //如果是返回基站(to_preparing_flat==4)，尝试使用充电指令触发自动回充
        if (to_preparing_flat == 4)
        {
            SYSTEM_MANAGER_LOG_USER("发送回基站充电指令");
            result = app_manager_robot_return_to_base(&target_station);
            if (result != ROBOT_CHASSIS_OK)
            {
                SYSTEM_MANAGER_LOG_ERROR("回仓失败，未配置有效充电基站: ret=%d",
                                         result);
                show_message_dialog(ui->preparing,
                                    "回仓失败: 未配置充电基站");
                navigate_to_screen(UI_SCREEN_INDEX);
                return;
            }
            g_nav_target_x = (float)target_station.x;
            g_nav_target_y = (float)target_station.y;
            g_nav_target_z = (float)target_station.z;
        }
        else
        {
            const system_location_t *location;

            if (current_selected_location_index < 0 ||
                    current_selected_location_index >= (int)g_location_count)
            {
                SYSTEM_MANAGER_LOG_ERROR("导航失败，站点索引非法: index=%d",
                                         current_selected_location_index);
                show_message_dialog(ui->preparing, "导航失败: 未选择目标站点");
                navigate_to_screen(UI_SCREEN_INDEX);
                return;
            }
            location = &g_location_settings[current_selected_location_index];
            SYSTEM_MANAGER_LOG_USER("准备导航到站点: name=%s", location->name);
            result = app_manager_robot_navigate(location->name,
                                                &target_station);
            if (result != ROBOT_CHASSIS_OK)
            {
                SYSTEM_MANAGER_LOG_ERROR("导航失败，未找到有效站点: name=%s ret=%d",
                                         location->name, result);
                show_message_dialog(ui->preparing, "导航失败: 未找到目标站点");
                navigate_to_screen(UI_SCREEN_INDEX);
                return;
            }
            g_nav_target_x = (float)target_station.x;
            g_nav_target_y = (float)target_station.y;
            g_nav_target_z = (float)target_station.z;
        }

        // 每秒读取新底盘服务的线程安全状态快照检查机器人位置
        g_nav_timeout_count = 0;
        if (g_nav_check_timer != NULL)
        {
            lv_timer_delete(g_nav_check_timer);
        }
        g_nav_check_timer = lv_timer_create(preparing_nav_check_timer_cb, 1000, ui);
    }
}

void system_manager_init(lv_ui *ui)
{
    if (load_mode_settings() != 0)
    {
        init_default_modes();
    }
    if (load_location_settings() != 0)
    {
        init_default_locations();
    }

    current_selected_mode_index = 0;
    //apply_mode_to_runtime(ui, 0);

    LV_UNUSED(ui);

    /* 初始首页已由 GUI Guider 创建，显式记录基站默认页面和方向。 */
    g_applied_link_status = LINK_STATUS_DOCKED;
    bucket_countdown_ensure_timer();

}

static void system_ui_apply_link_status(uint8_t link_status)
{
    ui_screen_id_t target_screen;
    lv_obj_t *target_obj;
    int rotation;
    int16_t previous_link_status = g_applied_link_status;

    if (link_status == LINK_STATUS_DOCKED)
    {
        target_screen = UI_SCREEN_INDEX;
        target_obj = guider_ui.index;
        rotation = UI_ROTATION_DOCKED;
    }
    else if (link_status == LINK_STATUS_DETACHED)
    {
        target_screen = UI_SCREEN_WORKING;
        target_obj = guider_ui.working;
        rotation = UI_ROTATION_DETACHED;
    }
    else
    {
        SYSTEM_MANAGER_LOG_WARN("忽略未知 LINK_STATUS: 0x%02X", link_status);
        return;
    }

    if (g_applied_link_status == (int16_t)link_status)
    {
        return;
    }
    if (lv_port_set_rotation(rotation) != 0)
    {
        SYSTEM_MANAGER_LOG_ERROR("应用 LINK_STATUS 方向失败: status=0x%02X rotation=%d",
                                 link_status, rotation);
        return;
    }

    g_applied_link_status = (int16_t)link_status;
    if (link_status == LINK_STATUS_DETACHED &&
            previous_link_status == LINK_STATUS_DOCKED)
    {
        bucket_auto_start_detached();
    }
    else if (link_status == LINK_STATUS_DOCKED)
    {
        bucket_auto_start_cancel();
    }
    if (lv_screen_active() != target_obj)
    {
        navigate_to_screen(target_screen);
    }
    SYSTEM_MANAGER_LOG_USER("已应用 LINK_STATUS UI: status=0x%02X rotation=%d",
                            link_status, rotation);
}

static void system_ui_apply_link_status_async(void *user_data)
{
    uint8_t link_status = (uint8_t)(uintptr_t)user_data;

    system_ui_apply_link_status(link_status);
}

void system_ui_schedule_link_status(uint8_t link_status)
{
    if (link_status != LINK_STATUS_DETACHED &&
            link_status != LINK_STATUS_DOCKED)
    {
        SYSTEM_MANAGER_LOG_WARN("忽略未知 LINK_STATUS: 0x%02X", link_status);
        return;
    }

    lv_async_call(system_ui_apply_link_status_async,
                  (void *)(uintptr_t)link_status);
}

size_t system_mode_get_count(void)
{
    return g_mode_count;
}

const system_mode_t *system_mode_get(size_t index)
{
    if (index >= g_mode_count)
    {
        return NULL;
    }

    return &g_mode_settings[index];
}

void conf_mode_detail_save_clicked(lv_event_t *e)
{
    int code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    size_t index = (size_t)current_selected_mode_index;

    if (index >= g_mode_count)
    {
        return ;
    }

    system_mode_t *mode = &g_mode_settings[index];

    if (mode == NULL || mode->name[0] == '\0')
    {
        return ;
    }

    if (index >= SYSTEM_MODE_MAX_COUNT)
    {
        return ;
    }

    update_mode_from_detail_ui(mode);
    conf_mode_detail_keyboard_cleanup(&guider_ui);

    if (mode->name[0] == '\0')
    {
        return ;
    }

    persist_mode_settings();
    show_message_dialog(guider_ui.conf_mode_detail, "设置已保存");
}

void conf_mode_detail_delete_clicked(lv_event_t *e)
{
    int code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    size_t index = (size_t)current_selected_mode_index;


    size_t i;

    if (index >= g_mode_count)
    {
        return ;
    }

    for (i = index; i + 1 < g_mode_count; ++i)
    {
        g_mode_settings[i] = g_mode_settings[i + 1];
    }

    if (g_mode_count > 0)
    {
        g_mode_count--;
    }

    if (g_mode_count == 0)
    {
        init_default_modes();
    }

    persist_mode_settings();
    show_message_dialog(guider_ui.conf_mode_detail, "模式已删除");
}

void conf_mode_detail_restore_default_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED)
    {
        return;
    }

    if (current_selected_mode_index < 0 || current_selected_mode_index >= 3)
    {
        return;
    }

    system_mode_t *mode = &g_mode_settings[current_selected_mode_index];
    switch (current_selected_mode_index)
    {
    case 0:
        snprintf(mode->name, sizeof(mode->name), "助眠");
        mode->icon_id = 0;
        mode->time_sec = 900;
        mode->water_level = 0;
        mode->temperature = 37;
        mode->use_drug1 = true;
        mode->use_drug2 = false;
        mode->position = 1;
        break;
    case 1:
        snprintf(mode->name, sizeof(mode->name), "放松");
        mode->icon_id = 3;
        mode->time_sec = 1200;
        mode->water_level = 1;
        mode->temperature = 39;
        mode->use_drug1 = false;
        mode->use_drug2 = true;
        mode->position = 2;
        break;
    case 2:
        snprintf(mode->name, sizeof(mode->name), "驱寒");
        mode->icon_id = 11;
        mode->time_sec = 1500;
        mode->water_level = 2;
        mode->temperature = 41;
        mode->use_drug1 = true;
        mode->use_drug2 = true;
        mode->position = 3;
        break;
    default:
        return;
    }

    if (persist_mode_settings() != 0)
    {
        /* 持久化失败，可添加错误处理（例如日志） */
    }

    conf_mode_detail_init(&guider_ui);
    show_message_dialog(guider_ui.conf_mode_detail, "模式恢复成默认设置");
}
int system_location_save()
{
    size_t index = (size_t)current_selected_location_index;

    if (index >= g_location_count)
    {
        return -1;
    }

    system_location_t *location = &g_location_settings[index];

    if (location == NULL || location->name[0] == '\0')
    {
        return -1;
    }

    if (index >= SYSTEM_LOCATION_MAX_COUNT)
    {
        return -1;
    }

    //update_location_from_detail_ui(location);
    //conf_location_detail_keyboard_cleanup(&guider_ui);

    if (location->name[0] == '\0')
    {
        return -1;
    }

    return persist_location_settings();
}

void plus_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            action_temp_plus(ui);
        }
        break;
    }
    default:
        break;
    }
}
void minus_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            action_temp_minus(ui);
        }
        break;
    }
    default:
        break;
    }
}
void timer_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            action_time_loop(ui);
        }
        break;
    }
    default:
        break;
    }
}
void water_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            action_water_loop(ui);
        }
        break;
    }
    default:
        break;
    }
}

void action_temp_plus(lv_ui *ui)
{
    if (temp_set < TEMP_MAX)
    {
        temp_set++;
    }
    update_temp_set(ui);
}
void action_temp_minus(lv_ui *ui)
{
    if (temp_set > TEMP_MIN)
    {
        temp_set--;
    }
    update_temp_set(ui);
}
void action_time_loop(lv_ui *ui)
{
    //时间设置，每设置一次加300，最大1800,最小600
    timer_set += 300;
    if (timer_set > 1800)
    {
        timer_set = 600;
    }
    update_timer_set(ui);
}
void action_water_loop(lv_ui *ui)
{
    //水位设置，每设置一次加1，最大3,最小0
    water_level++;
    if (water_level > 2)
    {
        water_level = 0;
    }
    apply_water_level_ui(water_level);
}
void action_use_drug1_toggle(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            use_drug1 = !use_drug1;
            apply_use_drug_ui(use_drug1, use_drug2);
        }
        break;
    }
    default:
        break;
    }

}
void action_use_drug2_toggle(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            use_drug2 = !use_drug2;
            apply_use_drug_ui(use_drug1, use_drug2);
        }
        break;
    }
    default:
        break;
    }
}
void action_clean_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        //隐藏cont_2,显示cont_clean
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            is_self_cleaning = true;
            lv_obj_add_flag(ui->index_cont_2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(ui->index_cont_clean, LV_OBJ_FLAG_HIDDEN);
        }
        break;
    }
    default:
        break;
    }
}

void update_temp_set(lv_ui *ui)
{
    char temp_str[16];

    if (ui == NULL || ui->index_temp_value == NULL)
    {
        return;
    }

    if (temp_set < TEMP_MIN)
    {
        temp_set = TEMP_MIN;
    }
    else if (temp_set > TEMP_MAX)
    {
        temp_set = TEMP_MAX;
    }

    snprintf(temp_str, sizeof(temp_str), "%d℃", temp_set);
    lv_label_set_text(ui->index_temp_value, temp_str);
}
void update_timer_set(lv_ui *ui)
{
    lv_obj_t *time_info = ui->index_time_info;
    char time_str[24];
    int min = timer_set / 60;
    snprintf(time_str, sizeof(time_str), "时间： %dmin", min);
    lv_label_set_text(time_info, time_str);
}
void apply_water_level_ui(int level)
{
    if (level <= 0)
    {
        lv_obj_remove_flag(guider_ui.index_img_water1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.index_img_water2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.index_img_water3, LV_OBJ_FLAG_HIDDEN);
    }
    else if (level == 1)
    {
        lv_obj_remove_flag(guider_ui.index_img_water1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(guider_ui.index_img_water2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(guider_ui.index_img_water3, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(guider_ui.index_img_water1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(guider_ui.index_img_water2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(guider_ui.index_img_water3, LV_OBJ_FLAG_HIDDEN);
    }
}
void apply_use_drug_ui(bool use_drug1, bool use_drug2)
{
    if (use_drug1)
    {
        lv_obj_remove_flag(guider_ui.index_img_medicine1, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(guider_ui.index_img_medicine1, LV_OBJ_FLAG_HIDDEN);
    }

    if (use_drug2)
    {
        lv_obj_remove_flag(guider_ui.index_img_medicine2, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(guider_ui.index_img_medicine2, LV_OBJ_FLAG_HIDDEN);
    }
}
static void display_preparing_settings(lv_ui *ui)
{
    char temp_str[16];
    char time_str[24];
    int minutes;

    if (ui == NULL || ui->preparing_label_temprature == NULL ||
            ui->preparing_label_timeleft == NULL)
    {
        return;
    }

    snprintf(temp_str, sizeof(temp_str), "%d℃", temp_set);
    lv_label_set_text(ui->preparing_label_temprature, temp_str);

    minutes = (int)(timer_set / 60);
    snprintf(time_str, sizeof(time_str), "%d:00", minutes);
    lv_label_set_text(ui->preparing_label_timeleft, time_str);

    if (water_level <= 0)
    {
        lv_obj_remove_flag(ui->preparing_img_water1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->preparing_img_water2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->preparing_img_water3, LV_OBJ_FLAG_HIDDEN);
    }
    else if (water_level == 1)
    {
        lv_obj_remove_flag(ui->preparing_img_water1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui->preparing_img_water2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->preparing_img_water3, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(ui->preparing_img_water1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui->preparing_img_water2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui->preparing_img_water3, LV_OBJ_FLAG_HIDDEN);
    }

    if (use_drug1)
    {
        lv_obj_remove_flag(ui->preparing_img_medicine1, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ui->preparing_img_medicine1, LV_OBJ_FLAG_HIDDEN);
    }

    if (use_drug2)
    {
        lv_obj_remove_flag(ui->preparing_img_medicine2, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ui->preparing_img_medicine2, LV_OBJ_FLAG_HIDDEN);
    }
}
static void update_self_cleaning_progress(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }
    if (ui->preparing_cleaning_tips == NULL)
    {
        return;
    }

    //根据base_main_status的值显示不同的提示语
    if (base_main_status == 4)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "自动上水中");
    }
    else if (base_main_status == 5)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "注水完成保温中");
    }
    else if (base_main_status == 6)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "注水完成等待排水");
    }
    else if (base_main_status == 7)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "清洁喷淋中");
    }
    else if (base_main_status == 8)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "清洁排水");
    }
    else if (base_main_status == 9)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "清水喷淋中");
    }
    else if (base_main_status == 10)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "清洁排水");
    }
    else if (base_main_status == 11)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "清洁已完成，暖风烘干中");
    }
    else if (base_main_status == 12)
    {
        lv_label_set_text(ui->preparing_cleaning_tips, "排水中");
    }
    else if (base_main_status == 3)
    {
        if (g_is_draining)
        {
            //排水完成，可以开始自清洁了
            g_is_draining = false;
            serial_base_auto_clean();
        }
        else
        {
            lv_label_set_text(ui->preparing_cleaning_tips, "待机");
        }

    }

}

void working_plus_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            bucket_temp_step(ui, 1);
        }
        break;
    }
    default:
        break;
    }

}
void working_minus_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            bucket_temp_step(ui, -1);
        }
        break;
    }
    default:
        break;
    }

}
void working_timer_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            bucket_timer_step(ui);
        }
        break;
    }
    default:
        break;
    }
}
void working_constant_temperature_toggle(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    lv_ui *ui = lv_event_get_user_data(e);
    if (ui == NULL)
    {
        return;
    }
    constant_temperature = !constant_temperature;
    if (constant_temperature)
    {
        int16_t target = g_bucket_temp_edit.editing ? g_bucket_temp_edit.target_temp :
                         bucket_temp_base_value();
        temp_set = bucket_temp_clamp(target);
        lv_obj_remove_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_HIDDEN);
        serial_bucket_heat();
        bucket_temp_clear_edit(ui->working_label_temperature);
    }
    else
    {
        lv_obj_add_flag(ui->working_img_constant_temperature, LV_OBJ_FLAG_HIDDEN);
        serial_bucket_stop_heat();
        bucket_temp_clear_edit(ui->working_label_temperature);
    }
}
void working_sterilization_toggle(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    lv_ui *ui = lv_event_get_user_data(e);
    if (ui == NULL)
    {
        return;
    }
    sterilization = !sterilization;
    if (sterilization)
    {
        lv_obj_remove_flag(ui->working_img_sterilization, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(ui->working_img_sterilization, LV_OBJ_FLAG_HIDDEN);
    }
    serial_bucket_uv();
}
void working_massage_intensity_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }

    lv_ui *ui = lv_event_get_user_data(e);
    if (ui == NULL)
    {
        return;
    }
    massage_intensity++;
    if (massage_intensity > 3)
    {
        massage_intensity = 0;
    }
    if (massage_intensity == 0)
    {
        lv_obj_add_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
    }
    else if (massage_intensity == 1)
    {
        lv_obj_remove_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
    }
    else if (massage_intensity == 2)
    {
        lv_obj_remove_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(ui->working_img_massage_intensity0, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui->working_img_massage_intensity1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui->working_img_massage_intensity2, LV_OBJ_FLAG_HIDDEN);
    }
    serial_bucket_massage();
}
void working_show_return_dialog(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }

    lv_ui *ui = lv_event_get_user_data(e);
    if (ui == NULL)
    {
        return;
    }
    //显示回仓确认对话框
    going_to_location = 0;
    lv_obj_clear_flag(ui->working_dialog, LV_OBJ_FLAG_HIDDEN);
    working_return_dialog_start_countdown();
}
void working_pause_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED)
    {
        bucket_standby_start_press();
        return;
    }
    if ((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) &&
            !g_bucket_standby.submitted)
    {
        bucket_standby_cancel_press();
    }
}
void working_play_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    lv_ui *ui = lv_event_get_user_data(e);
    lv_obj_add_flag(ui->working_cont_pause, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui->working_cont_playing, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_working_play_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(g_working_pause_btn, LV_OBJ_FLAG_HIDDEN);
    serial_bucket_timer();
}
void working_refresh(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }
    g_bucket_temp_edit.display_label = ui->working_label_temperature;
    if (!g_bucket_temp_edit.editing)
    {
        bucket_temp_show_actual(ui->working_label_temperature);
    }
    bucket_countdown_refresh();

}
void working_page_init(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }
    if (g_working_timer != NULL)
    {
        lv_timer_del(g_working_timer);
        g_working_timer = NULL;
    }
    working_return_dialog_cleanup();
    g_bucket_countdown.display_label = ui->working_label_time_left;
    bucket_countdown_set_label_visible(!g_bucket_countdown.editing ||
                                       g_bucket_countdown.blink_visible);
    bucket_countdown_refresh();

    //这里添加一个定时器，用于更新实际温度
    g_working_timer = lv_timer_create_basic();
    lv_timer_set_period(g_working_timer, 1000);
    lv_timer_set_repeat_count(g_working_timer, -1);
    lv_timer_set_user_data(g_working_timer, ui);
    lv_timer_set_cb(g_working_timer, working_refresh);
    g_bucket_temp_edit.display_label = ui->working_label_temperature;
    if (!g_bucket_temp_edit.editing)
    {
        bucket_temp_show_actual(ui->working_label_temperature);
    }

    if (!_is_working_page_initialized)
    {
        if (ui->working != NULL)
        {
            lv_obj_add_event_cb(ui->working, working_screen_lifecycle_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
            lv_obj_add_event_cb(ui->working, working_screen_lifecycle_cb, LV_EVENT_DELETE, NULL);
        }

        //根据当前模式设置，载入模式按钮的图标和名称
        size_t index = 0;
        //调用imgbtn_create添加模式按钮时，
        int x_offset = 0;
        int y_offset = 0;
        int w = 170;
        int h = 178;
        int w_span = 24;
        int h_span = 22;
        //恒温
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn_constant_temp = imgbtn_create(ui->working_cont_1, &_btn_worm_RGB565A8_100x100,
                                      "恒温", x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn_constant_temp, working_constant_temperature_toggle, LV_EVENT_ALL, ui);
        //按摩
        index++;
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn_massage = imgbtn_create(ui->working_cont_1, &_massage_RGB565A8_100x100, "按摩",
                                              x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn_massage, working_massage_intensity_clicked, LV_EVENT_ALL, ui);
        //除菌
        index++;
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn_sterilization = imgbtn_create(ui->working_cont_1, &_bacteria_RGB565A8_100x100,
                                      "除菌", x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn_sterilization, working_sterilization_toggle, LV_EVENT_ALL, ui);
        //定时
        index++;
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn_timer = imgbtn_create(ui->working_cont_1, &_timer_RGB565A8_100x100, "定时",
                                            x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn_timer, working_timer_clicked, LV_EVENT_ALL, ui);
        //自动回仓
        index++;
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn_auto_return = imgbtn_create(ui->working_cont_1, &_auto_back_RGB565A8_100x100,
                                    "自动回仓", x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn_auto_return, working_show_return_dialog, LV_EVENT_ALL, ui);
        //设置
        index++;
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn_setting = imgbtn_create(ui->working_cont_1, &_setting_RGB565A8_100x100, "设置",
                                              x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn_setting, working_to_setting, LV_EVENT_ALL, ui);
        //定位
        index++;
        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
        lv_obj_t *btn_position = imgbtn_create(ui->working_cont_1, &_location_RGB565A8_100x100, "定位",
                                               x_offset, y_offset, w, h);
        lv_obj_add_event_cb(btn_position, working_to_location, LV_EVENT_ALL, ui);

        lv_obj_t *btn_pause = working_create_power_button(ui->working_cont_2, 0, 0, 146, 211);
        lv_obj_add_event_cb(btn_pause, working_pause_clicked, LV_EVENT_ALL, ui);

        lv_obj_t *btn_play = working_create_power_button(ui->working_cont_2, 0, 0, 146, 211);
        lv_obj_add_event_cb(btn_play, working_play_clicked, LV_EVENT_ALL, ui);

        working_bind_play_pause_buttons(btn_pause, btn_play);
        //lv_obj_add_flag(btn_pause, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t *btn_plus = imgbtn_create(ui->working_cont_2, &_plus_RGB565A8_50x50, "", 171, 0, 92, 211);
        lv_obj_add_event_cb(btn_plus, working_plus_clicked, LV_EVENT_ALL, ui);

        lv_obj_t *btn_minus = imgbtn_create(ui->working_cont_2, &_minus_RGB565A8_50x50, "", 287, 0, 92,
                                            211);
        lv_obj_add_event_cb(btn_minus, working_minus_clicked, LV_EVENT_ALL, ui);

        lv_obj_set_style_shadow_width(ui->working_cont_playing, 8, 0);
        lv_obj_set_style_shadow_ofs_x(ui->working_cont_playing, 3, 0);
        lv_obj_set_style_shadow_ofs_y(ui->working_cont_playing, 3, 0);
        lv_obj_set_style_shadow_color(ui->working_cont_playing, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(ui->working_cont_playing, LV_OPA_40, 0);

        lv_obj_set_style_shadow_width(ui->working_cont_pause, 8, 0);
        lv_obj_set_style_shadow_ofs_x(ui->working_cont_pause, 3, 0);
        lv_obj_set_style_shadow_ofs_y(ui->working_cont_pause, 3, 0);
        lv_obj_set_style_shadow_color(ui->working_cont_pause, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(ui->working_cont_pause, LV_OPA_40, 0);


        _is_working_page_initialized = true;
    }

    //让桶子处于暂停状态
    lv_obj_remove_flag(ui->working_cont_playing, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui->working_cont_pause, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(g_working_pause_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_working_play_btn, LV_OBJ_FLAG_HIDDEN);
    bucket_working_sync_function_icons(ui);

}
