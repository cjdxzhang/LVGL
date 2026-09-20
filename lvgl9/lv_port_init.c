/**
 * @file lv_port_init.c
 *
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <syslog.h>

#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>

#include "lv_port_disp.h"
#include "lv_port_indev.h"

#if defined(LV_USE_RKADK) && LV_USE_RKADK
#include <lvgl/src/drivers/display/rkadk/rkadk.h>
#endif

#if LV_USE_PROFILER
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

static FILE *profiler_fd = NULL;
#endif

/* 0, 90, 180, 270 */
static int g_indev_rotation = 0;
static int g_port_rotation = -1;

static int log_level = LV_LOG_LEVEL_WARN;
static void append_logcat_message(const char *buf)
{
    FILE *fp = fopen("/var/log/messages", "a");
    if (fp == NULL)
    {
        return;
    }

    fprintf(fp, "[LVGL] %s\n", buf);
    fclose(fp);
}

static void print_cb(lv_log_level_t level, const char *buf)
{
    if (level >= log_level)
    {
        syslog(LOG_USER | LOG_NOTICE, "[LVGL] %s\n", buf);
        append_logcat_message(buf);
    }
}

static uint32_t lv_systick(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

static void lv_sysdelay(uint32_t ms)
{
    usleep(ms * 1000);
}

#if LV_USE_PROFILER
static void profiler_flush_cb(const char *buf)
{
    if (profiler_fd)
        fwrite(buf, 1, strlen(buf), profiler_fd);
}

static int profiler_tid_get_cb(void)
{
    return gettid();
}

static uint32_t profiler_tick_get_cb(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}
#endif

void lv_port_init(int width, int height, int rotation)
{
#if LV_USE_PROFILER
    lv_profiler_builtin_config_t profiler_config;
#endif
    const char *buf;

    lv_init();
    openlog("rk_demo_test", LOG_PID | LOG_CONS, LOG_USER);

    buf = getenv("LV_DEBUG");
    if (buf)
        log_level = buf[0] - '0';
    lv_log_register_print_cb(print_cb);
    lv_tick_set_cb(lv_systick);
    lv_delay_set_cb(lv_sysdelay);

#if LV_USE_PROFILER
    profiler_fd = fopen("/userdata/lvgl_profiler.txt", "wb+");
    lv_profiler_builtin_config_init(&profiler_config);
    profiler_config.tick_per_sec = 1000000;
    profiler_config.tick_get_cb = profiler_tick_get_cb;
    profiler_config.flush_cb = profiler_flush_cb;
    profiler_config.tid_get_cb = profiler_tid_get_cb;
    lv_profiler_builtin_init(&profiler_config);
#endif

    lv_port_disp_init(width, height, rotation);
    lv_port_indev_init(g_indev_rotation + rotation);
    g_port_rotation = rotation;
}

int lv_port_set_rotation(int rotation)
{
    lv_display_t *disp;
    lv_display_rotation_t lvgl_rotation;
    lv_display_rotation_t previous_lvgl_rotation;

    if (rotation != 0 && rotation != 180)
    {
        LV_LOG_ERROR("不支持的运行时旋转角度: %d", rotation);
        return -1;
    }
    if (g_port_rotation == rotation)
    {
        return 0;
    }

    disp = lv_display_get_default();
    if (disp == NULL)
    {
        LV_LOG_ERROR("默认显示设备尚未创建");
        return -1;
    }

    lvgl_rotation = (rotation == 180) ?
                    LV_DISPLAY_ROTATION_180 : LV_DISPLAY_ROTATION_0;
    previous_lvgl_rotation = (g_port_rotation == 180) ?
                             LV_DISPLAY_ROTATION_180 : LV_DISPLAY_ROTATION_0;

#if defined(LV_USE_RKADK) && LV_USE_RKADK
    if (lv_rkadk_disp_set_rotation(disp, lvgl_rotation) != 0)
    {
        LV_LOG_ERROR("RKADK 显示旋转失败: %d", rotation);
        return -1;
    }
#else
    lv_display_set_rotation(disp, lvgl_rotation);
#endif

    if (lv_port_indev_set_rotation(g_indev_rotation + rotation) != 0)
    {
        LV_LOG_ERROR("触摸方向旋转失败: %d", rotation);
#if defined(LV_USE_RKADK) && LV_USE_RKADK
        if (lv_rkadk_disp_set_rotation(disp, previous_lvgl_rotation) != 0)
        {
            LV_LOG_ERROR("显示方向回滚失败: %d", g_port_rotation);
        }
#else
        lv_display_set_rotation(disp, previous_lvgl_rotation);
#endif
        return -1;
    }

    g_port_rotation = rotation;
    lv_obj_invalidate(lv_screen_active());
    return 0;
}
