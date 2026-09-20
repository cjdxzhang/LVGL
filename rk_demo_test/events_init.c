/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include "page_initialized.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "custom.h"
#include "widgets_init.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "custom_imgbtn.h"
#include "system_manager.h"
#include "custom_imgbtn.h"
#include "system_manager.h"
#include "custom_imgbtn.h"
#include "system_manager.h"

#include "custom_imgbtn.h"
#include "system_manager.h"
#include "custom_imgbtn.h"
#include "system_manager.h"
#include "custom_imgbtn.h"
#include "system_manager.h"

#include "custom_imgbtn.h"
#include "system_manager.h"

#include "system_manager.h"
#include "system_manager.h"
#include "wifi_manager.h"
#include "custom_imgbtn.h"

static lv_obj_t *g_preparing_stop_button = NULL;

typedef struct
{
    lv_obj_t *home;
    lv_obj_t *back;
} page_navigation_buttons_t;

static page_navigation_buttons_t g_conf_advance_navigation = {0};
static page_navigation_buttons_t g_conf_wifi_navigation = {0};
static page_navigation_buttons_t g_conf_other_navigation = {0};

static void ensure_page_navigation_buttons(page_navigation_buttons_t *buttons,
        lv_obj_t *page,
        lv_event_cb_t home_event_cb,
        lv_event_cb_t back_event_cb,
        lv_ui *ui)
{
    bool home_valid;
    bool back_valid;

    home_valid = buttons->home != NULL && lv_obj_is_valid(buttons->home) &&
                 lv_obj_get_parent(buttons->home) == page;
    back_valid = buttons->back != NULL && lv_obj_is_valid(buttons->back) &&
                 lv_obj_get_parent(buttons->back) == page;
    if (home_valid && back_valid)
    {
        return;
    }

    if (buttons->home != NULL && lv_obj_is_valid(buttons->home))
    {
        lv_obj_delete(buttons->home);
    }
    if (buttons->back != NULL && lv_obj_is_valid(buttons->back))
    {
        lv_obj_delete(buttons->back);
    }

    buttons->home = imgbtn_create(page, &_home_RGB565A8_100x100, "", 615, 62, 170, 187);
    buttons->back = imgbtn_create(page, &_return_RGB565A8_100x100, "", 615, 274, 170, 187);
    lv_obj_add_event_cb(buttons->home, home_event_cb, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(buttons->back, back_event_cb, LV_EVENT_ALL, ui);
}

void preparing_stop_button_set_hidden(bool hidden)
{
    if (g_preparing_stop_button == NULL ||
            !lv_obj_is_valid(g_preparing_stop_button))
    {
        g_preparing_stop_button = NULL;
        return;
    }

    if (hidden)
    {
        lv_obj_add_flag(g_preparing_stop_button, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_remove_flag(g_preparing_stop_button, LV_OBJ_FLAG_HIDDEN);
    }
}

static void preparing_apply_stop_button_layout(void)
{
    lv_obj_t *icon;
    lv_obj_t *label;

    if (g_preparing_stop_button == NULL ||
            !lv_obj_is_valid(g_preparing_stop_button))
    {
        g_preparing_stop_button = NULL;
        return;
    }

    icon = lv_obj_get_child(g_preparing_stop_button, 0);
    label = lv_obj_get_child(g_preparing_stop_button, 1);

    lv_obj_set_pos(g_preparing_stop_button, 0, 215);
    lv_obj_set_size(g_preparing_stop_button, 170, 187);
    lv_obj_set_style_radius(g_preparing_stop_button, 20, 0);

    if (icon != NULL && lv_obj_is_valid(icon))
    {
        lv_image_set_scale(icon, LV_SCALE_NONE);
        lv_obj_align(icon, LV_ALIGN_CENTER, 0, -(187 / 6));
    }
    if (label != NULL && lv_obj_is_valid(label))
    {
        lv_obj_set_style_text_font(label, custom_get_small_text_font(), 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 187 / 6);
    }
}

static void conf_wifi_refresh_ssid_dropdown(lv_ui *ui)
{
    char options[4096] = {0};
    char ssid[128] = {0};
    size_t used = 0;
    int scan_count;

    if (ui == NULL || ui->conf_wifi_wifi_ssid == NULL)
    {
        return;
    }

    scan_count = wifi_manager_get_scan_count();
    for (int index = 0; index < scan_count; index++)
    {
        int written;

        if (wifi_manager_get_scan_ssid(index, ssid, sizeof(ssid)) != 0 || ssid[0] == '\0')
        {
            continue;
        }

        written = snprintf(options + used,
                           sizeof(options) - used,
                           "%s%s",
                           used == 0 ? "" : "\n",
                           ssid);
        if (written < 0 || (size_t)written >= sizeof(options) - used)
        {
            break;
        }

        used += (size_t)written;
    }

    lv_dropdown_set_options(ui->conf_wifi_wifi_ssid, options);
    if (used > 0)
    {
        lv_dropdown_set_selected(ui->conf_wifi_wifi_ssid, 0);
    }
}

static lv_obj_t *g_mode_option_dialog = NULL;
static lv_obj_t *g_mode_option_roller = NULL;
static lv_obj_t *g_mode_option_dropdown = NULL;
static lv_timer_t *g_conf_wifi_scan_timer = NULL;
static lv_timer_t *g_conf_wifi_ip_timer = NULL;
static lv_obj_t *g_conf_wifi_ip_timer_screen = NULL;

#define CONF_WIFI_IPV4_TEXT_SIZE 16
#define CONF_WIFI_IP_REFRESH_INTERVAL_MS 2000

typedef struct
{
    pthread_mutex_t mutex;
    bool running;
    bool finished;
    int result;
    char ssid[128];
    char psk[128];
    lv_ui *ui;
    lv_timer_t *poll_timer;
} conf_wifi_connect_job_t;

static conf_wifi_connect_job_t g_conf_wifi_connect_job =
{
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

static bool conf_wifi_connect_is_running(void)
{
    bool running;

    pthread_mutex_lock(&g_conf_wifi_connect_job.mutex);
    running = g_conf_wifi_connect_job.running;
    pthread_mutex_unlock(&g_conf_wifi_connect_job.mutex);
    return running;
}

static void conf_wifi_set_status(lv_ui *ui, const char *text, uint32_t color)
{
    if (ui == NULL || ui->conf_wifi_status_label == NULL ||
            !lv_obj_is_valid(ui->conf_wifi_status_label))
    {
        return;
    }

    lv_label_set_text(ui->conf_wifi_status_label, text != NULL ? text : "");
    lv_obj_set_style_text_color(ui->conf_wifi_status_label, lv_color_hex(color),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void conf_wifi_refresh_ip(lv_ui *ui)
{
    char ipv4[CONF_WIFI_IPV4_TEXT_SIZE] = {0};
    const char *current_text;

    if (ui == NULL || ui->conf_wifi_ip_label == NULL ||
            !lv_obj_is_valid(ui->conf_wifi_ip_label))
    {
        return;
    }

    if (wifi_manager_get_current_ipv4(ipv4, sizeof(ipv4)) != 0)
    {
        current_text = lv_label_get_text(ui->conf_wifi_ip_label);
        if (current_text != NULL && current_text[0] != '\0')
        {
            lv_label_set_text(ui->conf_wifi_ip_label, "");
        }
        if (!lv_obj_has_flag(ui->conf_wifi_ip_label, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_add_flag(ui->conf_wifi_ip_label, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    current_text = lv_label_get_text(ui->conf_wifi_ip_label);
    if (current_text == NULL || strcmp(current_text, ipv4) != 0)
    {
        lv_label_set_text(ui->conf_wifi_ip_label, ipv4);
    }
    lv_obj_remove_flag(ui->conf_wifi_ip_label, LV_OBJ_FLAG_HIDDEN);
}

static void conf_wifi_ip_timer_cb(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);

    conf_wifi_refresh_ip(ui);
}

static void conf_wifi_stop_ip_timer(lv_obj_t *screen)
{
    if (screen != NULL && screen != g_conf_wifi_ip_timer_screen)
    {
        return;
    }

    if (g_conf_wifi_ip_timer != NULL)
    {
        lv_timer_delete(g_conf_wifi_ip_timer);
        g_conf_wifi_ip_timer = NULL;
    }
    g_conf_wifi_ip_timer_screen = NULL;
}

static void conf_wifi_start_ip_timer(lv_ui *ui)
{
    if (ui == NULL || ui->conf_wifi == NULL)
    {
        return;
    }

    conf_wifi_stop_ip_timer(NULL);
    conf_wifi_refresh_ip(ui);
    g_conf_wifi_ip_timer = lv_timer_create(conf_wifi_ip_timer_cb,
                                           CONF_WIFI_IP_REFRESH_INTERVAL_MS,
                                           ui);
    if (g_conf_wifi_ip_timer != NULL)
    {
        g_conf_wifi_ip_timer_screen = ui->conf_wifi;
    }
}

static void conf_wifi_set_connect_button_running(lv_ui *ui, bool running)
{
    if (ui == NULL || ui->conf_wifi_btn_2 == NULL ||
            !lv_obj_is_valid(ui->conf_wifi_btn_2))
    {
        return;
    }

    if (ui->conf_wifi_btn_2_label != NULL && lv_obj_is_valid(ui->conf_wifi_btn_2_label))
    {
        lv_label_set_text(ui->conf_wifi_btn_2_label, "连接");
    }

    if (running)
    {
        lv_obj_add_state(ui->conf_wifi_btn_2, LV_STATE_DISABLED);
    }
    else
    {
        lv_obj_remove_state(ui->conf_wifi_btn_2, LV_STATE_DISABLED);
    }
}

static void *conf_wifi_connect_worker(void *arg)
{
    conf_wifi_connect_job_t *job = (conf_wifi_connect_job_t *)arg;
    int result = custom_save_wifi_info(job->ssid, job->psk);

    pthread_mutex_lock(&job->mutex);
    job->result = result;
    job->finished = true;
    pthread_mutex_unlock(&job->mutex);
    return NULL;
}

static void conf_wifi_connect_poll_timer_cb(lv_timer_t *timer)
{
    lv_ui *ui = NULL;
    bool finished;
    int result = -1;

    pthread_mutex_lock(&g_conf_wifi_connect_job.mutex);
    finished = g_conf_wifi_connect_job.finished;
    if (finished)
    {
        result = g_conf_wifi_connect_job.result;
        ui = g_conf_wifi_connect_job.ui;
        g_conf_wifi_connect_job.running = false;
        g_conf_wifi_connect_job.finished = false;
        g_conf_wifi_connect_job.poll_timer = NULL;
    }
    pthread_mutex_unlock(&g_conf_wifi_connect_job.mutex);

    if (!finished)
    {
        return;
    }

    conf_wifi_set_connect_button_running(ui, false);
    if (result == 0)
    {
        conf_wifi_set_status(ui, "连接成功", 0x4caf50);
    }
    else
    {
        conf_wifi_set_status(ui, "连接失败，请重试", 0xff6b6b);
    }
    lv_timer_delete(timer);
}

static int conf_wifi_connect_start(lv_ui *ui, const char *ssid, const char *psk)
{
    pthread_t worker;
    lv_timer_t *poll_timer;
    int create_result;

    if (ui == NULL || ssid == NULL || psk == NULL || ssid[0] == '\0' || psk[0] == '\0')
    {
        conf_wifi_set_status(ui, "请选择 WiFi 并输入密码", 0xff6b6b);
        return -1;
    }

    if (conf_wifi_connect_is_running())
    {
        return 0;
    }

    poll_timer = lv_timer_create(conf_wifi_connect_poll_timer_cb, 100, NULL);
    if (poll_timer == NULL)
    {
        conf_wifi_set_status(ui, "连接任务启动失败", 0xff6b6b);
        return -1;
    }

    pthread_mutex_lock(&g_conf_wifi_connect_job.mutex);
    snprintf(g_conf_wifi_connect_job.ssid, sizeof(g_conf_wifi_connect_job.ssid), "%s", ssid);
    snprintf(g_conf_wifi_connect_job.psk, sizeof(g_conf_wifi_connect_job.psk), "%s", psk);
    g_conf_wifi_connect_job.ui = ui;
    g_conf_wifi_connect_job.result = -1;
    g_conf_wifi_connect_job.finished = false;
    g_conf_wifi_connect_job.running = true;
    g_conf_wifi_connect_job.poll_timer = poll_timer;
    pthread_mutex_unlock(&g_conf_wifi_connect_job.mutex);

    conf_wifi_set_status(ui, "", 0xffffff);
    conf_wifi_set_connect_button_running(ui, true);
    create_result = pthread_create(&worker, NULL, conf_wifi_connect_worker, &g_conf_wifi_connect_job);
    if (create_result != 0)
    {
        pthread_mutex_lock(&g_conf_wifi_connect_job.mutex);
        g_conf_wifi_connect_job.running = false;
        g_conf_wifi_connect_job.finished = false;
        g_conf_wifi_connect_job.poll_timer = NULL;
        pthread_mutex_unlock(&g_conf_wifi_connect_job.mutex);
        lv_timer_delete(poll_timer);
        conf_wifi_set_connect_button_running(ui, false);
        conf_wifi_set_status(ui, "连接任务启动失败", 0xff6b6b);
        return -1;
    }

    pthread_detach(worker);
    return 0;
}

static void mode_option_dialog_deleted_event_cb(lv_event_t *e)
{
    if (lv_event_get_target(e) == g_mode_option_dialog)
    {
        g_mode_option_dialog = NULL;
        g_mode_option_roller = NULL;
        g_mode_option_dropdown = NULL;
    }
}

static void mode_option_dialog_close(void)
{
    if (g_mode_option_dialog != NULL && lv_obj_is_valid(g_mode_option_dialog))
    {
        lv_obj_del(g_mode_option_dialog);
    }
    else
    {
        g_mode_option_dialog = NULL;
        g_mode_option_roller = NULL;
        g_mode_option_dropdown = NULL;
    }
}

static void mode_option_dialog_cancel_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        mode_option_dialog_close();
    }
}

static void mode_option_dialog_confirm_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    if (g_mode_option_dropdown != NULL &&
            lv_obj_is_valid(g_mode_option_dropdown) &&
            g_mode_option_roller != NULL &&
            lv_obj_is_valid(g_mode_option_roller))
    {
        lv_dropdown_set_selected(g_mode_option_dropdown,
                                 lv_roller_get_selected(g_mode_option_roller));
        lv_obj_send_event(g_mode_option_dropdown, LV_EVENT_VALUE_CHANGED, NULL);
    }

    mode_option_dialog_close();
}

/* 刷新打开中的站点滚轮，按名称保留尚未确认的选择。 */
void mode_station_dialog_refresh(lv_ui *ui)
{
    char name[SYSTEM_LOCATION_NAME_LEN];
    uint32_t selected = 0;
    bool found = false;

    if (ui == NULL || g_mode_option_dropdown != ui->conf_mode_detail_combo_location ||
            g_mode_option_roller == NULL || !lv_obj_is_valid(g_mode_option_roller))
    {
        return;
    }
    if (g_location_count == 0u)
    {
        mode_option_dialog_close();
        return;
    }
    if (lv_roller_get_selected(g_mode_option_roller) == 0u)
    {
        found = true;
    }
    else
    {
        lv_roller_get_selected_str(g_mode_option_roller, name, sizeof(name));
        for (size_t index = 0; index < g_location_count; ++index)
        {
            if (strcmp(g_location_settings[index].name, name) == 0)
            {
                selected = (uint32_t)index + 1u;
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        mode_option_dialog_close();
        return;
    }
    lv_roller_set_options(g_mode_option_roller,
                          lv_dropdown_get_options(g_mode_option_dropdown), LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(g_mode_option_roller, selected, LV_ANIM_OFF);
}

static void mode_option_dialog_show(lv_ui *ui,
                                    lv_obj_t *dropdown,
                                    const char *title_text,
                                    const char *options,
                                    uint16_t option_count)
{
    lv_obj_t *panel;
    lv_obj_t *title;
    lv_obj_t *cancel_btn;
    lv_obj_t *cancel_label;
    lv_obj_t *confirm_btn;
    lv_obj_t *confirm_label;
    uint16_t selected;

    if (ui == NULL ||
            ui->conf_mode_detail == NULL ||
            !lv_obj_is_valid(ui->conf_mode_detail) ||
            dropdown == NULL ||
            !lv_obj_is_valid(dropdown) ||
            title_text == NULL ||
            options == NULL ||
            option_count == 0)
    {
        return;
    }

    if (g_mode_option_dialog != NULL && lv_obj_is_valid(g_mode_option_dialog))
    {
        return;
    }

    selected = lv_dropdown_get_selected(dropdown);
    if (selected >= option_count)
    {
        selected = (uint16_t)(option_count - 1);
    }
    g_mode_option_dropdown = dropdown;

    g_mode_option_dialog = lv_obj_create(ui->conf_mode_detail);
    lv_obj_set_pos(g_mode_option_dialog, 0, 0);
    lv_obj_set_size(g_mode_option_dialog, 800, 480);
    lv_obj_remove_flag(g_mode_option_dialog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_mode_option_dialog, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_scrollbar_mode(g_mode_option_dialog, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(g_mode_option_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_mode_option_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(g_mode_option_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_mode_option_dialog, lv_color_hex(0x000000),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_mode_option_dialog, 130, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(g_mode_option_dialog,
                        mode_option_dialog_deleted_event_cb,
                        LV_EVENT_DELETE,
                        NULL);

    panel = lv_obj_create(g_mode_option_dialog);
    lv_obj_set_size(panel, 360, 330);
    lv_obj_center(panel);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(panel, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(panel, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(panel, 80, LV_PART_MAIN | LV_STATE_DEFAULT);

    title = lv_label_create(panel);
    lv_label_set_text(title, title_text);
    lv_obj_set_style_text_font(title, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

    g_mode_option_roller = lv_roller_create(panel);
    lv_roller_set_options(g_mode_option_roller, options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(g_mode_option_roller, 3);
    lv_roller_set_selected(g_mode_option_roller, selected, LV_ANIM_OFF);
    lv_obj_set_width(g_mode_option_roller, 220);
    lv_obj_align(g_mode_option_roller, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_text_font(g_mode_option_roller, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(g_mode_option_roller, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_mode_option_roller, lv_color_hex(0x8a8a8a),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_mode_option_roller, lv_color_hex(0xf5f2f8),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_mode_option_roller, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_mode_option_roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_mode_option_roller, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_mode_option_roller, lv_color_hex(0x937dad),
                                LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_mode_option_roller, lv_color_hex(0xe7deef),
                              LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_mode_option_roller, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(g_mode_option_roller, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_SELECTED | LV_STATE_DEFAULT);
    if (dropdown == ui->conf_mode_detail_combo_location)
    {
        lv_obj_set_style_text_font(g_mode_option_roller, custom_get_dynamic_text_font(),
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(g_mode_option_roller, custom_get_dynamic_text_font(),
                                   LV_PART_SELECTED | LV_STATE_DEFAULT);
    }

    cancel_btn = lv_button_create(panel);
    lv_obj_set_pos(cancel_btn, 35, 260);
    lv_obj_set_size(cancel_btn, 130, 48);
    lv_obj_set_style_radius(cancel_btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xd8d8d8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(cancel_btn, mode_option_dialog_cancel_event_cb, LV_EVENT_CLICKED, NULL);
    cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_set_style_text_font(cancel_label, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(cancel_label);

    confirm_btn = lv_button_create(panel);
    lv_obj_set_pos(confirm_btn, 195, 260);
    lv_obj_set_size(confirm_btn, 130, 48);
    lv_obj_set_style_radius(confirm_btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x937dad), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(confirm_btn, mode_option_dialog_confirm_event_cb, LV_EVENT_CLICKED, NULL);
    confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "确认");
    lv_obj_set_style_text_font(confirm_label, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(confirm_label);
}

static void conf_wifi_cancel_scan_timer(void)
{
    if (g_conf_wifi_scan_timer != NULL)
    {
        lv_timer_delete(g_conf_wifi_scan_timer);
        g_conf_wifi_scan_timer = NULL;
    }
}

static void conf_wifi_scan_timer_cb(lv_timer_t *timer)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(timer);

    g_conf_wifi_scan_timer = NULL;
    lv_timer_delete(timer);

    if (ui == NULL ||
            ui->conf_wifi == NULL ||
            !lv_obj_is_valid(ui->conf_wifi) ||
            lv_screen_active() != ui->conf_wifi)
    {
        return;
    }

    if (wifi_manager_scan_start() != 0)
    {
        printf("wifi scan failed\n");
    }
    conf_wifi_refresh_ssid_dropdown(ui);
}

static void conf_wifi_schedule_scan(lv_ui *ui)
{
    conf_wifi_cancel_scan_timer();
    g_conf_wifi_scan_timer = lv_timer_create(conf_wifi_scan_timer_cb, 100, ui);
}
#include "custom.h"
#include "custom_imgbtn.h"
#include "system_manager.h"

#include "custom_imgbtn.h"
#include "system_manager.h"

#include "system_manager.h"

static void index_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            if (!_is_index_page_initialized)
            {
                custom_ui_init(ui);
            }

            index_page_init(ui);
            if (!_is_index_page_initialized)
            {

                lv_obj_t *btn_plus = imgbtn_create(ui->index, &_plus_RGB565A8_50x50, "", 186, 239, 92, 92);
                lv_obj_add_event_cb(btn_plus, plus_clicked, LV_EVENT_ALL, ui);

                lv_obj_t *btn_minus = imgbtn_create(ui->index, &_minus_RGB565A8_50x50, "", 302, 239, 92, 92);
                lv_obj_add_event_cb(btn_minus, minus_clicked, LV_EVENT_ALL, ui);

                lv_obj_t *btn_timer = imgbtn_create(ui->index, &_timer_RGB565A8_50x50, "", 186, 356, 92, 92);
                lv_obj_add_event_cb(btn_timer, timer_clicked, LV_EVENT_ALL, ui);

                lv_obj_t *btn_water = imgbtn_create(ui->index, &_water_RGB565A8_50x50, "", 302, 356, 92, 92);
                lv_obj_add_event_cb(btn_water, water_clicked, LV_EVENT_ALL, ui);

                lv_obj_t *btn_play = imgbtn_create(ui->index, &_play_RGB565A8_50x50, "", 15, 239, 146, 211);
                lv_obj_add_event_cb(btn_play, index_to_preparing, LV_EVENT_ALL, ui);

                lv_ui *ui = lv_event_get_user_data(e);


                lv_obj_set_style_shadow_width(ui->index_cont_2, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->index_cont_2, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->index_cont_2, 3, 0);
                lv_obj_set_style_shadow_color(ui->index_cont_2, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->index_cont_2, LV_OPA_40, 0);

                lv_obj_set_style_shadow_width(ui->index_cont_clean, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->index_cont_clean, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->index_cont_clean, 3, 0);
                lv_obj_set_style_shadow_color(ui->index_cont_clean, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->index_cont_clean, LV_OPA_40, 0);
                _is_index_page_initialized = true;
            }

        }


        break;
    }
    default:
        break;
    }
}

void events_init_index(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->index, index_event_handler, LV_EVENT_ALL, ui);
}

static void setting_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            if (!_is_setting_page_initialized)
            {
                lv_obj_t *btn_return = imgbtn_create(ui->setting, &_return_RGB565A8_100x100, "", 615, 274, 170,
                                                     187);
                lv_obj_add_event_cb(btn_return, setting_exit_to_entry, LV_EVENT_ALL, ui);

                lv_obj_t *btn_mode = imgbtn_create_without_bg(ui->setting_cont_1, &_mode_RGB565A8_100x100,
                                     "模式设置", 44, 91, 144, 144);
                lv_obj_add_event_cb(btn_mode, setting_to_conf_mode, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_timer = imgbtn_create_without_bg(ui->setting_cont_1, &_timer_RGB565A8_100x100,
                                      "声光时钟", 229, 91, 144, 144);
                lv_obj_add_event_cb(btn_timer, setting_to_conf_other, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_wifi = imgbtn_create_without_bg(ui->setting_cont_1, &_wifi_RGB565A8_100x100,
                                     "wifi设置", 421, 91, 144, 144);
                lv_obj_add_event_cb(btn_wifi, setting_to_conf_wifi, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_location = imgbtn_create_without_bg(ui->setting_cont_1, &_location_RGB565A8_100x100,
                                         "定位设置", 44, 251, 144, 144);
                lv_obj_add_event_cb(btn_location, setting_to_conf_location, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_advance = imgbtn_create_without_bg(ui->setting_cont_1, &_setting_RGB565A8_100x100,
                                        "高级设置", 229, 251, 144, 144);
                lv_obj_add_event_cb(btn_advance, setting_to_conf_advance, LV_EVENT_ALL, NULL);

                lv_obj_set_style_shadow_width(ui->setting_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->setting_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->setting_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->setting_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->setting_cont_1, LV_OPA_40, 0);
                _is_setting_page_initialized = true;
            }

        }
        break;
    }
    default:
        break;
    }
}

void events_init_setting(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->setting, setting_event_handler, LV_EVENT_ALL, ui);
}

// 整个prepare页面初始化
static void preparing_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            preparing_page_init(ui);
            if (!_is_preparing_page_initialized)
            {
                lv_obj_set_style_shadow_width(ui->preparing_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->preparing_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->preparing_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->preparing_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->preparing_cont_1, LV_OPA_40, 0);

                lv_obj_set_style_shadow_width(ui->preparing_cont_2, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->preparing_cont_2, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->preparing_cont_2, 3, 0);
                lv_obj_set_style_shadow_color(ui->preparing_cont_2, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->preparing_cont_2, LV_OPA_40, 0);

                lv_obj_set_style_shadow_width(ui->preparing_btn_yes, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->preparing_btn_yes, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->preparing_btn_yes, 3, 0);
                lv_obj_set_style_shadow_color(ui->preparing_btn_yes, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->preparing_btn_yes, LV_OPA_40, 0);

                lv_obj_set_style_shadow_width(ui->preparing_btn_no, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->preparing_btn_no, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->preparing_btn_no, 3, 0);
                lv_obj_set_style_shadow_color(ui->preparing_btn_no, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->preparing_btn_no, LV_OPA_40, 0);

                g_preparing_stop_button = imgbtn_create(ui->preparing_cont_3,
                                                        &_stop_RGB565A8_100x100,
                                                        "停止",
                                                        0, 215, 170, 187);
                lv_obj_add_event_cb(g_preparing_stop_button, stop_toast, LV_EVENT_ALL, ui);
                _is_preparing_page_initialized = true;
            }
            preparing_apply_stop_button_layout();
        }
        lv_label_set_text(guider_ui.preparing_preparing_tips, "");
        break;
    }
    default:
        break;
    }
}

static void preparing_btn_no_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_obj_add_flag(guider_ui.preparing_dialog1, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    default:
        break;
    }
}

// 处理准备页面的确认按钮点击事件
static void preparing_btn_yes_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        // 隐藏弹窗
        lv_obj_add_flag(guider_ui.preparing_dialog1, LV_OBJ_FLAG_HIDDEN);
        if (to_preparing_flat == 1)
        {
            system_auto_water_stop();
        }
        else if (to_preparing_flat == 2)
        {
            system_self_clean_stop();
        }
        break;
    }
    default:
        break;
    }
}

void events_init_preparing(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->preparing, preparing_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->preparing_btn_no, preparing_btn_no_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->preparing_btn_yes, preparing_btn_yes_event_handler, LV_EVENT_ALL, ui);
}

static void location_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            // custom_ui_init(ui);
            if (!_is_location_page_initialized)
            {
                location_page_init(ui);
                lv_obj_t *btn_play = imgbtn_create(ui->location, &_play_RGB565A8_70x70, "", 15, 239, 178, 222);
                lv_obj_add_event_cb(btn_play, location_to_preparing, LV_EVENT_ALL, ui);

                lv_obj_t *btn_return = imgbtn_create(ui->location, &_return_RGB565A8_70x70, "", 218, 239, 178, 222);
                lv_obj_add_event_cb(btn_return, location_back, LV_EVENT_ALL, ui);


                lv_obj_set_style_shadow_width(ui->location_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->location_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->location_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->location_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->location_cont_1, LV_OPA_40, 0);
                _is_location_page_initialized = true;
            }


        }


        break;
    }
    default:
        break;
    }
}

void events_init_location(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->location, location_event_handler, LV_EVENT_ALL, ui);
}

static void working_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_LOAD_START)
    {
        printf("[LINK_DIAG] working SCREEN_LOAD_START\n");
        fflush(stdout);
    }
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);

        printf("[LINK_DIAG] working SCREEN_LOADED enter ui=%p\n", (void *)ui);
        fflush(stdout);
        if (ui != NULL)
        {
            // custom_ui_init(ui);
            system_manager_working_page_init(ui);
        }

        printf("[LINK_DIAG] working SCREEN_LOADED leave\n");
        fflush(stdout);


        break;
    }
    case LV_EVENT_SCREEN_UNLOAD_START:
    case LV_EVENT_DELETE:
        system_manager_working_page_cleanup();
        break;
    default:
        break;
    }
}

