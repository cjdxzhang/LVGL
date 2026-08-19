#include "gui_guider.h"
#include "custom.h"
#include "custom_imgbtn.h"
#include "serial.h"
#include "system_manager.h"
#include <stdio.h>

int16_t to_preparing_flat =
    1; /* 记录转到preparing的标志 1--注水中，2--自清洁中，3--移动中，4--返回基站中*/
int16_t to_setting_flat =
    1; /* 记录转到setting的标志 1--从index跳转，2--从working跳转 */
int16_t to_location_flat =
    1; /* 记录转到location的标志 1--从index跳转，2--从working跳转 3--从preparing跳转 */

static lv_timer_t *g_stop_dialog_timer = NULL;
static int32_t g_stop_dialog_countdown = 0;
static lv_obj_t *g_message_dialog = NULL;
static bool g_message_dialog_should_navigate = false;
static ui_screen_id_t g_message_dialog_target_screen = UI_SCREEN_INDEX;

static bool stop_dialog_obj_is_valid(const lv_obj_t *obj)
{
    return obj != NULL && lv_obj_is_valid(obj);
}

static void stop_dialog_reset_no_label(void)
{
    // if (stop_dialog_obj_is_valid(guider_ui.preparing_btn_no_label)) {
    //     lv_label_set_text(guider_ui.preparing_btn_no_label, "否");
    // }
}

void stop_dialog_cleanup(void)
{
    if (g_stop_dialog_timer != NULL)
    {
        lv_timer_delete(g_stop_dialog_timer);
        g_stop_dialog_timer = NULL;
    }

    g_stop_dialog_countdown = 0;

    /* 仅在 dialog 容器对象有效时才重置标签文字 */
    if (guider_ui.preparing_dialog1 != NULL &&
            lv_obj_is_valid(guider_ui.preparing_dialog1))
    {
        stop_dialog_reset_no_label();
    }
}

static void stop_dialog_button_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    stop_dialog_cleanup();
}

static void stop_dialog_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (!stop_dialog_obj_is_valid(guider_ui.preparing_dialog1) ||
            !stop_dialog_obj_is_valid(guider_ui.preparing_btn_no))
    {
        stop_dialog_cleanup();
        return;
    }

    g_stop_dialog_countdown--;

    if (g_stop_dialog_countdown <= 0)
    {
        stop_dialog_cleanup();
        lv_obj_send_event(guider_ui.preparing_btn_no, LV_EVENT_CLICKED, NULL);
        return;
    }

    if (stop_dialog_obj_is_valid(guider_ui.preparing_btn_no_label))
    {
        lv_label_set_text_fmt(guider_ui.preparing_btn_no_label, "否\n(%lds后自动)",
                              (long)g_stop_dialog_countdown);
    }
}

