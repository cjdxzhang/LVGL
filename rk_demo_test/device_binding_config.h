#ifndef DEVICE_BINDING_CONFIG_H
#define DEVICE_BINDING_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#include "cJSON.h"

#define DEVICE_BINDING_USER_ID_MAX    128
#define DEVICE_BINDING_WIFI_SSID_MAX   64
#define DEVICE_BINDING_MQTT_JSON_MAX 1024

typedef struct
{
    char user_id[DEVICE_BINDING_USER_ID_MAX];
    char wifi_ssid[DEVICE_BINDING_WIFI_SSID_MAX];
    char mqtt_json[DEVICE_BINDING_MQTT_JSON_MAX];
    bool mqtt_configured;
    time_t updated_at;
} device_binding_config_t;

int device_binding_config_save(const char *user_id, const char *wifi_ssid,
                               const cJSON *mqtt_obj);
int device_binding_config_load(device_binding_config_t *config);

#endif // DEVICE_BINDING_CONFIG_H