static void working_btn_no_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_obj_add_flag(guider_ui.working_dialog, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    default:
        break;
    }
}

static void working_btn_yes_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        working_to_preparing_back(e);
        break;
    }
    default:
        break;
    }
}

void events_init_working(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->working, working_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->working_btn_no, working_btn_no_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->working_btn_yes, working_btn_yes_event_handler, LV_EVENT_ALL, ui);
}

static void conf_advance_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            lv_obj_set_style_shadow_width(ui->conf_advance_cont_3, 8, 0);
            lv_obj_set_style_shadow_ofs_x(ui->conf_advance_cont_3, 3, 0);
            lv_obj_set_style_shadow_ofs_y(ui->conf_advance_cont_3, 3, 0);
            lv_obj_set_style_shadow_color(ui->conf_advance_cont_3, lv_color_hex(0x000000), 0);
            lv_obj_set_style_shadow_opa(ui->conf_advance_cont_3, LV_OPA_40, 0);

            ensure_page_navigation_buttons(&g_conf_advance_navigation, ui->conf_advance,
                                           setting_exit_to_entry, conf_advance_to_setting, ui);
        }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_advance(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_advance, conf_advance_event_handler, LV_EVENT_ALL, ui);
}

static void conf_mode_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            conf_mode_init_listall(ui);
            if (!_is_conf_mode_page_initialized)
            {
                lv_obj_t *btn_home = imgbtn_create(ui->conf_mode, &_home_RGB565A8_100x100, "", 615, 62, 170, 187);
                lv_obj_t *btn_return = imgbtn_create(ui->conf_mode, &_return_RGB565A8_100x100, "", 615, 274, 170,
                                                     187);
                lv_obj_add_event_cb(btn_home, setting_exit_to_entry, LV_EVENT_ALL, ui);
                lv_obj_add_event_cb(btn_return, conf_mode_to_setting, LV_EVENT_ALL, ui);



                lv_obj_set_style_shadow_width(ui->conf_mode_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->conf_mode_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->conf_mode_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->conf_mode_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->conf_mode_cont_1, LV_OPA_40, 0);
                _is_conf_mode_page_initialized = true;
            }

        }

        break;
    }
    default:
        break;
    }
}

