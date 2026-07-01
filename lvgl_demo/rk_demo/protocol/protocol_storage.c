#include "protocol_storage.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <lvgl/lvgl.h>

void protocol_storage_defaults(footbath_settings_t *settings)
{
    if (settings == NULL) {
        return;
    }

    memset(settings, 0, sizeof(*settings));
    settings->default_water_level = 8;
    settings->default_temp = 42;
    settings->default_massage = 2;
    settings->default_timer = 3;
    snprintf(settings->uart_device, sizeof(settings->uart_device), "/dev/ttyS3");
    settings->baudrate = 115200;
}

static int ensure_storage_dir(void)
{
    if (access(FOOTBATH_STORAGE_DIR, F_OK) == 0) {
        return 0;
    }
    if (mkdir(FOOTBATH_STORAGE_DIR, 0755) == 0 || errno == EEXIST) {
        return 0;
    }
    return -1;
}

bool protocol_storage_load(footbath_settings_t *settings)
{
    FILE *fp;
    char key[64];
    char value[64];

    if (settings == NULL) {
        return false;
    }

    protocol_storage_defaults(settings);

    fp = fopen(FOOTBATH_STORAGE_FILE, "r");
    if (fp == NULL) {
        return false;
    }

    while (fscanf(fp, "%63[^=]=%63s\n", key, value) == 2) {
        if (strcmp(key, "default_water_level") == 0) {
            settings->default_water_level = (uint8_t)atoi(value);
        } else if (strcmp(key, "default_temp") == 0) {
            settings->default_temp = (uint8_t)atoi(value);
        } else if (strcmp(key, "default_massage") == 0) {
            settings->default_massage = (uint8_t)atoi(value);
        } else if (strcmp(key, "default_timer") == 0) {
            settings->default_timer = (uint8_t)atoi(value);
        } else if (strcmp(key, "uart_device") == 0) {
            snprintf(settings->uart_device, sizeof(settings->uart_device), "%s", value);
        } else if (strcmp(key, "baudrate") == 0) {
            settings->baudrate = atoi(value);
        }
    }

    fclose(fp);
    return true;
}

bool protocol_storage_save(const footbath_settings_t *settings)
{
    FILE *fp;
    const char *tmp_path = FOOTBATH_STORAGE_FILE ".tmp";

    if (settings == NULL) {
        return false;
    }

    if (ensure_storage_dir() < 0) {
        LV_LOG_WARN("[STORAGE] mkdir %s failed: %s", FOOTBATH_STORAGE_DIR, strerror(errno));
        return false;
    }

    fp = fopen(tmp_path, "w");
    if (fp == NULL) {
        LV_LOG_WARN("[STORAGE] open %s failed: %s", tmp_path, strerror(errno));
        return false;
    }

    fprintf(fp, "default_water_level=%u\n", settings->default_water_level);
    fprintf(fp, "default_temp=%u\n", settings->default_temp);
    fprintf(fp, "default_massage=%u\n", settings->default_massage);
    fprintf(fp, "default_timer=%u\n", settings->default_timer);
    fprintf(fp, "uart_device=%s\n", settings->uart_device);
    fprintf(fp, "baudrate=%d\n", settings->baudrate);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

    if (rename(tmp_path, FOOTBATH_STORAGE_FILE) < 0) {
        LV_LOG_WARN("[STORAGE] rename settings failed: %s", strerror(errno));
        unlink(tmp_path);
        return false;
    }

    return true;
}