static void stop_dialog_start_countdown(void)
{
    stop_dialog_cleanup();

    if (!stop_dialog_obj_is_valid(guider_ui.preparing_btn_no) ||
            !stop_dialog_obj_is_valid(guider_ui.preparing_btn_no_label) ||
            !stop_dialog_obj_is_valid(guider_ui.preparing_btn_yes))
    {
        return;
    }

    lv_obj_set_style_text_font(guider_ui.preparing_btn_yes,
                               custom_get_compact_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_font(guider_ui.preparing_btn_no,
                               custom_get_compact_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(guider_ui.preparing_btn_no_label,
                               custom_get_compact_dynamic_text_font(),
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_remove_event_cb(guider_ui.preparing_btn_yes, stop_dialog_button_event_cb);
    lv_obj_remove_event_cb(guider_ui.preparing_btn_no, stop_dialog_button_event_cb);
    lv_obj_add_event_cb(guider_ui.preparing_btn_yes, stop_dialog_button_event_cb, LV_EVENT_CLICKED,
                        NULL);
    lv_obj_add_event_cb(guider_ui.preparing_btn_no, stop_dialog_button_event_cb, LV_EVENT_CLICKED,
                        NULL);

    g_stop_dialog_countdown = 5;
    lv_label_set_text_fmt(guider_ui.preparing_btn_no_label, "否\n(%lds后自动)",
                          (long)g_stop_dialog_countdown);
    g_stop_dialog_timer = lv_timer_create(stop_dialog_timer_cb, 1000, NULL);
}



static bool *get_active_screen_del_flag(lv_ui *ui)
{
    lv_obj_t *act_scr = lv_screen_active();

    if (act_scr == ui->index)
    {
        return &ui->index_del;
    }
    if (act_scr == ui->setting)
    {
        return &ui->setting_del;
    }
    if (act_scr == ui->preparing)
    {
        return &ui->preparing_del;
    }
    if (act_scr == ui->location)
    {
        return &ui->location_del;
    }
    if (act_scr == ui->working)
    {
        return &ui->working_del;
    }
    if (act_scr == ui->conf_advance)
    {
        return &ui->conf_advance_del;
    }
    if (act_scr == ui->conf_mode)
    {
        return &ui->conf_mode_del;
    }
    if (act_scr == ui->conf_mode_detail)
    {
        return &ui->conf_mode_detail_del;
    }
    if (act_scr == ui->conf_wifi)
    {
        return &ui->conf_wifi_del;
    }
    if (act_scr == ui->conf_other)
    {
        return &ui->conf_other_del;
    }
    if (act_scr == ui->conf_location)
    {
        return &ui->conf_location_del;
    }

    return NULL;
}

bool navigate_to_screen(ui_screen_id_t target_screen)
{
    lv_obj_t **new_scr = NULL;
    bool *new_scr_del = NULL;
    bool *old_scr_del = get_active_screen_del_flag(&guider_ui);
    ui_setup_scr_t setup_scr = NULL;

    if (old_scr_del == NULL)
    {
        return false;
    }

    switch (target_screen)
    {
    case UI_SCREEN_INDEX:
        new_scr = &guider_ui.index;
        new_scr_del = &guider_ui.index_del;
        setup_scr = setup_scr_index; //首页
        break;
    case UI_SCREEN_SETTING:
        new_scr = &guider_ui.setting;
        new_scr_del = &guider_ui.setting_del;
        setup_scr = setup_scr_setting; // 设置页面
        break;
    case UI_SCREEN_PREPARING:
        new_scr = &guider_ui.preparing;
        new_scr_del = &guider_ui.preparing_del;
        setup_scr = setup_scr_preparing; // 准备页面
        break;
    case UI_SCREEN_LOCATION:
        new_scr = &guider_ui.location;
        new_scr_del = &guider_ui.location_del;
        setup_scr = setup_scr_location; // 定位页面
        break;
    case UI_SCREEN_WORKING:
        new_scr = &guider_ui.working;
        new_scr_del = &guider_ui.working_del;
        setup_scr = setup_scr_working; // 工作页面(桶体页面?)
        break;
    case UI_SCREEN_CONF_ADVANCE:
        new_scr = &guider_ui.conf_advance;
        new_scr_del = &guider_ui.conf_advance_del;
        setup_scr = setup_scr_conf_advance; // 高级设置页面
        break;
    case UI_SCREEN_CONF_MODE:
        new_scr = &guider_ui.conf_mode;
        new_scr_del = &guider_ui.conf_mode_del;
        setup_scr = setup_scr_conf_mode; // 模式设置页面
        break;
    case UI_SCREEN_CONF_MODE_DETAIL:
        new_scr = &guider_ui.conf_mode_detail;
        new_scr_del = &guider_ui.conf_mode_detail_del;
        setup_scr = setup_scr_conf_mode_detail; // 模式设置详细页面
        break;
    case UI_SCREEN_CONF_WIFI:
        new_scr = &guider_ui.conf_wifi;
        new_scr_del = &guider_ui.conf_wifi_del;
        setup_scr = setup_scr_conf_wifi; // Wi-Fi设置页面
        break;
    case UI_SCREEN_CONF_OTHER:
        new_scr = &guider_ui.conf_other;
        new_scr_del = &guider_ui.conf_other_del;
        setup_scr = setup_scr_conf_other; // 其他设置页面
        break;
    case UI_SCREEN_CONF_LOCATION:
        new_scr = &guider_ui.conf_location;
        new_scr_del = &guider_ui.conf_location_del;
        setup_scr = setup_scr_conf_location; // 定位设置页面
        break;
    default:
        return false;
    }

    ui_load_scr_animation(&guider_ui, new_scr, *new_scr_del, old_scr_del,
                          setup_scr, LV_SCR_LOAD_ANIM_NONE, 200, 0, false, false);
    return true;
}

ui_screen_id_t get_current_screen_id(void)
{
    lv_obj_t *active_screen = lv_screen_active();

    if (active_screen == guider_ui.setting) return UI_SCREEN_SETTING;
    if (active_screen == guider_ui.preparing) return UI_SCREEN_PREPARING;
    if (active_screen == guider_ui.location) return UI_SCREEN_LOCATION;
    if (active_screen == guider_ui.working) return UI_SCREEN_WORKING;
    if (active_screen == guider_ui.conf_advance) return UI_SCREEN_CONF_ADVANCE;
    if (active_screen == guider_ui.conf_mode) return UI_SCREEN_CONF_MODE;
    if (active_screen == guider_ui.conf_mode_detail) return UI_SCREEN_CONF_MODE_DETAIL;
    if (active_screen == guider_ui.conf_wifi) return UI_SCREEN_CONF_WIFI;
    if (active_screen == guider_ui.conf_other) return UI_SCREEN_CONF_OTHER;
    if (active_screen == guider_ui.conf_location) return UI_SCREEN_CONF_LOCATION;
    return UI_SCREEN_INDEX;
}

const char *get_current_screen_name(void)
{
    switch (get_current_screen_id())
    {
    case UI_SCREEN_INDEX:
        return "index";
    case UI_SCREEN_SETTING:
        return "setting";
    case UI_SCREEN_PREPARING:
        return "preparing";
    case UI_SCREEN_LOCATION:
        return "location";
    case UI_SCREEN_WORKING:
        return "working";
    case UI_SCREEN_CONF_ADVANCE:
        return "conf_advance";
    case UI_SCREEN_CONF_MODE:
        return "conf_mode";
    case UI_SCREEN_CONF_MODE_DETAIL:
        return "conf_mode_detail";
    case UI_SCREEN_CONF_WIFI:
        return "conf_wifi";
    case UI_SCREEN_CONF_OTHER:
        return "conf_other";
    case UI_SCREEN_CONF_LOCATION:
        return "conf_location";
    default:
        return "unknown";
    }
}

lv_obj_t *imgbtn_create(lv_obj_t *parent, const void *img_src, const char *text,
                        lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{

    /* 1. 创建面板作为按钮底板 */
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);

    /* 样式设置：白色半透明圆角背景 + 阴影 */
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_30, 0);      // 半透明
    lv_obj_set_style_radius(panel, 20, 0);              // 圆角
    lv_obj_set_style_border_width(panel, 0, 0);         // 无边框

    /* 阴影效果 */
    lv_obj_set_style_shadow_width(panel, 8, 0);
    lv_obj_set_style_shadow_ofs_x(panel, 3, 0);
    lv_obj_set_style_shadow_ofs_y(panel, 3, 0);
    lv_obj_set_style_shadow_color(panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(panel, LV_OPA_40, 0);

    /* 2. 创建图标（正中间偏上） */
    lv_obj_t *img = lv_img_create(panel);
    lv_img_set_src(img, img_src);
    lv_obj_center(img);
    // 计算偏移量，如果text有内容，使图标向上偏移 1/6 高度，文字向下偏移 1/6 高度
    if (text && text[0] != '\0')
    {
        lv_obj_set_pos(img, 0, -h / 6);   // 向上偏移 1/6 高度，实现偏上效果
    }
    else
    {
        lv_obj_set_pos(img, 0, 0);        // 无文字时图标居中
    }

    /* 3. 创建文字标签（正中间偏下） */
    if (text && text[0] != '\0')
    {
        lv_obj_t *label = lv_label_create(panel);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(label, custom_get_small_text_font(), 0);
        lv_obj_center(label);
        lv_obj_set_pos(label, 0, h / 6);   // 向下偏移 1/6 高度，实现偏下效果
    }


    /* 4. 使面板可点击并添加点击样式反馈（可选） */
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xE0E0E0), LV_STATE_PRESSED);

    return panel;
}
lv_obj_t *imgbtn_create_without_bg(lv_obj_t *parent, const void *img_src, const char *text,
                                   lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    /* 1. 创建一个透明的对象作为按钮容器 */
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_pos(container, x, y);
    lv_obj_set_size(container, w, h);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0); // 无边框
    lv_obj_set_style_shadow_width(container, 0, 0); // 无阴影

    /* 2. 创建图标（正中间偏上） */
    lv_obj_t *img = lv_img_create(container);
    lv_img_set_src(img, img_src);
    lv_obj_center(img);
    // 计算偏移量，如果text有内容，使图标向上偏移 1/6 高度，文字向下偏移 1/6 高度
    if (text && text[0] != '\0')
    {
        lv_obj_set_pos(img, 0, -h / 6);   // 向上偏移 1/6 高度，实现偏上效果
    }
    else
    {
        lv_obj_set_pos(img, 0, 0);        // 无文字时图标居中
    }

    /* 3. 创建文字标签（正中间偏下） */
    if (text && text[0] != '\0')
    {
        lv_obj_t *label = lv_label_create(container);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(label, custom_get_small_text_font(), 0);
        lv_obj_center(label);
        lv_obj_set_pos(label, 0, h / 4);   // 向下偏移 1/6 高度，实现偏下效果
    }


    /* 4. 使容器可点击并添加点击样式反馈（可选） */
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(container, lv_color_hex(0xE0E0E0), LV_STATE_PRESSED);

    return container;
}

