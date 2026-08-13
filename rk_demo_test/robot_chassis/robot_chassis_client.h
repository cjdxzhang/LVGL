#ifndef ROBOT_CHASSIS_CLIENT_H
#define ROBOT_CHASSIS_CLIENT_H

#include <sys/types.h>

#include "robot_chassis_types.h"

typedef ssize_t (*robot_chassis_send_fn_t)(int fd, const void *buffer, size_t length,
        int flags);

typedef struct
{
    int fd;
    char host[64];
    uint16_t port;
} robot_chassis_client_t;

int robot_chassis_client_init(robot_chassis_client_t *client, const char *host, uint16_t port);
int robot_chassis_client_connect(robot_chassis_client_t *client, int timeout_ms);
void robot_chassis_client_close(robot_chassis_client_t *client);
ssize_t robot_chassis_client_receive(robot_chassis_client_t *client, void *buffer, size_t length);
int robot_chassis_client_send_request(robot_chassis_client_t *client, const char *json,
                                      size_t length);
int robot_chassis_client_send_all_for_test(int fd, const uint8_t *data, size_t length,
        robot_chassis_send_fn_t send_fn);

#endif