void events_init_conf_mode(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_mode, conf_mode_event_handler, LV_EVENT_ALL, ui);
}

static void conf_mode_detail_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            conf_mode_detail_init(ui);
            if (!_is_conf_mode_detail_page_initialized)
            {

                lv_obj_t *btn_home = imgbtn_create(ui->conf_mode_detail, &_home_RGB565A8_100x100, "主页", 615, 62,
                                                   170, 187);
                lv_obj_t *btn_return = imgbtn_create(ui->conf_mode_detail, &_return_RGB565A8_100x100, "返回", 615,
                                                     274, 170, 187);
                lv_obj_add_event_cb(btn_home, setting_exit_to_entry, LV_EVENT_ALL, ui);
                lv_obj_add_event_cb(btn_return, conf_mode_detail_to_conf_mode, LV_EVENT_ALL, ui);
                _is_conf_mode_detail_page_initialized = true;
            }


        }
        break;
    }
    case LV_EVENT_SCREEN_UNLOAD_START:
    {
        mode_option_dialog_close();
        keyboard_manager_hide();
        break;
    }
    default:
        break;
    }
}

static void conf_mode_detail_btn_resume_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        conf_mode_detail_restore_default_clicked(e);
        break;
    }
    default:
        break;
    }
}

static void conf_mode_detail_btn_save_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        conf_mode_detail_save_clicked(e);
        break;
    }
    default:
        break;
    }
}

