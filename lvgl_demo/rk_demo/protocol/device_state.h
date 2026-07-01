#ifndef __DEVICE_STATE_H__
#define __DEVICE_STATE_H__

#include <stdbool.h>
#include <stdint.h>
#include "protocol.h"

#define DEVICE_FAULT_TEXT_LEN 256

typedef struct {
    protocol_frame_t frame;
    bool base_connected;
    bool comm_timeout;
    uint32_t last_valid_ms;
    uint32_t bucket_faults;
    uint32_t base_faults;
} device_state_t;

typedef void (*device_state_listener_t)(const device_state_t *state, void *user_data);

void device_state_init(void);
void device_state_update_from_frame(const protocol_frame_t *frame);
void device_state_get(device_state_t *out);
void device_state_check_timeout(uint32_t now_ms);
void device_state_register_listener(device_state_listener_t listener, void *user_data);
void device_state_fault_summary(char *buf, int len);

#endif
