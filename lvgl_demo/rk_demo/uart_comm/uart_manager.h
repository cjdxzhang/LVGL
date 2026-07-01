#ifndef __UART_MANAGER_H__
#define __UART_MANAGER_H__

#include <stdint.h>
#include <stddef.h>

/**
 * @brief 串口接收回调函数类型
 * @param data 接收到的数据
 * @param len 数据长度
 */
typedef void (*uart_recv_cb_t)(uint8_t *data, uint16_t len);

/**
 * @brief 初始化串口并启动接收线程
 * @param device 串口设备路径 (当前实现默认使用 /dev/ttyS3)
 * @param baudrate 波特率，如 115200
 * @return 0 成功，-1 失败
 */
int uart_manager_init(const char *device, int baudrate);

/**
 * @brief 注册接收回调（当串口收到数据并解析完成后调用）
 */
void uart_manager_register_cb(uart_recv_cb_t cb);

/**
 * @brief 发送原始数据到串口
 */
int uart_manager_send(uint8_t *data, uint16_t len);

/**
 * @brief 关闭串口并停止线程
 */
void uart_manager_deinit(void);

/**
 * @brief 格式化打印串口数据
 */
void serial_print_buf(const char *direction, unsigned char *buf, int len);

#endif
