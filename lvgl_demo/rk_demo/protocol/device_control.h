#ifndef __DEVICE_CONTROL_H__
#define __DEVICE_CONTROL_H__

#include <stdbool.h>
#include <stdint.h>
#include "protocol.h"
#include "protocol_storage.h"

typedef enum {
    DEVICE_CONTROL_OK = 0,
    DEVICE_CONTROL_ERR_PARAM = -1,
    DEVICE_CONTROL_ERR_SEND = -2,
} device_control_result_t;

void device_control_init(void);
void device_control_deinit(void);
void device_control_load_settings(void);
bool device_control_get_settings(footbath_settings_t *settings);
bool device_control_save_settings(const footbath_settings_t *settings);

int device_control_bucket_power_off(void);
int device_control_bucket_standby(void);
int device_control_bucket_heat_on(uint8_t temp);
int device_control_bucket_heat_off(void);
int device_control_bucket_massage(uint8_t level);
int device_control_bucket_uv(uint8_t on);
int device_control_bucket_timer(uint8_t timer);
int device_control_bucket_stop(void);
int device_control_bucket_selfcheck(void);
int device_control_bucket_low_batt(void);
int device_control_bucket_auto_warehouse(void);

int device_control_base_power_off(void);
int device_control_base_standby(void);
int device_control_base_auto_fill(uint8_t water_level, uint8_t temp,
                                  uint8_t herb1, uint8_t herb2);
int device_control_base_self_clean(uint8_t hot_spray_min,
                                   uint8_t cold_spray_min,
                                   uint8_t dry_units);
int device_control_base_start_cached(uint8_t water_level, uint8_t temp,
                                     uint8_t herb1, uint8_t herb2,
                                     uint8_t timer);
int device_control_base_force_drain(void);
int device_control_base_hot_spray(uint8_t hot_spray_min);
int device_control_base_cold_spray(uint8_t cold_spray_min);
int device_control_base_dry(uint8_t dry_units);
int device_control_base_selfcheck(void);
int device_control_system_reset(void);

#endif
