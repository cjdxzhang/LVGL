#include "device_control.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lvgl/lvgl.h>

static pthread_mutex_t g_control_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_periodic_thread;
static bool g_periodic_run;
static bool g_periodic_started;
static footbath_settings_t g_settings;

static bool valid_range(uint8_t value, uint8_t min, uint8_t max)
{
    return value >= min && value <= max;
}

static const char *cmd_name(uint8_t cmd_type)
{
    switch (cmd_type) {
        case CMD_IDLE:
            return "idle";
        case CMD_BUCKET_POWER_OFF:
            return "bucket_power_off";
        case CMD_BUCKET_STANDBY:
            return "bucket_standby";
        case CMD_BUCKET_HEAT_ON:
            return "heat_on";
        case CMD_BUCKET_HEAT_OFF:
            return "heat_off";
        case CMD_BUCKET_MASSAGE:
            return "massage";
        case CMD_BUCKET_UV:
            return "sterilize";
        case CMD_BUCKET_TIMER:
            return "timer";
        case CMD_BUCKET_STOP:
            return "bucket_stop";
        case CMD_BUCKET_SELFCHECK:
            return "bucket_selfcheck";
        case CMD_BUCKET_LOW_BATT:
            return "bucket_low_batt";
        case CMD_BUCKET_AUTO_WAREHOUSE:
            return "auto_warehouse";
        case CMD_BASE_POWER_OFF:
            return "base_power_off";
        case CMD_BASE_STANDBY:
            return "base_standby";
        case CMD_BASE_AUTO_FILL:
            return "base_auto_fill";
        case CMD_BASE_SELFCLEAN:
            return "base_self_clean";
        case CMD_BASE_FORCE_DRAIN:
            return "base_force_drain";
        case CMD_BASE_HOT_SPRAY:
            return "base_hot_spray";
        case CMD_BASE_COLD_SPRAY:
            return "base_cold_spray";
        case CMD_BASE_DRY:
            return "base_dry";
        case CMD_BASE_SELFCHECK:
            return "base_selfcheck";
        case CMD_SYSTEM_RESET:
            return "system_reset";
        default:
            return "unknown";
    }
}

static void log_frame_result(const char *button_name,
                             const protocol_frame_t *frame,
                             int ret,
                             int attempt)
{
    uint8_t raw[FRAME_LEN];
    char hex[FRAME_LEN * 3 + 1] = {0};

    protocol_build(frame, raw);
    for (int i = 0; i < FRAME_LEN; i++) {
        snprintf(hex + strlen(hex), sizeof(hex) - strlen(hex),
                 "%02X%s", raw[i], i + 1 < FRAME_LEN ? " " : "");
    }

    LV_LOG_USER("[CONTROL] button=%s cmd_type=0x%02X frame=%s result=%d attempt=%d",
                button_name ? button_name : cmd_name((uint8_t)frame->cmd_type),
                (uint8_t)frame->cmd_type,
                hex,
                ret,
                attempt);
}

static void default_idle_frame(protocol_frame_t *frame)
{
    memset(frame, 0, sizeof(*frame));
    frame->link_mode = LINK_MODE_MCU_COMMAND;
    frame->cmd_type = CMD_IDLE;
    frame->link_status = 0;
}

static int send_frame_once(const char *button_name, const protocol_frame_t *frame)
{
    int ret = protocol_send_uart(frame);
    log_frame_result(button_name, frame, ret, 1);
    if (ret == FRAME_LEN) {
        return DEVICE_CONTROL_OK;
    }

    LV_LOG_WARN("[CONTROL] send failed or short write, retry once: cmd=0x%02X ret=%d",
                (uint8_t)frame->cmd_type, ret);

    ret = protocol_send_uart(frame);
    log_frame_result(button_name, frame, ret, 2);
    return ret == FRAME_LEN ? DEVICE_CONTROL_OK : DEVICE_CONTROL_ERR_SEND;
}

static int send_frame_locked(const char *button_name, const protocol_frame_t *frame)
{
    int result;

    pthread_mutex_lock(&g_control_lock);
    result = send_frame_once(button_name, frame);
    pthread_mutex_unlock(&g_control_lock);

    return result;
}

