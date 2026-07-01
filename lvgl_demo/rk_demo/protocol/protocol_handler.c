#include "protocol.h"
#include "device_state.h"
#include "uart_manager.h"
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint8_t *data;
    uint16_t len;
} proto_msg_t;

static void log_rx_bytes(const uint8_t *data, uint16_t len)
{
    char hex[128] = {0};
    uint16_t max = len > 30 ? 30 : len;

    for (uint16_t i = 0; i < max; i++) {
        snprintf(hex + strlen(hex), sizeof(hex) - strlen(hex),
                 "%02X%s", data[i], i + 1 < max ? " " : "");
    }

    LV_LOG_USER("[PROTO] UART recv %u bytes: %s%s",
                len, hex, len > max ? " ..." : "");
}

static bool handle_parsed_frame(const protocol_frame_t *frame, void *user_data)
{
    LV_UNUSED(user_data);

    if (frame == NULL) {
        return false;
    }

    LV_LOG_USER("[PROTO] parsed frame: cmd=0x%02X link_status=0x%02X bucket_status=0x%02X base_status=0x%02X",
                frame->cmd_type,
                frame->link_status,
                frame->bucket_main_status,
                frame->base_main_status);

    device_state_update_from_frame(frame);

    return true;
}

static void protocol_dispatch_async(void *arg)
{
    proto_msg_t *msg = (proto_msg_t *)arg;

    log_rx_bytes(msg->data, msg->len);
    int parsed = protocol_stream_input(msg->data, msg->len,
                                       handle_parsed_frame, NULL);
    if (parsed > 0) {
        LV_LOG_USER("[PROTO] parsed %d frame(s) from UART chunk", parsed);
    } else {
        LV_LOG_USER("[PROTO] UART chunk buffered or ignored, no complete valid frame");
    }

    free(msg->data);
    free(msg);
}

static void uart_recv_callback(uint8_t *data, uint16_t len)
{
    if (len > 0) {
        proto_msg_t *msg = malloc(sizeof(proto_msg_t));
        if (msg) {
            msg->data = malloc(len);
            if (msg->data) {
                memcpy(msg->data, data, len);
                msg->len = len;
                lv_async_call(protocol_dispatch_async, msg);
            } else {
                free(msg);
            }
        }
    }
}

void protocol_handler_init(void)
{
    uart_manager_register_cb(uart_recv_callback);
    LV_LOG_USER("[PROTO] handler initialized");
}

void protocol_handler_deinit(void)
{
    uart_manager_register_cb(NULL);
    protocol_stream_reset();
}
