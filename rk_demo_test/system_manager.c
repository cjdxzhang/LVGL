#include "system_manager.h"
#include "serial.h"
#include "custom_imgbtn.h"
#include "custom.h"
#include "widgets_init.h"
#include "app_manager.h"
#include "app_log.h"
#include "lv_port_init.h"
#include "backlight_control.h"
#include "cleaning_animation.h"
#include "self_clean_flow.h"
#include "events_init.h"
#include "robot_tcp.h"
#include "voice_command_service.h"
#include "page_initialized.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define SYSTEM_MANAGER_LOG_USER(...) APP_LOG_USER("SYSTEM_MANAGER", __VA_ARGS__)
#define SYSTEM_MANAGER_LOG_WARN(...) APP_LOG_WARN("SYSTEM_MANAGER", __VA_ARGS__)
#define SYSTEM_MANAGER_LOG_ERROR(...) APP_LOG_ERROR("SYSTEM_MANAGER", __VA_ARGS__)

#define MODE_SETTINGS_DIR "./temp"
#define MODE_SETTINGS_PATH "./temp/mode_settings.bin"
#define MODE_SETTINGS_MAGIC 0x4d4f4445u
#define MODE_SETTINGS_VERSION 3u
#define MODE_SETTINGS_TMP_PATH MODE_SETTINGS_PATH ".tmp"
#define LOCATION_SETTINGS_DIR "./temp"
#define LOCATION_SETTINGS_PATH "./temp/location_settings.bin"
#define LOCATION_SETTINGS_MAGIC 0x4c4f4341u
#define LOCATION_SETTINGS_VERSION 2u
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
#define UI_ROTATION_DETACHED 180
#define UI_ROTATION_DOCKED 0
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
#define SELF_CLEAN_FINISH_AUTO_RETURN_MS 120000
#define MCU_REPORT_WATCHDOG_PERIOD_MS 1000u
#define MCU_REPORT_TIMEOUT_MS 5000u
#define MCU_OUTAGE_GUARD_SHOW_DELAY_MS 200u
#define MCU_LOW_BATTERY_THRESHOLD_PERCENT 20u
/* 与版本2磁盘布局保持一致，仅用于一次性迁移。 */
typedef struct
{
    char name[SYSTEM_MODE_NAME_LEN];
    int icon_id;
    uint32_t time_sec;
    uint8_t water_level;
    int16_t temperature;
    bool use_drug1;
    bool use_drug2;
    int position;
} system_mode_v2_t;

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

/* 旧位置文件只迁移名称对应的偏好，绝不恢复站点列表和坐标。 */
typedef struct
{
    char name[32];
    int index;
    int default_mode_index;
    system_location_coord_t coord;
} system_location_v1_t;

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

typedef struct
{
    StationInfo_t target_station;
    bool has_target;
    bool active;
    bool completion_handled;
    bool navigation_arrived;
    uint8_t previous_base_status;
    uint32_t last_report_sequence;
    int16_t target_temperature;
    uint32_t target_time_sec;
} auto_water_navigation_flow_t;

static cleaning_ui_t cleaning_ui;
static bucket_temp_edit_state_t g_bucket_temp_edit = {0};
static bucket_countdown_state_t g_bucket_countdown = {0};
static bucket_standby_state_t g_bucket_standby = {0};
static self_clean_flow_t g_self_clean_flow = {0};
static auto_water_navigation_flow_t g_auto_water_navigation = {0};
static char g_mode_edit_station_name[SYSTEM_LOCATION_NAME_LEN];
static system_base_flow_t g_base_flow = SYSTEM_BASE_FLOW_IDLE;

static int16_t g_applied_link_status = -1;
static lv_timer_t *g_mcu_report_watchdog_timer = NULL;
static uint32_t g_mcu_watchdog_last_sequence = 0u;
static uint32_t g_mcu_watchdog_last_report_tick = 0u;
static system_mcu_link_state_t g_mcu_link_state = SYSTEM_MCU_LINK_UNKNOWN;
static uint8_t g_mcu_pending_link_status = LINK_STATUS_DOCKED;
static bool g_mcu_status_ui_pending = false;
static bool g_mcu_low_battery_handled = false;

/* 全局变量定义（在头文件中用 extern 声明，在此处给出唯一定义） */
int16_t last_link_status =
    LINK_STATUS_DOCKED; /* 开机默认位于基站，收到有效帧后再更新 */
int16_t temp_set = 35;        /* 温度设置值，单位 °C */
uint32_t timer_set = 600;    /* 定时设置值，单位 秒 */
uint32_t remaining_seconds = 0; /* 桶体本地倒计时剩余秒数 */
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

static system_mode_t g_mode_settings[SYSTEM_MODE_MAX_COUNT];
system_location_t g_location_settings[SYSTEM_LOCATION_MAX_COUNT];
static size_t g_mode_count = 0;
size_t g_location_count = 0;
int current_selected_mode_index = -1;
int current_selected_location_index = 0;
static lv_obj_t *g_conf_mode_cont_2 = NULL;

static uint32_t g_preparing_countdown_remaining = 0;
static lv_timer_t *g_preparing_page_timer = NULL;
static lv_obj_t *g_self_clean_finish_button = NULL;
static lv_timer_t *g_self_clean_finish_timer = NULL;
static lv_timer_t *g_working_return_dialog_timer = NULL;
static lv_timer_t *g_working_timer = NULL;
lv_timer_t *g_auto_drain_timer = NULL;
static int32_t g_working_return_dialog_countdown = 0;

/* ========== 机器人导航相关 ========== */
static float g_nav_target_x = 0.0f;       /* 导航目标X坐标 */
static float g_nav_target_y = 0.0f;       /* 导航目标Y坐标 */
static float g_nav_target_z = 0.0f;       /* 导航目标Z朝向 */
static int   g_nav_timeout_count = 0;     /* 导航超时计数（秒） */
#define NAV_TIMEOUT_SEC   120              /* 导航最大等待秒数 */
#define NAV_ARRIVED_TOL   0.5f             /* 到达判定距离阈值（米） */

static bool g_return_base_pending = false;
static uint32_t g_return_base_report_sequence = 0u;
static uint32_t g_mcu_docked_report_sequence = 0u;
static lv_timer_t *g_nav_check_timer = NULL; /* 导航状态轮询定时器 */

static int persist_mode_settings(void);
static int persist_location_settings(void);

static void build_mode_dropdown_options(char *options, size_t options_size);
static void conf_location_default_mode_changed(lv_event_t *e);
static void conf_location_apply_label_style(lv_obj_t *label);
static void conf_location_apply_textarea_style(lv_ui *ui, lv_obj_t *textarea);
static void conf_location_apply_dropdown_style(lv_obj_t *dropdown);
static void conf_mode_detail_keyboard_cleanup(lv_ui *ui);
static void conf_mode_detail_screen_lifecycle_cb(lv_event_t *e);
static lv_obj_t *conf_location_create_info_label(lv_obj_t *parent);
static void display_preparing_settings(lv_ui *ui);
static void refresh_auto_water_status(lv_timer_t *timer);
static void mcu_report_watchdog_timer_cb(lv_timer_t *timer);
static void update_self_cleaning_progress(lv_ui *ui);
static void preparing_screen_lifecycle_cb(lv_event_t *e);
static void self_clean_finish_cleanup(void);
static void self_clean_finish_show(lv_ui *ui);
static bool system_location_is_selectable_station(const StationInfo_t *station);
static int system_location_find_by_station_id(const system_location_t *locations,
        size_t count,
        int station_id);
static int system_location_find_by_name(const system_location_t *locations,
                                        size_t count,
                                        const char *name);
static void system_location_update_entry_from_station(system_location_t *location,
        const StationInfo_t *station);
static int system_location_resolve_station(const char *target_name,
        int target_location_index,
        StationInfo_t *station);
static void system_location_sync_from_robot_cache_async(void *user_data);
static void system_location_refresh_pages(lv_ui *ui);
static void mode_station_dropdown_refresh(lv_ui *ui);
static void mode_station_changed(lv_event_t *e);
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
static void bucket_start_after_auto_water_navigation(void);
static bool auto_water_navigation_try_start_bucket(void);
static void bucket_working_sync_function_icons(lv_ui *ui);
static void bucket_standby_cancel_press(void);
static void bucket_standby_page_cleanup(void);
static void index_sync_selection_ui(lv_ui *ui);

static void auto_water_navigation_flow_arm(auto_water_navigation_flow_t *flow,
        const StationInfo_t *target, uint32_t report_sequence)
{
    if (flow == NULL)
    {
        return;
    }
    memset(flow, 0, sizeof(*flow));
    if (target != NULL)
    {
        flow->target_station = *target;
        flow->has_target = true;
    }
    flow->active = true;
    flow->last_report_sequence = report_sequence;
    flow->target_temperature = temp_set;
    flow->target_time_sec = timer_set;
}

static void auto_water_navigation_flow_cancel(auto_water_navigation_flow_t *flow)
{
    if (flow != NULL)
    {
        memset(flow, 0, sizeof(*flow));
    }
}

static bool auto_water_navigation_flow_observe(auto_water_navigation_flow_t *flow,
        uint8_t base_status, uint32_t report_sequence)
{
    bool completed;

    if (flow == NULL || !flow->active || flow->completion_handled ||
            report_sequence == flow->last_report_sequence)
    {
        return false;
    }
    flow->last_report_sequence = report_sequence;
    completed = flow->previous_base_status != 5u && base_status == 5u;
    flow->previous_base_status = base_status;
    if (completed)
    {
        flow->completion_handled = true;
    }
    return completed;
}


const char *system_base_flow_name(system_base_flow_t flow)
{
    switch (flow)
    {
    case SYSTEM_BASE_FLOW_IDLE:
        return "IDLE";
    case SYSTEM_BASE_FLOW_WATERING:
        return "WATERING";
    case SYSTEM_BASE_FLOW_SELF_CLEANING:
        return "SELF_CLEANING";
    case SYSTEM_BASE_FLOW_MOVING:
        return "MOVING";
    default:
        return "UNKNOWN";
    }
}

system_base_flow_t system_base_flow_get(void)
{
    return g_base_flow;
}

static const char *system_action_source_name(system_action_source_t source)
{
    switch (source)
    {
    case SYSTEM_ACTION_SOURCE_UI:
        return "ui";
    case SYSTEM_ACTION_SOURCE_VOICE:
        return "voice";
    case SYSTEM_ACTION_SOURCE_APP:
        return "app";
    default:
        return "unknown";
    }
}

bool system_base_flow_try_start(system_base_flow_t flow, const char *source)
{
    if (flow == SYSTEM_BASE_FLOW_MOVING && g_base_flow == SYSTEM_BASE_FLOW_MOVING)
    {
        return true;
    }
    if (flow == SYSTEM_BASE_FLOW_IDLE || g_base_flow != SYSTEM_BASE_FLOW_IDLE)
    {
        SYSTEM_MANAGER_LOG_WARN("基站流程启动被拒绝: source=%s request=%s active=%s",
                                source == NULL ? "<unknown>" : source,
                                system_base_flow_name(flow),
                                system_base_flow_name(g_base_flow));
        return false;
    }
    g_base_flow = flow;
    SYSTEM_MANAGER_LOG_USER("基站流程已占用: source=%s flow=%s",
                            source == NULL ? "<unknown>" : source,
                            system_base_flow_name(flow));
    return true;
}

