#ifndef CUSTOM_IMGBTN_H
#define CUSTOM_IMGBTN_H


#include "lvgl.h"

extern int16_t to_preparing_flat; /* 记录转到preparing的标志 1--注水中，2--自清洁中，3--移动中，4--返回基站中*/
extern int16_t to_setting_flat; /* 记录转到setting的标志 1--从index跳转，2--从working跳转 */
extern int16_t to_location_flat; /* 记录转到location的标志 1--从index跳转，2--从working跳转 3--从preparing跳转 */
typedef enum {
    UI_SCREEN_INDEX,
    UI_SCREEN_SETTING,
    UI_SCREEN_PREPARING,
    UI_SCREEN_LOCATION,
    UI_SCREEN_WORKING,
    UI_SCREEN_CONF_ADVANCE,
    UI_SCREEN_CONF_MODE,
    UI_SCREEN_CONF_MODE_DETAIL,
    UI_SCREEN_CONF_WIFI,
    UI_SCREEN_CONF_OTHER,
    UI_SCREEN_CONF_LOCATION,
} ui_screen_id_t;
/**
 * 创建自定义图片+文字按钮
 * @param parent 父对象
 * @param img_src 图片资源指针（例如 &_home_alpha_32x32）
 * @param text 按钮文字
 * @param x X坐标
 * @param y Y坐标
 * @param w 宽度
 * @param h 高度
 * @return 按钮容器对象指针
 */
lv_obj_t* imgbtn_create(lv_obj_t* parent, const void* img_src, const char* text, 
                        lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
lv_obj_t* imgbtn_create_without_bg(lv_obj_t* parent, const void* img_src, const char* text, 
                        lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
void navigate_to_screen(ui_screen_id_t target_screen);
void setting_back(lv_event_t *e);
/**  clicked事件，从setting->index **/
void setting_to_index(lv_event_t *e);
/**  clicked事件，从index->setting **/
void index_to_setting(lv_event_t *e);
/**  clicked事件，从setting->conf_mode **/
void setting_to_conf_mode(lv_event_t *e);
/**  clicked事件，从conf_mode->setting **/
void conf_mode_to_setting(lv_event_t *e);
/**  clicked事件，从conf_mode->index **/
void conf_mode_to_index(lv_event_t *e);
/**  clicked事件，从conf_mode_detail->index **/
void conf_mode_detail_to_index(lv_event_t *e);
/**  clicked事件，从conf_mode->conf_mode_detail **/
void conf_mode_to_conf_mode_detail(lv_event_t *e);
/**  clicked事件，从conf_mode_detail->conf_mode **/
void conf_mode_detail_to_conf_mode(lv_event_t *e);

/**  clicked事件，从conf_wifi->index **/
void conf_wifi_to_index(lv_event_t *e);
/**  clicked事件，从conf_wifi->setting **/
void conf_wifi_to_setting(lv_event_t *e);
/**  clicked事件，从setting->conf_wifi **/
void setting_to_conf_wifi(lv_event_t *e);

/**  clicked事件，从conf_advance->index **/
void conf_advance_to_index(lv_event_t *e);
/**  clicked事件，从conf_advance->setting **/
void conf_advance_to_setting(lv_event_t *e);
/**  clicked事件，从setting->conf_advance **/
void setting_to_conf_advance(lv_event_t *e);

/**  clicked事件，从conf_other->index **/
void conf_other_to_index(lv_event_t *e);
/**  clicked事件，从conf_other->setting **/
void conf_other_to_setting(lv_event_t *e);
/**  clicked事件，从setting->conf_other **/
void setting_to_conf_other(lv_event_t *e);

/**  clicked事件，从conf_location->index **/
void conf_location_to_index(lv_event_t *e);
/**  clicked事件，从conf_location->setting **/
void conf_location_to_setting(lv_event_t *e);
/**  clicked事件，从setting->conf_location **/
void setting_to_conf_location(lv_event_t *e);

/**  clicked事件，从index->preparing **/
void index_to_preparing(lv_event_t *e);
/**  clicked事件，从preparing->index **/
void preparing_to_index(lv_event_t *e);

/**  clicked事件，从index->location **/
void index_to_location(lv_event_t *e);
/**  clicked事件，从location->index **/
void location_to_index(lv_event_t *e);

/**  clicked事件，从location返回事件 */
void location_back(lv_event_t *e);
/**  clicked事件，从location->preparing **/
void location_to_preparing(lv_event_t *e);
/**  clicked事件，从location->preparing **/
void location_return_to_preparing(lv_event_t *e);
/**  clicked事件，从preparing->location **/
void preparing_moving_to_location(lv_event_t *e);
/**  clicked事件，从preparing->location **/
void preparing_to_location(lv_event_t *e);

/**  clicked事件，从preparing->working **/
void preparing_to_working(lv_event_t *e);
/**  clicked事件，从working->preparing **/
void working_to_preparing_back(lv_event_t *e);

/**  clicked事件，从working->setting **/
void working_to_setting(lv_event_t *e);
/**  clicked事件，从setting->working **/
void setting_to_working(lv_event_t *e);

/**  clicked事件，从working->location**/
void working_to_location(lv_event_t *e);
/**  clicked事件，从location->working **/
void location_to_working(lv_event_t *e);

/**  clicked事件，从clean->preparing **/
void clean_to_preparing(lv_event_t *e);

/**  clicked事件，点停止弹出提示对话框  **/
void stop_toast(lv_event_t *e);
void stop_dialog_cleanup(void);

/**
 * 显示通用提示对话框
 *
 * 创建一个全屏半透明遮罩，中间显示白色对话框容器，
 * 上面有提示文字（使用 #sym:SourceHanSansSC_Regular_25 字体），
 * 底部居中有一个"确定"按钮。点击"确定"按钮自动销毁对话框。
 *
 * @param parent  父对象（通常传入当前活动的屏幕对象，如 guider_ui.working）
 * @param message 要显示的提示文字内容
 */
void show_message_dialog(lv_obj_t *parent, const char *message);
#endif