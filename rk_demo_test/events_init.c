/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
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

static void conf_wifi_refresh_ssid_dropdown(lv_ui *ui)
{
    char options[4096] = {0};
    char ssid[128] = {0};
    size_t used = 0;
    int scan_count;

    if (ui == NULL || ui->conf_wifi_wifi_ssid == NULL) {
        return;
    }

    scan_count = wifi_manager_get_scan_count();
    for (int index = 0; index < scan_count; index++) {
        int written;

        if (wifi_manager_get_scan_ssid(index, ssid, sizeof(ssid)) != 0 || ssid[0] == '\0') {
            continue;
        }

        written = snprintf(options + used,
                           sizeof(options) - used,
                           "%s%s",
                           used == 0 ? "" : "\n",
                           ssid);
        if (written < 0 || (size_t)written >= sizeof(options) - used) {
            break;
        }

        used += (size_t)written;
    }

    lv_dropdown_set_options(ui->conf_wifi_wifi_ssid, options);
    if (used > 0) {
        lv_dropdown_set_selected(ui->conf_wifi_wifi_ssid, 0);
    }
}

static lv_obj_t *g_mode_option_dialog = NULL;
static lv_obj_t *g_mode_option_roller = NULL;
static lv_obj_t *g_mode_option_dropdown = NULL;
static lv_timer_t *g_conf_wifi_scan_timer = NULL;

typedef struct {
    pthread_mutex_t mutex;
    bool running;
    bool finished;
    int result;
    char ssid[128];
    char psk[128];
    lv_ui *ui;
    lv_timer_t *poll_timer;
} conf_wifi_connect_job_t;