system_command_decision_t system_base_flow_request_start(
    system_base_flow_t flow, system_action_source_t source)
{
    const char *source_name = system_action_source_name(source);
    system_command_decision_t decision =
    {
        SYSTEM_COMMAND_ACCEPTED, "BASE", system_base_flow_name(flow), "accepted"
    };

    if (flow == SYSTEM_BASE_FLOW_IDLE)
    {
        decision.result = SYSTEM_COMMAND_STATE_REJECTED;
        decision.state = "INVALID";
        decision.reason = "invalid-flow";
        return decision;
    }
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        SYSTEM_MANAGER_LOG_WARN(
            "基站流程启动被拒绝: source=%s request=%s reason=no-valid-mcu-status",
            source_name, system_base_flow_name(flow));
        decision.result = SYSTEM_COMMAND_STATE_REJECTED;
        decision.domain = "POSITION";
        decision.state = "UNKNOWN";
        decision.reason = "no-valid-mcu-status";
        return decision;
    }
    if (flow != SYSTEM_BASE_FLOW_MOVING && last_link_status != LINK_STATUS_DOCKED)
    {
        SYSTEM_MANAGER_LOG_WARN(
            "基站流程启动被拒绝: source=%s request=%s reason=docked-required link=%d",
            source_name, system_base_flow_name(flow), last_link_status);
        decision.result = SYSTEM_COMMAND_STATE_REJECTED;
        decision.domain = "POSITION";
        decision.state = "DETACHED";
        decision.reason = "docked-required";
        return decision;
    }
    if (!system_base_flow_try_start(flow, source_name))
    {
        decision.result = SYSTEM_COMMAND_INTERLOCK_REJECTED;
        decision.state = system_base_flow_name(g_base_flow);
        decision.reason = "base-flow-conflict";
    }
    return decision;
}

// 释放当前基站流程占用，返回到空闲状态
void system_base_flow_finish(system_base_flow_t expected, const char *source)
{
    if (g_base_flow != expected)
    {
        SYSTEM_MANAGER_LOG_WARN("忽略不匹配的基站流程释放: source=%s expected=%s active=%s",
                                source == NULL ? "<unknown>" : source,
                                system_base_flow_name(expected),
                                system_base_flow_name(g_base_flow));
        return;
    }
    SYSTEM_MANAGER_LOG_USER("基站流程已释放: source=%s flow=%s",
                            source == NULL ? "<unknown>" : source,
                            system_base_flow_name(expected));
    g_base_flow = SYSTEM_BASE_FLOW_IDLE;
}

system_bucket_state_t system_bucket_state_get(void)
{
    if (g_base_flow == SYSTEM_BASE_FLOW_MOVING)
    {
        return SYSTEM_BUCKET_STATE_MOVING;
    }
    if (g_bucket_standby.sleeping || g_bucket_standby.submitted)
    {
        return SYSTEM_BUCKET_STATE_STANDBY;
    }
    return SYSTEM_BUCKET_STATE_NORMAL;
}

const char *system_bucket_state_name(system_bucket_state_t state)
{
    switch (state)
    {
    case SYSTEM_BUCKET_STATE_NORMAL:
        return "NORMAL";
    case SYSTEM_BUCKET_STATE_STANDBY:
        return "STANDBY";
    case SYSTEM_BUCKET_STATE_MOVING:
        return "MOVING";
    default:
        return "UNKNOWN";
    }
}

static int system_start_navigation(lv_obj_t *dialog_parent,
                                   const char *target_name,
                                   int target_location_index,
                                   const StationInfo_t *snapshot)
{
    StationInfo_t resolved_station = {0};
    bool was_moving = g_base_flow == SYSTEM_BASE_FLOW_MOVING;
    int location_index;
    int result;

    if (system_base_flow_request_start(SYSTEM_BASE_FLOW_MOVING,
                                       SYSTEM_ACTION_SOURCE_UI).result !=
            SYSTEM_COMMAND_ACCEPTED)
    {
        system_auto_water_navigation_cancel();
        return -1;
    }

    if (snapshot != NULL)
    {
        resolved_station = *snapshot;
        result = 0;
    }
    else
    {
        result = system_location_resolve_station(target_name, target_location_index,
                 &resolved_station);
    }

    if (result != 0)
    {
        SYSTEM_MANAGER_LOG_ERROR("导航目标解析失败: request_id=%d name=%s ret=%d",
                                 target_location_index,
                                 target_name == NULL ? "<null>" : target_name,
                                 result);
        show_message_dialog_and_navigate(dialog_parent,
                                         "导航失败: 未找到目标站点",
                                         UI_SCREEN_INDEX);
        system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                                "navigation-resolve-failed");
        system_auto_water_navigation_cancel();
        return result;
    }
    location_index = system_location_find_by_station_id(g_location_settings,
                     g_location_count,
                     resolved_station.id);
    if (location_index < 0 && target_name != NULL && target_name[0] != '\0')
    {
        location_index = system_location_find_by_name(g_location_settings,
                         g_location_count,
                         target_name);
    }
    if (location_index >= 0 && snapshot == NULL)
    {
        system_location_update_entry_from_station(&g_location_settings[location_index],
                &resolved_station);
        if (persist_location_settings() != 0)
        {
            SYSTEM_MANAGER_LOG_WARN("导航前刷新定位配置保存失败: station_id=%d",
                                    resolved_station.id);
        }
    }

    serial_base_standby();
    result = RobotTcp_SendGoto(resolved_station.x,
                               resolved_station.y,
                               resolved_station.z);

    if (result != 0)
    {
        SYSTEM_MANAGER_LOG_ERROR("导航启动失败: station_id=%d name=%s ret=%d",
                                 resolved_station.id,
                                 resolved_station.name,
                                 result);
        show_message_dialog_and_navigate(dialog_parent,
                                         "导航失败: 未发送到代理",
                                         UI_SCREEN_INDEX);
        system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                                "navigation-submit-failed");
        system_auto_water_navigation_cancel();
        return result;
    }

    g_nav_target_x = resolved_station.x;
    g_nav_target_y = resolved_station.y;
    g_nav_target_z = resolved_station.z;
    g_nav_timeout_count = 0;
    going_to_location = resolved_station.id;
    SYSTEM_MANAGER_LOG_USER("导航目标已解析: request_id=%d resolved_id=%d name=%s pos=(%.2f, %.2f, %.2f)",
                            target_location_index,
                            resolved_station.id,
                            resolved_station.name,
                            resolved_station.x,
                            resolved_station.y,
                            resolved_station.z);
    g_return_base_pending = false;
    g_auto_water_navigation.navigation_arrived = false;
    to_preparing_flat = 3;
    if (lv_screen_active() == guider_ui.preparing &&
            (!was_moving || g_nav_check_timer == NULL))
    {
        /* 同页业务模式转换不会触发加载事件，需要显式启动移动显示和检测。 */
        preparing_page_init(&guider_ui);
    }
    else if (!was_moving && !navigate_to_screen(UI_SCREEN_PREPARING))
    {
        /* 底盘命令已经提交，保持 MOVING 互斥直到状态轮询结束。 */
        SYSTEM_MANAGER_LOG_ERROR("导航已提交但准备页切换失败，保持移动互斥");
        system_auto_water_navigation_cancel();
    }
    return 0;
}

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

// 启动返回对话框倒计时
static void working_return_dialog_start_countdown(void)
{
    working_return_dialog_cleanup();

    if (!working_return_dialog_obj_is_valid(guider_ui.working_btn_yes) ||
            !working_return_dialog_obj_is_valid(guider_ui.working_btn_no) ||
            !working_return_dialog_obj_is_valid(guider_ui.working_btn_yes_label) ||
            !working_return_dialog_obj_is_valid(guider_ui.working_btn_no_label))
    {
        return;
    }

    lv_obj_set_style_text_font(guider_ui.working_btn_yes_label,
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

/* 回仓使用本轮命令后的 MCU 连接确认，不复用普通导航目标坐标。 */
static bool preparing_return_base_try_finish(void)
{
    if (!g_return_base_pending || g_base_flow != SYSTEM_BASE_FLOW_MOVING ||
            to_preparing_flat != 4 ||
            system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE ||
            last_link_status != LINK_STATUS_DOCKED ||
            __atomic_load_n(&g_mcu_docked_report_sequence, __ATOMIC_ACQUIRE) ==
            g_return_base_report_sequence)
    {
        return false;
    }
    if (!navigate_to_screen(UI_SCREEN_INDEX))
    {
        return false;
    }
    g_return_base_pending = false;
    current_location = 0;
    to_preparing_flat = 0;
    preparing_common_cleanup();
    system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING, "return-base-connected");
    return true;
}

/* ========== 导航轮询定时器回调 ========== */
static void preparing_nav_check_timer_cb(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }

    if (to_preparing_flat == 4)
    {
        (void)preparing_return_base_try_finish();
        return;
    }

    RobotState_t state = {0};

    (void)RobotTcp_RequestStatus();
    RobotTcp_GetState(&state);

    /* 打印目标位置信息 */
    SYSTEM_MANAGER_LOG_USER("[导航检查] 目标位置: x=%.3f, y=%.3f, z=%.3f",
                            g_nav_target_x, g_nav_target_y, g_nav_target_z);

    /* 打印当前位置信息 */
    SYSTEM_MANAGER_LOG_USER("[导航检查] 当前位置: x=%.3f, y=%.3f, z=%.3f",
                            state.pos.x, state.pos.y, state.pos.z);

    /* 打印电量和地图信息 */
    SYSTEM_MANAGER_LOG_USER("[导航检查] 电量=%.1f%%, 地图=%s, innavmap=%d, emgStop=%d",
                            state.battery_percent,
                            state.map_name,
                            state.in_nav_map,
                            state.is_emg_stop);

    /* 计算当前位置与目标点的欧几里得距离 */
    float dx = state.pos.x - g_nav_target_x;
    float dy = state.pos.y - g_nav_target_y;
    float dist = sqrtf(dx * dx + dy * dy);

    SYSTEM_MANAGER_LOG_USER("[导航检查] 距离目标: %.3f (容差: %.3f)", dist, NAV_ARRIVED_TOL);

    if (dist <= NAV_ARRIVED_TOL)
    {
        SYSTEM_MANAGER_LOG_USER("[导航检查] ===== 到达目的地! =====");
        SYSTEM_MANAGER_LOG_USER("[导航检查] target=(%.3f, %.3f), current=(%.3f, %.3f), dist=%.3f",
                                g_nav_target_x, g_nav_target_y,
                                state.pos.x, state.pos.y, dist);
        g_nav_timeout_count = 0;
        if (g_auto_water_navigation.active &&
                g_auto_water_navigation.completion_handled)
        {
            if (g_nav_check_timer != NULL)
            {
                lv_timer_delete(g_nav_check_timer);
                g_nav_check_timer = NULL;
            }
            g_auto_water_navigation.navigation_arrived = true;
            if (!auto_water_navigation_try_start_bucket())
            {
                SYSTEM_MANAGER_LOG_USER(
                    "自动上水导航已到达，等待 LINK_STATUS=0x00");
            }
            return;
        }
        if (last_link_status == LINK_STATUS_DETACHED)
        {
            system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                                    "navigation-arrived");
            preparing_move_finished(timer);
            if (g_nav_check_timer != NULL)
            {
                lv_timer_delete(g_nav_check_timer);
                g_nav_check_timer = NULL;
            }
            return;
        }
    }

    /* 超时检查 */
    g_nav_timeout_count++;
    SYSTEM_MANAGER_LOG_USER("[导航检查] 已等待 %d 秒 / 超时阈值 %d 秒",
                            g_nav_timeout_count, NAV_TIMEOUT_SEC);

    if (g_nav_timeout_count >= NAV_TIMEOUT_SEC)
    {
        SYSTEM_MANAGER_LOG_WARN("[导航检查] ===== 导航超时! 取消导航 =====");
        SYSTEM_MANAGER_LOG_WARN("[导航检查] 超时时间: %ds, 最后位置: (%.3f, %.3f)",
                                NAV_TIMEOUT_SEC, state.pos.x, state.pos.y);
        if (g_nav_check_timer != NULL)
        {
            lv_timer_delete(g_nav_check_timer);
            g_nav_check_timer = NULL;
        }
        g_nav_timeout_count = 0;
        system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                                "navigation-timeout");
        system_auto_water_navigation_cancel();
        /* 超时后显示提示，回到首页 */
        show_message_dialog_and_navigate(ui->preparing,
                                         "导航超时，请检查机器人状态",
                                         UI_SCREEN_INDEX);
    }
}

static void self_clean_finish_return_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    if (g_self_clean_finish_timer != NULL)
    {
        lv_timer_delete(g_self_clean_finish_timer);
        g_self_clean_finish_timer = NULL;
    }
    navigate_to_screen(UI_SCREEN_INDEX);
}

static void self_clean_finish_auto_return_cb(lv_timer_t *timer)
{
    g_self_clean_finish_timer = NULL;
    (void)timer;
    navigate_to_screen(UI_SCREEN_INDEX);
}

