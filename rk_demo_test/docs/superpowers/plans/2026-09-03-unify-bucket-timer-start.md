# 桶体定时统一启动入口实施计划

> **执行要求：** 按当前会话内联执行；项目规则禁止为本次修改新增测试源码、测试目录或测试构建目标。

**目标：** 新增统一业务函数，使所有桶体定时请求同时下发 A6 并重启本地倒计时；`timer_set == 0` 时下发 A6 并取消本地倒计时。

**架构：** `serial_bucket_timer()` 继续只负责 A6 入队，`system_manager.c` 新增公开业务入口统一管理 A6 与本地倒计时状态。APP、触屏、语音及自动上水流程只调用业务入口，不再直接调用串口层定时 API。

**技术栈：** C、LVGL 9、Buildroot、OpenSpec。

## 全局约束

- 新增和修改的注释使用中文。
- 不新增重复倒计时状态或第二套 timer。
- `timer_set == 0` 不触发倒计时到期的 A1 或完成提示。
- 修改后执行 `make lvgl_demo-dirclean && make lvgl_demo-rebuild`。

---

### 任务一：同步规格与项目文档

**文件：**

- 修改：`openspec/changes/implement-footbath-v23-protocol/specs/bucket-timer-and-auto-start/spec.md`
- 修改：`openspec/changes/implement-footbath-v23-protocol/design.md`
- 修改：`openspec/changes/implement-footbath-v23-protocol/tasks.md`
- 修改：`rk3506/3、 mcu-protocol.md`

- [x] 明确统一入口、非零重启、零值取消和所有业务调用点复用规则。
- [x] 新增实现与上板验证任务。

### 任务二：实现并迁移统一入口

**文件：**

- 修改：`app/lvgl_demo/rk_demo_test/system_manager.h`
- 修改：`app/lvgl_demo/rk_demo_test/system_manager.c`
- 修改：`app/lvgl_demo/rk_demo_test/tcp_service.c`

**接口：**

- 产生：`void system_bucket_timer_apply(void)`
- 依赖：全局 `timer_set`、`serial_bucket_timer()` 和现有本地单调倒计时状态。

- [x] 新增取消本地倒计时且不触发到期处理的内部函数。
- [x] 新增带中文说明的统一业务函数。
- [x] 将五类业务调用点迁移到统一函数。
- [x] 使用 `rg` 确认除统一函数和串口底层外没有直接 A6 业务调用。

### 任务三：验证

**文件：**

- 检查：本次所有修改文件。

- [x] 执行 OpenSpec 校验和差异空白检查。
- [x] 在 `buildroot/` 执行 `make lvgl_demo-dirclean && make lvgl_demo-rebuild` 并确认退出码为 0。
- [x] 复核非零值重启、零值取消且不触发 A1、所有调用点统一三个条件。