static conf_wifi_connect_job_t g_conf_wifi_connect_job = {
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
        !lv_obj_is_valid(ui->conf_wifi_status_label)) {
        return;
    }

    lv_label_set_text(ui->conf_wifi_status_label, text != NULL ? text : "");
    lv_obj_set_style_text_color(ui->conf_wifi_status_label, lv_color_hex(color),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void conf_wifi_set_connect_button_running(lv_ui *ui, bool running)
{
    if (ui == NULL || ui->conf_wifi_btn_2 == NULL ||
        !lv_obj_is_valid(ui->conf_wifi_btn_2)) {
        return;
    }

    if (ui->conf_wifi_btn_2_label != NULL && lv_obj_is_valid(ui->conf_wifi_btn_2_label)) {
        lv_label_set_text(ui->conf_wifi_btn_2_label, "连接");
    }

    if (running) {
        lv_obj_add_state(ui->conf_wifi_btn_2, LV_STATE_DISABLED);
    } else {
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
    if (finished) {
        result = g_conf_wifi_connect_job.result;
        ui = g_conf_wifi_connect_job.ui;
        g_conf_wifi_connect_job.running = false;
        g_conf_wifi_connect_job.finished = false;
        g_conf_wifi_connect_job.poll_timer = NULL;
    }
    pthread_mutex_unlock(&g_conf_wifi_connect_job.mutex);

    if (!finished) {
        return;
    }

    conf_wifi_set_connect_button_running(ui, false);
    if (result == 0) {
        conf_wifi_set_status(ui, "连接成功", 0x4caf50);
    } else {
        conf_wifi_set_status(ui, "连接失败，请重试", 0xff6b6b);
    }
    lv_timer_delete(timer);
}

static int conf_wifi_connect_start(lv_ui *ui, const char *ssid, const char *psk)
{
    pthread_t worker;
    lv_timer_t *poll_timer;
    int create_result;

    if (ui == NULL || ssid == NULL || psk == NULL || ssid[0] == '\0' || psk[0] == '\0') {
        conf_wifi_set_status(ui, "请选择 WiFi 并输入密码", 0xff6b6b);
        return -1;
    }

    if (conf_wifi_connect_is_running()) {
        return 0;
    }

    poll_timer = lv_timer_create(conf_wifi_connect_poll_timer_cb, 100, NULL);
    if (poll_timer == NULL) {
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
    if (create_result != 0) {
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
    if (lv_event_get_target(e) == g_mode_option_dialog) {
        g_mode_option_dialog = NULL;
        g_mode_option_roller = NULL;
        g_mode_option_dropdown = NULL;
    }
}

static void mode_option_dialog_close(void)
{
    if (g_mode_option_dialog != NULL && lv_obj_is_valid(g_mode_option_dialog)) {
        lv_obj_del(g_mode_option_dialog);
    } else {
        g_mode_option_dialog = NULL;
        g_mode_option_roller = NULL;
        g_mode_option_dropdown = NULL;
    }
}

static void mode_option_dialog_cancel_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        mode_option_dialog_close();
    }
}

static void mode_option_dialog_confirm_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    if (g_mode_option_dropdown != NULL &&
        lv_obj_is_valid(g_mode_option_dropdown) &&
        g_mode_option_roller != NULL &&
        lv_obj_is_valid(g_mode_option_roller)) {
        lv_dropdown_set_selected(g_mode_option_dropdown,
                                 lv_roller_get_selected(g_mode_option_roller));
    }

    mode_option_dialog_close();
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
        option_count == 0) {
        return;
    }

    if (g_mode_option_dialog != NULL && lv_obj_is_valid(g_mode_option_dialog)) {
        return;
    }

    selected = lv_dropdown_get_selected(dropdown);
    if (selected >= option_count) {
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
    lv_obj_set_style_bg_color(g_mode_option_dialog, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_obj_set_style_text_font(title, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

    g_mode_option_roller = lv_roller_create(panel);
    lv_roller_set_options(g_mode_option_roller, options, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(g_mode_option_roller, 3);
    lv_roller_set_selected(g_mode_option_roller, selected, LV_ANIM_OFF);
    lv_obj_set_width(g_mode_option_roller, 220);
    lv_obj_align(g_mode_option_roller, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_text_font(g_mode_option_roller, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(g_mode_option_roller, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_mode_option_roller, lv_color_hex(0x8a8a8a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_mode_option_roller, lv_color_hex(0xf5f2f8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_mode_option_roller, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g_mode_option_roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(g_mode_option_roller, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_mode_option_roller, lv_color_hex(0x937dad), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_mode_option_roller, lv_color_hex(0xe7deef), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(g_mode_option_roller, 255, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(g_mode_option_roller, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_SELECTED | LV_STATE_DEFAULT);

    cancel_btn = lv_button_create(panel);
    lv_obj_set_pos(cancel_btn, 35, 260);
    lv_obj_set_size(cancel_btn, 130, 48);
    lv_obj_set_style_radius(cancel_btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xd8d8d8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(cancel_btn, mode_option_dialog_cancel_event_cb, LV_EVENT_CLICKED, NULL);
    cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_set_style_text_font(cancel_label, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_obj_set_style_text_font(confirm_label, &lv_font_SourceHanSansSC_Regular_25, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(confirm_label);
}

static void conf_wifi_cancel_scan_timer(void)
{
    if (g_conf_wifi_scan_timer != NULL) {
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
        lv_screen_active() != ui->conf_wifi) {
        return;
    }

    if (wifi_manager_scan_start() != 0) {
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
#include "robot_tcp.h"

static void index_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            custom_ui_init(ui);
            index_page_init(ui);
            if(!_is_index_page_initialized) {

                lv_obj_t* btn_plus = imgbtn_create(ui->index, &_plus_RGB565A8_50x50, "", 186, 239, 92, 92);
                lv_obj_add_event_cb(btn_plus, plus_clicked, LV_EVENT_ALL, ui);

                lv_obj_t* btn_minus = imgbtn_create(ui->index, &_minus_RGB565A8_50x50, "", 302, 239, 92, 92);
                lv_obj_add_event_cb(btn_minus, minus_clicked, LV_EVENT_ALL, ui);

                lv_obj_t* btn_timer = imgbtn_create(ui->index, &_timer_RGB565A8_50x50, "", 186, 356, 92, 92);
                lv_obj_add_event_cb(btn_timer, timer_clicked, LV_EVENT_ALL, ui);

                lv_obj_t* btn_weter= imgbtn_create(ui->index, &_water_RGB565A8_50x50, "", 302, 356, 92, 92);
                lv_obj_add_event_cb(btn_weter, water_clicked, LV_EVENT_ALL, ui);

                lv_obj_t* btn_play = imgbtn_create(ui->index, &_play_RGB565A8_50x50, "", 15, 239, 146, 211);
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
                _is_index_page_initialized=true;
            }

        }


        break;
    }
    default:
        break;
    }
}

void events_init_index (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->index, index_event_handler, LV_EVENT_ALL, ui);
}

static void setting_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            if(!_is_setting_page_initialized) {
                lv_obj_t* btn_return = imgbtn_create(ui->setting, &_return_RGB565A8_100x100, "", 615, 274, 170, 187);
                lv_obj_add_event_cb(btn_return, setting_back, LV_EVENT_ALL, ui);

                lv_obj_t *btn_mode = imgbtn_create_without_bg(ui->setting_cont_1, &_mode_RGB565A8_100x100,"模式设置", 44, 91, 144, 144);
                lv_obj_add_event_cb(btn_mode, setting_to_conf_mode, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_timer = imgbtn_create_without_bg(ui->setting_cont_1, &_timer_RGB565A8_100x100,"声光时钟", 229, 91, 144, 144);
                lv_obj_add_event_cb(btn_timer, setting_to_conf_other, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_wifi = imgbtn_create_without_bg(ui->setting_cont_1, &_wifi_RGB565A8_100x100,"wifi设置", 421, 91, 144, 144);
                lv_obj_add_event_cb(btn_wifi, setting_to_conf_wifi, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_location = imgbtn_create_without_bg(ui->setting_cont_1, &_location_RGB565A8_100x100,"定位设置", 44, 251, 144, 144);
                lv_obj_add_event_cb(btn_location, setting_to_conf_location, LV_EVENT_ALL, NULL);

                lv_obj_t *btn_advance = imgbtn_create_without_bg(ui->setting_cont_1, &_setting_RGB565A8_100x100,"高级设置", 229, 251, 144, 144);
                lv_obj_add_event_cb(btn_advance, setting_to_conf_advance, LV_EVENT_ALL, NULL);

                lv_obj_set_style_shadow_width(ui->setting_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->setting_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->setting_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->setting_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->setting_cont_1, LV_OPA_40, 0);
                _is_setting_page_initialized=true;
            }

        }
        break;
    }
    default:
        break;
    }
}

static void setting_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.conf_mode, guider_ui.conf_mode_del, &guider_ui.setting_del, setup_scr_conf_mode, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

static void setting_btn_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.conf_other, guider_ui.conf_other_del, &guider_ui.setting_del, setup_scr_conf_other, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void setting_btn_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.conf_wifi, guider_ui.conf_wifi_del, &guider_ui.setting_del, setup_scr_conf_wifi, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

static void setting_btn_5_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.conf_location, guider_ui.conf_location_del, &guider_ui.setting_del, setup_scr_conf_location, LV_SCR_LOAD_ANIM_NONE, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void setting_btn_6_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.conf_advance, guider_ui.conf_advance_del, &guider_ui.setting_del, setup_scr_conf_advance, LV_SCR_LOAD_ANIM_FADE_ON, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

void events_init_setting (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->setting, setting_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->setting_btn_2, setting_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->setting_btn_3, setting_btn_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->setting_btn_4, setting_btn_4_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->setting_btn_5, setting_btn_5_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->setting_btn_6, setting_btn_6_event_handler, LV_EVENT_ALL, ui);
}

static void preparing_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            preparing_page_init(ui);
            if(!_is_preparing_page_initialized) {
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

                lv_obj_t* btn_stop = imgbtn_create(ui->preparing_cont_3, &_stop_RGB565A8_100x100, "停止", 0, 215, 170, 187);
                lv_obj_add_event_cb(btn_stop, stop_toast, LV_EVENT_ALL, ui);
                _is_preparing_page_initialized=true;
            }
        }
        lv_label_set_text(guider_ui.preparing_preparing_tips, "");
        break;
    }
    default:
        break;
    }
}

static void preparing_btn_no_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_obj_add_flag(guider_ui.preparing_dialog1, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    default:
        break;
    }
}

static void preparing_btn_yes_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        preparing_page_stop(e);
        lv_obj_add_flag(guider_ui.preparing_dialog1, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    default:
        break;
    }
}

void events_init_preparing (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->preparing, preparing_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->preparing_btn_no, preparing_btn_no_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->preparing_btn_yes, preparing_btn_yes_event_handler, LV_EVENT_ALL, ui);
}

static void location_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            custom_ui_init(ui);
            if(!_is_location_page_initialized) {
                location_page_init(ui);
                lv_obj_t* btn_play = imgbtn_create(ui->location, &_play_RGB565A8_70x70, "", 15, 239, 178, 222);
                lv_obj_add_event_cb(btn_play, location_to_preparing, LV_EVENT_ALL, ui);

                lv_obj_t* btn_return= imgbtn_create(ui->location, &_return_RGB565A8_70x70, "", 218, 239, 178, 222);
                lv_obj_add_event_cb(btn_return, location_back, LV_EVENT_ALL, ui);


                lv_obj_set_style_shadow_width(ui->location_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->location_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->location_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->location_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->location_cont_1, LV_OPA_40, 0);
                _is_location_page_initialized=true;
            }


        }


        break;
    }
    default:
        break;
    }
}

