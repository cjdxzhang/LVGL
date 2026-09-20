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