void setting_back(lv_event_t *e)
{
    if (to_setting_flat == 1)
    {
        setting_to_index(e);
    }
    else if (to_setting_flat == 2)
    {
        setting_to_working(e);
    }
}
void setting_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}
void setting_to_working(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_WORKING);
        break;
    }
    default:
        break;
    }
}
void index_to_setting(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        to_setting_flat = 1; /* 转到setting，设置标志为1，表示从index跳转 */
        navigate_to_screen(UI_SCREEN_SETTING);
        break;
    }
    default:
        break;
    }
}
void preparing_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        preparing_page_cleanup();
        to_preparing_flat = 0;
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}

void index_to_preparing(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        if (is_self_cleaning)
        {
            if (!system_base_flow_try_start(SYSTEM_BASE_FLOW_SELF_CLEANING,
                                            "index-clean-start"))
            {
                return;
            }
            to_preparing_flat = 2; /* 转到preparing，设置标志为2，表示自清洁中 */
            system_auto_water_navigation_cancel();
        }
        else
        {
            if (!system_base_flow_try_start(SYSTEM_BASE_FLOW_WATERING,
                                            "index-water-start"))
            {
                return;
            }
            to_preparing_flat = 1; /* 转到preparing，设置标志为1，表示注水中 */
            system_auto_water_navigation_arm();
        }
        // 停止当前页面的自动排水计时器（如果存在），因为用户已经主动点击了按钮，说明不需要自动排水了
        // if (g_auto_drain_timer != NULL) {
        //     lv_timer_delete(g_auto_drain_timer);
        //     g_auto_drain_timer = NULL;
        //     printf("停止自动排水计时器\n");
        // }
        if (!navigate_to_screen(UI_SCREEN_PREPARING))
        {
            system_base_flow_finish(is_self_cleaning ?
                                    SYSTEM_BASE_FLOW_SELF_CLEANING :
                                    SYSTEM_BASE_FLOW_WATERING,
                                    "index-preparing-navigation-failed");
            if (is_self_cleaning)
            {
                is_self_cleaning = false;
            }
            else
            {
                system_auto_water_navigation_cancel();
            }
            to_preparing_flat = 0;
        }
        break;
    }
    default:
        break;
    }
}
void index_to_location(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        to_location_flat = 1; /* 转到location，设置标志为1，表示从index跳转 */
        navigate_to_screen(UI_SCREEN_LOCATION);
        break;
    }
    default:
        break;
    }
}
void location_back(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        if (to_location_flat == 1)
        {
            navigate_to_screen(UI_SCREEN_INDEX);
        }
        else if (to_location_flat == 2)
        {
            navigate_to_screen(UI_SCREEN_WORKING);
        }
        else if (to_location_flat == 3)
        {
            navigate_to_screen(UI_SCREEN_PREPARING);
        }
        break;
    }
    default:
        break;
    }
}
void location_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}
void location_to_preparing(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        (void)system_start_selected_location_navigation(guider_ui.location);
        break;
    }
    default:
        break;
    }
}

