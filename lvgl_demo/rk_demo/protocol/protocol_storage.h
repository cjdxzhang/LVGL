#ifndef __PROTOCOL_STORAGE_H__
#define __PROTOCOL_STORAGE_H__

#include <stdbool.h>
#include <stdint.h>

#define FOOTBATH_STORAGE_DIR "/userdata/footbath"
#define FOOTBATH_STORAGE_FILE FOOTBATH_STORAGE_DIR "/settings.conf"

typedef struct {
    uint8_t default_water_level;
    uint8_t default_temp;
    uint8_t default_massage;
    uint8_t default_timer;
    char uart_device[32];
    int baudrate;
} footbath_settings_t;

void protocol_storage_defaults(footbath_settings_t *settings);
bool protocol_storage_load(footbath_settings_t *settings);
bool protocol_storage_save(const footbath_settings_t *settings);

#endif