static void *periodic_thread_func(void *arg)
{
    (void)arg;

    while (true) {
        protocol_frame_t frame;
        bool running;

        pthread_mutex_lock(&g_control_lock);
        running = g_periodic_run;
        pthread_mutex_unlock(&g_control_lock);

        if (!running) {
            break;
        }

        default_idle_frame(&frame);
        pthread_mutex_lock(&g_control_lock);
        protocol_send_uart(&frame);
        pthread_mutex_unlock(&g_control_lock);
        sleep(1);
    }

    return NULL;
}

void device_control_load_settings(void)
{
    /* 当前阶段暂不启用持久化，启动时只使用安全默认值。 */
    protocol_storage_defaults(&g_settings);
}

bool device_control_get_settings(footbath_settings_t *settings)
{
    if (settings == NULL) {
        return false;
    }

    pthread_mutex_lock(&g_control_lock);
    *settings = g_settings;
    pthread_mutex_unlock(&g_control_lock);
    return true;
}

bool device_control_save_settings(const footbath_settings_t *settings)
{
    if (settings == NULL) {
        return false;
    }

    pthread_mutex_lock(&g_control_lock);
    g_settings = *settings;
    pthread_mutex_unlock(&g_control_lock);

    /* 当前阶段暂不写入 /userdata/footbath/，只保留内存中的设置。 */
    return true;
}

void device_control_init(void)
{
    pthread_mutex_lock(&g_control_lock);
    if (!g_periodic_started) {
        g_periodic_run = true;
        if (pthread_create(&g_periodic_thread, NULL, periodic_thread_func, NULL) == 0) {
            g_periodic_started = true;
        } else {
            g_periodic_run = false;
            LV_LOG_WARN("[CONTROL] create periodic thread failed");
        }
    }
    pthread_mutex_unlock(&g_control_lock);
}

void device_control_deinit(void)
{
    pthread_mutex_lock(&g_control_lock);
    if (!g_periodic_started) {
        pthread_mutex_unlock(&g_control_lock);
        return;
    }
    g_periodic_run = false;
    pthread_mutex_unlock(&g_control_lock);

    pthread_join(g_periodic_thread, NULL);

    pthread_mutex_lock(&g_control_lock);
    g_periodic_started = false;
    pthread_mutex_unlock(&g_control_lock);
}

int device_control_bucket_power_off(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_POWER_OFF;
    return send_frame_locked("bucket_power_off", &frame);
}

int device_control_bucket_standby(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_STANDBY;
    return send_frame_locked("bucket_standby", &frame);
}

