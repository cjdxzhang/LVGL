// custom.c（最终简化版）
#include "custom.h"
#include "app_manager.h"
#include "wifi_manager.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* 自定义 30px CJK 字库：由 NotoSansCJKSC-Regular 生成，
 * 包含所有运行时中文文本所需的汉字（上、等、待、排、喷、淋、机等），
 * 避免 GUI Guider 子集字体缺失汉字显示为方框(□)。 */
LV_FONT_DECLARE(lv_font_custom_cjk_30)
LV_FONT_DECLARE(lv_font_custom_punctuation_18)

static lv_font_t g_dynamic_text_font_18;
static lv_font_t g_dynamic_text_font_25;
static lv_font_t g_dynamic_text_font_30;
static const lv_font_t *g_small_text_font = &lv_font_SourceHanSansSC_Regular_25;
static bool g_custom_fonts_ready = false;

static void custom_prepare_runtime_font(lv_font_t *font_copy, const lv_font_t *source_font,
                                        const lv_font_t *fallback_font)
{
    if (font_copy == NULL || source_font == NULL)
    {
        return;
    }

    memcpy(font_copy, source_font, sizeof(*font_copy));
    font_copy->fallback = fallback_font;
}

static void custom_prepare_fonts(void)
{
    if (g_custom_fonts_ready)
    {
        return;
    }

    /*
     * GUI Guider exported multiple SourceHan subsets with different glyph coverage.
     * Use the broad-coverage 18/25 px fonts for any runtime Chinese text.
     */
    custom_prepare_runtime_font(&g_dynamic_text_font_18,
                                &lv_font_SourceHanSansSC_Regular_18,
                                &lv_font_custom_punctuation_18);
    custom_prepare_runtime_font(&g_dynamic_text_font_25, &lv_font_SourceHanSansSC_Regular_25,
                                &g_dynamic_text_font_18);

    /*
     * 自定义 30px 字库 (custom/lv_font_custom_cjk_30.c) 由 NotoSansCJKSC-Regular 生成，
     * 包含所有运行时中文文本需要的汉字（自动上水中注完成保温等待排清洁喷淋清已，暖风烘干待机），
     * 以及 ASCII 和全角逗号。GUI Guider 生成的 30px 子集字体仅含 65 个设计时汉字，
     * 运行时通过 lv_label_set_text() 设置的文字中缺失的字符会显示为 □。
     *
     * custom_get_dynamic_text_font_30() 返回的字体以自定义全量字库为主，
     * 以 GUI Guider 原 30px 字体为 fallback（保留原图标(PUA)），
     * 确保所有汉字和图标均能正确渲染。
     */
    custom_prepare_runtime_font(&g_dynamic_text_font_30,
                                &lv_font_custom_cjk_30,
                                &lv_font_SourceHanSansSC_Regular_30);
    g_small_text_font = &g_dynamic_text_font_25;
    g_custom_fonts_ready = true;
}

void custom_init(lv_ui *ui)
{
    (void)ui;
    custom_prepare_fonts();
}

void custom_ui_init(lv_ui *ui)
{
    custom_prepare_fonts();

    // 应用管理器接管所有初始化工作
    app_manager_init(ui);
}

void custom_ui_load_scr_animation(lv_ui *ui, lv_obj_t **new_scr, bool new_scr_del,
                                  bool *old_scr_del,
                                  ui_setup_scr_t setup_scr, lv_screen_load_anim_t anim_type,
                                  uint32_t time, uint32_t delay, bool is_clean, bool auto_del)
{
    lv_obj_t *act_scr = lv_screen_active();

    LV_UNUSED(is_clean);
    LV_UNUSED(auto_del);

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "gg_external_data.h"
    if (auto_del)
    {
        gg_edata_task_clear(act_scr);
    }
#endif

    if (new_scr_del)
    {
        setup_scr(ui);
    }

    /*
     * 屏幕缓存：强制不删除旧屏幕，实现界面复用。
     *
     * 问题根源：navigate_to_screen 每次切换都销毁旧界面、重建新界面，
     * setup_scr → lv_obj_create(NULL) 创建新屏幕时，默认背景为白色。
     * 在模拟器（PC）上渲染极快看不到，但在 RK3506B 上首帧渲染耗时
     * 约半秒，白色背景短暂可见，形成白闪。
     *
     * 解决方案：将 auto_del 覆盖为 false——
     *   - 每个界面只创建一次（首次 setup_scr 后常驻内存）
     *   - 切走时保留，切回时直接复用已有对象
     *   - 彻底消除因 recreate 带来的白屏延迟
     */
    lv_screen_load_anim(*new_scr, anim_type, time, delay, false);
    *old_scr_del = false;
}

void __wrap_ui_load_scr_animation(lv_ui *ui, lv_obj_t **new_scr, bool new_scr_del,
                                  bool *old_scr_del,
                                  ui_setup_scr_t setup_scr, lv_screen_load_anim_t anim_type,
                                  uint32_t time, uint32_t delay, bool is_clean, bool auto_del)
{
    custom_ui_load_scr_animation(ui, new_scr, new_scr_del, old_scr_del,
                                 setup_scr, anim_type, time, delay, is_clean, auto_del);
}

const lv_font_t *custom_get_small_text_font(void)
{
    custom_prepare_fonts();
    return g_small_text_font;
}

const lv_font_t *custom_get_dynamic_text_font(void)
{
    custom_prepare_fonts();
    return &g_dynamic_text_font_25;
}

const lv_font_t *custom_get_compact_dynamic_text_font(void)
{
    custom_prepare_fonts();
    return &g_dynamic_text_font_18;
}

const lv_font_t *custom_get_dynamic_text_font_30(void)
{
    custom_prepare_fonts();
    return &g_dynamic_text_font_30;
}

int custom_save_wifi_info(const char *ssid, const char *psk)
{
    if (ssid == NULL || ssid[0] == '\0')
    {
        return -1;
    }

    if (psk == NULL)
    {
        psk = "";
    }

    if (wifi_manager_connect(ssid, psk) != 0)
    {
        return -1;
    }

    return wifi_manager_persist_config(ssid, psk);
}
