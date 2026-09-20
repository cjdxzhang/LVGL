#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdbool.h>

#define SERIAL_FRAME_LEN 30u

#define SERIAL_LIQUID_SHORTAGE_DRUG1 0x01u
#define SERIAL_LIQUID_SHORTAGE_DRUG2 0x02u
#define SERIAL_LIQUID_SHORTAGE_CLEAN 0x04u
#define SERIAL_LIQUID_SHORTAGE_MASK  0x07u

#define SERIAL_BUCKET_ERROR1_LACK_WATER          0x01u
#define SERIAL_BUCKET_ERROR1_DRAIN_TIMEOUT       0x02u
#define SERIAL_BUCKET_ERROR1_TEMPERATURE         0x04u
#define SERIAL_BUCKET_ERROR1_HEATING_TIMEOUT     0x08u
#define SERIAL_BUCKET_ERROR1_TEMP_SENSOR         0x10u
#define SERIAL_BUCKET_ERROR1_MASK                0x1Fu

#define SERIAL_BUCKET_ERROR2_WATER_PUMP          0x02u
#define SERIAL_BUCKET_ERROR2_MASSAGE_MOTOR       0x08u
#define SERIAL_BUCKET_ERROR2_BATTERY_VOLTAGE     0x20u
#define SERIAL_BUCKET_ERROR2_MASK                0x2Au

#define SERIAL_BASE_ERROR1_LIQUID_SENSOR_BOARD   0x08u
#define SERIAL_BASE_ERROR1_MASK                  0x08u

typedef struct
{
    uint8_t bucket_error1;  /* data[14] Bit0-Bit4 */
    uint8_t bucket_error2;  /* data[15] Bit1/Bit3/Bit5 */
    uint8_t base_error1;    /* data[25] Bit3 */
} serial_mcu_error_state_t;

// ========== 串口通讯初始化 ==========
void serial_init(void);
uint32_t serial_mcu_status_report_sequence(
    void);            // 校验通过的 MCU 状态帧累计序号
uint8_t serial_mcu_liquid_shortage_mask(void);               // 最近合法帧 data[25] Bit0-Bit2
bool serial_mcu_error_state_get(serial_mcu_error_state_t *state);
/* 最近有效MCU电量百分比，开机默认100，非法值和断连保留最近值。 */
uint8_t serial_mcu_battery_percent(void);

// ========== 桶体命令 (0xA0-0xA9)：每次调用只发送一次 ==========
void serial_bucket_shutdown(void);       // 0xA0 桶体关机
void serial_bucket_standby(void);        // 0xA1 桶体待机
void serial_bucket_heat(void);           // 0xA2 运行恒温(保温)
void serial_bucket_stop_heat(void);      // 0xA3 停止恒温(不保温)
void serial_bucket_massage(
    void);        // 0xA4 运行按摩 (使用 massage_intensity 全局变量)
void serial_bucket_uv(void);             // 0xA5 运行UV灯 (使用 sterilization 全局变量)
void serial_bucket_timer(void);          // 0xA6 运行定时 (使用 timer_set 全局变量)
void serial_bucket_stop_all(void);       // 0xA7 停止所有电器件
void serial_bucket_self_check(void);     // 0xA8 桶体自检
void serial_bucket_low_power(void);      // 0xA9 进入低电保护

// ========== 基站命令 (0xB0-0xB8)：每次调用只发送一次 ==========
void serial_base_shutdown(void);                             // 0xB0 基站关机
void serial_base_standby(void);                              // 0xB1 基站待机
void serial_base_water(
    void);                                // 0xB2 自动注水（使用温度、水位全局变量）
void serial_base_water_custom(uint8_t water_level_ui,
                              uint8_t herb1_seconds,
                              uint8_t herb2_seconds,
                              uint8_t cleaner_seconds);
void serial_base_auto_clean();                               // 0xB3 自清洁
void serial_base_auto_clean_custom(uint8_t spray_hot_minutes,
                                   uint8_t spray_cold_minutes,
                                   uint8_t dry_time_x10min);


void serial_base_force_drain(void);                          // 0xB4 强制排水
void serial_base_clean_spray(uint8_t minutes);               // 0xB5 清洁喷淋(热水+清洁液)
void serial_base_clean_water_spray(uint8_t minutes);         // 0xB6 清水喷淋
void serial_base_hot_dry(uint8_t time_x10min);               // 0xB7 热风烘干
void serial_base_self_check(void);                           // 0xB8 基站自检
bool serial_get_last_rx_frame(uint8_t frame_out[SERIAL_FRAME_LEN]);
bool serial_get_last_tx_frame(uint8_t frame_out[SERIAL_FRAME_LEN]);

#endif
