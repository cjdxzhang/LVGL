#include "protocol.h"
#include "uart_manager.h"
#include <string.h>
#include <lvgl/lvgl.h>

void protocol_build(const protocol_frame_t *frame, uint8_t *out)
{
    memset(out, 0, FRAME_LEN);

    out[PROTO_OFFSET_HEAD1] = FRAME_HEAD1;
    out[PROTO_OFFSET_HEAD2] = FRAME_HEAD2;
    out[PROTO_OFFSET_LINK_MODE] = (uint8_t)frame->link_mode;
    out[PROTO_OFFSET_CMD_TYPE] = (uint8_t)frame->cmd_type;
    out[PROTO_OFFSET_LINK_STATUS] = frame->link_status;

    out[PROTO_OFFSET_BUCKET_WATER] = frame->bucket_water;
    out[PROTO_OFFSET_BUCKET_TEMP] = frame->bucket_temp;
    out[PROTO_OFFSET_BUCKET_MASSAGE] = frame->bucket_massage;
    out[PROTO_OFFSET_BUCKET_UV] = frame->bucket_uv;
    out[PROTO_OFFSET_BUCKET_TIMER] = frame->bucket_timer;
    out[PROTO_OFFSET_BUCKET_VOLT_H] = (uint8_t)(frame->bucket_volt >> 8);
    out[PROTO_OFFSET_BUCKET_VOLT_L] = (uint8_t)(frame->bucket_volt & 0xFF);
    out[PROTO_OFFSET_BUCKET_MAIN_STATUS] = frame->bucket_main_status;
    out[PROTO_OFFSET_BUCKET_SUB_STATUS] = frame->bucket_sub_status;
    out[PROTO_OFFSET_BUCKET_ERR1] = frame->bucket_err1;
    out[PROTO_OFFSET_BUCKET_ERR2] = frame->bucket_err2;

    out[PROTO_OFFSET_BASE_WATER_SUPPLY] = frame->base_water_supply;
    out[PROTO_OFFSET_BASE_SPRAY_HOT_T] = frame->base_spray_hot_t;
    out[PROTO_OFFSET_BASE_SPRAY_COLD_T] = frame->base_spray_cold_t;
    out[PROTO_OFFSET_BASE_DRY_TIME] = frame->base_dry_time;
    out[PROTO_OFFSET_BASE_HERB1] = frame->base_herb1;
    out[PROTO_OFFSET_BASE_HERB2] = frame->base_herb2;
    out[PROTO_OFFSET_BASE_CLEANER] = frame->base_cleaner;
    out[PROTO_OFFSET_BASE_MAIN_STATUS] = frame->base_main_status;
    out[PROTO_OFFSET_BASE_SUB_STATUS] = frame->base_sub_status;
    out[PROTO_OFFSET_BASE_ERR1] = frame->base_err1;
    out[PROTO_OFFSET_BASE_ERR2] = frame->base_err2;
    out[PROTO_OFFSET_BASE_AMBIENT_LED] = frame->base_ambient_led;

    out[PROTO_OFFSET_EXT_BYTE] = frame->ext_byte;

    out[PROTO_OFFSET_CHECKSUM] = protocol_calc_checksum(out + PROTO_CHECKSUM_START,
                                                        PROTO_CHECKSUM_LEN);
}

int protocol_send_uart(const protocol_frame_t *frame)
{
    if (frame == NULL) {
        return -1;
    }

    uint8_t buf[FRAME_LEN];
    protocol_build(frame, buf);
    LV_LOG_USER("[PROTO] UART send: cmd=0x%02X link=%d", frame->cmd_type, frame->link_mode);
    return uart_manager_send(buf, FRAME_LEN);
}

int protocol_send_net(const protocol_frame_t *frame)
{
    uint8_t buf[FRAME_LEN];
    protocol_build(frame, buf);
    LV_LOG_USER("[PROTO] NET send: cmd=0x%02X link=%d", frame->cmd_type, frame->link_mode);
    return 0;
}
