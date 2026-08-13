#include "status_bar.h"
#include "app_manager.h"
#include "wifi_manager.h"
#include "app_log.h"
#include <stdint.h>
#include <time.h>
#include <stdio.h>

#define STATUS_BAR_LOG_USER(...) APP_LOG_USER("STATUS_BAR", __VA_ARGS__)
#define STATUS_BAR_LOG_WARN(...) APP_LOG_WARN("STATUS_BAR", __VA_ARGS__)
#define STATUS_BAR_LOG_ERROR(...) APP_LOG_ERROR("STATUS_BAR", __VA_ARGS__)

// 静态变量，保存状态栏上的控件对象
static lv_obj_t *status_container = NULL; //顶部状态栏对象指针
static lv_obj_t *wifi_label = NULL;
static lv_obj_t *link_label = NULL;
static lv_obj_t *sound_label = NULL;
static lv_obj_t *battery_label = NULL;
static lv_obj_t *time_label = NULL;
static bool has_wifi_state = false;
static bool last_wifi_connected = false;

static void align_time_label(void)
{
    if (time_label == NULL || battery_label == NULL)
    {
        return;
    }

    lv_obj_align_to(time_label, battery_label, LV_ALIGN_OUT_LEFT_MID, -8, 0);
}

static void update_wifi_async(void *user_data)
{
    bool connected = (bool)(uintptr_t)user_data;

    status_bar_update_wifi(connected);
}

// WiFi 状态变化回调（注册给 wifi_manager）
static void on_wifi_status_changed(bool connected, const char *ssid, void *user_data)
{
    (void)ssid;      // 未使用参数，避免编译警告
    (void)user_data; // 未使用参数
    lv_async_call(update_wifi_async, (void *)(uintptr_t)connected);
}

// 时间更新定时器回调
static void time_update_timer_cb(lv_timer_t *timer)
{
    robot_chassis_status_t status = {0};

    (void)timer;
    status_bar_update_time();

    // if (app_manager_robot_get_status(&status) == ROBOT_CHASSIS_OK &&
    //         (status.valid_fields &
    //          ROBOT_CHASSIS_STATUS_VALID_BATTERY_PERCENT) != 0U)
    // {
    //     status_bar_update_battery((int)status.battery_percent);
    // }
}

lv_obj_t *status_bar_create(lv_ui *ui)
{
    int ret;
    lv_timer_t *timer;

    (void)ui;
    if (status_container != NULL)
    {
        STATUS_BAR_LOG_USER("状态栏已创建，复用现有对象");
        return status_container; // 已经创建过了
    }

    // 1. 获取顶层图层
    lv_obj_t *top_layer = lv_layer_top();

    // 2. 创建状态栏容器
    status_container = lv_obj_create(top_layer);
    lv_obj_set_size(status_container, LV_HOR_RES, 45);  // 高度40像素
    lv_obj_set_style_bg_opa(status_container, LV_OPA_0, 0);   // 完全透明背景
    lv_obj_set_style_border_width(status_container, 0, 0);    // 无边框
    lv_obj_set_style_pad_all(status_container, 0, 0);
    lv_obj_clear_flag(status_container, LV_OBJ_FLAG_SCROLLABLE);

    // 3. 在状态栏容器上创建控件
    wifi_label = lv_label_create(status_container);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(wifi_label, 8, 10);

    link_label = lv_label_create(status_container);
    lv_label_set_text(link_label, LV_SYMBOL_USB);
    lv_obj_set_style_text_color(link_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(link_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(link_label, 46, 10);

    sound_label = lv_label_create(status_container);
    lv_label_set_text(sound_label, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(sound_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(sound_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sound_label, 84, 10);

    // 右上角：时间标签
    time_label = lv_label_create(status_container);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_20, 0);
    lv_label_set_text(time_label, "2026-01-01 00:00:00");

    // 电池图标（使用字体符号，避免图片资源缺失导致链接错误）
    battery_label = lv_label_create(status_container);
    lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(battery_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(battery_label, LV_HOR_RES - 38, 10);

    align_time_label();

    // 4. 注册 WiFi 状态变化回调
    ret = wifi_manager_register_callback(on_wifi_status_changed, NULL);
    if (ret != 0)
    {
        STATUS_BAR_LOG_ERROR("注册 Wi-Fi 状态回调失败: ret=%d", ret);
    }

    // 5. 立即获取一次当前 WiFi 状态，更新图标
    status_bar_update_wifi(wifi_manager_is_connected());

    // 6. 立即更新一次电量
    status_bar_update_battery(100);

    // 7. 启动定时器，每秒更新时间
    timer = lv_timer_create(time_update_timer_cb, 1000, NULL);
    if (timer == NULL)
    {
        STATUS_BAR_LOG_ERROR("创建状态栏更新时间定时器失败");
    }

    STATUS_BAR_LOG_USER("状态栏创建完成");

    return status_container;
}

void status_bar_update_wifi(bool connected)
{
    if (wifi_label == NULL)
    {
        STATUS_BAR_LOG_ERROR("更新 Wi-Fi 图标失败: 控件未创建");
        return;
    }

    if (has_wifi_state && connected == last_wifi_connected)
    {
        return;
    }

    if (connected)
    {
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(0xffffff), 0);
    }
    else
    {
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(0x999999), 0);
    }

    has_wifi_state = true;
    last_wifi_connected = connected;
    STATUS_BAR_LOG_USER("Wi-Fi 图标状态更新: connected=%d", connected ? 1 : 0);
}

void status_bar_update_time(void)
{
    if (time_label == NULL) return;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    lv_label_set_text(time_label, time_str);
    align_time_label();
}

void status_bar_update_battery(int battery_level)
{
    if (battery_label == NULL) return;

    if (battery_level < 5)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_EMPTY);
    }
    else if (battery_level < 20)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_1);
    }
    else if (battery_level < 50)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_2);
    }
    else if (battery_level < 80)
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_3);
    }
    else
    {
        lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_FULL);
    }
}

lv_obj_t *status_bar_get_container(void)
{
    return status_container;
}