static void conf_mode_detail_combo_event_handler(lv_event_t *e)
{
    lv_ui *ui;
    lv_obj_t *dropdown;
    const char *title_text = NULL;
    const char *options = NULL;
    uint16_t option_count = 0;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    ui = (lv_ui *)lv_event_get_user_data(e);
    dropdown = lv_event_get_target(e);
    if (ui == NULL || dropdown == NULL || !lv_obj_is_valid(dropdown))
    {
        return;
    }

    if (dropdown == ui->conf_mode_detail_combo_temperature)
    {
        title_text = "温度设置";
        options = "35℃\n36℃\n37℃\n38℃\n39℃\n40℃\n41℃\n42℃\n43℃\n44℃\n45℃\n46℃\n47℃\n48℃";
        option_count = 14;
    }
    else if (dropdown == ui->conf_mode_detail_combo_timer)
    {
        title_text = "足浴时间";
        options = "10min\n15min\n20min\n25min\n30min";
        option_count = 5;
    }
    else if (dropdown == ui->conf_mode_detail_combo_water_level)
    {
        title_text = "水位设置";
        options = "1档\n2档\n3档";
        option_count = 3;
    }
    else if (dropdown == ui->conf_mode_detail_combo_medicinal)
    {
        title_text = "药液设置";
        options = "无\n药液1\n药液2\n药液1+药液2";
        option_count = 4;
    }
    else if (dropdown == ui->conf_mode_detail_combo_location)
    {
        if (g_location_count == 0u || lv_obj_has_state(dropdown, LV_STATE_DISABLED))
        {
            return;
        }
        title_text = "定位设置";
        options = lv_dropdown_get_options(dropdown);
        option_count = lv_dropdown_get_option_count(dropdown);
    }
    else
    {
        return;
    }

    lv_dropdown_close(dropdown);
    mode_option_dialog_show(ui, dropdown, title_text, options, option_count);
}

