#include "protocol.h"
#include "uart_manager.h"
#include <string.h>
#include <lvgl/lvgl.h>

void protocol_build(const protocol_frame_t *frame, uint8_t *out)
{
    memset(out, 0, FRAME_LEN);

    out[0] = FRAME_HEAD1;
    out[1] = FRAME_HEAD2;
    out[2] = (uint8_t)frame->link_mode;
    out[3] = (uint8_t)frame->cmd_type;
    out[4] = frame->link_status;

    out[5] = frame->bucket_water;
    out[6] = frame->bucket_temp;
    out[7] = frame->bucket_massage;
    out[8] = frame->bucket_uv;
    out[9] = frame->bucket_timer;
    out[10] = (uint8_t)(frame->bucket_volt >> 8);
    out[11] = (uint8_t)(frame->bucket_volt & 0xFF);
    out[12] = frame->bucket_main_status;
    out[13] = frame->bucket_sub_status;
    out[14] = frame->bucket_err1;
    out[15] = frame->bucket_err2;

    out[16] = frame->base_water_supply;
    out[17] = frame->base_spray_hot_t;
    out[18] = frame->base_spray_cold_t;
    out[19] = frame->base_dry_time;
    out[20] = frame->base_herb1;
    out[21] = frame->base_herb2;
    out[22] = frame->base_cleaner;
    out[23] = frame->base_main_status;
    out[24] = frame->base_sub_status;
    out[25] = frame->base_err1;
    out[26] = frame->base_err2;
    out[27] = frame->base_ambient_led;

    out[28] = frame->ext_byte;

    out[29] = protocol_calc_checksum(out + 2, FRAME_LEN - 3);
}

int protocol_send_uart(const protocol_frame_t *frame)
{
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
