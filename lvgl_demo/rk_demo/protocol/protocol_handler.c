#include "protocol.h"
#include "uart_manager.h"
#include <lvgl/lvgl.h>
#include <string.h>
#include <stdlib.h>

static cmd_handler_t g_cmd_handlers[CMD_MAX] = {0};

void protocol_register_cmd(cmd_type_t cmd, cmd_handler_t handler)
{
    if (cmd < CMD_MAX) {
        g_cmd_handlers[cmd] = handler;
    }
}

static void protocol_notify(const protocol_frame_t *frame)
{
    if (frame->cmd_type >= CMD_MAX) {
        LV_LOG_WARN("[PROTO] cmd out of range: 0x%02X", frame->cmd_type);
        return;
    }
    cmd_handler_t handler = g_cmd_handlers[frame->cmd_type];
    if (handler) {
        handler(frame);
    } else {
        LV_LOG_WARN("[PROTO] unhandled cmd: 0x%02X", frame->cmd_type);
    }
}

typedef struct {
    uint8_t *data;
    uint16_t len;
} proto_msg_t;

static void protocol_dispatch_async(void *arg)
{
    proto_msg_t *msg = (proto_msg_t *)arg;
    protocol_frame_t frame;
    if (protocol_parse(msg->data, msg->len, &frame)) {
        protocol_notify(&frame);
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
