#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdbool.h>

// ========== 串口通讯初始化 ==========
void serial_init(void);

// ========== 桶体命令 (0xA0-0xA9)：每次调用只发送一次 ==========
void serial_bucket_shutdown(void);       // 0xA0 桶体关机
void serial_bucket_standby(void);        // 0xA1 桶体待机
void serial_bucket_heat(void);           // 0xA2 运行恒温(保温)
void serial_bucket_stop_heat(void);      // 0xA3 停止恒温(不保温)
void serial_bucket_massage(void);        // 0xA4 运行按摩 (使用 massage_intensity 全局变量)
void serial_bucket_uv(void);             // 0xA5 运行UV灯 (使用 sterilization 全局变量)
void serial_bucket_timer(void);          // 0xA6 运行定时 (使用 timer_set 全局变量)
void serial_bucket_stop_all(void);       // 0xA7 停止所有电器件
void serial_bucket_self_check(void);     // 0xA8 桶体自检
void serial_bucket_low_power(void);      // 0xA9 进入低电保护

// ========== 基站命令 (0xB0-0xB8)：每次调用只发送一次 ==========
void serial_base_shutdown(void);                             // 0xB0 基站关机
void serial_base_standby(void);                              // 0xB1 基站待机
void serial_base_water(void);                                // 0xB2 自动注水（使用温度、水位全局变量）
void serial_base_auto_clean();                               // 0xB3 自清洁
                            

void serial_base_force_drain(void);                          // 0xB4 强制排水
void serial_base_clean_spray(uint8_t minutes);               // 0xB5 清洁喷淋(热水+清洁液)
void serial_base_clean_water_spray(uint8_t minutes);         // 0xB6 清水喷淋
void serial_base_hot_dry(uint8_t time_x10min);               // 0xB7 热风烘干
void serial_base_self_check(void);                           // 0xB8 基站自检

#endif
