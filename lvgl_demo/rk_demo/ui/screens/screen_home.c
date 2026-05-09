#include "screen_home.h"
#include <lvgl/lvgl.h>
#include "components/page_clean.h"
#include "components/top_status.h"
#include "components/card.h"
#include "components/btn.h"
#include "components/page_location.h"
#include "components/setting.h"
#include "protocol.h"
#include "uart_manager.h"

/******* 图标和背景 *****/
LV_IMAGE_DECLARE(q0ULJ1);
LV_FONT_DECLARE(back_btn);

#define RIGHT_VIEW_X      441
#define RIGHT_VIEW_Y      56
#define RIGHT_VIEW_W      346
#define RIGHT_VIEW_H      395

#define RIGHT_THUMB_X     792
#define RIGHT_THUMB_Y     56
#define RIGHT_THUMB_W     5
#define RIGHT_THUMB_H     82

static lv_obj_t * g_right_viewport = NULL;
static lv_obj_t * g_right_thumb = NULL;
static lv_obj_t * s_page_location_main = NULL;


/* 静态函数声明 */
static void create_home_background(lv_obj_t *parent);
static void create_home_overlay(lv_obj_t *parent);
static void create_right_function_area(lv_obj_t *parent);
static void right_area_scroll_cb(lv_event_t *e);
static void update_right_thumb_pos(void);
static void home_btn_event_handler(btn_event_id_t id);
static void hide_all_pages(void);
static void show_page(lv_obj_t * page);
static lv_obj_t * create_func_page(lv_obj_t * parent, const char * title);
static void func_page_back_btn_cb(lv_event_t * e);
static void add_back_button(lv_obj_t * page);
static void setting_back_to_home_cb(void);

/* 右侧功能区的每一页都提前创建好，后续根据需要显示和隐藏 */
static lv_obj_t * page_main = NULL;
static lv_obj_t * page_sleep = NULL;
static lv_obj_t * page_relax = NULL;
static lv_obj_t * page_warm = NULL;
static lv_obj_t * page_bath1 = NULL;
static lv_obj_t * page_bath2 = NULL;
static lv_obj_t * page_clean_confirm = NULL;
static lv_obj_t * page_clean_running = NULL;
static lv_obj_t * page_clean_finish  = NULL;
static bool s_clean_pending = false;

void screen_home_create(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* 清屏，避免以后重复进入首页时对象叠加 */
    lv_obj_clean(scr);

    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    create_home_background(scr);
    create_home_overlay(scr);
}

static void create_home_background(lv_obj_t *parent)
{
    lv_obj_t *bg = lv_image_create(parent);
    lv_image_set_src(bg, &q0ULJ1);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_clear_flag(bg, LV_OBJ_FLAG_SCROLLABLE);
}

