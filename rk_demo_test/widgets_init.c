/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include "gui_guider.h"
#include "widgets_init.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define KEYBOARD_DOCK_Y 316
#define KEYBOARD_DOCK_HEIGHT 164
#define KEYBOARD_TOOLBAR_Y 272
#define KEYBOARD_TOOLBAR_HEIGHT 44
#define KEYBOARD_LANGUAGE_WIDTH 48

typedef enum {
    KEYBOARD_LANGUAGE_ENGLISH = 0,
    KEYBOARD_LANGUAGE_CHINESE
} keyboard_language_t;

typedef struct {
    lv_ui *ui;
    lv_obj_t *english_keyboard;
    lv_obj_t *chinese_keyboard;
    lv_obj_t *pinyin_ime;
    lv_obj_t *candidate_panel;
    lv_obj_t *toolbar;
    lv_obj_t *chinese_button;
    lv_obj_t *chinese_button_label;
    lv_obj_t *english_button;
    lv_obj_t *english_button_label;
    lv_obj_t *hint_label;
    lv_obj_t *active_textarea;
    keyboard_language_t language;
    bool chinese_needs_reset;
    bool initialized;
} keyboard_manager_t;

static keyboard_manager_t g_keyboard_manager;

static void keyboard_manager_set_language(keyboard_language_t language);

static bool keyboard_obj_valid(lv_obj_t *obj)
{
    return obj != NULL && lv_obj_is_valid(obj);
}