static void conf_mode_detail_btn_delete_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        conf_mode_detail_delete_clicked(e);
        break;
    }
    default:
        break;
    }
}

void events_init_conf_mode_detail(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_mode_detail, conf_mode_detail_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_btn_resume, conf_mode_detail_btn_resume_event_handler,
                        LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_btn_save, conf_mode_detail_btn_save_event_handler,
                        LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_temperature, conf_mode_detail_combo_event_handler,
                        LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_timer, conf_mode_detail_combo_event_handler,
                        LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_water_level, conf_mode_detail_combo_event_handler,
                        LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_medicinal, conf_mode_detail_combo_event_handler,
                        LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_location, conf_mode_detail_combo_event_handler,
                        LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_btn_delete, conf_mode_detail_btn_delete_event_handler,
                        LV_EVENT_ALL, ui);
}

static void conf_wifi_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            ensure_page_navigation_buttons(&g_conf_wifi_navigation, ui->conf_wifi,
                                           setting_exit_to_entry, conf_wifi_to_setting, ui);
            conf_wifi_set_connect_button_running(ui, conf_wifi_connect_is_running());
            conf_wifi_start_ip_timer(ui);
            if (!conf_wifi_connect_is_running())
            {
                conf_wifi_schedule_scan(ui);
            }
        }
        break;
    }
    case LV_EVENT_SCREEN_UNLOAD_START:
    {
        conf_wifi_cancel_scan_timer();
        conf_wifi_stop_ip_timer(lv_event_get_target(e));
        keyboard_manager_hide();
        break;
    }
    case LV_EVENT_DELETE:
    {
        conf_wifi_cancel_scan_timer();
        conf_wifi_stop_ip_timer(lv_event_get_target(e));
        break;
    }
    default:
        break;
    }
}

