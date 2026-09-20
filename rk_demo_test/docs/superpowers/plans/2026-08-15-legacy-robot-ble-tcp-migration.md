# 旧版机器人、BLE 与 TCP 服务迁移实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**目标：** 删除目标应用的新 `robot_chassis` 链路，并从仓库根目录 `rk_demo_test` 迁入旧 `RobotTcp_*`、设备绑定、BLE 配网和本地 TCP 服务及其业务调用。

**架构：** 目标端保留现有足浴 MCU、语音、自清洁、Wi-Fi 和 UI 主体，只在模块边界和机器人调用点进行定向替换。旧 `RobotTcp_*` 直接承担机器人连接、状态和站点操作；TCP service 与 BLE provisioning 作为外部控制入口，由 `app_manager` 统一管理生命周期。

**技术栈：** C99、LVGL、pthread、POSIX socket、cJSON、D-Bus/BlueZ、CMake、Buildroot。

## 全局约束

- 所有新增注释、日志说明、文档和提交信息使用中文。
- 根目录 `rk_demo_test` 只作为源代码来源，不修改，不复制 `build/` 产物。
- 不保留 `robot_chassis` 与 `RobotTcp_*` 两套实现。
- 保留目标端现有足浴 MCU V2.3、语音、自清洁、Wi-Fi 和页面修复。
- 采用旧版机器人站点 ID 逻辑，回仓使用站点 ID `0` 和旧版 `(0,0,0)` 兜底。
- 不修改 BLE、TCP 或机器人协议字段和响应语义。
- 最终必须执行 `make lvgl_demo-dirclean && make lvgl_demo-rebuild`。

---

### 任务 1：建立迁移前依赖与残留检查

**文件：**
- 读取：`rk_demo_test/{robot_tcp,device_binding_config,ble_manager,ble_provisioning,tcp_service}.{c,h}`
- 读取：`app/lvgl_demo/rk_demo_test/{app_manager,system_manager,status_bar,CMakeLists,custom}.c*`

**接口：**
- 输入：源端旧模块和目标端现有业务接口。
- 输出：需要复制的十个源文件，以及待删除/替换引用的确定清单。

- [ ] **步骤 1：记录用户已有改动，禁止覆盖**

运行：

```bash
git -C app/lvgl_demo status --short
git -C app/lvgl_demo diff -- rk_demo_test/system_manager.c rk_demo_test/voice_command_service.c rk_demo_test/voice_command_service.h
```

预期：明确现有未提交修改；后续只对机器人相关代码块做最小补丁。

- [ ] **步骤 2：生成旧机器人残留基线**

运行：

```bash
rg -n "robot_chassis|app_manager_robot_|robot_gateway|mode_navigation_flow" app/lvgl_demo/rk_demo_test --glob '*.[ch]' --glob 'CMakeLists.txt' --glob '*.mk'
```

预期：输出全部生产调用、构建项和头文件依赖，供任务 4 的零残留检查使用。

### 任务 2：迁入旧版独立模块

**文件：**
- 创建：`app/lvgl_demo/rk_demo_test/robot_tcp.c`
- 创建：`app/lvgl_demo/rk_demo_test/robot_tcp.h`
- 创建：`app/lvgl_demo/rk_demo_test/device_binding_config.c`
- 创建：`app/lvgl_demo/rk_demo_test/device_binding_config.h`
- 创建：`app/lvgl_demo/rk_demo_test/ble_manager.c`
- 创建：`app/lvgl_demo/rk_demo_test/ble_manager.h`
- 创建：`app/lvgl_demo/rk_demo_test/ble_provisioning.c`
- 创建：`app/lvgl_demo/rk_demo_test/ble_provisioning.h`
- 创建：`app/lvgl_demo/rk_demo_test/tcp_service.c`
- 创建：`app/lvgl_demo/rk_demo_test/tcp_service.h`

**接口：**
- 产生：`RobotTcp_Init/Start/Deinit`、`RobotTcp_GotoStationById`、`RobotTcp_GetStationCoordsById`、`RobotTcp_RequestStatus/GetState`、`tcp_service_init/deinit`、`ble_manager_init/start_advertising`、`ble_provisioning_stop`。
- 依赖：目标端 `serial.h`、`system_manager.h`、`wifi_manager.h`、`custom_imgbtn.h`、cJSON 和 D-Bus。

- [ ] **步骤 1：复制十个源文件**

使用逐文件复制，来源固定为仓库根目录 `rk_demo_test/`，目标固定为 `app/lvgl_demo/rk_demo_test/`；不得复制 `build/`、生成 UI 文件或其他源端快照。

