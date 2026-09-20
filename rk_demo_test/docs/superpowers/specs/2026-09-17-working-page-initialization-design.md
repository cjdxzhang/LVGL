# 工作页初始化整理设计

## 目标

将工作页控件创建集中到 `setup_scr_working.c`，保留系统管理模块对温度、倒计时、待机和页面生命周期的管理；同时精简工作页 Label 默认样式，并集中保存各页面初始化标志。

## 设计

- `setup_scr_working.c` 提供 `WorkPageInit()`，在页面根对象首次创建时一次性创建全部静态控件、功能按钮和事件回调。
- `working_event_handler()` 在工作页加载完成时直接调用 `system_manager_working_page_init()`，在卸载或删除时调用 `system_manager_working_page_cleanup()`。
- `system_manager.c` 不再创建工作页按钮，只管理工作页进入时的状态绑定、刷新定时器、离开清理和事件业务。
- `page_initialized.h` 只声明初始化标志，`page_initialized.c` 提供唯一的零值定义，避免头文件多重定义。
- 工作页 Label 保留已有且未注释的尺寸、长文本模式、字体、颜色、必要对齐和非默认透明度，删除其余默认样式。

## 验证

- 检查 `working_page_init()`、`setup_scr_working()` 和 `_is_working_page_initialized` 已无引用。
- 检查页面初始化标志只在 `page_initialized.c` 定义。
- 执行 `git diff --check`。
- 执行 `make lvgl_demo-dirclean && make lvgl_demo-rebuild`。
