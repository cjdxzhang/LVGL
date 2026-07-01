#include "device_state.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <lvgl/lvgl.h>

static device_state_t g_state;
static pthread_mutex_t g_state_lock = PTHREAD_MUTEX_INITIALIZER;
static device_state_listener_t g_listener;
static void *g_listener_user_data;

static uint32_t get_tick_ms(void)
{
    return lv_tick_get();
}

static uint32_t decode_bucket_faults(const protocol_frame_t *frame)
{
    return ((uint32_t)frame->bucket_err2 << 8) | frame->bucket_err1;
}

static uint32_t decode_base_faults(const protocol_frame_t *frame)
{
    return ((uint32_t)frame->base_err2 << 8) | frame->base_err1;
}

void device_state_init(void)
{
    pthread_mutex_lock(&g_state_lock);
    memset(&g_state, 0, sizeof(g_state));
    g_state.comm_timeout = true;
    pthread_mutex_unlock(&g_state_lock);
}

void device_state_register_listener(device_state_listener_t listener, void *user_data)
{
    pthread_mutex_lock(&g_state_lock);
    g_listener = listener;
    g_listener_user_data = user_data;
    pthread_mutex_unlock(&g_state_lock);
}

void device_state_update_from_frame(const protocol_frame_t *frame)
{
    device_state_t snapshot;
    device_state_listener_t listener;
    void *user_data;

    if (frame == NULL) {
        return;
    }

    pthread_mutex_lock(&g_state_lock);
    g_state.frame = *frame;
    g_state.base_connected = frame->link_status == 0x01;
    g_state.comm_timeout = false;
    g_state.last_valid_ms = get_tick_ms();
    g_state.bucket_faults = decode_bucket_faults(frame);
    g_state.base_faults = decode_base_faults(frame);
    snapshot = g_state;
    listener = g_listener;
    user_data = g_listener_user_data;
    pthread_mutex_unlock(&g_state_lock);

    if (listener) {
        listener(&snapshot, user_data);
    }
}

void device_state_get(device_state_t *out)
{
    if (out == NULL) {
        return;
    }

    pthread_mutex_lock(&g_state_lock);
    *out = g_state;
    pthread_mutex_unlock(&g_state_lock);
}

void device_state_check_timeout(uint32_t now_ms)
{
    device_state_t snapshot;
    device_state_listener_t listener = NULL;
    void *user_data = NULL;
    bool changed = false;

    pthread_mutex_lock(&g_state_lock);
    if (!g_state.comm_timeout && 
        now_ms - g_state.last_valid_ms > PROTO_RX_TIMEOUT_MS) {
        g_state.comm_timeout = true;
        changed = true;
    }
    snapshot = g_state;
    if (changed) {
        listener = g_listener;
        user_data = g_listener_user_data;
    }
    pthread_mutex_unlock(&g_state_lock);

    if (listener) {
        listener(&snapshot, user_data);
    }
}

static void append_fault(char *buf, int len, const char *text)
{
    if (buf == NULL || len <= 0 || text == NULL) {
        return;
    }

    if (buf[0] != '\0') {
        strncat(buf, ",", len - strlen(buf) - 1);
    }
    strncat(buf, text, len - strlen(buf) - 1);
}

void device_state_fault_summary(char *buf, int len)
{
    device_state_t state;

    if (buf == NULL || len <= 0) {
        return;
    }

    buf[0] = '\0';
    device_state_get(&state);

    if (state.bucket_faults & BUCKET_ERR1_LACK_OF_WATER) {
        append_fault(buf, len, "桶体缺水");
    }
    if (state.bucket_faults & BUCKET_ERR1_DRAIN_TIMEOUT) {
        append_fault(buf, len, "桶体排水超时");
    }
    if (state.bucket_faults & BUCKET_ERR1_HIGH_TEMP) {
        append_fault(buf, len, "桶体温度异常");
    }
    if (state.bucket_faults & BUCKET_ERR2_MASSAGE_FAULT) {
        append_fault(buf, len, "桶体按摩电机故障");
    }
    if (state.base_faults & BASE_ERR1_HERB1_LACK) {
        append_fault(buf, len, "药浴1缺液");
    }
    if (state.base_faults & ((uint32_t)BASE_ERR2_DRAIN_PUMP_FAULT << 8)) {
        append_fault(buf, len, "基站排水泵故障");
    }
    if (buf[0] == '\0') {
        snprintf(buf, len, "无故障");
    }
}
