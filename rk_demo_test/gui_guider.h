/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *index;
	bool index_del;
	lv_obj_t *index_img_1;
	lv_obj_t *index_panel_status;
	lv_obj_t *index_panel_status_label;
	lv_obj_t *index_cont_1;
	lv_obj_t *index_cont_2;
	lv_obj_t *index_water_info;
	lv_obj_t *index_time_info;
	lv_obj_t *index_temp_value;
	lv_obj_t *index_img_water1;
	lv_obj_t *index_img_water2;
	lv_obj_t *index_img_water3;
	lv_obj_t *index_img_medicine1;
	lv_obj_t *index_img_medicine2;
	lv_obj_t *index_img_need_liquid;
	lv_obj_t *index_label_need_liquid;
	lv_obj_t *index_cont_clean;
	lv_obj_t *index_img_2;
	lv_obj_t *index_label_1;
	lv_obj_t *setting;
	bool setting_del;
	lv_obj_t *setting_img_1;
	lv_obj_t *setting_btn_2;
	lv_obj_t *setting_btn_2_label;
	lv_obj_t *setting_btn_3;
	lv_obj_t *setting_btn_3_label;
	lv_obj_t *setting_btn_4;
	lv_obj_t *setting_btn_4_label;
	lv_obj_t *setting_btn_5;
	lv_obj_t *setting_btn_5_label;
	lv_obj_t *setting_btn_6;
	lv_obj_t *setting_btn_6_label;
	lv_obj_t *setting_btn_7;
	lv_obj_t *setting_btn_7_label;
	lv_obj_t *setting_cont_1;
	lv_obj_t *setting_label_1;
	lv_obj_t *setting_cont_2;
	lv_obj_t *setting_line_1;
	lv_obj_t *preparing;
	bool preparing_del;
	lv_obj_t *preparing_img_1;
	lv_obj_t *preparing_cont_1;
	lv_obj_t *preparing_cont_cleaning;
	lv_obj_t *preparing_label_clean_time_left;
	lv_obj_t *preparing_cleaning_tips;
	lv_obj_t *preparing_label_9;
	lv_obj_t *preparing_label_10;
	lv_obj_t *preparing_img_3;
	lv_obj_t *preparing_label_12;
	lv_obj_t *preparing_cont_move_finished;
	lv_obj_t *preparing_label_17;
	lv_obj_t *preparing_label_timer_tips;
	lv_obj_t *preparing_cont_moving;
	lv_obj_t *preparing_img_2;
	lv_obj_t *preparing_label_11;
	lv_obj_t *preparing_cont_preparing;
	lv_obj_t *preparing_label_4;
	lv_obj_t *preparing_label_5;
	lv_obj_t *preparing_label_temprature;
	lv_obj_t *preparing_label_3;
	lv_obj_t *preparing_label_13;
	lv_obj_t *preparing_img_4;
	lv_obj_t *preparing_label_timeleft;
	lv_obj_t *preparing_label_15;
	lv_obj_t *preparing_img_water3;
	lv_obj_t *preparing_img_water2;
	lv_obj_t *preparing_img_water1;
	lv_obj_t *preparing_preparing_tips;
	lv_obj_t *preparing_img_medicine2;
	lv_obj_t *preparing_img_medicine1;
	lv_obj_t *preparing_cont_3;
	lv_obj_t *preparing_dialog1;
	lv_obj_t *preparing_img_5;
	lv_obj_t *preparing_cont_2;
	lv_obj_t *preparing_btn_no;
	lv_obj_t *preparing_btn_no_label;
	lv_obj_t *preparing_btn_yes;
	lv_obj_t *preparing_btn_yes_label;
	lv_obj_t *preparing_label_6;
	lv_obj_t *location;
	bool location_del;
	lv_obj_t *location_img_1;
	lv_obj_t *location_cont_1;
	lv_obj_t *location_label_moving_tips;
	lv_obj_t *location_img_2;
	lv_obj_t *location_cont_2;
	lv_obj_t *working;
	bool working_del;
	lv_obj_t *working_img_1;
	lv_obj_t *working_cont_playing;
	lv_obj_t *working_label_time_left;
	lv_obj_t *working_label_temperature;
	lv_obj_t *working_img_2;
	lv_obj_t *working_img_massage_intensity0;
	lv_obj_t *working_img_massage_intensity1;
	lv_obj_t *working_img_massage_intensity2;
	lv_obj_t *working_img_constant_temperature;
	lv_obj_t *working_img_sterilization;
	lv_obj_t *working_label_5;
	lv_obj_t *working_label_9;
	lv_obj_t *working_cont_pause;
	lv_obj_t *working_label_6;
	lv_obj_t *working_cont_2;
	lv_obj_t *working_btn_6;
	lv_obj_t *working_btn_6_label;
	lv_obj_t *working_cont_1;
	lv_obj_t *working_dialog;
	lv_obj_t *working_img_3;
	lv_obj_t *working_cont_3;
	lv_obj_t *working_label_8;
	lv_obj_t *working_btn_no;
	lv_obj_t *working_btn_no_label;
	lv_obj_t *working_btn_yes;
	lv_obj_t *working_btn_yes_label;
	lv_obj_t *working_label_7;
	lv_obj_t *conf_advance;
	bool conf_advance_del;
	lv_obj_t *conf_advance_img_1;
	lv_obj_t *conf_advance_cont_3;
	lv_obj_t *conf_advance_label_4;
	lv_obj_t *conf_advance_cont_4;
	lv_obj_t *conf_advance_line_1;
	lv_obj_t *conf_advance_cont_9;
	lv_obj_t *conf_advance_cont_2;
	lv_obj_t *conf_advance_label_3;
	lv_obj_t *conf_advance_label_2;
	lv_obj_t *conf_advance_sw_1;
	lv_obj_t *conf_advance_cont_8;
	lv_obj_t *conf_advance_label_12;
	lv_obj_t *conf_advance_label_11;
	lv_obj_t *conf_advance_sw_5;
	lv_obj_t *conf_advance_cont_7;
	lv_obj_t *conf_advance_label_10;
	lv_obj_t *conf_advance_label_9;
	lv_obj_t *conf_advance_sw_4;
	lv_obj_t *conf_advance_cont_6;
	lv_obj_t *conf_advance_label_8;
	lv_obj_t *conf_advance_label_7;
	lv_obj_t *conf_advance_sw_3;
	lv_obj_t *conf_advance_cont_5;
	lv_obj_t *conf_advance_label_6;
	lv_obj_t *conf_advance_label_5;
	lv_obj_t *conf_advance_sw_2;
	lv_obj_t *conf_mode;
	bool conf_mode_del;
	lv_obj_t *conf_mode_img_1;
	lv_obj_t *conf_mode_cont_1;
	lv_obj_t *conf_mode_label_1;
	lv_obj_t *conf_mode_cont_2;
	lv_obj_t *conf_mode_line_1;
	lv_obj_t *conf_mode_detail;
	bool conf_mode_detail_del;
	lv_obj_t *conf_mode_detail_img_1;
	lv_obj_t *conf_mode_detail_cont_1;
	lv_obj_t *conf_mode_detail_label_1;
	lv_obj_t *conf_mode_detail_cont_2;
	lv_obj_t *conf_mode_detail_combo_water_level;
	lv_obj_t *conf_mode_detail_label_2;
	lv_obj_t *conf_mode_detail_combo_medicinal;
	lv_obj_t *conf_mode_detail_label_3;
	lv_obj_t *conf_mode_detail_label_4;
	lv_obj_t *conf_mode_detail_combo_location;
	lv_obj_t *conf_mode_detail_label_5;
	lv_obj_t *conf_mode_detail_label_6;
	lv_obj_t *conf_mode_detail_label_7;
	lv_obj_t *conf_mode_detail_btn_resume;
	lv_obj_t *conf_mode_detail_btn_resume_label;
	lv_obj_t *conf_mode_detail_btn_save;
	lv_obj_t *conf_mode_detail_btn_save_label;
	lv_obj_t *conf_mode_detail_combo_temperature;
	lv_obj_t *conf_mode_detail_combo_timer;
	lv_obj_t *conf_mode_detail_text_mode_name;
	lv_obj_t *conf_mode_detail_btn_delete;
	lv_obj_t *conf_mode_detail_btn_delete_label;
	lv_obj_t *conf_mode_detail_line_1;
	lv_obj_t *conf_wifi;
	bool conf_wifi_del;
	lv_obj_t *conf_wifi_img_1;
	lv_obj_t *conf_wifi_cont_1;
	lv_obj_t *conf_wifi_cont_2;
	lv_obj_t *conf_wifi_line_1;
	lv_obj_t *conf_wifi_btn_2;
	lv_obj_t *conf_wifi_btn_2_label;
	lv_obj_t *conf_wifi_status_label;
	lv_obj_t *conf_wifi_label_3;
	lv_obj_t *conf_wifi_wifi_ssid;
	lv_obj_t *conf_wifi_wifi_pwd;
	lv_obj_t *conf_wifi_label_4;
	lv_obj_t *conf_wifi_img_2;
	lv_obj_t *conf_wifi_img_3;
	lv_obj_t *conf_other;
	bool conf_other_del;
	lv_obj_t *conf_other_img_1;
	lv_obj_t *conf_other_cont_1;
	lv_obj_t *conf_other_label_1;
	lv_obj_t *conf_other_cont_2;
	lv_obj_t *conf_other_line_1;
	lv_obj_t *conf_other_cont_3;
	lv_obj_t *conf_other_cont_4;
	lv_obj_t *conf_other_label_2;
	lv_obj_t *conf_other_sw_1;
	lv_obj_t *conf_other_cont_5;
	lv_obj_t *conf_other_label_3;
	lv_obj_t *conf_other_sw_2;
	lv_obj_t *conf_other_cont_6;
	lv_obj_t *conf_other_label_4;
	lv_obj_t *conf_other_label_5;
	lv_obj_t *conf_other_imgbtn_1;
	lv_obj_t *conf_other_imgbtn_1_label;
	lv_obj_t *conf_location;
	bool conf_location_del;
	lv_obj_t *conf_location_img_1;
	lv_obj_t *conf_location_cont_1;
	lv_obj_t *conf_location_label_5;
	lv_obj_t *conf_location_cont_6;
	lv_obj_t *conf_location_line_1;
	lv_obj_t *conf_location_cont_2;
	lv_obj_t *conf_location_label_6;
	lv_obj_t *conf_location_ta_1;
	lv_obj_t *conf_location_ddlist_1;
	lv_obj_t *conf_location_label_7;
	lv_obj_t *conf_location_btn_save;
	lv_obj_t *conf_location_btn_save_label;
	lv_obj_t *conf_location_btn_create_map;
	lv_obj_t *conf_location_btn_create_map_label;
	lv_obj_t *screen_img_list;
	bool screen_img_list_del;
	lv_obj_t *screen_img_list_img_1;
	lv_obj_t *screen_img_list_img_2;
	lv_obj_t *screen_img_list_img_3;
	lv_obj_t *screen_img_list_img_5;
	lv_obj_t *screen_img_list_img_6;
	lv_obj_t *screen_img_list_img_7;
	lv_obj_t *screen_img_list_img_8;
	lv_obj_t *screen_img_list_img_9;
	lv_obj_t *screen_img_list_img_10;
	lv_obj_t *screen_img_list_img_11;
	lv_obj_t *screen_img_list_img_12;
	lv_obj_t *screen_img_list_img_13;
	lv_obj_t *screen_img_list_img_14;
	lv_obj_t *screen_img_list_img_15;
	lv_obj_t *screen_img_list_img_16;
	lv_obj_t *screen_img_list_img_17;
	lv_obj_t *screen_img_list_img_18;
	lv_obj_t *screen_img_list_img_19;
	lv_obj_t *screen_img_list_img_20;
	lv_obj_t *screen_img_list_img_21;
	lv_obj_t *screen_img_list_img_22;
	lv_obj_t *screen_img_list_img_23;
	lv_obj_t *screen_img_list_img_24;
	lv_obj_t *screen_img_list_img_25;
	lv_obj_t *screen_img_list_img_26;
	lv_obj_t *screen_img_list_img_27;
	lv_obj_t *screen_img_list_img_28;
	lv_obj_t *screen_img_list_img_29;
	lv_obj_t *screen_img_list_img_30;
	lv_obj_t *screen_img_list_img_31;
	lv_obj_t *screen_img_list_img_33;
	lv_obj_t *screen_img_list_img_37;
	lv_obj_t *screen_img_list_img_38;
	lv_obj_t *screen_img_list_img_39;
	lv_obj_t *screen_img_list_img_40;
	lv_obj_t *screen_img_list_img_41;
	lv_obj_t *screen_img_list_img_42;
	lv_obj_t *screen_img_list_img_43;
	lv_obj_t *screen_img_list_img_44;
	lv_obj_t *screen_img_list_img_45;
	lv_obj_t *screen_img_list_img_46;
	lv_obj_t *screen_img_list_img_47;
	lv_obj_t *screen_img_list_img_48;
	lv_obj_t *screen_img_list_img_49;
	lv_obj_t *screen_img_list_img_50;
	lv_obj_t *g_kb_top_layer;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_index(lv_ui *ui);
