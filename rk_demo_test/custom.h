/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void custom_init(lv_ui *ui);
void custom_ui_init(lv_ui *ui);
void custom_ui_load_scr_animation(lv_ui *ui, lv_obj_t **new_scr, bool new_scr_del,
                                  bool *old_scr_del,
                                  ui_setup_scr_t setup_scr, lv_screen_load_anim_t anim_type,
                                  uint32_t time, uint32_t delay, bool is_clean, bool auto_del);
const lv_font_t *custom_get_small_text_font(void);
const lv_font_t *custom_get_dynamic_text_font(void);
const lv_font_t *custom_get_compact_dynamic_text_font(void);
const lv_font_t *custom_get_dynamic_text_font_30(void);
int custom_save_wifi_info(const char *ssid, const char *psk);

/* 自定义 30px CJK 字库：包含运行时所需全部汉字，避免子集字体缺失 */
LV_FONT_DECLARE(lv_font_custom_cjk_30)
LV_FONT_DECLARE(lv_font_custom_punctuation_18)

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
