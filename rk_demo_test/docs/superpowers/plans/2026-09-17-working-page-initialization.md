# 工作页初始化整理实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**目标：** 集中工作页控件初始化、精简 Label 样式并统一页面初始化标志。

**架构：** `setup_scr_working.c` 创建全部工作页控件，`system_manager.c` 通过 `system_manager_working_page_init()` 管理业务状态和定时器。`page_initialized.h/.c` 集中声明和定义页面初始化标志。

**技术栈：** C、LVGL 9、Buildroot。

## 全局约束

- 不改变长按待机、双击唤醒、温度编辑、倒计时和回仓行为。
- 不新增测试文件或测试构建目标。
- 修改后必须清理并重编译 `lvgl_demo`。

---

### 任务1：集中页面初始化标志

**文件：**
- 新建：`page_initialized.h`
- 新建：`page_initialized.c`
- 修改：`system_manager.c`、`system_manager.h`、`events_init.c`、`custom_imgbtn.c`

- [x] 将八个 `_is_*_page_initialized` 声明移入专用头文件。
- [x] 将八个变量的唯一零值定义移入专用源文件。
- [x] 更新使用文件的包含关系并检查无旧声明。

### 任务2：集中工作页控件初始化

**文件：**
- 修改：`setup_scr_working.c`
- 修改：`system_manager.c`
- 修改：`system_manager.h`

- [x] 将电源按钮辅助创建函数和工作页动态按钮创建移入 `setup_scr_working.c`。
- [x] 将原业务初始化部分改名为 `system_manager_working_page_init(lv_ui *ui)`。
- [x] 将全部工作页控件创建收口到 `WorkPageInit(lv_ui *ui)`，页面加载后由事件处理器直接初始化业务。
- [x] 保持按钮事件、阴影和初始显示状态不变。

### 任务3：精简工作页 Label 样式并验证

**文件：**
- 修改：`setup_scr_working.c`

- [x] 保留已有有效尺寸和 `lv_label_set_long_mode()`。
- [x] 保留字体、颜色、必要对齐和非默认透明度，删除其余 Label 默认样式。
- [x] 执行引用检查和 `git diff --check`。
- [x] 执行 `make lvgl_demo-dirclean && make lvgl_demo-rebuild`，预期链接生成 `rk_demo_test`。

### 任务4：收口工作页创建与进入生命周期

**文件：**
- 修改：`setup_scr_working.c`、`events_init.c`、`system_manager.c/.h`
- 修改：`custom_imgbtn.c`、`gui_guider.h`、`page_initialized.c/.h`

- [x] 将 `setup_scr_working()` 重命名为 `WorkPageInit()`，并在其中一次性创建全部工作页控件和事件回调。
- [x] 删除 `working_page_init()` 和 `_is_working_page_initialized`。
- [x] 在 `working_event_handler()` 中直接分发加载、卸载和删除生命周期。
- [x] 执行引用检查、差异检查和 `lvgl_demo` 完整重编译。
