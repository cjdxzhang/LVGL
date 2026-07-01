#include "protocol_selftest.h"
#include "protocol.h"
#include "device_control.h"
#include "device_state.h"
#include <string.h>
#include <lvgl/lvgl.h>

static int expect_true(bool value, const char *name)
{
    if (!value) {
        LV_LOG_WARN("[PROTO-TEST] failed: %s", name);
        return -1;
    }
    return 0;
}

static bool selftest_stream_cb(const protocol_frame_t *frame, void *user_data)
{
    int *count = (int *)user_data;
    if (frame->cmd_type == CMD_BUCKET_MASSAGE) {
        (*count)++;
    }
    return true;
}

static bool selftest_state_cb(const protocol_frame_t *frame, void *user_data)
{
    int *count = (int *)user_data;
    device_state_update_from_frame(frame);
    (*count)++;
    return true;
}

static int expect_bytes(const uint8_t *actual, const uint8_t *expected,
                        int len, const char *name)
{
    if (memcmp(actual, expected, len) != 0) {
        LV_LOG_WARN("[PROTO-TEST] failed: %s", name);
        return -1;
    }
    return 0;
}

int protocol_selftest_run(void)
{
    static const uint8_t expected_idle[FRAME_LEN] = {
        0x55, 0xAA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    };
    uint8_t raw[FRAME_LEN];
    protocol_frame_t in;
    protocol_frame_t out;
    int errors = 0;

    memset(&in, 0, sizeof(in));
    in.link_mode = LINK_MODE_MCU_COMMAND;
    in.cmd_type = CMD_IDLE;
    protocol_build(&in, raw);
    errors += expect_bytes(raw, expected_idle, FRAME_LEN,
                           "idle command matches V2.3 test case");

    memset(&in, 0, sizeof(in));
    in.link_mode = LINK_MODE_MCU_COMMAND;
    in.cmd_type = CMD_BUCKET_MASSAGE;
    in.link_status = 0;
    in.bucket_water = 8;
    in.bucket_temp = 42;
    in.bucket_massage = 2;
    in.bucket_uv = 1;
    in.bucket_timer = 3;

    protocol_build(&in, raw);
    errors += expect_true(raw[0] == FRAME_HEAD1 && raw[1] == FRAME_HEAD2,
                          "frame header");
    errors += expect_true(protocol_verify_frame(raw, FRAME_LEN),
                          "verify built frame");
    errors += expect_true(protocol_parse(raw, FRAME_LEN, &out),
                          "parse built frame");
    errors += expect_true(out.cmd_type == CMD_BUCKET_MASSAGE &&
                            out.bucket_temp == 42 &&
                            out.bucket_massage == 2,
                            "round trip fields");

    raw[PROTO_OFFSET_CHECKSUM] ^= 0x01;
    errors += expect_true(!protocol_verify_frame(raw, FRAME_LEN),
                          "reject bad checksum");
    raw[PROTO_OFFSET_CHECKSUM] ^= 0x01;

    protocol_stream_reset();
    {
        uint8_t split1[10];
        uint8_t split2[FRAME_LEN - 10];
        int count = 0;

        memcpy(split1, raw, sizeof(split1));
        memcpy(split2, raw + sizeof(split1), sizeof(split2));
        protocol_stream_input(split1, sizeof(split1), selftest_stream_cb, &count);
        protocol_stream_input(split2, sizeof(split2), selftest_stream_cb, &count);
        errors += expect_true(count == 1, "stream split frame");
    }

    errors += expect_true(device_control_bucket_massage(4) == DEVICE_CONTROL_ERR_PARAM,
                          "reject invalid massage");

    protocol_stream_reset();
    device_state_init();
    {
        device_state_t state;
        int count = 0;

        memset(&in, 0, sizeof(in));
        in.link_mode = LINK_MODE_MCU_COMMAND;
        in.cmd_type = CMD_IDLE;
        in.link_status = 0x00;
        protocol_build(&in, raw);
        protocol_stream_input(raw, FRAME_LEN, selftest_state_cb, &count);
        device_state_get(&state);
        errors += expect_true(count == 1 && !state.base_connected &&
                                !state.comm_timeout &&
                                state.frame.link_status == 0x00,
                                "link_status 0 updates disconnected state");

        in.link_status = 0x01;
        protocol_build(&in, raw);
        protocol_stream_input(raw, FRAME_LEN, selftest_state_cb, &count);
        device_state_get(&state);
        errors += expect_true(count == 2 && state.base_connected &&
                                state.frame.link_status == 0x01,
                                "link_status 1 updates connected state");

        raw[PROTO_OFFSET_CHECKSUM] ^= 0x01;
        protocol_stream_input(raw, FRAME_LEN, selftest_state_cb, &count);
        errors += expect_true(count == 2, "bad checksum does not update state");
    }

    if (errors == 0) {
        LV_LOG_USER("[PROTO-TEST] selftest passed");
    }

    return errors;
}