static void conf_wifi_btn_2_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        char ssid[128] = {0};
        const char *psk;

        if (ui == NULL || ui->conf_wifi_wifi_ssid == NULL || ui->conf_wifi_wifi_pwd == NULL)
        {
            printf("wifi ui control is not ready\n");
            break;
        }

        lv_dropdown_get_selected_str(ui->conf_wifi_wifi_ssid, ssid, sizeof(ssid));
        psk = lv_textarea_get_text(ui->conf_wifi_wifi_pwd);

        if (conf_wifi_connect_start(ui, ssid, psk) != 0)
        {
            printf("start wifi connection failed\n");
        }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_wifi(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_wifi, conf_wifi_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_wifi_btn_2, conf_wifi_btn_2_event_handler, LV_EVENT_ALL, ui);
}

static void conf_other_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            lv_obj_set_style_shadow_width(ui->conf_other_cont_1, 8, 0);
            lv_obj_set_style_shadow_ofs_x(ui->conf_other_cont_1, 3, 0);
            lv_obj_set_style_shadow_ofs_y(ui->conf_other_cont_1, 3, 0);
            lv_obj_set_style_shadow_color(ui->conf_other_cont_1, lv_color_hex(0x000000), 0);
            lv_obj_set_style_shadow_opa(ui->conf_other_cont_1, LV_OPA_40, 0);

            ensure_page_navigation_buttons(&g_conf_other_navigation, ui->conf_other,
                                           setting_exit_to_entry, conf_other_to_setting, ui);
        }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_other(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_other, conf_other_event_handler, LV_EVENT_ALL, ui);
}