void location_return_to_preparing(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        to_preparing_flat = 1; /* 转到preparing，设置标志为5，表示返回注水中 */
        if (!navigate_to_screen(UI_SCREEN_PREPARING))
        {
            system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                                    "return-base-page-failed");
            to_preparing_flat = 0;
        }
        break;
    }
    default:
        break;
    }
}
void preparing_moving_to_location(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        preparing_page_cleanup();
        to_preparing_flat = 3; /* 转到preparing，设置标志为3，表示移动中 */
        to_location_flat = 3; /* 转到location，设置标志为3，表示从preparing跳转 */
        navigate_to_screen(UI_SCREEN_LOCATION);
        break;
    }
    default:
        break;
    }
}
void preparing_to_location(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        preparing_page_cleanup();
        to_preparing_flat = 1; /* 转到preparing，设置标志为3，表示移动中 */
        to_location_flat = 3; /* 转到location，设置标志为3，表示从preparing跳转 */
        navigate_to_screen(UI_SCREEN_LOCATION);
        break;
    }
    default:
        break;
    }
}
void preparing_to_working(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        preparing_page_cleanup();
        navigate_to_screen(UI_SCREEN_WORKING);
        break;
    }
    default:
        break;
    }
}
void working_to_preparing_back(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        if (!system_base_flow_try_start(SYSTEM_BASE_FLOW_MOVING,
                                        "working-return-base"))
        {
            return;
        }
        to_preparing_flat = 4; /* 转到preparing，设置标志为4，表示返回基站中 */
        if (!navigate_to_screen(UI_SCREEN_PREPARING))
        {
            system_base_flow_finish(SYSTEM_BASE_FLOW_MOVING,
                                    "return-base-page-failed");
            to_preparing_flat = 0;
        }
        break;
    }
    default:
        break;
    }
}
void working_to_setting(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        to_setting_flat = 2; /* 转到setting，设置标志为2，表示从working跳转 */
        navigate_to_screen(UI_SCREEN_SETTING);
        break;
    }
    default:
        break;
    }
}

