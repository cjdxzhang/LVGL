# Global MCU Report Watchdog Implementation Plan

> **For agentic workers:** Implement inline in the current workspace and review each task before continuing. Do not create TDD tests or test build targets.

**Goal:** Detect any 10-second absence of valid MCU status reports globally, issue one standby command and warning per outage, and prevent automatic water filling from consuming stale status.

**Architecture:** The serial layer owns an atomic monotonically increasing valid-frame sequence. The LVGL system manager polls the sequence once per second, latches each outage after 10 unchanged samples, and rearms after a new frame. Automatic-water navigation snapshots the same sequence when armed and ignores cached state until the sequence changes.

**Tech Stack:** C99, GCC atomic built-ins, LVGL 9 timers/dialogs, existing serial command FIFO and OpenSpec.

## Global Constraints

- Do not add TDD test cases, test source files, test directories, or test build targets.
- The timeout is exactly 10 consecutive seconds without a checksum-valid MCU status frame.
- Each outage emits one `MCU未上报消息` log, one `0xB1`, and one current-page dialog.
- Dialog confirmation closes the dialog and stays on the current page.

### Task 1: Valid MCU frame sequence

**Files:** Modify `serial.c` and `serial.h`.

- [x] Increment a `uint32_t` sequence after each valid frame updates runtime state, using GCC `__atomic_add_fetch`.
- [x] Expose `uint32_t serial_mcu_status_report_sequence(void)` using `__atomic_load_n`.
- [x] Confirm invalid or partial frames cannot increment the sequence.

### Task 2: Global watchdog

**Files:** Modify `system_manager.c`.

- [x] Add a one-second LVGL timer with last sequence, silent-second count, and outage-latched state.
- [x] At 10 unchanged samples, log `MCU未上报消息`, call `serial_base_standby()`, and call `show_message_dialog(lv_screen_active(), "设备状态异常，请回仓重启")` once.
- [x] On sequence change, clear the silent count and outage latch so a later outage can trigger again.

### Task 3: Ignore stale automatic-water status

**Files:** Modify `mode_navigation_flow.h`, `mode_navigation_flow.c`, and `system_manager.c`.

- [x] Store the report sequence snapshot when automatic-water navigation is armed.
- [x] Ignore observations whose report sequence has not changed since the previous accepted observation.
- [x] Preserve the existing single-consumption behavior when a new report has `data[23] = 5`.

### Task 4: Verification

**Files:** Update the existing OpenSpec task checkboxes only after verification succeeds.

- [x] 相关验收条件已整合到 `implement-footbath-v23-protocol`，并通过严格校验。
- [x] Check that `robot_chassis/tests/` and new test targets do not exist.
- [x] Configure and build the RK3506 LVGL9 application with the Buildroot toolchain.