static void conf_location_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL)
        {
            if (!_is_conf_location_page_initialized)
            {
                lv_obj_set_style_shadow_width(ui->conf_location_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->conf_location_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->conf_location_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->conf_location_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->conf_location_cont_1, LV_OPA_40, 0);

                lv_obj_t *btn_home = imgbtn_create(ui->conf_location, &_home_RGB565A8_100x100, "", 615, 62, 170,
                                                   187);
                lv_obj_add_event_cb(btn_home, setting_exit_to_entry, LV_EVENT_ALL, ui);

                lv_obj_t *btn_return = imgbtn_create(ui->conf_location, &_return_RGB565A8_100x100, "", 615, 274,
                                                     170, 187);
                lv_obj_add_event_cb(btn_return, conf_location_to_setting, LV_EVENT_ALL, ui);

                conf_location_init(ui);
                _is_conf_location_page_initialized = true;
            }

        }
        break;
    }
    case LV_EVENT_SCREEN_UNLOAD_START:
    {
        keyboard_manager_hide();
        break;
    }
    default:
        break;
    }
}

static void conf_location_btn_save_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        system_location_save();
        break;
    }
    default:
        break;
    }
}

static void conf_location_btn_create_map_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
        /* 新协议不使用 t=80；机器人坐标只接受 App 单站点下发。 */
        printf("站点坐标由 App 下发，本机不再从底盘导入站点列表\n");
        if (ui != NULL)
        {
            conf_location_init(ui);
        }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_location(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_location, conf_location_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_location_btn_save, conf_location_btn_save_event_handler, LV_EVENT_ALL,
                        ui);
    lv_obj_add_event_cb(ui->conf_location_btn_create_map, conf_location_btn_create_map_event_handler,
                        LV_EVENT_ALL, ui);
}
