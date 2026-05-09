#include "protocol.h"
#include <string.h>
#include <lvgl/lvgl.h>

static int find_frame_head(const uint8_t *data, int len)
{
    for (int i = 0; i <= len - FRAME_LEN; i++) {
        if (data[i] == FRAME_HEAD1 && data[i + 1] == FRAME_HEAD2) {
            return i;
        }
    }
    return -1;
}

uint8_t protocol_calc_checksum(const uint8_t *data, int len)
{
    uint32_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

bool protocol_parse(const uint8_t *raw, int raw_len, protocol_frame_t *frame)
{
    if (raw_len < FRAME_LEN || frame == NULL)
        return false;

    int offset = find_frame_head(raw, raw_len);
    if (offset < 0)
        return false;

    const uint8_t *p = raw + offset;

    uint8_t calc = protocol_calc_checksum(p + 2, FRAME_LEN - 3);
    if (calc != p[FRAME_LEN - 1]) {
        LV_LOG_WARN("[PROTO] checksum mismatch: calc=0x%02X recv=0x%02X", calc, p[FRAME_LEN - 1]);
        return false;
    }

    memset(frame, 0, sizeof(protocol_frame_t));

    frame->link_mode        = (link_mode_t)p[2];
    frame->cmd_type         = (cmd_type_t)p[3];
    frame->link_status      = p[4];

    frame->bucket_water     = p[5];
    frame->bucket_temp      = p[6];
    frame->bucket_massage   = p[7];
    frame->bucket_uv        = p[8];
    frame->bucket_timer     = p[9];
    frame->bucket_volt      = ((uint16_t)p[10] << 8) | p[11];
    frame->bucket_main_status = p[12];
    frame->bucket_sub_status  = p[13];
    frame->bucket_err1      = p[14];
    frame->bucket_err2      = p[15];

    frame->base_water_supply  = p[16];
    frame->base_spray_hot_t   = p[17];
    frame->base_spray_cold_t  = p[18];
    frame->base_dry_time      = p[19];
    frame->base_herb1         = p[20];
    frame->base_herb2         = p[21];
    frame->base_cleaner       = p[22];
    frame->base_main_status   = p[23];
    frame->base_sub_status    = p[24];
    frame->base_err1          = p[25];
    frame->base_err2          = p[26];
    frame->base_ambient_led   = p[27];

    frame->ext_byte           = p[28];

    return true;
}

void protocol_init(void)
{
}

void protocol_deinit(void)
{
}
