#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

#define FRAME_LEN       30
#define FRAME_HEAD1     0x55
#define FRAME_HEAD2     0xAA
#define CMD_MAX         0x1E

typedef enum {
    LINK_MODE_BUCKET_ONLY = 0x01,
    LINK_MODE_TRANSPARENT = 0x02,
    LINK_MODE_DIRECT      = 0x03,
} link_mode_t;

typedef enum {
    CMD_IDLE        = 0x00,

    CMD_BUCKET_STANDBY  = 0xA0,
    CMD_BUCKET_IDLE     = 0xA1,
    CMD_BUCKET_FOOTBATH = 0xA4,
    CMD_BUCKET_SELFCHECK= 0xA7,
    CMD_BUCKET_DRAIN    = 0xA8,

    CMD_BASE_SELFCLEAN  = 0xB3,
    CMD_BASE_DRAIN      = 0xB5,

    CMD_SYSTEM_PAIR     = 0xC0,
    CMD_SYSTEM_OTA      = 0xC1,
} cmd_type_t;

typedef enum {
    BUCKET_STATUS_POWER_ON   = 0x00,
    BUCKET_STATUS_SELFTEST   = 0x01,
    BUCKET_STATUS_STANDBY    = 0x02,
    BUCKET_STATUS_RUNNING    = 0x03,
    BUCKET_STATUS_LOW_BATT   = 0x04,
} bucket_status_t;

typedef enum {
    BASE_STATUS_IDLE              = 0x00,
    BASE_STATUS_AUTO_FILLING      = 0x01,
    BASE_STATUS_FILL_DONE_KEEP    = 0x02,
    BASE_STATUS_FILL_DONE_WAIT    = 0x03,
    BASE_STATUS_CLEAN_SPRAY       = 0x04,
    BASE_STATUS_CLEAN_DRAIN1      = 0x05,
    BASE_STATUS_RINSE_SPRAY       = 0x06,
    BASE_STATUS_CLEAN_DRAIN2      = 0x07,
    BASE_STATUS_DRY               = 0x08,
    BASE_STATUS_FORCE_DRAIN       = 0x09,
    BASE_STATUS_SOLO_CLEAN_SPRAY  = 0x0A,
    BASE_STATUS_SOLO_RINSE_SPRAY  = 0x0B,
    BASE_STATUS_SOLO_DRY          = 0x0C,
} base_status_t;

typedef enum {
    AMBIENT_ORANGE = 0x01,
    AMBIENT_RED    = 0x02,
    AMBIENT_YELLOW = 0x03,
    AMBIENT_GREEN  = 0x04,
    AMBIENT_BLUE   = 0x05,
    AMBIENT_CYAN   = 0x06,
    AMBIENT_PURPLE = 0x07,
    AMBIENT_WHITE  = 0x08,
} ambient_led_t;

#define BUCKET_ERR1_LACK_OF_WATER     (1 << 0)
#define BUCKET_ERR1_DRAIN_TIMEOUT     (1 << 1)
#define BUCKET_ERR1_HIGH_TEMP         (1 << 2)
#define BUCKET_ERR1_HEAT_TIMEOUT      (1 << 3)
#define BUCKET_ERR1_TEMP_SENSOR       (1 << 4)
#define BUCKET_ERR1_WATER_SENSOR      (1 << 5)

#define BUCKET_ERR2_HEATER_FAULT      (1 << 0)
#define BUCKET_ERR2_PUMP_FAULT        (1 << 1)
#define BUCKET_ERR2_VALVE_FAULT       (1 << 2)
#define BUCKET_ERR2_MASSAGE_FAULT     (1 << 3)
#define BUCKET_ERR2_UV_FAULT          (1 << 4)
#define BUCKET_ERR2_BATTERY_FAULT     (1 << 5)

#define BASE_ERR1_HERB1_LACK          (1 << 0)
#define BASE_ERR1_HERB2_LACK          (1 << 1)
#define BASE_ERR1_CLEANER_LACK        (1 << 2)
#define BASE_ERR1_SENSOR_FAULT        (1 << 3)
#define BASE_ERR1_PUMP1_FAULT         (1 << 4)
#define BASE_ERR1_PUMP2_FAULT         (1 << 5)
#define BASE_ERR1_PUMP3_FAULT         (1 << 6)

#define BASE_ERR2_INLET_VALVE_FAULT   (1 << 0)
#define BASE_ERR2_INSTANT_HEAT_FAULT  (1 << 1)
#define BASE_ERR2_SPRAY_MOTOR_FAULT   (1 << 2)
#define BASE_ERR2_DRAIN_PUMP_FAULT    (1 << 3)
#define BASE_ERR2_DRYER_FAULT         (1 << 4)
#define BASE_ERR2_AMBIENT_FAULT       (1 << 5)
#define BASE_ERR2_IR_BOARD_FAULT      (1 << 6)

typedef struct {
    link_mode_t link_mode;
    cmd_type_t  cmd_type;
    uint8_t     link_status;

    uint8_t     bucket_water;
    uint8_t     bucket_temp;
    uint8_t     bucket_massage;
    uint8_t     bucket_uv;
    uint8_t     bucket_timer;
    uint16_t    bucket_volt;
    uint8_t     bucket_main_status;
    uint8_t     bucket_sub_status;
    uint8_t     bucket_err1;
    uint8_t     bucket_err2;

    uint8_t     base_water_supply;
    uint8_t     base_spray_hot_t;
    uint8_t     base_spray_cold_t;
    uint8_t     base_dry_time;
    uint8_t     base_herb1;
    uint8_t     base_herb2;
    uint8_t     base_cleaner;
    uint8_t     base_main_status;
    uint8_t     base_sub_status;
    uint8_t     base_err1;
    uint8_t     base_err2;
    uint8_t     base_ambient_led;

    uint8_t     ext_byte;
} protocol_frame_t;

typedef void (*cmd_handler_t)(const protocol_frame_t *frame);

void protocol_register_cmd(cmd_type_t cmd, cmd_handler_t handler);

uint8_t protocol_calc_checksum(const uint8_t *data, int len);

bool protocol_parse(const uint8_t *raw, int raw_len, protocol_frame_t *frame);

void protocol_build(const protocol_frame_t *frame, uint8_t *out);

int protocol_send_uart(const protocol_frame_t *frame);
int protocol_send_net(const protocol_frame_t *frame);

void protocol_init(void);
void protocol_deinit(void);

void protocol_handler_init(void);

#endif