// 清空自清洁流程中的定时器和按钮对象
static void self_clean_finish_cleanup(void)
{
    if (g_self_clean_finish_timer != NULL)
    {
        lv_timer_delete(g_self_clean_finish_timer);
        g_self_clean_finish_timer = NULL;
    }
    if (g_self_clean_finish_button != NULL)
    {
        if (lv_obj_is_valid(g_self_clean_finish_button))
        {
            lv_obj_delete(g_self_clean_finish_button);
        }
        g_self_clean_finish_button = NULL;
    }
}

static void self_clean_finish_show(lv_ui *ui)
{
    lv_obj_t *icon;
    lv_obj_t *label;

    if (ui == NULL || ui->preparing_cont_3 == NULL)
    {
        return;
    }

    self_clean_finish_cleanup();
    preparing_stop_button_set_hidden(true);

    g_self_clean_finish_button = imgbtn_create(ui->preparing_cont_3,
                                 &_home_RGB565A8_100x100,
                                 "返回首页\n(2min后自动)",
                                 0, 215, 170, 187);
    if (g_self_clean_finish_button == NULL)
    {
        return;
    }

    lv_obj_set_style_radius(g_self_clean_finish_button, 20, 0);
    icon = lv_obj_get_child(g_self_clean_finish_button, 0);
    label = lv_obj_get_child(g_self_clean_finish_button, 1);
    if (icon != NULL && lv_obj_is_valid(icon))
    {
        lv_image_set_scale(icon, LV_SCALE_NONE);
        lv_obj_align(icon, LV_ALIGN_CENTER, 0, -(187 / 6));
    }
    if (label != NULL && lv_obj_is_valid(label))
    {
        lv_obj_set_size(label, 170, 70);
        lv_obj_set_style_text_font(label,
                                   &lv_font_SourceHanSansSC_Regular_25,
                                   0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 187 / 6 + 12);
    }
    lv_obj_add_event_cb(g_self_clean_finish_button,
                        self_clean_finish_return_cb,
                        LV_EVENT_CLICKED,
                        NULL);

    g_self_clean_finish_timer = lv_timer_create(
                                    self_clean_finish_auto_return_cb,
                                    SELF_CLEAN_FINISH_AUTO_RETURN_MS,
                                    ui);
    if (g_self_clean_finish_timer != NULL)
    {
        lv_timer_set_repeat_count(g_self_clean_finish_timer, 1);
    }
}

// 清理准备页的定时器和对话框资源
void preparing_common_cleanup(void)
{
    g_return_base_pending = false;
    if (g_nav_check_timer != NULL)
    {
        lv_timer_delete(g_nav_check_timer);
        g_nav_check_timer = NULL;
    }
    g_nav_timeout_count = 0;

    if (g_preparing_page_timer != NULL)
    {
        lv_timer_delete(g_preparing_page_timer);
        g_preparing_page_timer = NULL;
    }

    stop_dialog_cleanup();
    message_dialog_cleanup();
}

void auto_water_page_cleanup(void)
{
    system_auto_water_navigation_cancel();
}

void self_clean_page_cleanup(void)
{
    cleaning_animation_cleanup();
    self_clean_flow_stop(&g_self_clean_flow);
    self_clean_finish_cleanup();
}

static void preparing_screen_lifecycle_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_UNLOAD_START || code == LV_EVENT_DELETE)
    {
        if (to_preparing_flat == 1)
        {
            auto_water_page_cleanup();
        }
        else if (to_preparing_flat == 2)
        {
            self_clean_page_cleanup();
        }
        preparing_common_cleanup();
    }
}

void system_manager_working_page_cleanup(void)
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

// 获取桶体温度的基准值，用于显示和编辑
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

// 取消桶体温度编辑状态的闪烁效果，并重置相关计数器
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

// 显示桶体温度的实际值，如果没有有效的实际温度，则显示回退值
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

// 桶体页面调整温度
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
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
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
    system_bucket_timer_apply();
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
    system_bucket_timer_apply();
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
    system_bucket_timer_apply();
}

// 刷新桶体工作页面图标显示
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

static bool system_bucket_control_is_allowed(const char *operation)
{
    system_bucket_state_t bucket_state;

    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        SYSTEM_MANAGER_LOG_WARN("桶体功能设置被拒绝: operation=%s reason=mcu-offline",
                                operation);
        return false;
    }
    if (last_link_status != LINK_STATUS_DETACHED)
    {
        SYSTEM_MANAGER_LOG_WARN("桶体功能设置被拒绝: operation=%s reason=detached-required",
                                operation);
        return false;
    }
    bucket_state = system_bucket_state_get();
    if (bucket_state != SYSTEM_BUCKET_STATE_NORMAL)
    {
        SYSTEM_MANAGER_LOG_WARN("桶体功能设置被拒绝: operation=%s bucket=%s",
                                operation,
                                system_bucket_state_name(bucket_state));
        return false;
    }
    if (g_base_flow != SYSTEM_BASE_FLOW_IDLE)
    {
        SYSTEM_MANAGER_LOG_WARN("桶体功能设置被拒绝: operation=%s base=%s",
                                operation,
                                system_base_flow_name(g_base_flow));
        return false;
    }
    return true;
}

bool system_bucket_heat_set(bool enable)
{
    int16_t target;

    if (!system_bucket_control_is_allowed(enable ? "heat" : "stop-heat"))
    {
        return false;
    }

    constant_temperature = enable;
    if (enable)
    {
        target = g_bucket_temp_edit.editing ? g_bucket_temp_edit.target_temp :
                 bucket_temp_base_value();
        temp_set = bucket_temp_clamp(target);
        serial_bucket_heat();
    }
    else
    {
        serial_bucket_stop_heat();
    }
    bucket_temp_clear_edit(guider_ui.working_label_temperature);
    bucket_working_sync_function_icons(&guider_ui);
    return true;
}

bool system_bucket_massage_set(int intensity)
{
    if (intensity < 0 || intensity > 3)
    {
        SYSTEM_MANAGER_LOG_WARN("按摩档位设置被拒绝: intensity=%d", intensity);
        return false;
    }
    if (!system_bucket_control_is_allowed("massage"))
    {
        return false;
    }

    massage_intensity = intensity;
    serial_bucket_massage();
    bucket_working_sync_function_icons(&guider_ui);
    return true;
}