static void create_home_overlay(lv_obj_t *parent)
{
    lv_obj_t *panel = lv_obj_create(parent);
    page_main = panel;
    lv_obj_set_size(panel, 800, 480);
    lv_obj_set_pos(panel, 0, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* 顶部状态栏 */
    status_bar_create(panel);

    /* 左上信息卡 */
    card_create_info(panel);
    card_set_clean_idle();

    /* 左半边固定按钮 */
    btn_create_left_group(panel);

    /* 右侧功能区 */
    create_right_function_area(panel);

    page_sleep   = create_func_page(parent, "助眠功能页");
    page_relax   = create_func_page(parent, "放松功能页");
    page_warm    = create_func_page(parent, "驱寒功能页");
    page_bath1   = create_func_page(parent, "药浴1功能页");
    page_bath2   = create_func_page(parent, "药浴2功能页");

    page_clean_set_t clean_set = page_clean_create(parent, show_page, page_main);
    page_clean_confirm = clean_set.page_confirm;
    page_clean_running = clean_set.page_running;
    page_clean_finish  = clean_set.page_finish;

    page_location_set_t location_set = page_location_create(parent, show_page, page_main);
    s_page_location_main = location_set.page_location_main;
}

static void create_right_function_area(lv_obj_t *parent)
{
    /* 右侧可视区域：真正可滚动的是它 */
    lv_obj_t * viewport = lv_obj_create(parent);
    lv_obj_set_size(viewport, RIGHT_VIEW_W, RIGHT_VIEW_H);
    lv_obj_set_pos(viewport, RIGHT_VIEW_X, RIGHT_VIEW_Y);
    lv_obj_set_style_radius(viewport, 0, 0);
    lv_obj_set_style_border_width(viewport, 0, 0);
    lv_obj_set_style_bg_opa(viewport, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(viewport, 0, 0);
    lv_obj_set_style_clip_corner(viewport, true, 0);

    /* 打开纵向滚动，关闭默认滚动条显示 */
    lv_obj_add_flag(viewport, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(viewport, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(viewport, LV_SCROLLBAR_MODE_OFF);

    /* 关闭弹性和惯性，避免超过边界 */
    lv_obj_clear_flag(viewport, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(viewport, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    /* 启用整页吸附 */
    lv_obj_set_scroll_snap_y(viewport, LV_SCROLL_SNAP_START);

    /* 绑定滚动事件，用来更新右侧滑块 */
    lv_obj_add_event_cb(viewport, right_area_scroll_cb, LV_EVENT_SCROLL, NULL);

    g_right_viewport = viewport;

    /* ===== 第 1 页 ===== */
    lv_obj_t * page1 = lv_obj_create(viewport);
    lv_obj_set_size(page1, RIGHT_VIEW_W, RIGHT_VIEW_H);
    lv_obj_set_pos(page1, 0, 0);
    lv_obj_set_style_radius(page1, 0, 0);
    lv_obj_set_style_border_width(page1, 0, 0);
    lv_obj_set_style_bg_opa(page1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page1, 0, 0);
    lv_obj_clear_flag(page1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page1, LV_OBJ_FLAG_SNAPPABLE);

    btn_create_right_page1(page1);

    /* ===== 第 2 页 ===== */
    lv_obj_t * page2 = lv_obj_create(viewport);
    lv_obj_set_size(page2, RIGHT_VIEW_W, RIGHT_VIEW_H);
    lv_obj_set_pos(page2, 0, RIGHT_VIEW_H);
    lv_obj_set_style_radius(page2, 0, 0);
    lv_obj_set_style_border_width(page2, 0, 0);
    lv_obj_set_style_bg_opa(page2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(page2, 0, 0);
    lv_obj_clear_flag(page2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page2, LV_OBJ_FLAG_SNAPPABLE);

    btn_create_right_page2(page2);

    btn_set_event_callback(home_btn_event_handler);
    /* ===== 单条滑块 ===== */
    lv_obj_t * thumb = lv_obj_create(parent);
    lv_obj_set_size(thumb, RIGHT_THUMB_W, RIGHT_THUMB_H);
    lv_obj_set_pos(thumb, RIGHT_THUMB_X, RIGHT_THUMB_Y);
    lv_obj_set_style_radius(thumb, 4, 0);
    lv_obj_set_style_border_width(thumb, 0, 0);
    lv_obj_set_style_bg_color(thumb, lv_color_hex(0xCBBBE6), 0);
    lv_obj_set_style_bg_opa(thumb, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(thumb, 0, 0);
    lv_obj_clear_flag(thumb, LV_OBJ_FLAG_SCROLLABLE);

    g_right_thumb = thumb;

    /* 初始显示第一页 */
    lv_obj_scroll_to_y(viewport, 0, LV_ANIM_OFF);
    update_right_thumb_pos();
}

static void right_area_scroll_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    update_right_thumb_pos();
}

static void update_right_thumb_pos(void)
{
    if(g_right_viewport == NULL || g_right_thumb == NULL) {
        return;
    }

    /* 当前已经滚过去的距离 */
    lv_coord_t scroll_top = lv_obj_get_scroll_top(g_right_viewport);

    /* 最大可滚动距离 = 上面被卷走 + 下面还没显示 */
    lv_coord_t max_scroll = scroll_top + lv_obj_get_scroll_bottom(g_right_viewport);

    /* 滑块可移动的总行程 */
    lv_coord_t travel = RIGHT_VIEW_H - RIGHT_THUMB_H;

    lv_coord_t thumb_y = RIGHT_THUMB_Y;

    if(max_scroll > 0) {
        thumb_y = RIGHT_THUMB_Y + (lv_coord_t)((int32_t)scroll_top * travel / max_scroll);
    }

    /* 边界保护 */
    if(thumb_y < RIGHT_THUMB_Y) {
        thumb_y = RIGHT_THUMB_Y;
    }
    if(thumb_y > RIGHT_THUMB_Y + travel) {
        thumb_y = RIGHT_THUMB_Y + travel;
    }

    lv_obj_set_pos(g_right_thumb, RIGHT_THUMB_X, thumb_y);
}

static bool s_proto_inited = false;

static void handle_bucket_footbath(const protocol_frame_t *frame)
{
    LV_LOG_USER("[HOME] footbath: water=%d temp=%d massage=%d",
                frame->bucket_water, frame->bucket_temp, frame->bucket_massage);
}

static void handle_bucket_standby(const protocol_frame_t *frame)
{
    LV_LOG_USER("[HOME] bucket standby");
}

static void handle_bucket_selfcheck(const protocol_frame_t *frame)
{
    LV_LOG_USER("[HOME] bucket selfcheck: status=%d", frame->bucket_main_status);
}

static void handle_base_selfclean(const protocol_frame_t *frame)
{
    LV_LOG_USER("[HOME] base selfclean");
}

static void register_proto_handlers(void)
{
    protocol_register_cmd(CMD_BUCKET_FOOTBATH,  handle_bucket_footbath);
    protocol_register_cmd(CMD_BUCKET_STANDBY,   handle_bucket_standby);
    protocol_register_cmd(CMD_BUCKET_SELFCHECK, handle_bucket_selfcheck);
    protocol_register_cmd(CMD_BASE_SELFCLEAN,   handle_base_selfclean);
}

static void ensure_proto_init(void)
{
    if (!s_proto_inited) {
        protocol_init();
        protocol_handler_init();
        register_proto_handlers();
        s_proto_inited = true;
        LV_LOG_USER("[HOME] protocol initialized");
    }
}

static void home_btn_event_handler(btn_event_id_t id)
{
    switch(id) {
    case BTN_EVENT_MODE_SLEEP:
        ensure_proto_init();
        {
            protocol_frame_t frame = {0};
            frame.link_mode = LINK_MODE_BUCKET_ONLY;
            frame.cmd_type  = CMD_BUCKET_FOOTBATH;
            frame.bucket_water = 50;
            frame.bucket_temp  = 42;
            protocol_send_uart(&frame);
            LV_LOG_USER("[HOME] sleep mode: send footbath cmd");
        }
        s_clean_pending = false;
        card_set_clean_idle();
        show_page(page_sleep);
        break;

    case BTN_EVENT_MODE_RELAX:
        s_clean_pending = false;
        card_set_clean_idle();
        show_page(page_relax);
        break;

    case BTN_EVENT_MODE_WARM:
        s_clean_pending = false;
        card_set_clean_idle();
        show_page(page_warm);
        break;

    case BTN_EVENT_MODE_CLEAN:
        s_clean_pending = true;
        card_set_clean_pending();
        break;

    case BTN_EVENT_START:
        if(s_clean_pending) {
            page_clean_set_countdown(10, 0);
            show_page(page_clean_running);
        }
        break;

    case BTN_EVENT_BATH1:
        s_clean_pending = false;
        card_set_clean_idle();
        show_page(page_bath1);
        break;

    case BTN_EVENT_BATH2:
        s_clean_pending = false;
        card_set_clean_idle();
        show_page(page_bath2);
        break;

    case BTN_EVENT_LOCATE:
        s_clean_pending = false;
        card_set_clean_idle();
        show_page(s_page_location_main);
        break;

    case BTN_EVENT_SETTING:
    s_clean_pending = false;
    card_set_clean_idle();
    setting_set_back_callback(setting_back_to_home_cb);
    show_page(setting_create(lv_scr_act()));
    break;

    default:
        break;
    }
}

/* 隐藏所有页面 */
static void hide_all_pages(void)
{
    if(page_main)    lv_obj_add_flag(page_main, LV_OBJ_FLAG_HIDDEN);
    if(page_sleep)   lv_obj_add_flag(page_sleep, LV_OBJ_FLAG_HIDDEN);
    if(page_relax)   lv_obj_add_flag(page_relax, LV_OBJ_FLAG_HIDDEN);
    if(page_warm)    lv_obj_add_flag(page_warm, LV_OBJ_FLAG_HIDDEN);
    if(page_clean_confirm) lv_obj_add_flag(page_clean_confirm, LV_OBJ_FLAG_HIDDEN);
    if(page_clean_running) lv_obj_add_flag(page_clean_running, LV_OBJ_FLAG_HIDDEN);
    if(page_clean_finish)  lv_obj_add_flag(page_clean_finish, LV_OBJ_FLAG_HIDDEN);
    if(page_bath1)   lv_obj_add_flag(page_bath1, LV_OBJ_FLAG_HIDDEN);
    if(page_bath2)   lv_obj_add_flag(page_bath2, LV_OBJ_FLAG_HIDDEN);
    if(s_page_location_main) lv_obj_add_flag(s_page_location_main, LV_OBJ_FLAG_HIDDEN);
    if(setting_get_root()) lv_obj_add_flag(setting_get_root(), LV_OBJ_FLAG_HIDDEN);
}

static void show_page(lv_obj_t * page)
{
    hide_all_pages();
    if(page) {
        lv_obj_clear_flag(page, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_obj_t * create_func_page(lv_obj_t * parent, const char * title)
{
    lv_obj_t * page = lv_obj_create(parent);
    lv_obj_set_size(page, 800, 480);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x081018), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * label = lv_label_create(page);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    lv_obj_center(label);

    add_back_button(page);

    return page;
}

static void func_page_back_btn_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    show_page(page_main);
}

static void add_back_button(lv_obj_t * page)
{
    lv_obj_t * back_icon = lv_label_create(page);
    lv_label_set_text(back_icon, "\xEE\x98\x81");
    lv_obj_set_style_text_font(back_icon, &back_btn, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(back_icon, 25, 25);

    lv_obj_add_flag(back_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(back_icon, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(back_icon, func_page_back_btn_cb, LV_EVENT_CLICKED, NULL);
}

void screen_home_set_clean_pending(bool pending)
{
    s_clean_pending = pending;
}

static void setting_back_to_home_cb(void)
{
    show_page(page_main);
}