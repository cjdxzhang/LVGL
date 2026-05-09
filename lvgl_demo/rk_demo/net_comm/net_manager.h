#ifndef __NET_MANAGER_H__
#define __NET_MANAGER_H__

#include <stdint.h>

typedef void (*net_recv_cb_t)(uint8_t *data, uint16_t len);

int net_manager_init(const char *ip, int port);

void net_manager_register_cb(net_recv_cb_t cb);

int net_manager_send(uint8_t *data, uint16_t len);

void net_manager_deinit(void);

#endif
