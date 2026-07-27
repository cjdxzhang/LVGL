#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stddef.h>

// WiFi 状态回调函数类型
typedef void (*wifi_status_callback_t)(bool connected, const char *ssid, void *user_data);

// 初始化 WiFi 管理模块
void wifi_manager_init(void);

// 注册状态变化回调（支持多个回调）
int wifi_manager_register_callback(wifi_status_callback_t callback, void *user_data);

// 扫描 WiFi 网络（异步，结果通过回调或单独获取）
int wifi_manager_scan_start(void);

// 获取上次扫描的结果数量
int wifi_manager_get_scan_count(void);

// 获取上次扫描结果中第 index 个的 SSID（拷贝到 buffer）
int wifi_manager_get_scan_ssid(int index, char *buffer, size_t buffer_size);

// 连接指定的 WiFi
int wifi_manager_connect(const char *ssid, const char *psk);

// 获取当前连接状态
bool wifi_manager_is_connected(void);

// 获取当前连接的 SSID（拷贝到 buffer）
int wifi_manager_get_current_ssid(char *buffer, size_t buffer_size);

// 断开当前连接
int wifi_manager_disconnect(void);

// 保存当前配置到 /etc/wpa_supplicant.conf 并设置开机自启
int wifi_manager_persist_config(const char *ssid, const char *psk);

#endif // WIFI_MANAGER_H