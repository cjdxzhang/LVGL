#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

// 蓝牙设备信息结构体
typedef struct
{
    char address[18];
    char name[64];
    int rssi;
} ble_device_info_t;

// 回调函数类型：用于通知扫描到的设备
typedef void (*ble_scan_callback_t)(ble_device_info_t *device, void *user_data);

// 初始化蓝牙模块
int ble_manager_init(void);

// 设置本地设备名称（用于广播）
int ble_manager_set_name(const char *name);

// 开启蓝牙广播（让手机能发现）
int ble_manager_start_advertising(void);

// 停止蓝牙广播
int ble_manager_stop_advertising(void);

// 开始扫描附近的蓝牙设备
int ble_manager_start_scan(ble_scan_callback_t callback, void *user_data);

// 停止扫描
int ble_manager_stop_scan(void);

// 获取蓝牙连接状态
bool ble_manager_is_connected(void);

#endif // BLE_MANAGER_H