void events_init_location (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->location, location_event_handler, LV_EVENT_ALL, ui);
}

static void working_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            custom_ui_init(ui);
            working_page_init(ui);
        }


        break;
    }
    default:
        break;
    }
}

static void working_btn_no_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_obj_add_flag(guider_ui.working_dialog, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    default:
        break;
    }
}

static void working_btn_yes_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        working_to_preparing_back(e);
        break;
    }
    default:
        break;
    }
}

void events_init_working (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->working, working_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->working_btn_no, working_btn_no_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->working_btn_yes, working_btn_yes_event_handler, LV_EVENT_ALL, ui);
}

static void conf_advance_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            lv_obj_set_style_shadow_width(ui->conf_advance_cont_3, 8, 0);
            lv_obj_set_style_shadow_ofs_x(ui->conf_advance_cont_3, 3, 0);
            lv_obj_set_style_shadow_ofs_y(ui->conf_advance_cont_3, 3, 0);
            lv_obj_set_style_shadow_color(ui->conf_advance_cont_3, lv_color_hex(0x000000), 0);
            lv_obj_set_style_shadow_opa(ui->conf_advance_cont_3, LV_OPA_40, 0);

            lv_obj_t* btn_home = imgbtn_create(ui->conf_advance, &_home_RGB565A8_100x100, "", 615, 62, 170, 187);
            lv_obj_add_event_cb(btn_home, conf_advance_to_index, LV_EVENT_ALL, ui);

            lv_obj_t* btn_return = imgbtn_create(ui->conf_advance, &_return_RGB565A8_100x100, "", 615, 274, 170, 187);
            lv_obj_add_event_cb(btn_return, conf_advance_to_setting, LV_EVENT_ALL, ui);

        }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_advance (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_advance, conf_advance_event_handler, LV_EVENT_ALL, ui);
}