void working_to_location(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        to_location_flat = 2; /* 转到location，设置标志为2，表示从working跳转 */
        navigate_to_screen(UI_SCREEN_LOCATION);
        break;
    }
    default:
        break;
    }
}
void location_to_working(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_WORKING);
        break;
    }
    default:
        break;
    }
}
void cleaning_to_preparing(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        if (!system_base_flow_try_start(SYSTEM_BASE_FLOW_SELF_CLEANING,
                                        "cleaning-page-start"))
        {
            return;
        }
        is_self_cleaning = true;
        to_preparing_flat = 2; /* 转到preparing，设置标志为2，表示自清洁中 */
        if (!navigate_to_screen(UI_SCREEN_PREPARING))
        {
            system_base_flow_finish(SYSTEM_BASE_FLOW_SELF_CLEANING,
                                    "cleaning-page-failed");
            is_self_cleaning = false;
            to_preparing_flat = 0;
        }
        break;
    }
    default:
        break;
    }
}
void preparing_to_cleaning(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        preparing_page_cleanup();
        to_preparing_flat = 2; /* 转到preparing，设置标志为2，表示自清洁中 */
        navigate_to_screen(UI_SCREEN_CONF_MODE);
        break;
    }
    default:
        break;
    }
}
void setting_to_conf_mode(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_CONF_MODE);
        break;
    }
    default:
        break;
    }
}
void conf_mode_to_setting(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_SETTING);
        break;
    }
    default:
        break;
    }
}
void conf_mode_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}
void conf_mode_to_conf_mode_detail(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_CONF_MODE_DETAIL);
        break;
    }
    default:
        break;
    }
}
void conf_mode_detail_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}
void conf_mode_detail_to_conf_mode(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_CONF_MODE);
        break;
    }
    default:
        break;
    }
}
void conf_wifi_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}

void conf_wifi_to_setting(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_SETTING);
        break;
    }
    default:
        break;
    }
}
void setting_to_conf_wifi(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_CONF_WIFI);
        break;
    }
    default:
        break;
    }
}

void conf_advance_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}
void conf_advance_to_setting(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_SETTING);
        break;
    }
    default:
        break;
    }
}
void setting_to_conf_advance(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_CONF_ADVANCE);
        break;
    }
    default:
        break;
    }
}

