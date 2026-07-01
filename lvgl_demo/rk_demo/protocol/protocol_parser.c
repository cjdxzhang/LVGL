#include "protocol.h"
#include "device_control.h"
#include "device_state.h"
#include <string.h>
#include <lvgl/lvgl.h>

static uint8_t s_stream_buf[FRAME_LEN * 4];
static uint16_t s_stream_len;

static int find_frame_head(const uint8_t *data, int len)
{
    for (int i = 0; i <= len - 2; i++) {
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

bool protocol_verify_frame(const uint8_t *raw, int raw_len)
{
    if (raw == NULL || raw_len < FRAME_LEN) {
        return false;
    }

    if (raw[PROTO_OFFSET_HEAD1] != FRAME_HEAD1 || 
        raw[PROTO_OFFSET_HEAD2] != FRAME_HEAD2) {
        return false;
    }

    uint8_t calc = protocol_calc_checksum(raw + PROTO_CHECKSUM_START,
                                          PROTO_CHECKSUM_LEN);
    if (calc != raw[PROTO_OFFSET_CHECKSUM]) {
        LV_LOG_WARN("[PROTO] checksum mismatch: calc=0x%02X recv=0x%02X",
                    calc, raw[PROTO_OFFSET_CHECKSUM]);
        return false;
    }

    return true;
}

bool protocol_parse(const uint8_t *raw, int raw_len, protocol_frame_t *frame)
{
    if (raw_len < FRAME_LEN || frame == NULL) {
        return false;
    }

    int offset = find_frame_head(raw, raw_len);
    if (offset < 0) {
        return false;
    }

    const uint8_t *p = raw + offset;

    if (!protocol_verify_frame(p, FRAME_LEN)) {
        return false;
    }

    memset(frame, 0, sizeof(protocol_frame_t));

    frame->link_mode        = (link_mode_t)p[PROTO_OFFSET_LINK_MODE];
    frame->cmd_type         = (cmd_type_t)p[PROTO_OFFSET_CMD_TYPE];
    frame->link_status      = p[PROTO_OFFSET_LINK_STATUS];

    frame->bucket_water     = p[PROTO_OFFSET_BUCKET_WATER];
    frame->bucket_temp      = p[PROTO_OFFSET_BUCKET_TEMP];
    frame->bucket_massage   = p[PROTO_OFFSET_BUCKET_MASSAGE];
    frame->bucket_uv        = p[PROTO_OFFSET_BUCKET_UV];
    frame->bucket_timer     = p[PROTO_OFFSET_BUCKET_TIMER];
    frame->bucket_volt      = ((uint16_t)p[PROTO_OFFSET_BUCKET_VOLT_H] << 8) |
                                p[PROTO_OFFSET_BUCKET_VOLT_L];
    frame->bucket_main_status = p[PROTO_OFFSET_BUCKET_MAIN_STATUS];
    frame->bucket_sub_status  = p[PROTO_OFFSET_BUCKET_SUB_STATUS];
    frame->bucket_err1      = p[PROTO_OFFSET_BUCKET_ERR1];
    frame->bucket_err2      = p[PROTO_OFFSET_BUCKET_ERR2];

    frame->base_water_supply  = p[PROTO_OFFSET_BASE_WATER_SUPPLY];
    frame->base_spray_hot_t   = p[PROTO_OFFSET_BASE_SPRAY_HOT_T];
    frame->base_spray_cold_t  = p[PROTO_OFFSET_BASE_SPRAY_COLD_T];
    frame->base_dry_time      = p[PROTO_OFFSET_BASE_DRY_TIME];
    frame->base_herb1         = p[PROTO_OFFSET_BASE_HERB1];
    frame->base_herb2         = p[PROTO_OFFSET_BASE_HERB2];
    frame->base_cleaner       = p[PROTO_OFFSET_BASE_CLEANER];
    frame->base_main_status   = p[PROTO_OFFSET_BASE_MAIN_STATUS];
    frame->base_sub_status    = p[PROTO_OFFSET_BASE_SUB_STATUS];
    frame->base_err1          = p[PROTO_OFFSET_BASE_ERR1];
    frame->base_err2          = p[PROTO_OFFSET_BASE_ERR2];
    frame->base_ambient_led   = p[PROTO_OFFSET_BASE_AMBIENT_LED];

    frame->ext_byte           = p[PROTO_OFFSET_EXT_BYTE];

    return true;
}

void protocol_stream_reset(void)
{
    s_stream_len = 0;
}

static void compact_stream(uint16_t offset)
{
    if (offset >= s_stream_len) {
        s_stream_len = 0;
        return;
    }

    memmove(s_stream_buf, s_stream_buf + offset, s_stream_len - offset);
    s_stream_len -= offset;
}

int protocol_stream_input(const uint8_t *data, uint16_t len,
                          protocol_frame_cb_t cb, void *user_data)
{
    int count = 0;

    if (data == NULL || len == 0 || cb == NULL) {
        return 0;
    }

    for (uint16_t i = 0; i < len; i++) {
        if (s_stream_len >= sizeof(s_stream_buf)) {
            LV_LOG_WARN("[PROTO] stream overflow, reset buffer");
            protocol_stream_reset();
        }
        s_stream_buf[s_stream_len++] = data[i];
    }

    while (s_stream_len >= 2) {
        int head = find_frame_head(s_stream_buf, s_stream_len);
        if (head < 0) {
            uint8_t keep = (s_stream_buf[s_stream_len - 1] == FRAME_HEAD1);
            s_stream_buf[0] = s_stream_buf[s_stream_len - 1];
            s_stream_len = keep ? 1 : 0;
            break;
        }

        if (head > 0) {
            LV_LOG_WARN("[PROTO] drop %d noise bytes before frame", head);
            compact_stream((uint16_t)head);
        }

        if (s_stream_len < FRAME_LEN) {
            break;
        }

        if (protocol_verify_frame(s_stream_buf, FRAME_LEN)) {
            protocol_frame_t frame;
            if (protocol_parse(s_stream_buf, FRAME_LEN, &frame)) {
                cb(&frame, user_data);
                count++;
            }
            compact_stream(FRAME_LEN);
        } else {
            compact_stream(1);
        }
    }

    return count;
}

void protocol_init(void)
{
    protocol_stream_reset();
    device_state_init();
    /* 当前阶段暂不启用持久化恢复。 */
    device_control_load_settings();
    /* 当前阶段启动时不跑协议自检，避免影响设备正常启动节奏。 */
    /* protocol_selftest_run(); */
    device_control_init();
}

void protocol_deinit(void)
{
    device_control_deinit();
    protocol_stream_reset();
}