bool system_bucket_uv_set(bool enable)
{
    if (!system_bucket_control_is_allowed("uv"))
    {
        return false;
    }

    sterilization = enable;
    serial_bucket_uv();
    bucket_working_sync_function_icons(&guider_ui);
    return true;
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

// 取消倒计时，更新时间为0
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

// 清理桶体运行时的状态，包括温度编辑、倒计时和功能状态
static void bucket_runtime_cleanup(void)
{
    bucket_temp_clear_edit(guider_ui.working_label_temperature);
    bucket_countdown_stop_for_standby();
    bucket_working_clear_function_state();
}

static void bucket_standby_commit(void)
{
    if (g_bucket_standby.submitted)
    {
        return;
    }
    g_bucket_standby.submitted = true;
    g_bucket_standby.pressing = false;

    SYSTEM_MANAGER_LOG_USER("暂停键长按满3秒，进入息屏待机");
    system_bucket_stop();

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
        SYSTEM_MANAGER_LOG_ERROR("关闭屏幕背光失败，A1和桶体状态清理已完成");
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
    if (g_bucket_countdown.expiry_handled)
    {
        return;
    }
    g_bucket_countdown.expiry_handled = true;
    SYSTEM_MANAGER_LOG_USER("桶体定时到期，统一停止泡脚流程");
    system_bucket_stop();
    (void)voice_command_service_play_prompt(VOICE_PROMPT_FOOTBATH_FINISH);
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

static void bucket_countdown_cancel(void)
{
    bucket_countdown_cancel_edit();
    remaining_seconds = 0u;
    g_bucket_countdown.remaining_seconds = 0u;
    g_bucket_countdown.deadline_ms = 0u;
    g_bucket_countdown.active = false;
    g_bucket_countdown.has_value = true;
    g_bucket_countdown.expiry_handled = true;
    bucket_countdown_update_label();
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

    system_bucket_timer_apply();
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

/*
 * 桶体定时的统一业务入口：A6 下发与屏幕倒计时保持为同一次操作。
 * timer_set 为0时仍向 MCU 下发取消定时值，但本地只取消倒计时，
 * 不进入自然到期处理，因此不会额外下发 A1或播放完成提示。
 */
void system_bucket_timer_apply(void)
{
    bucket_auto_start_cancel();
    serial_bucket_timer();

    if (timer_set == 0u)
    {
        bucket_countdown_cancel();
        SYSTEM_MANAGER_LOG_USER("桶体定时已取消并下发 A6");
        return;
    }

    bucket_countdown_cancel_edit();
    bucket_countdown_start(timer_set);
    SYSTEM_MANAGER_LOG_USER("桶体定时已下发 A6并重启倒计时: %us",
                            (unsigned int)timer_set);
}

static void bucket_start_after_auto_water_navigation(void)
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
    SYSTEM_MANAGER_LOG_USER("自动上水导航完成后下发 A2: target_temp=%d", temp_set);

    g_bucket_countdown.delayed_start_timer = lv_timer_create(bucket_auto_start_delay_cb,
            BUCKET_AUTO_TIMER_DELAY_MS,
            NULL);
    if (g_bucket_countdown.delayed_start_timer != NULL)
    {
        lv_timer_set_repeat_count(g_bucket_countdown.delayed_start_timer, 1);
    }
    else
    {
        SYSTEM_MANAGER_LOG_ERROR("创建自动上水导航完成后延时 A6 timer 失败");
    }
}

static bool auto_water_navigation_try_start_bucket(void)
{
    int16_t target_temperature;
    uint32_t target_time_sec;

    if (!g_auto_water_navigation.active ||
            !g_auto_water_navigation.completion_handled ||
            !g_auto_water_navigation.navigation_arrived ||
            last_link_status != LINK_STATUS_DETACHED)
    {
        return false;
    }

    target_temperature = g_auto_water_navigation.target_temperature;
    target_time_sec = g_auto_water_navigation.target_time_sec;
    temp_set = target_temperature;
    timer_set = target_time_sec;
    if (!navigate_to_screen(UI_SCREEN_WORKING))
    {
        SYSTEM_MANAGER_LOG_ERROR("自动上水导航完成但工作页切换失败，暂不启动泡脚");
        return false;
    }
    system_auto_water_navigation_cancel();
    system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                            "auto-water-navigation-arrived");
    current_location = going_to_location;
    to_preparing_flat = 0;
    bucket_start_after_auto_water_navigation();
    SYSTEM_MANAGER_LOG_USER(
        "自动上水导航到达且已离站，切换工作页并启动泡脚: temp=%d time=%us",
        temp_set, (unsigned int)timer_set);
    return true;
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
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
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
    SYSTEM_MANAGER_LOG_USER("桶体定时按钮更新候选值，等待确认: %us",
                            (unsigned int)timer_set);
}

void bucket_temp_update_actual_from_mcu(int16_t temp)
{
    if (!bucket_temp_valid(temp))
    {
        return;
    }

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
    snprintf(mode->station_name, sizeof(mode->station_name), "%s", g_mode_edit_station_name);
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
        mode.station_name[0] = '\0';

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
    system_mode_store_t store = {0};

    store.magic = MODE_SETTINGS_MAGIC;
    store.version = MODE_SETTINGS_VERSION;
    store.count = (uint32_t)g_mode_count;
    memcpy(store.modes, g_mode_settings, g_mode_count * sizeof(system_mode_t));
    if (ensure_mode_settings_dir() != 0)
    {
        return -1;
    }
    fp = fopen(MODE_SETTINGS_TMP_PATH, "wb");
    if (fp == NULL)
    {
        return -1;
    }
    if (fwrite(&store, sizeof(store), 1, fp) != 1 || fflush(fp) != 0 ||
            fsync(fileno(fp)) != 0)
    {
        fclose(fp);
        return -1;
    }
    if (fclose(fp) != 0)
    {
        return -1;
    }
    return rename(MODE_SETTINGS_TMP_PATH, MODE_SETTINGS_PATH);
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
    g_mode_settings[0].station_name[0] = '\0';

    snprintf(g_mode_settings[1].name, sizeof(g_mode_settings[1].name), "放松");
    g_mode_settings[1].icon_id = 3;
    g_mode_settings[1].time_sec = 1200;
    g_mode_settings[1].water_level = 1;
    g_mode_settings[1].temperature = 39;
    g_mode_settings[1].use_drug1 = false;
    g_mode_settings[1].use_drug2 = true;
    g_mode_settings[1].station_name[0] = '\0';

    snprintf(g_mode_settings[2].name, sizeof(g_mode_settings[2].name), "驱寒");
    g_mode_settings[2].icon_id = 11;
    g_mode_settings[2].time_sec = 1500;
    g_mode_settings[2].water_level = 2;
    g_mode_settings[2].temperature = 41;
    g_mode_settings[2].use_drug1 = true;
    g_mode_settings[2].use_drug2 = true;
    g_mode_settings[2].station_name[0] = '\0';

    g_mode_count = 3;
}

static int load_mode_settings(void)
{
    FILE *fp = fopen(MODE_SETTINGS_PATH, "rb");
    uint32_t header[3];
    system_mode_v2_t old_modes[SYSTEM_MODE_MAX_COUNT];
    bool migrated = false;
    bool valid = false;

    if (fp == NULL)
    {
        init_default_modes();
        return persist_mode_settings();
    }
    if (fread(header, sizeof(header), 1, fp) == 1 &&
            header[0] == MODE_SETTINGS_MAGIC && header[2] > 0 &&
            header[2] <= SYSTEM_MODE_MAX_COUNT)
    {
        if (header[1] == MODE_SETTINGS_VERSION)
        {
            valid = fread(g_mode_settings, sizeof(g_mode_settings), 1, fp) == 1;
        }
        else if (header[1] == 2u && fread(old_modes, sizeof(old_modes), 1, fp) == 1)
        {
            memset(g_mode_settings, 0, sizeof(g_mode_settings));
            for (size_t index = 0; index < header[2]; ++index)
            {
                system_mode_t *mode = &g_mode_settings[index];
                const system_mode_v2_t *old = &old_modes[index];
                memcpy(mode->name, old->name, sizeof(mode->name));
                mode->icon_id = old->icon_id;
                mode->time_sec = old->time_sec;
                mode->water_level = old->water_level;
                mode->temperature = old->temperature;
                mode->use_drug1 = old->use_drug1;
                mode->use_drug2 = old->use_drug2;
            }
            migrated = true;
            valid = true;
        }
    }
    fclose(fp);
    if (!valid)
    {
        init_default_modes();
        return persist_mode_settings();
    }
    g_mode_count = header[2];
    for (size_t index = 0; index < g_mode_count; ++index)
    {
        system_mode_t *mode = &g_mode_settings[index];
        mode->name[sizeof(mode->name) - 1u] = '\0';
        if (memchr(mode->station_name, '\0', sizeof(mode->station_name)) == NULL ||
                strpbrk(mode->station_name, "\r\n") != NULL)
        {
            mode->station_name[0] = '\0';
            SYSTEM_MANAGER_LOG_WARN("模式站点名称无效，清空绑定: %u", (unsigned int)index);
        }
    }
    if (migrated)
    {
        SYSTEM_MANAGER_LOG_USER("模式配置已从版本2迁移，原序号绑定清空");
        if (persist_mode_settings() != 0)
        {
            SYSTEM_MANAGER_LOG_WARN("模式迁移落盘失败，保留内存参数");
        }
    }
    return 0;
}


static bool system_location_is_selectable_station(const StationInfo_t *station)
{
    if (station == NULL)
    {
        return false;
    }

    if (station->is_enable == 0)
    {
        return false;
    }

    if (station->type == 2 || station->id == 0)
    {
        return false;
    }

    return station->name[0] != '\0';
}

static int system_location_find_by_station_id(const system_location_t *locations,
        size_t count,
        int station_id)
{
    size_t index;

    if (locations == NULL)
    {
        return -1;
    }

    for (index = 0; index < count; ++index)
    {
        if (locations[index].index == station_id)
        {
            return (int)index;
        }
    }

    return -1;
}

static int system_location_find_by_name(const system_location_t *locations,
                                        size_t count,
                                        const char *name)
{
    size_t index;

    if (locations == NULL || name == NULL || name[0] == '\0')
    {
        return -1;
    }

    for (index = 0; index < count; ++index)
    {
        if (strcmp(locations[index].name, name) == 0)
        {
            return (int)index;
        }
    }

    return -1;
}

static void system_location_update_entry_from_station(system_location_t *location,
        const StationInfo_t *station)
{
    if (location == NULL || station == NULL)
    {
        return;
    }

    snprintf(location->name, sizeof(location->name), "%s", station->name);
    location->index = station->id;
    location->coord.x = (lv_coord_t)lroundf(station->x);
    location->coord.y = (lv_coord_t)lroundf(station->y);
}

static int system_location_resolve_station(const char *target_name,
        int target_location_index,
        StationInfo_t *station)
{
    MapStations_t list = {0};
    int matches = 0;
    bool has_name = target_name != NULL && target_name[0] != '\0';

    if (station == NULL || RobotTcp_GetStationList(&list) != 0)
    {
        return -1;
    }
    memset(station, 0, sizeof(*station));
    for (int index = 0; index < list.station_count; ++index)
    {
        const StationInfo_t *candidate = &list.stations[index];
        if (!system_location_is_selectable_station(candidate))
        {
            continue;
        }
        if ((has_name && strcmp(candidate->name, target_name) == 0) ||
                (!has_name && target_location_index >= 0 && candidate->id == target_location_index))
        {
            *station = *candidate;
            ++matches;
        }
    }
    RobotTcp_FreeStationList(&list);
    if (matches != 1)
    {
        memset(station, 0, sizeof(*station));
        return -1;
    }
    return 0;
}

static void restore_location_preferences(void)
{
    FILE *fp = fopen(LOCATION_SETTINGS_PATH, "rb");
    uint32_t header[3];
    system_location_t saved[SYSTEM_LOCATION_MAX_COUNT] = {0};
    system_location_v1_t old[SYSTEM_LOCATION_MAX_COUNT];
    bool valid = false;

    if (fp == NULL)
    {
        return;
    }
    if (fread(header, sizeof(header), 1, fp) == 1 &&
            header[0] == LOCATION_SETTINGS_MAGIC && header[2] <= SYSTEM_LOCATION_MAX_COUNT)
    {
        if (header[1] == LOCATION_SETTINGS_VERSION)
        {
            valid = fread(saved, sizeof(saved), 1, fp) == 1;
        }
        else if (header[1] == 1u && fread(old, sizeof(old), 1, fp) == 1)
        {
            for (size_t i = 0; i < header[2]; ++i)
            {
                old[i].name[sizeof(old[i].name) - 1u] = '\0';
                snprintf(saved[i].name, sizeof(saved[i].name), "%s", old[i].name);
                saved[i].default_mode_index = old[i].default_mode_index;
            }
            valid = true;
        }
    }
    fclose(fp);
    if (!valid)
    {
        return;
    }
    for (size_t i = 0; i < g_location_count; ++i)
    {
        for (size_t j = 0; j < header[2]; ++j)
        {
            if (memchr(saved[j].name, '\0', sizeof(saved[j].name)) != NULL &&
                    strcmp(g_location_settings[i].name, saved[j].name) == 0 &&
                    saved[j].default_mode_index >= 0 &&
                    (size_t)saved[j].default_mode_index < g_mode_count)
            {
                g_location_settings[i].default_mode_index = saved[j].default_mode_index;
                break;
            }
        }
    }
}

static void mode_station_changed(lv_event_t *e)
{
    uint32_t selected = lv_dropdown_get_selected(lv_event_get_target(e));
    if (selected == 0u)
    {
        g_mode_edit_station_name[0] = '\0';
    }
    else if (selected <= g_location_count)
    {
        snprintf(g_mode_edit_station_name, sizeof(g_mode_edit_station_name), "%s",
                 g_location_settings[selected - 1u].name);
    }
}

static void mode_station_dropdown_refresh(lv_ui *ui)
{
    lv_obj_t *dropdown = ui->conf_mode_detail_combo_location;
    char options[(SYSTEM_LOCATION_MAX_COUNT + 2) * (SYSTEM_LOCATION_NAME_LEN + 1)];
    size_t used;
    uint32_t selected = 0;
    int index;

    if (dropdown == NULL || !lv_obj_is_valid(dropdown))
    {
        return;
    }
    lv_dropdown_close(dropdown);
    if (g_location_count == 0u)
    {
        lv_dropdown_set_options(dropdown, "暂无站点");
        lv_obj_add_state(dropdown, LV_STATE_DISABLED);
        mode_station_dialog_refresh(ui);
        return;
    }
    lv_obj_remove_state(dropdown, LV_STATE_DISABLED);
    used = (size_t)snprintf(options, sizeof(options), "未绑定");
    for (size_t i = 0; i < g_location_count; ++i)
    {
        used += (size_t)snprintf(options + used, sizeof(options) - used, "\n%s",
                                 g_location_settings[i].name);
    }
    index = system_location_find_by_name(g_location_settings, g_location_count,
                                         g_mode_edit_station_name);
    if (index >= 0)
    {
        selected = (uint32_t)index + 1u;
    }
    else if (g_mode_edit_station_name[0] != '\0')
    {
        /* 失效名称保留在编辑缓存，不让刷新悄悄替用户清空绑定。 */
        snprintf(options + used, sizeof(options) - used, "\n原绑定站点已失效");
        selected = (uint32_t)g_location_count + 1u;
    }
    lv_dropdown_set_options(dropdown, options);
    lv_dropdown_set_selected(dropdown, selected);
    apply_dropdown_list_font(dropdown);
    mode_station_dialog_refresh(ui);
}

static void system_location_refresh_pages(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }

    if (ui->location_cont_2 != NULL && lv_obj_is_valid(ui->location_cont_2))
    {
        location_page_init(ui);
    }
    if (ui->conf_location_cont_2 != NULL && lv_obj_is_valid(ui->conf_location_cont_2))
    {
        conf_location_init(ui);
    }
}

void system_location_sync_from_robot_cache(void)
{
    if (lv_async_call(system_location_sync_from_robot_cache_async, NULL) != LV_RESULT_OK)
    {
        SYSTEM_MANAGER_LOG_ERROR("站点同步调度失败");
    }
}