static void conf_mode_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            conf_mode_init_listall(ui);
            if(!_is_conf_mode_page_initialized) {
                lv_obj_t* btn_home = imgbtn_create(ui->conf_mode, &_home_RGB565A8_100x100, "", 615, 62, 170, 187);
                lv_obj_t* btn_return = imgbtn_create(ui->conf_mode, &_return_RGB565A8_100x100, "", 615, 274, 170, 187);
                lv_obj_add_event_cb(btn_home, conf_mode_to_index, LV_EVENT_ALL, ui);
                lv_obj_add_event_cb(btn_return, conf_mode_to_setting, LV_EVENT_ALL, ui);



                lv_obj_set_style_shadow_width(ui->conf_mode_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->conf_mode_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->conf_mode_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->conf_mode_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->conf_mode_cont_1, LV_OPA_40, 0);
                _is_conf_mode_page_initialized=true;
            }

        }

        break;
    }
    default:
        break;
    }
}

void events_init_conf_mode (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_mode, conf_mode_event_handler, LV_EVENT_ALL, ui);
}

static void conf_mode_detail_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            conf_mode_detail_init(ui);
            if(!_is_conf_mode_detail_page_initialized) {

                lv_obj_t* btn_home = imgbtn_create(ui->conf_mode_detail, &_home_RGB565A8_100x100, "主页", 615, 62, 170, 187);
                lv_obj_t* btn_return = imgbtn_create(ui->conf_mode_detail, &_return_RGB565A8_100x100, "返回", 615, 274, 170, 187);
                lv_obj_add_event_cb(btn_home, conf_mode_detail_to_index, LV_EVENT_ALL, ui);
                lv_obj_add_event_cb(btn_return, conf_mode_detail_to_conf_mode, LV_EVENT_ALL, ui);
                _is_conf_mode_detail_page_initialized=true;
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

static void conf_mode_detail_btn_resume_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        conf_mode_detail_restore_default_clicked(e);
        break;
    }
    default:
        break;
    }
}

static void conf_mode_detail_btn_save_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        conf_mode_detail_save_clicked(e);
        break;
    }
    default:
        break;
    }
}