int device_control_bucket_heat_on(uint8_t temp)
{
    protocol_frame_t frame;
    if (!valid_range(temp, 0, 50)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_HEAT_ON;
    frame.bucket_temp = temp;
    return send_frame_locked("heat_on", &frame);
}

int device_control_bucket_heat_off(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_HEAT_OFF;
    return send_frame_locked("heat_off", &frame);
}

int device_control_bucket_massage(uint8_t level)
{
    protocol_frame_t frame;
    if (!valid_range(level, 0, 3)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_MASSAGE;
    frame.bucket_massage = level;
    return send_frame_locked("massage", &frame);
}

int device_control_bucket_uv(uint8_t on)
{
    protocol_frame_t frame;
    if (on > 1) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_UV;
    frame.bucket_uv = on;
    return send_frame_locked("sterilize", &frame);
}

int device_control_bucket_timer(uint8_t timer)
{
    protocol_frame_t frame;
    if (!valid_range(timer, 0, 6)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_TIMER;
    frame.bucket_timer = timer;
    return send_frame_locked("timer", &frame);
}

int device_control_bucket_stop(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_STOP;
    return send_frame_locked("bucket_stop", &frame);
}

int device_control_bucket_selfcheck(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_SELFCHECK;
    return send_frame_locked("bucket_selfcheck", &frame);
}

int device_control_bucket_low_batt(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_LOW_BATT;
    return send_frame_locked("bucket_low_batt", &frame);
}

int device_control_bucket_auto_warehouse(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BUCKET_AUTO_WAREHOUSE;
    return send_frame_locked("auto_warehouse", &frame);
}

int device_control_base_power_off(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_POWER_OFF;
    return send_frame_locked("base_power_off", &frame);
}

int device_control_base_standby(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_STANDBY;
    return send_frame_locked("base_standby", &frame);
}

int device_control_base_auto_fill(uint8_t water_level, uint8_t temp,
                                  uint8_t herb1, uint8_t herb2)
{
    protocol_frame_t frame;
    if (!valid_range(water_level, 1, 20) || !valid_range(temp, 35, 48) || 
        !valid_range(herb1, 0, 15) || !valid_range(herb2, 0, 15)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_AUTO_FILL;
    frame.bucket_temp = temp;
    frame.base_water_supply = water_level;
    frame.base_herb1 = herb1;
    frame.base_herb2 = herb2;
    return send_frame_locked("base_auto_fill", &frame);
}

int device_control_base_start_cached(uint8_t water_level, uint8_t temp,
                                     uint8_t herb1, uint8_t herb2,
                                     uint8_t timer)
{
    protocol_frame_t fill_frame;
    protocol_frame_t timer_frame;
    int result;

    if (!valid_range(water_level, 1, 20) || !valid_range(temp, 35, 48) || 
        !valid_range(herb1, 0, 15) || !valid_range(herb2, 0, 15) ||
        !valid_range(timer, 0, 6)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&fill_frame);
    fill_frame.cmd_type = CMD_BASE_AUTO_FILL;
    fill_frame.bucket_temp = temp;
    fill_frame.base_water_supply = water_level;
    fill_frame.base_herb1 = herb1;
    fill_frame.base_herb2 = herb2;

    default_idle_frame(&timer_frame);
    timer_frame.cmd_type = CMD_BUCKET_TIMER;
    timer_frame.bucket_timer = timer;

    pthread_mutex_lock(&g_control_lock);
    result = send_frame_once("start_auto_fill", &fill_frame);
    if (result == DEVICE_CONTROL_OK) {
        result = send_frame_once("start_timer", &timer_frame);
    }
    pthread_mutex_unlock(&g_control_lock);

    return result;
}

int device_control_base_self_clean(uint8_t hot_spray_min,
                                   uint8_t cold_spray_min,
                                   uint8_t dry_units)
{
    protocol_frame_t frame;
    if (!valid_range(hot_spray_min, 0, 10) || 
        !valid_range(cold_spray_min, 0, 10) ||
        !valid_range(dry_units, 0, 9)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_SELFCLEAN;
    frame.base_spray_hot_t = hot_spray_min;
    frame.base_spray_cold_t = cold_spray_min;
    frame.base_dry_time = dry_units;
    return send_frame_locked("base_self_clean", &frame);
}

int device_control_base_force_drain(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_FORCE_DRAIN;
    return send_frame_locked("base_force_drain", &frame);
}

int device_control_base_hot_spray(uint8_t hot_spray_min)
{
    protocol_frame_t frame;
    if (!valid_range(hot_spray_min, 0, 10)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_HOT_SPRAY;
    frame.base_spray_hot_t = hot_spray_min;
    return send_frame_locked("base_hot_spray", &frame);
}

int device_control_base_cold_spray(uint8_t cold_spray_min)
{
    protocol_frame_t frame;
    if (!valid_range(cold_spray_min, 0, 10)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_COLD_SPRAY;
    frame.base_spray_cold_t = cold_spray_min;
    return send_frame_locked("base_cold_spray", &frame);
}

int device_control_base_dry(uint8_t dry_units)
{
    protocol_frame_t frame;
    if (!valid_range(dry_units, 0, 9)) {
        return DEVICE_CONTROL_ERR_PARAM;
    }

    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_DRY;
    frame.base_dry_time = dry_units;
    return send_frame_locked("base_dry", &frame);
}

int device_control_base_selfcheck(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_BASE_SELFCHECK;
    return send_frame_locked("base_selfcheck", &frame);
}

int device_control_system_reset(void)
{
    protocol_frame_t frame;
    default_idle_frame(&frame);
    frame.cmd_type = CMD_SYSTEM_RESET;
    return send_frame_locked("system_reset", &frame);
}