static void system_location_sync_from_robot_cache_async(void *user_data)
{
    MapStations_t station_list = {0};
    system_location_t old_locations[SYSTEM_LOCATION_MAX_COUNT];
    char selected_name[SYSTEM_LOCATION_NAME_LEN] = {0};
    size_t old_count = g_location_count;
    size_t count = 0;

    LV_UNUSED(user_data);
    if (RobotTcp_GetStationList(&station_list) != 0)
    {
        SYSTEM_MANAGER_LOG_ERROR("读取站点缓存失败");
        return;
    }
    memcpy(old_locations, g_location_settings, sizeof(old_locations));
    if (current_selected_location_index >= 0 &&
            (size_t)current_selected_location_index < old_count)
    {
        snprintf(selected_name, sizeof(selected_name), "%s",
                 old_locations[current_selected_location_index].name);
    }
    memset(g_location_settings, 0, sizeof(g_location_settings));
    for (int index = 0; index < station_list.station_count; ++index)
    {
        const StationInfo_t *station = &station_list.stations[index];
        bool ambiguous = false;
        if (!system_location_is_selectable_station(station))
        {
            continue;
        }
        for (int other = 0; other < station_list.station_count; ++other)
        {
            if (other != index && system_location_is_selectable_station(&station_list.stations[other]) &&
                    strcmp(station->name, station_list.stations[other].name) == 0)
            {
                ambiguous = true;
                break;
            }
        }
        if (ambiguous)
        {
            SYSTEM_MANAGER_LOG_WARN("同名站点无法唯一确定，跳过: %s", station->name);
            continue;
        }
        if (count == SYSTEM_LOCATION_MAX_COUNT)
        {
            SYSTEM_MANAGER_LOG_WARN("站点数量超过页面容量: %u", SYSTEM_LOCATION_MAX_COUNT);
            break;
        }
        system_location_update_entry_from_station(&g_location_settings[count], station);
        int old_index = system_location_find_by_name(old_locations, old_count, station->name);
        g_location_settings[count].default_mode_index =
            old_index >= 0 ? old_locations[old_index].default_mode_index : 0;
        ++count;
    }
    RobotTcp_FreeStationList(&station_list);
    g_location_count = count;
    if (old_count == 0u)
    {
        restore_location_preferences();
    }
    current_selected_location_index =
        system_location_find_by_name(g_location_settings, count, selected_name);
    /* 列表变化不修改模式绑定，也不重载正在编辑的表单。 */
    system_location_refresh_pages(&guider_ui);
    mode_station_dropdown_refresh(&guider_ui);
    SYSTEM_MANAGER_LOG_USER("站点列表已同步: 数量=%u", (unsigned int)count);
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

int system_start_selected_location_navigation(lv_obj_t *dialog_parent)
{
    const system_location_t *location;

    if (current_selected_location_index < 0 ||
            current_selected_location_index >= (int)g_location_count)
    {
        show_message_dialog_and_navigate(dialog_parent,
                                         "导航失败: 未找到目标站点",
                                         UI_SCREEN_INDEX);
        return -1;
    }
    location = &g_location_settings[current_selected_location_index];
    return system_start_navigation(dialog_parent, location->name,
                                   location->index, NULL);
}

int system_start_return_base(void)
{
    if (g_return_base_pending && to_preparing_flat == 4)
    {
        return 0;
    }
    if (system_base_flow_request_start(SYSTEM_BASE_FLOW_MOVING,
                                       SYSTEM_ACTION_SOURCE_UI).result != SYSTEM_COMMAND_ACCEPTED)
    {
        return -1;
    }
    system_auto_water_navigation_cancel();
    to_preparing_flat = 4;
    if (lv_screen_active() == guider_ui.preparing)
    {
        preparing_page_init(&guider_ui);
    }
    else if (!navigate_to_screen(UI_SCREEN_PREPARING))
    {
        system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING, "return-base-page-failed");
        to_preparing_flat = 0;
        return -1;
    }
    return 0;
}

// 自动上水开始时复制目标，列表更新只影响下一轮。
void system_auto_water_navigation_arm(void)
{
    StationInfo_t target = {0};
    const char *name = "";
    bool valid = false;

    if (current_selected_mode_index >= 0 &&
            current_selected_mode_index < (int)g_mode_count)
    {
        name = g_mode_settings[current_selected_mode_index].station_name;
        valid = system_location_resolve_station(name, -1, &target) == 0;
    }
    auto_water_navigation_flow_arm(&g_auto_water_navigation, valid ? &target : NULL,
                                   serial_mcu_status_report_sequence());
    SYSTEM_MANAGER_LOG_USER("自动上水目标快照: 模式=%d 名称=%s 有效=%d",
                            current_selected_mode_index, name, valid);
}

void system_auto_water_navigation_cancel(void)
{
    auto_water_navigation_flow_cancel(&g_auto_water_navigation);
}

// 整合的停止泡脚接口
void system_bucket_stop(void)
{
    bucket_runtime_cleanup();
    SYSTEM_MANAGER_LOG_USER("泡脚流程已停止，下发 A1");
    serial_bucket_standby();
}

// 整合的停止自动上水接口
void system_auto_water_stop(void)
{
    if (g_base_flow != SYSTEM_BASE_FLOW_WATERING)
    {
        SYSTEM_MANAGER_LOG_WARN("忽略非自动上水停止请求: active=%s",
                                system_base_flow_name(g_base_flow));
        return;
    }

    serial_base_standby();
    auto_water_page_cleanup();
    system_base_flow_finish(SYSTEM_BASE_FLOW_WATERING,
                            "water-stop");
    preparing_common_cleanup();
    to_preparing_flat = 0;
    (void)navigate_to_screen(UI_SCREEN_INDEX);
}

void system_self_clean_stop(void)
{
    if (g_base_flow != SYSTEM_BASE_FLOW_SELF_CLEANING)
    {
        SYSTEM_MANAGER_LOG_WARN("忽略非自清洁停止请求: active=%s",
                                system_base_flow_name(g_base_flow));
        return;
    }

    serial_base_standby();
    self_clean_page_cleanup();
    preparing_common_cleanup();
    is_self_cleaning = false;
    to_preparing_flat = 0;
    system_base_flow_finish(SYSTEM_BASE_FLOW_SELF_CLEANING,
                            "self-clean-stop");
    (void)navigate_to_screen(UI_SCREEN_INDEX);
}

// 整合的停止所有活动流程接口
void system_active_flow_stop(void)
{
    if (g_base_flow == SYSTEM_BASE_FLOW_MOVING)
    {
        system_auto_water_navigation_cancel();
        if (last_link_status == LINK_STATUS_DETACHED)
        {
            serial_bucket_standby();
        }
        else if (last_link_status == LINK_STATUS_DOCKED)
        {
            serial_base_standby();
        }
        else
        {
            SYSTEM_MANAGER_LOG_WARN("移动期间未知LINK_STATUS: 0x%02X",
                                    (unsigned int)last_link_status);
        }
        return;
    }

    if (last_link_status == LINK_STATUS_DETACHED)
    {
        system_bucket_stop();
        return;
    }
    if (last_link_status != LINK_STATUS_DOCKED)
    {
        SYSTEM_MANAGER_LOG_WARN("未知LINK_STATUS，不下发待机命令: 0x%02X",
                                (unsigned int)last_link_status);
        return;
    }

    switch (g_base_flow)
    {
    case SYSTEM_BASE_FLOW_WATERING:
        system_auto_water_stop();
        break;
    case SYSTEM_BASE_FLOW_SELF_CLEANING:
        system_self_clean_stop();
        break;
    case SYSTEM_BASE_FLOW_IDLE:
        serial_base_standby();
        (void)navigate_to_screen(UI_SCREEN_INDEX);
        break;
    case SYSTEM_BASE_FLOW_MOVING:
    default:
        break;
    }
}

static void system_base_runtime_cleanup_without_command(const char *source)
{
    system_base_flow_t active_flow = g_base_flow;
    bool has_auto_water_runtime;
    bool has_self_clean_runtime;

    if (active_flow == SYSTEM_BASE_FLOW_MOVING)
    {
        system_auto_water_navigation_cancel();
        return;
    }

    has_auto_water_runtime = active_flow == SYSTEM_BASE_FLOW_WATERING ||
                             to_preparing_flat == 1;
    has_self_clean_runtime = active_flow == SYSTEM_BASE_FLOW_SELF_CLEANING ||
                             to_preparing_flat == 2 ||
                             is_self_cleaning ||
                             g_self_clean_flow.active;

    if (has_auto_water_runtime)
    {
        auto_water_page_cleanup();
    }
    if (has_self_clean_runtime)
    {
        self_clean_page_cleanup();
    }
    if (has_auto_water_runtime || has_self_clean_runtime)
    {
        preparing_common_cleanup();
    }
    is_self_cleaning = false;
    to_preparing_flat = 0;
    if (active_flow != SYSTEM_BASE_FLOW_IDLE)
    {
        system_base_flow_finish(active_flow, source);
    }
}

static void system_global_stop(void)
{
    if (system_mcu_link_state_get() == SYSTEM_MCU_LINK_UNKNOWN)
    {
        bucket_runtime_cleanup();
        system_base_runtime_cleanup_without_command("global-stop-no-position");
        SYSTEM_MANAGER_LOG_WARN("全局停止已清理本地状态，无有效MCU位置，不下发待机命令");
        return;
    }

    if (last_link_status == LINK_STATUS_DETACHED &&
            g_base_flow != SYSTEM_BASE_FLOW_MOVING)
    {
        system_bucket_stop();
        return;
    }

    bucket_runtime_cleanup();
    system_active_flow_stop();
}

static void system_voice_massage_start(void)
{
    if (massage_intensity < 3)
    {
        massage_intensity++;
    }
    bucket_working_sync_function_icons(&guider_ui);
    serial_bucket_massage();
}

static void system_voice_temp_step(int delta)
{
    temp_set = bucket_temp_clamp((int)temp_set + delta);
    if (guider_ui.working_label_temperature != NULL &&
            lv_obj_is_valid(guider_ui.working_label_temperature))
    {
        bucket_temp_start_edit(guider_ui.working_label_temperature, temp_set);
    }
    if (guider_ui.index_temp_value != NULL &&
            lv_obj_is_valid(guider_ui.index_temp_value))
    {
        update_temp_set(&guider_ui);
    }
    serial_bucket_heat();
}

static system_command_decision_t system_voice_decision(
    system_command_result_t result, const char *domain,
    const char *state, const char *reason)
{
    system_command_decision_t decision = {result, domain, state, reason};

    return decision;
}

static bool system_voice_is_detached_command(voice_command_t command)
{
    return command == VOICE_COMMAND_FOOTBATH_CONTINUE ||
           command == VOICE_COMMAND_FOOTBATH_STOP ||
           command == VOICE_COMMAND_MASSAGE_START ||
           command == VOICE_COMMAND_MASSAGE_STOP ||
           command == VOICE_COMMAND_TEMP_UP ||
           command == VOICE_COMMAND_TEMP_DOWN;
}

static bool system_voice_is_docked_command(voice_command_t command)
{
    return command == VOICE_COMMAND_FOOTBATH_START ||
           command == VOICE_COMMAND_CLEAN_START ||
           command == VOICE_COMMAND_CLEAN_STOP;
}

