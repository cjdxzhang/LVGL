# 旧版机器人、BLE 与 TCP 服务迁移设计

## 目标

删除 `app/lvgl_demo/rk_demo_test` 当前基于 `robot_chassis` 的机器人实现、公开接口及调用点，并以仓库根目录 `rk_demo_test` 为来源，迁移旧版 `RobotTcp_*` 机器人实现、设备绑定配置、BLE 配网和本地 TCP 服务。

迁移后采用旧版机器人行为，包括机器人自身站点列表、站点 ID 导航，以及回仓时使用站点 ID `0` 和旧版坐标兜底规则。

## 范围

### 删除

- `robot_chassis/` 模块及其构建目标。
- `robot_gateway_ipc.c/.h` 和 `tools/robot_gateway_cli.c`。
- `mode_navigation_flow.c/.h`。
- `app_manager_robot_*` 公开接口、站点存储、机器人状态快照和地图备份 IPC 生命周期。
- `system_manager.c`、`status_bar.c` 等文件中对上述新接口的调用和类型依赖。

### 迁移

从根目录 `rk_demo_test` 迁移：

- `robot_tcp.c/.h`
- `device_binding_config.c/.h`
- `ble_manager.c/.h`
- `ble_provisioning.c/.h`
- `tcp_service.c/.h`

源目录仅作为迁移来源，不修改其内容，不迁移其中的构建产物。

### 保留

目标目录现有的足浴 MCU V2.3 业务、语音服务、自清洁流程、Wi-Fi 状态管理、页面修复和其他后续功能继续保留。不会用根目录版本整文件覆盖目标端的 `app_manager.c`、`system_manager.c`、`wifi_manager.c` 或 UI 文件。

## 架构和生命周期

`app_manager_init()` 保持应用总入口职责，按以下顺序初始化新增模块：

1. 目标端原有 Wi-Fi、状态栏、系统管理、足浴串口和语音服务。
2. `RobotTcp_Init()` 和 `RobotTcp_Start()`。
3. `tcp_service_init()`。
4. `ble_manager_init()`，成功后调用 `ble_manager_start_advertising()`。

退出时先停止可能接受外部请求的 BLE/TCP 服务，再调用 `RobotTcp_Deinit()`，避免退出过程中继续向机器人或业务模块提交请求。现有语音、串口和其他资源仍按目标端生命周期释放。

BLE GATT 服务通过系统 D-Bus 与 BlueZ 通信；构建系统需要加入 `dbus-1` 的包含目录和链接参数。设备绑定信息由 `device_binding_config` 按源实现持久化。

## 机器人业务替换

### 普通站点导航

目标端页面选定站点后恢复旧版调用：

1. 使用旧版站点 ID。
2. 调用 `RobotTcp_GotoStationById()` 提交导航。
3. 调用 `RobotTcp_GetStationCoordsById()` 保存目标坐标。
4. 准备页定时调用 `RobotTcp_RequestStatus()` 和 `RobotTcp_GetState()`，使用最新位置判断到达。

原有导航前基站待机动作继续保留，但删除 `mode_navigation_flow` 和 `app_manager_robot_*` 中间层。

### 回仓充电

恢复旧版 `RobotTcp_SetCharge(1)` 行为。到达判断坐标从站点 ID `0` 获取；获取失败时按源代码行为使用 `(0,0,0)`。不再读取 `/mnt/udisk/stations.json` 中的唯一充电基站。

### 状态栏

若源端状态栏存在有效的机器人电量显示调用，则按旧 `RobotTcp_GetState()` 接口迁移；如果源端代码本身处于注释或未启用状态，则保持未启用，不额外扩大功能。

## TCP 与 BLE 数据流

本地 TCP service 保留源端命令协议和功能集合，包括设备状态、足浴控制、机器人状态刷新和 `robot_command`。它直接调用目标端现有的串口、系统管理、Wi-Fi 和页面接口，以及迁入的 `RobotTcp_*` 和设备绑定接口。

BLE manager 负责适配器和广播管理，BLE provisioning 注册 GATT 服务。配网成功后调用目标端 `wifi_manager`，并通过 `device_binding_config` 保存用户、SSID 和 MQTT 配置。迁移时仅做目标端 API 和构建兼容所需调整，不改变源端协议字段和响应语义。

## 构建调整

- 从目标 `CMakeLists.txt` 删除 `add_subdirectory(robot_chassis)`、`robot_chassis` 链接和 `robot_gateway_cli` 目标。
- 目标目录源码扫描自动纳入新迁移的 `.c` 文件。
- 为主程序加入 D-Bus 编译和链接配置。
- 更新 `custom.mk`：删除旧机器人/IPC/导航流程条目，加入迁移模块。
- 保留 cJSON 的既有顶层构建来源，避免重复编译符号。

## 错误处理

- 单个外部服务初始化失败时记录错误，其他足浴/UI 功能继续运行。
- 机器人初始化或启动失败时不假装可用，后续调用沿用 `RobotTcp_*` 返回错误。
- TCP、BLE 线程退出必须先关闭监听或停止事件入口，再回收线程资源。
- 不修改源端网络协议和旧站点兜底语义，但修复阻止目标模块编译或安全退出的明显生命周期缺口。

## 验证

1. 检索确认目标目录不再包含 `robot_chassis`、`app_manager_robot_*`、`robot_gateway` 和 `mode_navigation_flow` 的生产引用。
2. 检查新模块所有头文件、目标端全局状态和外部函数依赖均已解析。
3. 检查 CMake 实际编译文件和链接库，避免 cJSON 重复定义。
4. 执行 `make lvgl_demo-dirclean && make lvgl_demo-rebuild`，以完整模块编译作为完成门槛。
5. 若交叉编译成功但当前环境无法运行 BlueZ、机器人和手机联调，明确记录板端仍需验证的 BLE 广播/配网、TCP 命令和机器人导航行为。

## 非目标

- 不保留新旧两套机器人实现或兼容适配层。
- 不修改根目录 `rk_demo_test` 源代码。
- 不重新设计 BLE、TCP 或机器人协议。
- 不顺带重构无关 UI、足浴 MCU 或语音业务。
