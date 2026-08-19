#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include "lvgl.h"
#include "gui_guider.h"

/**
 * 应用管理器初始化
 * @param ui GUI Guider 生成的 ui 对象，包含所有屏幕和控件的指针
 *
 * 职责：
 * 1. 初始化所有底层模块（wifi_manager、modbus_485、robot_tcp 等）
 * 2. 创建全局 UI 组件（状态栏等）
 * 3. 注册模块间的回调（如 WiFi 状态变化通知状态栏更新）
 * 4. 启动必要的后台任务（时间同步、状态轮询等）
 */
void app_manager_init(lv_ui *ui);
void app_manager_deinit(void);

/**
 * 获取 ui 对象的全局指针（供其他模块访问控件时使用）
 * 注意：仅在 app_manager_init 调用后有效
 */
lv_ui *app_manager_get_ui(void);

/**
 * 获取状态栏容器（供需要动态添加控件的场景使用）
 */
lv_obj_t *app_manager_get_status_bar(void);

#endif // APP_MANAGER_H
