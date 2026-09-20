#ifndef TCP_SERVICE_H
#define TCP_SERVICE_H

#include <stdbool.h>

typedef struct cJSON cJSON;

// 位置结构体
typedef struct
{
    double x;
    double y;
    double z;
    int powquantity;
    double power;
    double velSpeed;
    double velAngle;
    int emgStop;
    bool inbuildmap;
    bool innavmap;
    char mapname[64];
    char timestamp[32];
} robot_position_t;

int tcp_service_init(void);
void tcp_service_deinit(void);
int tcp_service_send_proxy_json(cJSON *json_message);
/* 仅由LVGL主循环调用，消费TCP线程提交的页面业务。 */
void tcp_service_process_main_thread_actions(void);

// 获取最新机器人位置
bool tcp_service_get_robot_position(robot_position_t *pos);

#endif
