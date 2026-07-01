#include "net_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <lvgl/lvgl.h>

static int sock_fd = -1;
static pthread_t recv_thread;
static int thread_run = 0;
static net_recv_cb_t user_cb = NULL;

static void *net_recv_thread_func(void *arg)
{
    uint8_t buffer[256];

    LV_LOG_USER("[NET] recv thread started");

    while (thread_run) {
        int len = recv(sock_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (len > 0) {
            LV_LOG_USER("[NET] recv %d bytes", len);
            if (user_cb) {
                user_cb(buffer, (uint16_t)len);
            }
        } else if (len == 0) {
            LV_LOG_WARN("[NET] connection closed");
            break;
        }
        usleep(10000);
    }
    return NULL;
}

int net_manager_init(const char *ip, int port)
{
    struct sockaddr_in addr;

    LV_LOG_USER("[NET] connecting to %s:%d", ip, port);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        LV_LOG_ERROR("[NET] socket create failed");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LV_LOG_ERROR("[NET] connect failed");
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }

    thread_run = 1;
    if (pthread_create(&recv_thread, NULL, net_recv_thread_func, NULL) != 0) {
        LV_LOG_ERROR("[NET] recv thread create failed");
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }

    LV_LOG_USER("[NET] connected");
    return 0;
}

void net_manager_register_cb(net_recv_cb_t cb)
{
    user_cb = cb;
}

int net_manager_send(uint8_t *data, uint16_t len)
{
    if (sock_fd < 0) {
        return -1;
    }
    return send(sock_fd, data, len, 0);
}

void net_manager_deinit(void)
{
    thread_run = 0;
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
    }
    pthread_join(recv_thread, NULL);
}
