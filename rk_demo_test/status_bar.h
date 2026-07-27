#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include "lvgl.h"
#include "gui_guider.h"

// 状态栏创建（在顶层图层上创建）
// ui: GUI Guider 生成的 ui 对象（用于获取图片资源，如果没有传入 NULL）
// 返回值：状态栏容器对象
lv_obj_t * status_bar_create(lv_ui *ui);

// 更新 WiFi 图标（由外部调用或回调触发）
void status_bar_update_wifi(bool connected);

// 更新时间显示（可由定时器调用）
void status_bar_update_time(void);

// 更新电池电量（battery_level: 0-100）
void status_bar_update_battery(int battery_level);

// 获取状态栏容器，用于外部添加控件（可选）
lv_obj_t * status_bar_get_container(void);

#endif // STATUS_BAR_H