static void conf_mode_detail_combo_event_handler (lv_event_t *e)
{
    lv_ui *ui;
    lv_obj_t *dropdown;
    const char *title_text = NULL;
    const char *options = NULL;
    uint16_t option_count = 0;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    ui = (lv_ui *)lv_event_get_user_data(e);
    dropdown = lv_event_get_target(e);
    if (ui == NULL || dropdown == NULL || !lv_obj_is_valid(dropdown)) {
        return;
    }

    if (dropdown == ui->conf_mode_detail_combo_temperature) {
        title_text = "温度设置";
        options = "35℃\n36℃\n37℃\n38℃\n39℃\n40℃\n41℃\n42℃\n43℃\n44℃\n45℃\n46℃\n47℃\n48℃";
        option_count = 14;
    } else if (dropdown == ui->conf_mode_detail_combo_timer) {
        title_text = "足浴时间";
        options = "10min\n15min\n20min\n25min\n30min";
        option_count = 5;
    } else if (dropdown == ui->conf_mode_detail_combo_water_level) {
        title_text = "水位设置";
        options = "1档\n2档\n3档";
        option_count = 3;
    } else if (dropdown == ui->conf_mode_detail_combo_medicinal) {
        title_text = "药液设置";
        options = "无\n药液1\n药液2\n药液1+药液2";
        option_count = 4;
    } else if (dropdown == ui->conf_mode_detail_combo_location) {
        title_text = "定位设置";
        options = "客厅\n书房\n主卧\n小孩房";
        option_count = 4;
    } else {
        return;
    }

    lv_dropdown_close(dropdown);
    mode_option_dialog_show(ui, dropdown, title_text, options, option_count);
}

static void conf_mode_detail_btn_delete_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        conf_mode_detail_delete_clicked(e);
        break;
    }
    default:
        break;
    }
}

void events_init_conf_mode_detail (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_mode_detail, conf_mode_detail_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_btn_resume, conf_mode_detail_btn_resume_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_btn_save, conf_mode_detail_btn_save_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_temperature, conf_mode_detail_combo_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_timer, conf_mode_detail_combo_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_water_level, conf_mode_detail_combo_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_medicinal, conf_mode_detail_combo_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_combo_location, conf_mode_detail_combo_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_mode_detail_btn_delete, conf_mode_detail_btn_delete_event_handler, LV_EVENT_ALL, ui);
}

static void conf_wifi_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            lv_obj_t* btn_home = imgbtn_create(ui->conf_wifi, &_home_RGB565A8_100x100, "", 615, 62, 170, 187);
            lv_obj_t* btn_return = imgbtn_create(ui->conf_wifi, &_return_RGB565A8_100x100, "", 615, 274, 170, 187);
            lv_obj_add_event_cb(btn_home, conf_wifi_to_index, LV_EVENT_ALL, ui);
            lv_obj_add_event_cb(btn_return, conf_wifi_to_setting, LV_EVENT_ALL, ui);
            conf_wifi_set_connect_button_running(ui, conf_wifi_connect_is_running());
            if (!conf_wifi_connect_is_running()) {
                conf_wifi_schedule_scan(ui);
            }
        }
        break;
    }
    case LV_EVENT_SCREEN_UNLOAD_START:
    {
        conf_wifi_cancel_scan_timer();
        keyboard_manager_hide();
        break;
    }
    default:
        break;
    }
}

static void conf_wifi_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        char ssid[128] = {0};
        const char *psk;

        if (ui == NULL || ui->conf_wifi_wifi_ssid == NULL || ui->conf_wifi_wifi_pwd == NULL) {
            printf("wifi ui control is not ready\n");
            break;
        }

        lv_dropdown_get_selected_str(ui->conf_wifi_wifi_ssid, ssid, sizeof(ssid));
        psk = lv_textarea_get_text(ui->conf_wifi_wifi_pwd);

        if (conf_wifi_connect_start(ui, ssid, psk) != 0) {
            printf("start wifi connection failed\n");
        }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_wifi (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_wifi, conf_wifi_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_wifi_btn_2, conf_wifi_btn_2_event_handler, LV_EVENT_ALL, ui);
}

static void conf_other_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            lv_obj_set_style_shadow_width(ui->conf_other_cont_1, 8, 0);
            lv_obj_set_style_shadow_ofs_x(ui->conf_other_cont_1, 3, 0);
            lv_obj_set_style_shadow_ofs_y(ui->conf_other_cont_1, 3, 0);
            lv_obj_set_style_shadow_color(ui->conf_other_cont_1, lv_color_hex(0x000000), 0);
            lv_obj_set_style_shadow_opa(ui->conf_other_cont_1, LV_OPA_40, 0);

            lv_obj_t* btn_home = imgbtn_create(ui->conf_other, &_home_RGB565A8_100x100, "", 615, 62, 170, 187);
            lv_obj_add_event_cb(btn_home, conf_other_to_index, LV_EVENT_ALL, ui);

            lv_obj_t* btn_return = imgbtn_create(ui->conf_other, &_return_RGB565A8_100x100, "", 615, 274, 170, 187);
            lv_obj_add_event_cb(btn_return, conf_other_to_setting, LV_EVENT_ALL, ui);

        }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_other (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_other, conf_other_event_handler, LV_EVENT_ALL, ui);
}