system_command_decision_t system_voice_command_execute(voice_command_t command)
{
    system_bucket_state_t bucket_state;

    if (command == VOICE_COMMAND_STOP_ALL)
    {
        system_global_stop();
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "GLOBAL", "ANY", "accepted");
    }
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        return system_voice_decision(SYSTEM_COMMAND_STATE_REJECTED,
                                     "POSITION", "UNKNOWN",
                                     "no-valid-mcu-status");
    }
    if (system_voice_is_detached_command(command) &&
            last_link_status != LINK_STATUS_DETACHED)
    {
        return system_voice_decision(SYSTEM_COMMAND_STATE_REJECTED,
                                     "POSITION", "DOCKED",
                                     "detached-required");
    }
    if (system_voice_is_docked_command(command) &&
            last_link_status != LINK_STATUS_DOCKED)
    {
        return system_voice_decision(SYSTEM_COMMAND_STATE_REJECTED,
                                     "POSITION", "DETACHED",
                                     "docked-required");
    }
    if (system_voice_is_detached_command(command))
    {
        bucket_state = system_bucket_state_get();
        if (bucket_state != SYSTEM_BUCKET_STATE_NORMAL)
        {
            return system_voice_decision(SYSTEM_COMMAND_STATE_REJECTED,
                                         "BUCKET",
                                         system_bucket_state_name(bucket_state),
                                         "bucket-state-conflict");
        }
        if (g_base_flow != SYSTEM_BASE_FLOW_IDLE)
        {
            return system_voice_decision(SYSTEM_COMMAND_INTERLOCK_REJECTED,
                                         "BASE",
                                         system_base_flow_name(g_base_flow),
                                         "base-flow-conflict");
        }
    }

    switch (command)
    {
    case VOICE_COMMAND_FOOTBATH_CONTINUE:
        constant_temperature = true;
        bucket_working_sync_function_icons(&guider_ui);
        serial_bucket_heat();
        system_voice_massage_start();
        timer_set = 20u * 60u;
        system_bucket_timer_apply();
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "BUCKET", "NORMAL", "accepted");
    case VOICE_COMMAND_FOOTBATH_STOP:
        system_bucket_stop();
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "BUCKET", "NORMAL", "accepted");
    case VOICE_COMMAND_MASSAGE_START:
        system_voice_massage_start();
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "BUCKET", "NORMAL", "accepted");
    case VOICE_COMMAND_MASSAGE_STOP:
        massage_intensity = 0;
        bucket_working_sync_function_icons(&guider_ui);
        serial_bucket_massage();
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "BUCKET", "NORMAL", "accepted");
    case VOICE_COMMAND_TEMP_UP:
        system_voice_temp_step(2);
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "BUCKET", "NORMAL", "accepted");
    case VOICE_COMMAND_TEMP_DOWN:
        system_voice_temp_step(-2);
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "BUCKET", "NORMAL", "accepted");
    case VOICE_COMMAND_FOOTBATH_START:
    {
        system_command_decision_t start_decision =
            system_base_flow_request_start(SYSTEM_BASE_FLOW_WATERING,
                                           SYSTEM_ACTION_SOURCE_VOICE);

        if (start_decision.result != SYSTEM_COMMAND_ACCEPTED)
        {
            return start_decision;
        }
        to_preparing_flat = 1;
        system_auto_water_navigation_arm();
        if (!navigate_to_screen(UI_SCREEN_PREPARING))
        {
            to_preparing_flat = 0;
            system_auto_water_navigation_cancel();
            system_base_flow_finish(SYSTEM_BASE_FLOW_WATERING,
                                    "voice-water-page-failed");
            return system_voice_decision(SYSTEM_COMMAND_INTERLOCK_REJECTED,
                                         "BASE", "IDLE",
                                         "page-navigation-failed");
        }
    }
    return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                 "BASE", "WATERING", "accepted");
    case VOICE_COMMAND_CLEAN_START:
    {
        system_command_decision_t start_decision =
            system_base_flow_request_start(
                SYSTEM_BASE_FLOW_SELF_CLEANING,
                SYSTEM_ACTION_SOURCE_VOICE);

        if (start_decision.result != SYSTEM_COMMAND_ACCEPTED)
        {
            return start_decision;
        }
    }
    is_self_cleaning = true;
    to_preparing_flat = 2;
    system_auto_water_navigation_cancel();
    if (!navigate_to_screen(UI_SCREEN_PREPARING))
    {
        is_self_cleaning = false;
        to_preparing_flat = 0;
        system_base_flow_finish(SYSTEM_BASE_FLOW_SELF_CLEANING,
                                "voice-clean-page-failed");
        return system_voice_decision(SYSTEM_COMMAND_INTERLOCK_REJECTED,
                                     "BASE", "IDLE",
                                     "page-navigation-failed");
    }
    return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                 "BASE", "SELF_CLEANING", "accepted");
    case VOICE_COMMAND_CLEAN_STOP:
        if (g_base_flow != SYSTEM_BASE_FLOW_SELF_CLEANING)
        {
            return system_voice_decision(SYSTEM_COMMAND_INTERLOCK_REJECTED,
                                         "BASE",
                                         system_base_flow_name(g_base_flow),
                                         "self-clean-not-active");
        }
        system_self_clean_stop();
        return system_voice_decision(SYSTEM_COMMAND_ACCEPTED,
                                     "BASE", "IDLE", "accepted");
    default:
        return system_voice_decision(SYSTEM_COMMAND_INTERLOCK_REJECTED,
                                     "COMMAND", "UNSUPPORTED",
                                     "unsupported-command");
    }
}

// 点击模式按钮后，应用该模式的设置
void apply_mode_to_runtime(lv_event_t *e)
{
    int mode_index;

    if (e == NULL)
    {
        return;
    }
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        return;
    }

    mode_index = (int)(uintptr_t)lv_event_get_user_data(e);
    if (system_mode_apply_index(mode_index) != 0)
    {
        return;
    }
    system_manager_refresh_base_ui();
}

// 缺液字体拼接
static void liquid_shortage_build_text(uint8_t shortage_mask,
                                       char *text,
                                       size_t text_size)
{
    static const uint8_t shortage_bits[] =
    {
        SERIAL_LIQUID_SHORTAGE_DRUG1,
        SERIAL_LIQUID_SHORTAGE_DRUG2,
        SERIAL_LIQUID_SHORTAGE_CLEAN,
    };
    size_t used;
    size_t index;
    bool has_number = false;

    if (text == NULL || text_size == 0u)
    {
        return;
    }

    used = (size_t)snprintf(text, text_size, "药液");
    for (index = 0u;
            index < sizeof(shortage_bits) / sizeof(shortage_bits[0]);
            ++index)
    {
        int written;

        if ((shortage_mask & shortage_bits[index]) == 0u)
        {
            continue;
        }
        written = snprintf(text + used,
                           text_size - used,
                           "%s%u",
                           has_number ? "、" : "",
                           (unsigned int)(index + 1u));
        if (written < 0 || (size_t)written >= text_size - used)
        {
            text[text_size - 1u] = '\0';
            return;
        }
        used += (size_t)written;
        has_number = true;
    }
    (void)snprintf(text + used, text_size - used, "缺液");
}