void conf_other_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}
void conf_other_to_setting(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_SETTING);
        break;
    }
    default:
        break;
    }
}
void setting_to_conf_other(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_CONF_OTHER);
        break;
    }
    default:
        break;
    }
}

void conf_location_to_index(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_INDEX);
        break;
    }
    default:
        break;
    }
}
void conf_location_to_setting(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_SETTING);
        break;
    }
    default:
        break;
    }
}
void setting_to_conf_location(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        navigate_to_screen(UI_SCREEN_CONF_LOCATION);
        break;
    }
    default:
        break;
    }
}

void stop_toast(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        //preparing_btn_no
        lv_obj_remove_flag(guider_ui.preparing_dialog1, LV_OBJ_FLAG_HIDDEN);
        stop_dialog_start_countdown();
        break;
    }
    default:
        break;
    }
}

/* ────────── 通用提示对话框 ────────── */

static void message_dialog_deleted(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_DELETE)
    {
        return;
    }

    if (lv_event_get_target(e) == g_message_dialog)
    {
        g_message_dialog = NULL;
        g_message_dialog_should_navigate = false;
        g_message_dialog_target_screen = UI_SCREEN_INDEX;
    }
}

void message_dialog_cleanup(void)
{
    if (g_message_dialog != NULL && lv_obj_is_valid(g_message_dialog))
    {
        lv_obj_del(g_message_dialog);
    }

    g_message_dialog = NULL;
    g_message_dialog_should_navigate = false;
    g_message_dialog_target_screen = UI_SCREEN_INDEX;
}

/** 点击"确定"按钮时销毁整个对话框，并按需跳转页面 */
static void message_dialog_btn_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    bool should_navigate;
    ui_screen_id_t target_screen;

    if (code != LV_EVENT_CLICKED)
    {
        return;
    }

    should_navigate = g_message_dialog_should_navigate;
    target_screen = g_message_dialog_target_screen;
    message_dialog_cleanup();

    if (should_navigate)
    {
        navigate_to_screen(target_screen);
    }
}

static void message_dialog_show(lv_obj_t *parent,
                                const char *message,
                                bool should_navigate,
                                ui_screen_id_t target_screen)
{
    lv_obj_t *dialog;
    lv_obj_t *cont;
    lv_obj_t *label;
    lv_obj_t *btn;
    lv_obj_t *btn_label;

    if (parent == NULL || message == NULL)
    {
        return;
    }

    message_dialog_cleanup();

    /* 全屏半透明遮罩 */
    dialog = lv_obj_create(parent);
    g_message_dialog = dialog;
    g_message_dialog_should_navigate = should_navigate;
    g_message_dialog_target_screen = target_screen;
    lv_obj_add_event_cb(dialog, message_dialog_deleted, LV_EVENT_DELETE, NULL);
    lv_obj_set_size(dialog, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(dialog, 0, 0);
    lv_obj_set_style_bg_opa(dialog, LV_OPA_60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dialog, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);

    /* 白色对话框容器 */
    cont = lv_obj_create(dialog);
    lv_obj_set_size(cont, 440, 200);
    lv_obj_center(cont);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(cont, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(cont, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(cont, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    /* 提示消息 —— 使用 #sym:SourceHanSansSC_Regular_25 即 lv_font_SourceHanSansSC_Regular_25 */
    label = lv_label_create(cont);
    lv_label_set_text(label, message);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, LV_PCT(85));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 25);

    /* 底部居中"确定"按钮 */
    btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 120, 42);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "确定");
    lv_obj_center(btn_label);
    lv_obj_set_style_text_font(btn_label, &lv_font_SourceHanSansSC_Regular_25,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 单击"确定" → 销毁整个对话框 */
    lv_obj_add_event_cb(btn, message_dialog_btn_clicked, LV_EVENT_CLICKED, NULL);
}

void show_message_dialog(lv_obj_t *parent, const char *message)
{
    message_dialog_show(parent, message, false, UI_SCREEN_INDEX);
}

void show_message_dialog_and_navigate(lv_obj_t *parent,
                                      const char *message,
                                      ui_screen_id_t target_screen)
{
    message_dialog_show(parent, message, true, target_screen);
}