static void conf_location_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        lv_ui *ui = lv_event_get_user_data(e);
        if (ui != NULL) {
            if(!_is_conf_location_page_initialized) {
                lv_obj_set_style_shadow_width(ui->conf_location_cont_1, 8, 0);
                lv_obj_set_style_shadow_ofs_x(ui->conf_location_cont_1, 3, 0);
                lv_obj_set_style_shadow_ofs_y(ui->conf_location_cont_1, 3, 0);
                lv_obj_set_style_shadow_color(ui->conf_location_cont_1, lv_color_hex(0x000000), 0);
                lv_obj_set_style_shadow_opa(ui->conf_location_cont_1, LV_OPA_40, 0);

                lv_obj_t* btn_home = imgbtn_create(ui->conf_location, &_home_RGB565A8_100x100, "", 615, 62, 170, 187);
                lv_obj_add_event_cb(btn_home, conf_location_to_index, LV_EVENT_ALL, ui);

                lv_obj_t* btn_return = imgbtn_create(ui->conf_location, &_return_RGB565A8_100x100, "", 615, 274, 170, 187);
                lv_obj_add_event_cb(btn_return, conf_location_to_setting, LV_EVENT_ALL, ui);

                conf_location_init(ui);
                _is_conf_location_page_initialized=true;
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

static void conf_location_btn_save_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        system_location_save();
        break;
    }
    default:
        break;
    }
}

static void conf_location_btn_create_map_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
        MapStations_t out_stations = {0};

        printf("Requesting robot station list...\n");
        int res = RobotTcp_GetStationList(&out_stations);

        if (res == 0 && out_stations.stations != NULL && out_stations.station_count > 0) {
            printf("Map: %s\n", out_stations.map_name);
            printf("Station count: %d\n", out_stations.station_count);

            // Clear current location list, repopulate from robot station data
            memset(g_location_settings, 0, sizeof(g_location_settings));
            g_location_count = 0;

            int count = out_stations.station_count;
            if (count > SYSTEM_LOCATION_MAX_COUNT) {
                count = SYSTEM_LOCATION_MAX_COUNT;
            }

            for (int i = 0; i < count; i++) {
                StationInfo_t *station = &(out_stations.stations[i]);
                system_location_t *location = &g_location_settings[i];

                snprintf(location->name, sizeof(location->name), "%s", station->name);
                location->index = station->id;
                location->default_mode_index = 0;
                location->coord.x = (lv_coord_t)(station->x * 10.0f);
                location->coord.y = (lv_coord_t)(station->y * 10.0f);

                printf("[%d] id=%d, name='%s', pos=(%.2f,%.2f,%.2f), type=%d, enable=%d\n",
                       i, station->id, station->name,
                       station->x, station->y, station->z,
                       station->type, station->is_enable);

                g_location_count++;
            }

            RobotTcp_FreeStationList(&out_stations);

            // Persist new location list
            system_location_save();

            // Refresh conf_location page UI
            if (ui != NULL) {
                conf_location_init(ui);
            }
        }
        // else {
        //     printf("Failed to get station list (res=%d)\n", res);

        //     // Show error dialog
        //     if (ui != NULL) {
        //         lv_obj_t *msgbox = lv_msgbox_create(NULL);
        //         lv_msgbox_add_title(msgbox, "error");
        //         lv_msgbox_add_text(msgbox, "Failed to get station list, please check network connection");
        //         lv_obj_t *btn_ok = lv_msgbox_add_footer_button(msgbox, "OK");
        //         lv_obj_add_event_cb(btn_ok, [](lv_event_t *ev) {
        //             lv_obj_t *target = lv_event_get_target(ev);
        //             lv_msgbox_close(lv_obj_get_parent(target));
        //         }, LV_EVENT_CLICKED, NULL);
        //         lv_obj_center(msgbox);
        //     }
        // }
        break;
    }
    default:
        break;
    }
}

void events_init_conf_location (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->conf_location, conf_location_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_location_btn_save, conf_location_btn_save_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->conf_location_btn_create_map, conf_location_btn_create_map_event_handler, LV_EVENT_ALL, ui);
}