// 缺液提示UI更新
static void system_ui_apply_liquid_shortage(uint8_t shortage_mask)
{
    lv_obj_t *icon = guider_ui.index_img_need_liquid;
    lv_obj_t *label = guider_ui.index_label_need_liquid;
    char text[32];

    if (icon == NULL || label == NULL ||
            !lv_obj_is_valid(icon) || !lv_obj_is_valid(label))
    {
        return;
    }

    shortage_mask &= SERIAL_LIQUID_SHORTAGE_MASK;
    if (shortage_mask == 0u)
    {
        lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    liquid_shortage_build_text(shortage_mask, text, sizeof(text));
    lv_obj_set_style_text_font(label,
                               custom_get_compact_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label, text);
    lv_obj_remove_flag(icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
}

static void system_ui_apply_liquid_shortage_async(void *user_data)
{
    uint8_t shortage_mask = (uint8_t)(uintptr_t)user_data;

    system_ui_apply_liquid_shortage(shortage_mask);
}

void system_ui_schedule_liquid_shortage(uint8_t shortage_mask)
{
    lv_async_call(system_ui_apply_liquid_shortage_async,
                  (void *)(uintptr_t)(shortage_mask));
}

// 刷新首页UI显示
void index_page_init(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }

    index_sync_selection_ui(ui);
    //刷新温度显示
    apply_water_level_ui(water_level);
    update_temp_set(ui);
    apply_use_drug_ui(use_drug1, use_drug2);
    system_ui_apply_liquid_shortage(serial_mcu_liquid_shortage_mask());

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


    for (index = 0; index < g_location_count; ++index)
    {
        const system_location_t *location = &g_location_settings[index];

        x_offset = (int)(index % 2) * (w + w_span);
        y_offset = (int)(index / 2) * (h + h_span);
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
        lv_obj_add_event_cb(ui->conf_mode_detail_combo_location, mode_station_changed,
                            LV_EVENT_VALUE_CHANGED, NULL);
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
    snprintf(g_mode_edit_station_name, sizeof(g_mode_edit_station_name), "%s", mode->station_name);
    mode_station_dropdown_refresh(ui);
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
        snprintf(label_text, sizeof(label_text), "定位%u", (unsigned int)(index + 1u));
        lv_label_set_text(label, label_text);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        conf_location_apply_label_style(label);

        textarea = lv_textarea_create(ui->conf_location_cont_2);
        lv_obj_set_pos(textarea, CONF_LOCATION_NAME_X, row_y);
        lv_obj_set_size(textarea, CONF_LOCATION_NAME_WIDTH, CONF_LOCATION_FIELD_HEIGHT);
        lv_textarea_set_text(textarea, location->name);
        conf_location_apply_textarea_style(ui, textarea);
        /* 地图站点名称由 APP 保存，本地只读。 */
        lv_obj_add_state(textarea, LV_STATE_DISABLED);

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
    }
}

/* 已到达后继续等待连接；不得用等待时长推断连接成功。 */
static void preparing_wait_link_timer_cb(lv_timer_t *timer)
{
    if (last_link_status != LINK_STATUS_DOCKED)
    {
        return;
    }
    if (g_preparing_page_timer == timer)
    {
        g_preparing_page_timer = NULL;
    }
    lv_timer_delete(timer);
    navigate_to_screen(UI_SCREEN_INDEX);
}

void preparing_move_finished(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);
    if (ui == NULL)
    {
        return;
    }

    //显示cont_move_finished
    lv_obj_set_style_text_font(ui->preparing_label_17,
                               &lv_font_SourceHanSansSC_Regular_40,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->preparing_label_timer_tips,
                               &lv_font_SourceHanSansSC_Regular_30,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(ui->preparing_label_17, "已到达指定位置");
    lv_label_set_text(ui->preparing_label_timer_tips, "(5s后享受健康足疗)");
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
            //1小时内，没有点首页任何按钮的话，自动排水 可能app点击
            // g_auto_drain_timer = lv_timer_create_basic();
            // lv_timer_set_period(g_auto_drain_timer, 60 * 60 * 1000); // 1小时
            // lv_timer_set_repeat_count(g_auto_drain_timer, 1);
            // lv_timer_set_cb(g_auto_drain_timer, auto_drain_timer_cb);
            return;
        }
        else
        {
            /* 等待资源由准备页统一管理，退出时取消。 */
            lv_obj_add_flag(ui->preparing_cont_move_finished, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui->preparing_cont_moving, LV_OBJ_FLAG_HIDDEN);
            g_preparing_page_timer = lv_timer_create(preparing_wait_link_timer_cb, 1000, ui);
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
        if (auto_water_navigation_flow_observe(
                    &g_auto_water_navigation,
                    base_main_status,
                    serial_mcu_status_report_sequence()))
        {
            StationInfo_t target = g_auto_water_navigation.target_station;

            if (!g_auto_water_navigation.has_target)
            {
                SYSTEM_MANAGER_LOG_WARN("上水完成但无有效绑定站点，跳过导航和自动泡脚");
                g_auto_water_navigation.active = false;
                return;
            }

            if (g_preparing_page_timer == timer)
            {
                g_preparing_page_timer = NULL;
            }
            lv_timer_delete(timer);
            // 自动注水完成
            system_base_flow_finish(SYSTEM_BASE_FLOW_WATERING,
                                    "water-complete");
            // 开始移动
            (void)system_start_navigation(ui->preparing, target.name, target.id, &target);
            return;
        }
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

// 根据不同to_preparing_flat初始化不同的准备页面
void preparing_page_init(lv_ui *ui)
{
    /* 自动上水导航已在进入页面前武装，此处不能调用 auto_water_page_cleanup()。 */
    preparing_common_cleanup();
    if (to_preparing_flat == 2)
    {
        self_clean_page_cleanup();
    }
    preparing_stop_button_set_hidden(false);
    /* 恢复准备页默认卡片布局；自清洁分支会临时套用参考界面尺寸。 */
    lv_obj_set_pos(ui->preparing_cont_1, 14, 59);
    lv_obj_set_size(ui->preparing_cont_1, 574, 403);
    lv_obj_set_style_radius(ui->preparing_cont_1, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
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
        }
    }


    //根据to_preparing_flat的参数，显示当前模式的准备状态
    //1--注水中，2--自清洁中，3--移动中,4--返回基站中
    if (to_preparing_flat == 1)
    {
        if (g_base_flow != SYSTEM_BASE_FLOW_WATERING)
        {
            SYSTEM_MANAGER_LOG_WARN("准备页拒绝 B2: active=%s",
                                    system_base_flow_name(g_base_flow));
            return;
        }
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
        if (g_base_flow != SYSTEM_BASE_FLOW_SELF_CLEANING)
        {
            SYSTEM_MANAGER_LOG_WARN("准备页拒绝 B3: active=%s",
                                    system_base_flow_name(g_base_flow));
            return;
        }
        //显示cont_self_cleaning
        lv_obj_set_pos(ui->preparing_cont_1, 24, 72);
        lv_obj_set_size(ui->preparing_cont_1, 580, 390);
        lv_obj_set_style_radius(ui->preparing_cont_1,
                                34,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(ui->preparing_cont_cleaning, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->preparing_cont_cleaning, 0, 0);
        lv_obj_set_size(ui->preparing_cont_cleaning, 580, 390);
        lv_obj_add_flag(ui->preparing_label_clean_time_left, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->preparing_label_9, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->preparing_label_10, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->preparing_cleaning_tips, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->preparing_img_3, 38, 16);
        lv_obj_set_pos(ui->preparing_label_12, 128, 41);

        /* 将 preparing_cleaning_tips 的字体替换为自定义全量 30px CJK 字库，
         * 避免 GUI Guider 生成的子集字体(仅65个汉字)缺失运行时汉字而显示 □ */
        if (ui->preparing_cleaning_tips != NULL)
        {
            /* 调试：先用已验证可用的 25px 全量字体测试渲染是否正常 */
            lv_obj_set_style_text_font(ui->preparing_cleaning_tips,
                                       &lv_font_SourceHanSansSC_Regular_25,
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        cleaning_animation_create(ui->preparing_cont_cleaning);
        self_clean_flow_start(&g_self_clean_flow);

        g_preparing_page_timer = lv_timer_create_basic();
        lv_timer_set_period(g_preparing_page_timer, 1000);
        lv_timer_set_repeat_count(g_preparing_page_timer, -1);
        lv_timer_set_user_data(g_preparing_page_timer, ui);
        lv_timer_set_cb(g_preparing_page_timer, preparing_refresh_flat2);
        serial_base_auto_clean();

    }
    else if (to_preparing_flat == 3 || to_preparing_flat == 4)
    {
        if (g_base_flow != SYSTEM_BASE_FLOW_MOVING)
        {
            SYSTEM_MANAGER_LOG_WARN("准备页拒绝移动: active=%s",
                                    system_base_flow_name(g_base_flow));
            return;
        }
        //显示cont_moving
        lv_obj_clear_flag(ui->preparing_cont_moving, LV_OBJ_FLAG_HIDDEN);

        //如果是返回基站(to_preparing_flat==4)，尝试使用充电指令触发自动回充
        if (to_preparing_flat == 4)
        {
            SYSTEM_MANAGER_LOG_USER("发送回基站充电指令");
            if (RobotTcp_ReturnToBase() != 0)
            {
                SYSTEM_MANAGER_LOG_ERROR("回仓充电命令发送失败");
                show_message_dialog_and_navigate(ui->preparing,
                                                 "回仓失败: 命令发送失败",
                                                 UI_SCREEN_INDEX);
                system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                                        "return-base-failed");
                return;
            }
            going_to_location = 0;
            g_return_base_report_sequence =
                __atomic_load_n(&g_mcu_docked_report_sequence, __ATOMIC_ACQUIRE);
            g_return_base_pending = true;
        }

        // 每秒请求并读取机器人状态缓存，检查机器人位置
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
    /* 站点仅由代理查询恢复，旧位置文件不得恢复虚构房间。 */
    memset(g_location_settings, 0, sizeof(g_location_settings));
    g_location_count = 0;
    current_selected_location_index = -1;

    LV_UNUSED(ui);

    /* 初始首页已由 GUI Guider 创建，显式记录基站默认页面和方向。 */
    g_applied_link_status = LINK_STATUS_DOCKED;
    g_base_flow = SYSTEM_BASE_FLOW_IDLE;
    if (system_mode_apply_index(0) != 0)
    {
        SYSTEM_MANAGER_LOG_ERROR("开机应用助眠模式失败");
    }
    system_manager_refresh_base_ui();
    bucket_countdown_ensure_timer();

    g_mcu_watchdog_last_sequence = serial_mcu_status_report_sequence();
    g_mcu_watchdog_last_report_tick = lv_tick_get();
    __atomic_store_n(&g_mcu_link_state, SYSTEM_MCU_LINK_UNKNOWN,
                     __ATOMIC_RELEASE);
    __atomic_store_n(&g_mcu_low_battery_handled, false,
                     __ATOMIC_RELEASE);
    if (g_mcu_report_watchdog_timer == NULL)
    {
        g_mcu_report_watchdog_timer = lv_timer_create(
                                          mcu_report_watchdog_timer_cb,
                                          MCU_REPORT_WATCHDOG_PERIOD_MS,
                                          NULL);
    }

}

/**
 * @brief 在目标页面稳定后显示MCU断连保护层。
 *
 * @param delayed true表示等待页面切换完成后再显示，false表示立即挂到活动页。
 */
static void mcu_watchdog_show_outage_guard(bool delayed)
{
    lv_obj_t *active_screen;

    if (delayed)
    {
        mcu_outage_guard_show_delayed(MCU_OUTAGE_GUARD_SHOW_DELAY_MS);
        return;
    }

    active_screen = lv_screen_active();
    if (active_screen != NULL)
    {
        mcu_outage_guard_show(active_screen);
    }
}

/**
 * @brief 处理看门狗观察到的新合法状态帧并刷新链路时间基线。
 *
 * @param report_sequence 最新合法状态帧序号。
 * @param now_tick 当前LVGL时钟值。
 */
static void mcu_watchdog_handle_report(uint32_t report_sequence,
                                       uint32_t now_tick)
{
    system_mcu_link_state_t previous_state;

    g_mcu_watchdog_last_sequence = report_sequence;
    g_mcu_watchdog_last_report_tick = now_tick;
    previous_state = __atomic_exchange_n(&g_mcu_link_state,
                                         SYSTEM_MCU_LINK_ONLINE,
                                         __ATOMIC_ACQ_REL);
    if (previous_state == SYSTEM_MCU_LINK_OUTAGE)
    {
        mcu_outage_guard_destroy();
        SYSTEM_MANAGER_LOG_USER("MCU状态上报已恢复");
    }
}

/**
 * @brief 处理开机后始终未收到首帧的超时场景。
 */
static void mcu_watchdog_handle_initial_timeout(void)
{
    lv_obj_t *active_screen;

    __atomic_store_n(&g_mcu_link_state, SYSTEM_MCU_LINK_OUTAGE,
                     __ATOMIC_RELEASE);
    SYSTEM_MANAGER_LOG_ERROR("MCU开机超时未上报消息");
    serial_base_standby();
    active_screen = lv_screen_active();
    if (active_screen != guider_ui.index)
    {
        (void)navigate_to_screen(UI_SCREEN_INDEX);
        mcu_watchdog_show_outage_guard(true);
    }
    else if (active_screen != NULL)
    {
        mcu_watchdog_show_outage_guard(false);
    }
}

/**
 * @brief 处理已在线设备停止上报后的安全停止、页面归位和触摸保护。
 */
static void mcu_watchdog_handle_online_timeout(void)
{
    bool delays_guard_for_navigation;

    __atomic_store_n(&g_mcu_link_state, SYSTEM_MCU_LINK_OUTAGE,
                     __ATOMIC_RELEASE);
    SYSTEM_MANAGER_LOG_ERROR("MCU未上报消息");
    delays_guard_for_navigation = last_link_status == LINK_STATUS_DOCKED &&
                                  g_base_flow != SYSTEM_BASE_FLOW_MOVING;
    system_active_flow_stop();

    if (last_link_status == LINK_STATUS_DETACHED &&
            g_base_flow != SYSTEM_BASE_FLOW_MOVING &&
            lv_screen_active() != guider_ui.working)
    {
        (void)navigate_to_screen(UI_SCREEN_WORKING);
        delays_guard_for_navigation = true;
    }

    /* 页面归位后延迟获取活动页，避免把保护层挂回动画中的旧页面。 */
    mcu_watchdog_show_outage_guard(delays_guard_for_navigation);
}

/**
 * @brief 根据当前MCU链路状态分发首次超时或运行中断连处理。
 */
static void mcu_watchdog_handle_timeout(void)
{
    system_mcu_link_state_t current_state = system_mcu_link_state_get();

    if (current_state == SYSTEM_MCU_LINK_UNKNOWN)
    {
        mcu_watchdog_handle_initial_timeout();
    }
    else if (current_state == SYSTEM_MCU_LINK_ONLINE)
    {
        mcu_watchdog_handle_online_timeout();
    }
}

/**
 * @brief 周期检查MCU合法状态帧是否更新，并在超过阈值时触发断连处理。
 */
static void mcu_report_watchdog_timer_cb(lv_timer_t *timer)
{
    uint32_t report_sequence = serial_mcu_status_report_sequence();
    uint32_t now_tick = lv_tick_get();

    LV_UNUSED(timer);
    if (report_sequence != g_mcu_watchdog_last_sequence)
    {
        mcu_watchdog_handle_report(report_sequence, now_tick);
        return;
    }

    if (lv_tick_elaps(g_mcu_watchdog_last_report_tick) <
            MCU_REPORT_TIMEOUT_MS)
    {
        return;
    }

    mcu_watchdog_handle_timeout();
}

system_mcu_link_state_t system_mcu_link_state_get(void)
{
    return __atomic_load_n(&g_mcu_link_state, __ATOMIC_ACQUIRE);
}

static void system_ui_apply_link_status(uint8_t link_status)
{
    ui_screen_id_t target_screen;
    lv_obj_t *target_obj;
    int rotation;
    int16_t previous_link_status = g_applied_link_status;
    bool keep_navigation_page;

    keep_navigation_page = g_base_flow == SYSTEM_BASE_FLOW_MOVING &&
                           (g_return_base_pending ||
                            (g_auto_water_navigation.active &&
                             g_auto_water_navigation.completion_handled));

    if (link_status == LINK_STATUS_DOCKED)
    {
        target_screen = keep_navigation_page ? UI_SCREEN_PREPARING : UI_SCREEN_INDEX;
        target_obj = keep_navigation_page ? guider_ui.preparing : guider_ui.index;
        rotation = UI_ROTATION_DOCKED;
    }
    else if (link_status == LINK_STATUS_DETACHED)
    {
        target_screen = keep_navigation_page ?
                        UI_SCREEN_PREPARING : UI_SCREEN_WORKING;
        target_obj = keep_navigation_page ?
                     guider_ui.preparing : guider_ui.working;
        rotation = UI_ROTATION_DETACHED;
    }
    else
    {
        SYSTEM_MANAGER_LOG_WARN("忽略未知 LINK_STATUS: 0x%02X", link_status);
        return;
    }

    if (link_status == LINK_STATUS_DOCKED && preparing_return_base_try_finish())
    {
        keep_navigation_page = false;
        target_screen = UI_SCREEN_INDEX;
        target_obj = guider_ui.index;
    }

    if (g_applied_link_status == (int16_t)link_status)
    {
        return;
    }
    SYSTEM_MANAGER_LOG_USER("LINK_DIAG rotation begin: rotation=%d", rotation);
    if (lv_port_set_rotation(rotation) != 0)
    {
        SYSTEM_MANAGER_LOG_ERROR("应用 LINK_STATUS 方向失败: status=0x%02X rotation=%d",
                                 link_status, rotation);
        return;
    }
    SYSTEM_MANAGER_LOG_USER("LINK_DIAG rotation end");

    g_applied_link_status = (int16_t)link_status;
    if (link_status == LINK_STATUS_DETACHED &&
            previous_link_status == LINK_STATUS_DOCKED)
    {
        if (keep_navigation_page)
        {
            SYSTEM_MANAGER_LOG_USER(
                "导航离站期间保持移动页并等待到达");
            if (auto_water_navigation_try_start_bucket())
            {
                return;
            }
        }
        else
        {
            if (g_base_flow == SYSTEM_BASE_FLOW_WATERING ||
                    g_base_flow == SYSTEM_BASE_FLOW_SELF_CLEANING)
            {
                SYSTEM_MANAGER_LOG_USER(
                    "基站流程期间检测到离站，仅清理本地状态: active=%s",
                    system_base_flow_name(g_base_flow));
                system_base_runtime_cleanup_without_command(
                    "link-status-detached");
            }
            SYSTEM_MANAGER_LOG_USER("普通离站仅切换 working 页面，不自动启动 A2/A6");
        }
    }
    else if (link_status == LINK_STATUS_DOCKED)
    {
        bucket_auto_start_cancel();
    }
    if (lv_screen_active() != target_obj)
    {
        SYSTEM_MANAGER_LOG_USER("LINK_DIAG navigate begin: screen=%d", target_screen);
        navigate_to_screen(target_screen);
        SYSTEM_MANAGER_LOG_USER("LINK_DIAG navigate end");
    }
    SYSTEM_MANAGER_LOG_USER("已应用 LINK_STATUS UI: status=0x%02X rotation=%d",
                            link_status, rotation);
}

static void system_mcu_status_ui_schedule(uint8_t link_status);

static void system_low_battery_protection_apply(uint8_t link_status)
{
    bool was_handled;
    lv_obj_t *active_screen;

    if (link_status != LINK_STATUS_DETACHED ||
            system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE ||
            serial_mcu_battery_percent() >= MCU_LOW_BATTERY_THRESHOLD_PERCENT)
    {
        return;
    }

    was_handled = __atomic_exchange_n(&g_mcu_low_battery_handled, true,
                                      __ATOMIC_ACQ_REL);
    if (was_handled)
    {
        return;
    }

    SYSTEM_MANAGER_LOG_WARN("检测到离站低电，停止泡脚并下发A1");
    system_bucket_stop();
    active_screen = lv_screen_active();
    if (active_screen != NULL)
    {
        show_message_dialog(active_screen, "电量过低");
    }
}

static void system_mcu_status_ui_async(void *user_data)
{
    uint8_t link_status;
    uint8_t latest_link_status;

    LV_UNUSED(user_data);
    link_status = __atomic_load_n(&g_mcu_pending_link_status,
                                  __ATOMIC_ACQUIRE);

    if (system_mcu_link_state_get() == SYSTEM_MCU_LINK_ONLINE)
    {
        /* 在同一个LVGL事务内先清理保护层，再应用页面和方向。 */
        mcu_outage_guard_destroy();
        system_ui_apply_link_status(link_status);
        system_low_battery_protection_apply(link_status);
    }

    __atomic_store_n(&g_mcu_status_ui_pending, false, __ATOMIC_RELEASE);
    latest_link_status = __atomic_load_n(&g_mcu_pending_link_status,
                                         __ATOMIC_ACQUIRE);
    if (latest_link_status != link_status &&
            system_mcu_link_state_get() == SYSTEM_MCU_LINK_ONLINE)
    {
        system_mcu_status_ui_schedule(latest_link_status);
    }
}

static void system_mcu_status_ui_schedule(uint8_t link_status)
{
    bool was_pending;

    __atomic_store_n(&g_mcu_pending_link_status, link_status, __ATOMIC_RELEASE);
    was_pending = __atomic_exchange_n(&g_mcu_status_ui_pending, true,
                                      __ATOMIC_ACQ_REL);
    if (was_pending)
    {
        return;
    }

    if (lv_async_call(system_mcu_status_ui_async, NULL) != LV_RESULT_OK)
    {
        __atomic_store_n(&g_mcu_status_ui_pending, false, __ATOMIC_RELEASE);
        SYSTEM_MANAGER_LOG_ERROR(
            "投递MCU状态恢复事务失败: link_status=0x%02X", link_status);
    }
}

void system_mcu_status_report_notify(uint8_t link_status)
{
    system_mcu_link_state_t previous_state;

    previous_state = __atomic_exchange_n(&g_mcu_link_state,
                                         SYSTEM_MCU_LINK_ONLINE,
                                         __ATOMIC_ACQ_REL);
    if (previous_state == SYSTEM_MCU_LINK_OUTAGE)
    {
        SYSTEM_MANAGER_LOG_USER("MCU合法状态帧已恢复链路");
    }

    if (link_status != LINK_STATUS_DETACHED &&
            link_status != LINK_STATUS_DOCKED)
    {
        SYSTEM_MANAGER_LOG_WARN("忽略未知 LINK_STATUS: 0x%02X", link_status);
        return;
    }

    /* 单独发布在站帧序号，避免旧在站状态与新离站帧序号拼成回仓确认。 */
    if (link_status == LINK_STATUS_DOCKED)
    {
        __atomic_store_n(&g_mcu_docked_report_sequence,
                         serial_mcu_status_report_sequence(), __ATOMIC_RELEASE);
    }
    system_mcu_status_ui_schedule(link_status);
}

void system_mcu_battery_report_notify(uint8_t battery_percent)
{
    if (battery_percent >= MCU_LOW_BATTERY_THRESHOLD_PERCENT)
    {
        __atomic_store_n(&g_mcu_low_battery_handled, false,
                         __ATOMIC_RELEASE);
    }
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

int system_mode_apply_index(int mode_index)
{
    const system_mode_t *mode;

    if (mode_index < 0 || mode_index >= (int)g_mode_count)
    {
        return -1;
    }
    mode = &g_mode_settings[mode_index];
    current_selected_mode_index = mode_index;
    is_self_cleaning = false;
    temp_set = mode->temperature;
    timer_set = mode->time_sec;
    water_level = mode->water_level;
    use_drug1 = mode->use_drug1;
    use_drug2 = mode->use_drug2;
    return 0;
}

int16_t system_current_temp_get(void)
{
    return g_bucket_temp_edit.has_actual_temp ?
           g_bucket_temp_edit.actual_temp : 0;
}

// 刷新基站页面显示
void system_manager_refresh_base_ui(void)
{
    index_sync_selection_ui(&guider_ui);
    update_temp_set(&guider_ui);
    update_timer_set(&guider_ui);
    apply_water_level_ui(water_level);
    apply_use_drug_ui(use_drug1, use_drug2);
}

// 刷新桶体页面显示
void system_manager_refresh_working_ui(void)
{
    bucket_temp_show_actual(guider_ui.working_label_temperature);
    bucket_countdown_update_label();
    bucket_working_sync_function_icons(&guider_ui);
}

void system_manager_refresh_ui(void)
{
    if (g_applied_link_status == LINK_STATUS_DETACHED)
    {
        system_manager_refresh_working_ui();
        return;
    }
    system_manager_refresh_base_ui();
}

void system_start_flow(void)
{
    system_base_flow_t flow = is_self_cleaning ?
                              SYSTEM_BASE_FLOW_SELF_CLEANING :
                              SYSTEM_BASE_FLOW_WATERING;

    if (system_base_flow_request_start(flow, SYSTEM_ACTION_SOURCE_APP).result !=
            SYSTEM_COMMAND_ACCEPTED)
    {
        return;
    }
    to_preparing_flat = is_self_cleaning ? 2 : 1;
    if (is_self_cleaning)
    {
        system_auto_water_navigation_cancel();
    }
    else
    {
        system_auto_water_navigation_arm();
    }
    if (!navigate_to_screen(UI_SCREEN_PREPARING))
    {
        system_base_flow_finish(flow, "tcp-start-page-failed");
        to_preparing_flat = 0;
    }
}

void system_select_self_clean(void)
{
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        return;
    }
    is_self_cleaning = true;
    system_manager_refresh_base_ui();
}

void conf_mode_detail_save_clicked(lv_event_t *e)
{
    size_t index = (size_t)current_selected_mode_index;
    system_mode_t old_mode;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED || index >= g_mode_count)
    {
        return;
    }
    old_mode = g_mode_settings[index];
    update_mode_from_detail_ui(&g_mode_settings[index]);
    if (persist_mode_settings() != 0)
    {
        g_mode_settings[index] = old_mode;
        SYSTEM_MANAGER_LOG_ERROR("模式保存失败，未应用本次修改");
        show_message_dialog(guider_ui.conf_mode_detail, "设置保存失败");
        return;
    }
    conf_mode_detail_keyboard_cleanup(&guider_ui);
    SYSTEM_MANAGER_LOG_USER("模式已保存: %s 站点=%s",
                            g_mode_settings[index].name, g_mode_settings[index].station_name);
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
        mode->station_name[0] = '\0';
        break;
    case 1:
        snprintf(mode->name, sizeof(mode->name), "放松");
        mode->icon_id = 3;
        mode->time_sec = 1200;
        mode->water_level = 1;
        mode->temperature = 39;
        mode->use_drug1 = false;
        mode->use_drug2 = true;
        mode->station_name[0] = '\0';
        break;
    case 2:
        snprintf(mode->name, sizeof(mode->name), "驱寒");
        mode->icon_id = 11;
        mode->time_sec = 1500;
        mode->water_level = 2;
        mode->temperature = 41;
        mode->use_drug1 = true;
        mode->use_drug2 = true;
        mode->station_name[0] = '\0';
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
        if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
        {
            return;
        }
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
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        return;
    }
    if (temp_set < TEMP_MAX)
    {
        temp_set++;
    }
    update_temp_set(ui);
}
void action_temp_minus(lv_ui *ui)
{
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        return;
    }
    if (temp_set > TEMP_MIN)
    {
        temp_set--;
    }
    update_temp_set(ui);
}
void action_time_loop(lv_ui *ui)
{
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        return;
    }
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
    if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
    {
        return;
    }
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
        if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
        {
            return;
        }
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
        if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
        {
            return;
        }
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

// 点击自清洁图标
void action_clean_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        if (system_mcu_link_state_get() != SYSTEM_MCU_LINK_ONLINE)
        {
            return;
        }
        //隐藏cont_2,显示cont_clean
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            is_self_cleaning = true;
            index_sync_selection_ui(ui);
        }
        break;
    }
    default:
        break;
    }
}