static void keyboard_apply_style(lv_obj_t *keyboard)
{
    if (!keyboard_obj_valid(keyboard)) {
        return;
    }

    lv_obj_set_size(keyboard, 800, KEYBOARD_DOCK_HEIGHT);
    /* lv_keyboard_create() defaults to LV_ALIGN_BOTTOM_MID.  Reset the
     * alignment so KEYBOARD_DOCK_Y is an absolute top-left coordinate. */
    lv_obj_align(keyboard, LV_ALIGN_TOP_LEFT, 0, KEYBOARD_DOCK_Y);
    lv_keyboard_set_popovers(keyboard, false);
    lv_obj_set_style_bg_opa(keyboard, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0xeee8f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(keyboard, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(keyboard, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(keyboard, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(keyboard, 4, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_opa(keyboard, 255, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(keyboard, lv_color_hex(0x594867), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(keyboard, &lv_font_SourceHanSansSC_Regular_18, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(keyboard, 8, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(keyboard, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(keyboard, 2, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(keyboard, 35, LV_PART_ITEMS | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x937dad), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(keyboard, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_PRESSED);
}

static void keyboard_update_language_style(void)
{
    bool chinese_active = g_keyboard_manager.language == KEYBOARD_LANGUAGE_CHINESE;

    if (!keyboard_obj_valid(g_keyboard_manager.chinese_button) ||
        !keyboard_obj_valid(g_keyboard_manager.english_button)) {
        return;
    }

    lv_obj_set_style_bg_color(g_keyboard_manager.chinese_button,
                              lv_color_hex(chinese_active ? 0x937dad : 0xffffff),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g_keyboard_manager.english_button,
                              lv_color_hex(chinese_active ? 0xffffff : 0x937dad),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_keyboard_manager.chinese_button_label,
                                lv_color_hex(chinese_active ? 0xffffff : 0x937dad),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_keyboard_manager.english_button_label,
                                lv_color_hex(chinese_active ? 0x937dad : 0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void keyboard_sync_candidate_hint(void)
{
    bool candidate_visible;

    if (!keyboard_obj_valid(g_keyboard_manager.hint_label)) {
        return;
    }

    if (g_keyboard_manager.language == KEYBOARD_LANGUAGE_ENGLISH) {
        lv_label_set_text(g_keyboard_manager.hint_label, "英文输入");
        lv_obj_remove_flag(g_keyboard_manager.hint_label, LV_OBJ_FLAG_HIDDEN);
        if (keyboard_obj_valid(g_keyboard_manager.candidate_panel)) {
            lv_obj_add_flag(g_keyboard_manager.candidate_panel, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    candidate_visible = keyboard_obj_valid(g_keyboard_manager.candidate_panel) &&
                        !lv_obj_has_flag(g_keyboard_manager.candidate_panel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(g_keyboard_manager.hint_label, "请输入拼音");
    if (candidate_visible) {
        lv_obj_add_flag(g_keyboard_manager.hint_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(g_keyboard_manager.hint_label, LV_OBJ_FLAG_HIDDEN);
    }
}

static void keyboard_candidate_state_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        keyboard_sync_candidate_hint();
    }
}

static void keyboard_destroy_chinese_engine(void)
{
    lv_obj_t *ime = g_keyboard_manager.pinyin_ime;
    lv_obj_t *keyboard = g_keyboard_manager.chinese_keyboard;
    lv_obj_t *candidate = g_keyboard_manager.candidate_panel;

    g_keyboard_manager.pinyin_ime = NULL;
    g_keyboard_manager.chinese_keyboard = NULL;
    g_keyboard_manager.candidate_panel = NULL;

    if (keyboard_obj_valid(ime)) {
        lv_obj_delete(ime);
    } else {
        if (keyboard_obj_valid(candidate)) {
            lv_obj_delete(candidate);
        }
        if (keyboard_obj_valid(keyboard)) {
            lv_obj_delete(keyboard);
        }
    }
}

static bool keyboard_create_chinese_engine(void)
{
    lv_obj_t *keyboard;
    lv_obj_t *ime;
    lv_obj_t *candidate;

    keyboard = lv_keyboard_create(lv_layer_top());
    if (keyboard == NULL) {
        return false;
    }
    keyboard_apply_style(keyboard);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);

    ime = lv_ime_pinyin_create(lv_layer_top());
    if (ime == NULL) {
        lv_obj_delete(keyboard);
        return false;
    }

    lv_ime_pinyin_set_keyboard(ime, keyboard);
    lv_ime_pinyin_set_mode(ime, LV_IME_PINYIN_MODE_K26);
    lv_ime_pinyin_set_dict(ime, gg_pinyin_dict);
    lv_obj_set_style_text_font(ime, &lv_font_SourceHanSansSC_Regular_18,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    candidate = lv_ime_pinyin_get_cand_panel(ime);
    if (candidate == NULL) {
        lv_obj_delete(ime);
        return false;
    }

    lv_obj_set_pos(candidate, 120, KEYBOARD_TOOLBAR_Y);
    lv_obj_set_size(candidate, 668, KEYBOARD_TOOLBAR_HEIGHT);
    lv_obj_set_style_bg_opa(candidate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(candidate, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(candidate, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(candidate, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(candidate, 230, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(candidate, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(candidate, lv_color_hex(0x594867), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(candidate, &lv_font_SourceHanSansSC_Regular_18,
                               LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(candidate, 8, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(candidate, lv_color_hex(0x937dad),
                              LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(candidate, lv_color_hex(0xffffff),
                                LV_PART_ITEMS | LV_STATE_PRESSED);

    g_keyboard_manager.chinese_keyboard = keyboard;
    g_keyboard_manager.pinyin_ime = ime;
    g_keyboard_manager.candidate_panel = candidate;
    g_keyboard_manager.chinese_needs_reset = false;

    lv_obj_add_event_cb(keyboard, kb_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(keyboard, keyboard_candidate_state_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(candidate, keyboard_candidate_state_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(candidate, LV_OBJ_FLAG_HIDDEN);
    return true;
}

static bool keyboard_prepare_chinese_engine(void)
{
    if (g_keyboard_manager.chinese_needs_reset) {
        keyboard_destroy_chinese_engine();
        g_keyboard_manager.chinese_needs_reset = false;
    }

    if (!keyboard_obj_valid(g_keyboard_manager.chinese_keyboard) ||
        !keyboard_obj_valid(g_keyboard_manager.pinyin_ime) ||
        !keyboard_obj_valid(g_keyboard_manager.candidate_panel)) {
        keyboard_destroy_chinese_engine();
        return keyboard_create_chinese_engine();
    }

    return true;
}

static void keyboard_show_for_textarea(lv_obj_t *textarea)
{
    if (!g_keyboard_manager.initialized || !keyboard_obj_valid(textarea)) {
        return;
    }

    g_keyboard_manager.active_textarea = textarea;
    lv_obj_remove_flag(g_keyboard_manager.toolbar, LV_OBJ_FLAG_HIDDEN);

    if (g_keyboard_manager.language == KEYBOARD_LANGUAGE_CHINESE) {
        if (!keyboard_prepare_chinese_engine()) {
            g_keyboard_manager.language = KEYBOARD_LANGUAGE_ENGLISH;
        }
    }

    if (g_keyboard_manager.language == KEYBOARD_LANGUAGE_CHINESE) {
        lv_keyboard_set_textarea(g_keyboard_manager.english_keyboard, NULL);
        lv_obj_add_flag(g_keyboard_manager.english_keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(g_keyboard_manager.chinese_keyboard, textarea);
        lv_obj_remove_flag(g_keyboard_manager.chinese_keyboard, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (keyboard_obj_valid(g_keyboard_manager.chinese_keyboard)) {
            lv_keyboard_set_textarea(g_keyboard_manager.chinese_keyboard, NULL);
            lv_obj_add_flag(g_keyboard_manager.chinese_keyboard, LV_OBJ_FLAG_HIDDEN);
        }
        if (keyboard_obj_valid(g_keyboard_manager.candidate_panel)) {
            lv_obj_add_flag(g_keyboard_manager.candidate_panel, LV_OBJ_FLAG_HIDDEN);
        }
        lv_keyboard_set_textarea(g_keyboard_manager.english_keyboard, textarea);
        lv_obj_remove_flag(g_keyboard_manager.english_keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    keyboard_update_language_style();
    keyboard_sync_candidate_hint();
}

void keyboard_manager_hide(void)
{
    lv_obj_t *textarea = g_keyboard_manager.active_textarea;

    g_keyboard_manager.active_textarea = NULL;
    if (!g_keyboard_manager.initialized) {
        return;
    }

    if (keyboard_obj_valid(g_keyboard_manager.english_keyboard)) {
        lv_keyboard_set_textarea(g_keyboard_manager.english_keyboard, NULL);
        lv_obj_add_flag(g_keyboard_manager.english_keyboard, LV_OBJ_FLAG_HIDDEN);
    }
    if (keyboard_obj_valid(g_keyboard_manager.chinese_keyboard)) {
        lv_keyboard_set_textarea(g_keyboard_manager.chinese_keyboard, NULL);
        lv_obj_add_flag(g_keyboard_manager.chinese_keyboard, LV_OBJ_FLAG_HIDDEN);
        g_keyboard_manager.chinese_needs_reset = true;
    }
    if (keyboard_obj_valid(g_keyboard_manager.candidate_panel)) {
        lv_obj_add_flag(g_keyboard_manager.candidate_panel, LV_OBJ_FLAG_HIDDEN);
    }
    if (keyboard_obj_valid(g_keyboard_manager.toolbar)) {
        lv_obj_add_flag(g_keyboard_manager.toolbar, LV_OBJ_FLAG_HIDDEN);
    }

    if (keyboard_obj_valid(textarea)) {
        lv_obj_remove_state(textarea, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, textarea);
    }
}

static void keyboard_manager_set_language(keyboard_language_t language)
{
    lv_obj_t *textarea;

    if (!g_keyboard_manager.initialized || language == g_keyboard_manager.language) {
        return;
    }

    textarea = g_keyboard_manager.active_textarea;
    if (g_keyboard_manager.language == KEYBOARD_LANGUAGE_CHINESE &&
        keyboard_obj_valid(g_keyboard_manager.chinese_keyboard)) {
        lv_keyboard_set_textarea(g_keyboard_manager.chinese_keyboard, NULL);
        lv_obj_add_flag(g_keyboard_manager.chinese_keyboard, LV_OBJ_FLAG_HIDDEN);
        if (keyboard_obj_valid(g_keyboard_manager.candidate_panel)) {
            lv_obj_add_flag(g_keyboard_manager.candidate_panel, LV_OBJ_FLAG_HIDDEN);
        }
        g_keyboard_manager.chinese_needs_reset = true;
    } else if (keyboard_obj_valid(g_keyboard_manager.english_keyboard)) {
        lv_keyboard_set_textarea(g_keyboard_manager.english_keyboard, NULL);
        lv_obj_add_flag(g_keyboard_manager.english_keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    g_keyboard_manager.language = language;
    if (keyboard_obj_valid(textarea)) {
        keyboard_show_for_textarea(textarea);
    } else {
        keyboard_update_language_style();
        keyboard_sync_candidate_hint();
    }
}

static void keyboard_language_button_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        keyboard_manager_set_language((keyboard_language_t)(uintptr_t)lv_event_get_user_data(e));
    }
}

static lv_obj_t *keyboard_create_language_button(lv_obj_t *parent, int32_t x, const char *text,
                                                  keyboard_language_t language, lv_obj_t **label_out)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_t *label;

    lv_obj_set_pos(button, x, 4);
    lv_obj_set_size(button, KEYBOARD_LANGUAGE_WIDTH, 36);
    lv_obj_set_style_radius(button, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_flag(button, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(button, keyboard_language_button_event_cb, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)language);

    label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_SourceHanSansSC_Regular_18,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    *label_out = label;
    return button;
}

void keyboard_manager_init(lv_ui *ui)
{
#if LV_USE_KEYBOARD
    keyboard_manager_t *manager = &g_keyboard_manager;

    if (manager->initialized) {
        if (ui != NULL) {
            ui->g_kb_top_layer = manager->english_keyboard;
        }
        return;
    }

    memset(manager, 0, sizeof(*manager));
    manager->ui = ui;
    manager->language = KEYBOARD_LANGUAGE_ENGLISH;

    manager->toolbar = lv_obj_create(lv_layer_top());
    lv_obj_set_pos(manager->toolbar, 0, KEYBOARD_TOOLBAR_Y);
    lv_obj_set_size(manager->toolbar, 800, KEYBOARD_TOOLBAR_HEIGHT);
    lv_obj_clear_flag(manager->toolbar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(manager->toolbar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(manager->toolbar, lv_color_hex(0xeee8f3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(manager->toolbar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(manager->toolbar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(manager->toolbar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    manager->chinese_button = keyboard_create_language_button(manager->toolbar, 8, "中",
                                                               KEYBOARD_LANGUAGE_CHINESE,
                                                               &manager->chinese_button_label);
    manager->english_button = keyboard_create_language_button(manager->toolbar, 60, "EN",
                                                               KEYBOARD_LANGUAGE_ENGLISH,
                                                               &manager->english_button_label);

    manager->hint_label = lv_label_create(manager->toolbar);
    lv_obj_set_pos(manager->hint_label, 120, 8);
    lv_obj_set_size(manager->hint_label, 668, 28);
    lv_label_set_text(manager->hint_label, "英文输入");
    lv_obj_set_style_text_font(manager->hint_label, &lv_font_SourceHanSansSC_Regular_18,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(manager->hint_label, lv_color_hex(0x6c587a),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(manager->hint_label, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    manager->english_keyboard = lv_keyboard_create(lv_layer_top());
    keyboard_apply_style(manager->english_keyboard);
    lv_keyboard_set_mode(manager->english_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_add_event_cb(manager->english_keyboard, kb_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(manager->english_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(manager->toolbar, LV_OBJ_FLAG_HIDDEN);

    manager->initialized = true;
    if (ui != NULL) {
        ui->g_kb_top_layer = manager->english_keyboard;
    }
    keyboard_update_language_style();
#else
    LV_UNUSED(ui);
#endif
}

__attribute__((unused)) void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        keyboard_manager_hide();
    }
}

__attribute__((unused)) void ta_event_cb(lv_event_t *e)
{
#if LV_USE_KEYBOARD
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *textarea = lv_event_get_target(e);

    LV_UNUSED(lv_event_get_user_data(e));
    if (code == LV_EVENT_FOCUSED) {
        lv_indev_t *indev = lv_indev_active();
        if (indev == NULL || lv_indev_get_type(indev) != LV_INDEV_TYPE_KEYPAD) {
            keyboard_show_for_textarea(textarea);
        }
    } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        keyboard_manager_hide();
    } else if (code == LV_EVENT_DEFOCUSED && textarea == g_keyboard_manager.active_textarea) {
        keyboard_manager_hide();
    }
#else
    LV_UNUSED(e);
#endif
}

void clock_count(int *hour, int *min, int *sec)
{
    (*sec)++;
    if(*sec == 60)
    {
        *sec = 0;
        (*min)++;
    }
    if(*min == 60)
    {
        *min = 0;
        if(*hour < 12)
        {
            (*hour)++;
        } else {
            (*hour)++;
            *hour = *hour %12;
        }
    }
}

void digital_clock_count(int * hour, int * minute, int * seconds, char * meridiem)
{

    (*seconds)++;
    if(*seconds == 60) {
        *seconds = 0;
        (*minute)++;
    }
    if(*minute == 60) {
        *minute = 0;
        if(*hour < 12) {
            (*hour)++;
        }
        else {
            (*hour)++;
            (*hour) = (*hour) % 12;
        }
    }
    if(*hour == 12 && *seconds == 0 && *minute == 0) {
        if((lv_strcmp(meridiem, "PM") == 0)) {
            lv_strcpy(meridiem, "AM");
        }
        else {
            lv_strcpy(meridiem, "PM");
        }
    }
}
