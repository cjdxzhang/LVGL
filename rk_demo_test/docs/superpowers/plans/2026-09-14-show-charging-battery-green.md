# 在站充电电池图标实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 当设备在站且 MCU 电量低于满格档阈值 80% 时，将状态栏电池图标显示为绿色。

**Architecture:** 继续由 `serial.c` 提供 MCU 电量缓存，状态栏一秒定时器在 LVGL 线程调用现有刷新函数。刷新函数使用最近有效 `LINK_STATUS` 和电量共同决定颜色，同时沿用现有五档电池符号。

**Tech Stack:** C、LVGL 9、Buildroot、OpenSpec

## Global Constraints

- `LINK_STATUS == 0x01` 且电量低于 80% 时显示绿色。
- 80～100% 满格档以及所有离站电量档位显示白色。
- 不新增测试源码、测试目录或测试构建目标；使用静态路径核查和模块编译验证。
- 不改变 MCU 电量解析、缓存、低电保护、页面导航和按钮行为。
- 当前子仓库已有未提交修改，只提交本需求明确涉及的局部差异。

---

### Task 1: 同步 OpenSpec 与协议说明

**Files:**
- Modify: `openspec/changes/use-mcu-battery-for-status-icon/proposal.md`
- Modify: `openspec/changes/use-mcu-battery-for-status-icon/design.md`
- Modify: `openspec/changes/use-mcu-battery-for-status-icon/specs/mcu-battery-status-display/spec.md`
- Modify: `openspec/changes/use-mcu-battery-for-status-icon/tasks.md`
- Modify: `rk3506/3、 mcu-protocol.md`

**Interfaces:**
- Consumes: 已确认的 80% 满格档边界和现有五档电池图标规则。
- Produces: 可验证的在站充电颜色需求及实施任务。

- [x] **Step 1: 在规格中增加充电颜色要求**

在“电池图标刷新”要求中加入：在站且电量为 0～79% 时显示绿色；80～100% 或离站时显示白色；覆盖 79/80% 和位置切换场景。

- [x] **Step 2: 同步 proposal、design、tasks 和 MCU 文档**

记录状态栏联合读取 MCU 电量与最近有效 `LINK_STATUS`，颜色仅用于显示，不改变协议或低电保护。

- [x] **Step 3: 校验 OpenSpec**

Run: `openspec validate use-mcu-battery-for-status-icon`
Expected: `Change 'use-mcu-battery-for-status-icon' is valid`

### Task 2: 实现状态栏充电颜色

**Files:**
- Modify: `app/lvgl_demo/rk_demo_test/status_bar.c`

**Interfaces:**
- Consumes: `serial_mcu_battery_percent(void)` 和 `last_link_status`。
- Produces: `status_bar_update_battery(int battery_level)` 在选择图标后同步刷新颜色。

- [x] **Step 1: 引入位置状态并集中定义阈值和颜色**

在 `status_bar.c` 引入 `system_manager.h`，定义满格阈值 80、普通白色 `0xffffff` 和充电绿色 `0x00ff00`。

- [x] **Step 2: 增加充电态判定**

增加文件内私有函数，当 `last_link_status == 0x01` 且 `battery_level < 80` 时返回充电态。

- [x] **Step 3: 在现有刷新函数中同步颜色**

保留原五档符号分支，并在每次 `status_bar_update_battery()` 调用末尾把充电态设为绿色，其余状态设为白色。

### Task 3: 验证最终实现

**Files:**
- Review: `app/lvgl_demo/rk_demo_test/status_bar.c`
- Review: `openspec/changes/use-mcu-battery-for-status-icon/tasks.md`

**Interfaces:**
- Consumes: Task 1 的规格和 Task 2 的状态栏实现。
- Produces: 可编译、格式合规且 OpenSpec 状态同步的变更。

- [x] **Step 1: 静态核查边界和数据流**

核查在站 79% 为三格绿色、在站 80% 为满格白色、离站 79% 为三格白色，并确认状态栏仍仅在 LVGL 线程更新。

- [x] **Step 2: 检查差异格式**

Run: `git -C app/lvgl_demo diff --check -- rk_demo_test/status_bar.c`
Expected: 无输出，退出码为 0。

- [x] **Step 3: 清理并重编 LVGL demo**

Run: `make lvgl_demo-dirclean && make lvgl_demo-rebuild`（目录：`buildroot/`）
Expected: `[100%] Built target rk_demo_test`，并成功安装至 staging 和 target。

- [x] **Step 4: 更新任务状态并再次校验**

勾选本需求相关 OpenSpec 任务，重新执行 `openspec validate use-mcu-battery-for-status-icon` 并确认通过；上板颜色观察保留为硬件验证项。