// 主页左上角框内容切换
static void index_sync_selection_ui(lv_ui *ui)
{
    if (ui == NULL || ui->index_cont_2 == NULL ||
            ui->index_cont_clean == NULL ||
            !lv_obj_is_valid(ui->index_cont_2) ||
            !lv_obj_is_valid(ui->index_cont_clean))
    {
        return;
    }

    if (is_self_cleaning)
    {
        lv_obj_add_flag(ui->index_cont_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(ui->index_cont_clean, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(ui->index_cont_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui->index_cont_clean, LV_OBJ_FLAG_HIDDEN);
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

// 自动注水页面显示
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
    self_clean_flow_event_t event;

    if (ui == NULL)
    {
        return;
    }
    if (!self_clean_flow_update(&g_self_clean_flow, base_main_status, &event))
    {
        return;
    }

    switch (event)
    {
    case SELF_CLEAN_FLOW_EVENT_STAGE_1:
        cleaning_animation_set_stage(1u);
        break;
    case SELF_CLEAN_FLOW_EVENT_STAGE_2:
        cleaning_animation_set_stage(2u);
        break;
    case SELF_CLEAN_FLOW_EVENT_STAGE_3:
        cleaning_animation_set_stage(3u);
        break;
    case SELF_CLEAN_FLOW_EVENT_STAGE_4:
        cleaning_animation_set_stage(4u);
        break;
    case SELF_CLEAN_FLOW_EVENT_STAGE_5:
        cleaning_animation_set_stage(5u);
        break;
    case SELF_CLEAN_FLOW_EVENT_COMPLETE:
        is_self_cleaning = false;
        system_base_flow_finish(SYSTEM_BASE_FLOW_SELF_CLEANING,
                                "self-clean-complete");
        (void)voice_command_service_play_prompt(VOICE_PROMPT_CLEAN_FINISH);
        if (g_preparing_page_timer != NULL)
        {
            lv_timer_delete(g_preparing_page_timer);
            g_preparing_page_timer = NULL;
        }
        cleaning_animation_cleanup();
        lv_obj_add_flag(ui->preparing_cont_cleaning, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_text_font(ui->preparing_label_17,
                                   custom_get_dynamic_text_font_30(),
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui->preparing_label_timer_tips,
                                   custom_get_dynamic_text_font_30(),
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui->preparing_label_17, "自清洁已完成");
        lv_label_set_text(ui->preparing_label_timer_tips,
                          "设备已进入待机状态");
        lv_obj_clear_flag(ui->preparing_cont_move_finished,
                          LV_OBJ_FLAG_HIDDEN);
        self_clean_finish_show(ui);
        break;
    case SELF_CLEAN_FLOW_EVENT_NONE:
    default:
        break;
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

// 按键控制恒温
void working_constant_temperature_toggle(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    (void)system_bucket_heat_set(!constant_temperature);
}

// 按UV灯
void working_sterilization_toggle(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    (void)system_bucket_uv_set(!sterilization);
}

// 设置按摩强度
void working_massage_intensity_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }
    (void)system_bucket_massage_set((massage_intensity + 1) % 4);
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
void system_manager_working_page_init(lv_ui *ui)
{
    if (ui == NULL)
    {
        return;
    }

    SYSTEM_MANAGER_LOG_USER("LINK_DIAG system_manager_working_page_init enter");
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

    // 创建工作页定时器，每秒刷新实际温度。
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

    lv_obj_remove_flag(ui->working_cont_playing, LV_OBJ_FLAG_HIDDEN);
    bucket_working_sync_function_icons(ui);
    SYSTEM_MANAGER_LOG_USER("LINK_DIAG system_manager_working_page_init leave");
}