- [ ] **步骤 2：验证复制内容一致**

运行：

```bash
for name in robot_tcp device_binding_config ble_manager ble_provisioning tcp_service; do
    cmp "rk_demo_test/${name}.c" "app/lvgl_demo/rk_demo_test/${name}.c"
    cmp "rk_demo_test/${name}.h" "app/lvgl_demo/rk_demo_test/${name}.h"
done
```

预期：所有 `cmp` 返回 0；若后续兼容修复改变目标副本，则只修改目标副本。

### 任务 3：替换应用生命周期和机器人业务调用

**文件：**
- 修改：`app/lvgl_demo/rk_demo_test/app_manager.c`
- 修改：`app/lvgl_demo/rk_demo_test/app_manager.h`
- 修改：`app/lvgl_demo/rk_demo_test/system_manager.c`
- 修改：`app/lvgl_demo/rk_demo_test/status_bar.c`
- 修改：`app/lvgl_demo/rk_demo_test/custom_imgbtn.c`（仅在旧调用语义需要时）

**接口：**
- 使用：任务 2 产生的旧模块公开函数。
- 删除：全部 `app_manager_robot_*` 接口、状态快照、站点存储和机器人 IPC 生命周期。

- [ ] **步骤 1：替换 `app_manager` 初始化**

在保留目标端 Wi-Fi、状态栏、系统管理、串口和语音初始化后加入：

```c
if (RobotTcp_Init() != 0 || RobotTcp_Start() != 0) {
    APP_MANAGER_LOG_ERROR("旧版机器人 TCP 模块初始化失败");
}
if (tcp_service_init() != 0) {
    APP_MANAGER_LOG_ERROR("本地 TCP 控制服务初始化失败");
}
if (ble_manager_init() == 0) {
    if (ble_manager_start_advertising() != 0) {
        APP_MANAGER_LOG_ERROR("BLE 广播启动失败");
    }
} else {
    APP_MANAGER_LOG_ERROR("BLE 管理模块初始化失败");
}
```

具体返回值以源函数实现为准，避免在 `RobotTcp_Init()` 失败后误启动线程。

- [ ] **步骤 2：替换退出顺序**

在 `app_manager_deinit()` 中先关闭外部入口，再关闭机器人：

```c
tcp_service_deinit();
ble_provisioning_stop();
RobotTcp_Deinit();
```

保持调用幂等，避免未初始化模块退出时崩溃。

- [ ] **步骤 3：删除 `app_manager_robot_*` 接口和内部状态**

删除 `app_manager.h` 的机器人类型包含与声明，并从 `app_manager.c` 删除 `g_robot_*`、站点存储、状态回调、IPC、地图备份及相关锁。

- [ ] **步骤 4：恢复旧版导航调用**

从源端 `system_manager.c` 定向迁入以下逻辑，同时保留目标端现有的导航互斥、基站待机和页面切换：

```c
int ret = RobotTcp_GotoStationById(target_station_id);
if (ret == 0) {
    (void)RobotTcp_GetStationCoordsById(target_station_id,
                                        &g_nav_target_x,
                                        &g_nav_target_y,
                                        &g_nav_target_z,
                                        NULL,
                                        0);
}
```

函数签名必须以 `robot_tcp.h` 的实际声明为准。

- [ ] **步骤 5：恢复旧版回仓与状态轮询**

回仓调用 `RobotTcp_SetCharge(1)`，然后读取站点 ID `0` 坐标；读取失败时显式设置三轴为 `0.0f`。导航定时器每次先调用 `RobotTcp_RequestStatus()`，再通过 `RobotTcp_GetState()` 读取位置和充电状态。

- [ ] **步骤 6：处理状态栏接口**

对照源端 `status_bar.c`：仅迁移实际启用的 `RobotTcp_GetState()` 电量读取；源端若未启用则删除目标端遗留的注释类型依赖。

### 任务 4：删除新机器人实现并调整构建

**文件：**
- 删除：`app/lvgl_demo/rk_demo_test/robot_chassis/`
- 删除：`app/lvgl_demo/rk_demo_test/robot_gateway_ipc.c`
- 删除：`app/lvgl_demo/rk_demo_test/robot_gateway_ipc.h`
- 删除：`app/lvgl_demo/rk_demo_test/tools/robot_gateway_cli.c`
- 删除：`app/lvgl_demo/rk_demo_test/mode_navigation_flow.c`
- 删除：`app/lvgl_demo/rk_demo_test/mode_navigation_flow.h`
- 修改：`app/lvgl_demo/rk_demo_test/CMakeLists.txt`
- 修改：`app/lvgl_demo/rk_demo_test/custom.mk`