void setup_scr_setting(lv_ui *ui);
void setup_scr_preparing(lv_ui *ui);
void setup_scr_location(lv_ui *ui);
void setup_scr_working(lv_ui *ui);
void setup_scr_conf_advance(lv_ui *ui);
void setup_scr_conf_mode(lv_ui *ui);
void setup_scr_conf_mode_detail(lv_ui *ui);
void setup_scr_conf_wifi(lv_ui *ui);
void setup_scr_conf_other(lv_ui *ui);
void setup_scr_conf_location(lv_ui *ui);
void setup_scr_screen_img_list(lv_ui *ui);
LV_IMAGE_DECLARE(_bg_RGB565A8_800x480);
LV_IMAGE_DECLARE(_water_RGB565A8_30x30);
LV_IMAGE_DECLARE(_medicine1_RGB565A8_30x30);
LV_IMAGE_DECLARE(_medicine2_RGB565A8_30x30);
LV_IMAGE_DECLARE(_need_liquid_RGB565A8_30x30);
LV_IMAGE_DECLARE(_clean_RGB565A8_100x100);
LV_IMAGE_DECLARE(_clean_RGB565A8_57x57);
LV_IMAGE_DECLARE(_warnning_RGB565A8_100x100);
LV_IMAGE_DECLARE(_water_RGB565A8_57x57);
LV_IMAGE_DECLARE(_local_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_foot3_RGB565A8_34x34);
LV_IMAGE_DECLARE(_minus_RGB565A8_34x34);
LV_IMAGE_DECLARE(_worm_RGB565A8_32x27);
LV_IMAGE_DECLARE(_bacteria_RGB565A8_34x34);
LV_IMAGE_DECLARE(_wifi_holder_RGB565A8_30x30);
LV_IMAGE_DECLARE(_pwd_holder_RGB565A8_30x30);
LV_IMAGE_DECLARE(_timer_refresh_RGB565A8_30x30);
LV_IMAGE_DECLARE(_home_RGB565A8_100x100);
LV_IMAGE_DECLARE(_return_RGB565A8_100x100);
LV_IMAGE_DECLARE(_add_one_RGB565A8_100x100);
LV_IMAGE_DECLARE(_medicine1_RGB565A8_100x100);
LV_IMAGE_DECLARE(_massage_RGB565A8_100x100);
LV_IMAGE_DECLARE(_location_RGB565A8_100x100);
LV_IMAGE_DECLARE(_local1_RGB565A8_100x100);
LV_IMAGE_DECLARE(_bacteria_RGB565A8_100x100);
LV_IMAGE_DECLARE(_auto_back_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_foot2_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_foot1_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_RGB565A8_100x100);
LV_IMAGE_DECLARE(_minus_RGB565A8_50x50);
LV_IMAGE_DECLARE(_medicine2_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_relax1_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_foot4_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_relax2_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_foot3_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_sleep3_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_sleep2_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_sleep1_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_relax4_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_relax3_RGB565A8_100x100);
LV_IMAGE_DECLARE(_setting_RGB565A8_100x100);
LV_IMAGE_DECLARE(_plus_RGB565A8_50x50);
LV_IMAGE_DECLARE(_play_RGB565A8_100x100);
LV_IMAGE_DECLARE(_need_liquid_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_worm4_RGB565A8_100x100);
LV_IMAGE_DECLARE(_mode_worm3_RGB565A8_100x100);
LV_IMAGE_DECLARE(_wifi_RGB565A8_100x100);
LV_IMAGE_DECLARE(_water_RGB565A8_100x100);
LV_IMAGE_DECLARE(_timer_RGB565A8_50x50);
LV_IMAGE_DECLARE(_stop_RGB565A8_100x100);
LV_IMAGE_DECLARE(_timer_RGB565A8_100x100);
LV_IMAGE_DECLARE(_water_RGB565A8_50x50);
LV_IMAGE_DECLARE(_play_RGB565A8_50x50);
LV_IMAGE_DECLARE(_return_RGB565A8_50x50);
LV_IMAGE_DECLARE(_stop_RGB565A8_50x50);
LV_IMAGE_DECLARE(_btn_worm_RGB565A8_100x100);
LV_IMAGE_DECLARE(_stop_RGB565A8_70x70);
LV_IMAGE_DECLARE(_play_RGB565A8_70x70);
LV_IMAGE_DECLARE(_return_RGB565A8_70x70);

LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_20)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_70)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_25)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_40)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_30)
LV_FONT_DECLARE(lv_font_montserratMedium_80)
LV_FONT_DECLARE(lv_font_montserratMedium_40)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_80)
LV_FONT_DECLARE(lv_font_montserratMedium_90)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_16)
LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_18)


#ifdef __cplusplus
}
#endif
#endif