**接口：**
- 输入：任务 2 的新源文件和任务 3 的调用点。
- 输出：只构建旧机器人、BLE、绑定配置和 TCP service 的单一程序。

- [ ] **步骤 1：删除新机器人文件**

使用补丁逐项删除上述受版本控制文件，确认不触及用户无关改动和根目录源文件。

- [ ] **步骤 2：调整 CMake**

删除 `add_subdirectory(robot_chassis)`、`target_link_libraries(... robot_chassis)` 和 `robot_gateway_cli`。通过 pkg-config 引入 D-Bus：

```cmake
pkg_check_modules(DBUS REQUIRED dbus-1)
target_include_directories(${PROJECT_NAME} PRIVATE ${DBUS_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${DBUS_CFLAGS_OTHER})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DBUS_LIBRARIES})
```

顶层已加载 `PkgConfig`；不得在子目录重复加入 cJSON 源文件。

- [ ] **步骤 3：调整 `custom.mk`**

删除 `robot_gateway_ipc.c` 和 `mode_navigation_flow.c`，加入：

```make
GEN_CSRCS += ble_manager.c
GEN_CSRCS += ble_provisioning.c
GEN_CSRCS += device_binding_config.c
GEN_CSRCS += robot_tcp.c
GEN_CSRCS += tcp_service.c
```

- [ ] **步骤 4：验证旧实现零残留**

运行：

```bash
rg -n "robot_chassis|app_manager_robot_|robot_gateway|mode_navigation_flow" app/lvgl_demo/rk_demo_test --glob '*.[ch]' --glob 'CMakeLists.txt' --glob '*.mk'
```

预期：生产代码与构建文件无匹配；设计/计划文档匹配不计入生产残留。

### 任务 5：编译驱动的兼容修复

**文件：**
- 修改：任务 2 迁入的目标副本及任务 3、4 修改文件。
- 不修改：`rk_demo_test/` 源目录。

**接口：**
- 输入：目标 Buildroot 工具链和真实目标端 API。
- 输出：无编译和链接错误的 `rk_demo_test`。

- [ ] **步骤 1：执行完整模块清理重编译**

运行：

```bash
make lvgl_demo-dirclean && make lvgl_demo-rebuild
```

工作目录：`buildroot/`。

预期：首次构建可能暴露迁移兼容错误，但必须实际进入 `rk_demo_test` 编译。

- [ ] **步骤 2：逐项修复真实错误**

只修复以下类别：目标端函数签名差异、缺失声明、D-Bus 编译链接、线程退出、重复符号、C99/Werror。每次修复后重新执行同一完整模块构建，不使用生成目录源码作为修改来源。

- [ ] **步骤 3：确认构建成功**

再次运行：

```bash
make lvgl_demo-dirclean && make lvgl_demo-rebuild
```

预期：命令返回 0，并成功生成/安装 `rk_demo_test`；不能以静态检查或局部 harness 替代。

### 任务 6：最终验证与文档同步

**文件：**
- 修改：`rk3506/2、 buildroot-lvgl.md`
- 修改：`rk3506/4、 chassis-protocol.md`
- 检查：`openspec/changes/implement-robot-chassis/`

**接口：**
- 输入：最终实际代码行为。
- 输出：与实现一致的项目维护文档和交付证据。

- [ ] **步骤 1：同步项目文档**

将正式应用机器人说明改为旧 `robot_tcp.c/.h`，补充 BLE、设备绑定和 TCP service 文件入口，删除“旧 RobotTcp 已退出生产”的失效描述，并明确站点 ID `0` 回仓行为。

- [ ] **步骤 2：记录 OpenSpec 差异**

检查 `openspec/changes/implement-robot-chassis/`。本次用户明确要求回退旧逻辑，因此不把旧 change 当作当前实现；若不归档或删除该 change，则在文档中明确其与现实现状不一致，避免误导。

- [ ] **步骤 3：最终静态检查**

运行：

```bash
git -C app/lvgl_demo diff --check
rg -n "robot_chassis|app_manager_robot_|robot_gateway|mode_navigation_flow" app/lvgl_demo/rk_demo_test --glob '*.[ch]' --glob 'CMakeLists.txt' --glob '*.mk'
git -C app/lvgl_demo status --short
```

预期：`diff --check` 无错误，生产残留检索无输出，状态列表只包含本任务和用户原有修改。

- [ ] **步骤 4：提交交付说明**

汇报迁移文件、删除内容、完整构建命令和结果，并列出板端尚需验证的 BLE 广播/配网、本地 TCP 命令和机器人导航/回仓行为。未经用户明确要求，不提交用户已有的未提交修改。
