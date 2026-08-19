#include "device_binding_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define DEVICE_BINDING_CONFIG_PATH "/etc/rk3506_binding.json"
#define DEVICE_BINDING_CONFIG_TMP  "/tmp/rk3506_binding.json.tmp"

static int write_text_file_atomic(const char *path, const char *tmp_path,
                                  const char *content)
{
    FILE *fp;
    size_t len;

    printf("[DBG] write_text_file_atomic: ENTER\n");
    printf("[DBG]   path: %s\n", path);
    printf("[DBG]   tmp_path: %s\n", tmp_path);

    if (path == NULL || tmp_path == NULL || content == NULL)
    {
        printf("[ERR] write_text_file_atomic: invalid parameters\n");
        return -1;
    }

    printf("[DBG] write_text_file_atomic: opening tmp file for writing...\n");
    fp = fopen(tmp_path, "w");
    if (fp == NULL)
    {
        printf("[ERR] write_text_file_atomic: fopen(%s) failed: %s\n", tmp_path, strerror(errno));
        return -1;
    }
    printf("[DBG] write_text_file_atomic: tmp file opened\n");

    len = strlen(content);
    printf("[DBG] write_text_file_atomic: writing %zu bytes...\n", len);
    if (fwrite(content, 1, len, fp) != len)
    {
        printf("[ERR] write_text_file_atomic: fwrite failed: %s\n", strerror(errno));
        fclose(fp);
        unlink(tmp_path);
        return -1;
    }
    printf("[DBG] write_text_file_atomic: write completed\n");

    if (fclose(fp) != 0)
    {
        printf("[ERR] write_text_file_atomic: fclose failed: %s\n", strerror(errno));
        unlink(tmp_path);
        return -1;
    }
    printf("[DBG] write_text_file_atomic: tmp file closed\n");

    printf("[DBG] write_text_file_atomic: renaming %s to %s...\n", tmp_path, path);
    if (rename(tmp_path, path) != 0)
    {
        printf("[ERR] write_text_file_atomic: rename failed: %s\n", strerror(errno));
        unlink(tmp_path);
        return -1;
    }
    printf("[DBG] write_text_file_atomic: rename successful\n");

    return 0;
}
static int write_text_file_direct(const char *path, const char *content)
{
    FILE *fp;
    size_t len;

    fp = fopen(path, "w");
    if (fp == NULL)
    {
        printf("[ERR] write_text_file_direct: fopen(%s) failed: %s\n", path, strerror(errno));
        return -1;
    }

    len = strlen(content);
    if (fwrite(content, 1, len, fp) != len)
    {
        printf("[ERR] write_text_file_direct: fwrite failed: %s\n", strerror(errno));
        fclose(fp);
        return -1;
    }

    if (fclose(fp) != 0)
    {
        printf("[ERR] write_text_file_direct: fclose failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

// 然后在 device_binding_config_save 中调用这个函数，而不是 write_text_file_atomic
static cJSON *load_root_from_file(void)
{
    FILE *fp;
    long file_size;
    size_t read_size;
    char *content;
    cJSON *root;

    printf("[DBG] load_root_from_file: ENTER\n");
    fp = fopen(DEVICE_BINDING_CONFIG_PATH, "r");
    if (fp == NULL)
    {
        printf("[DBG] load_root_from_file: file %s not found\n", DEVICE_BINDING_CONFIG_PATH);
        return NULL;
    }
    printf("[DBG] load_root_from_file: file opened\n");

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        printf("[ERR] load_root_from_file: fseek failed\n");
        fclose(fp);
        return NULL;
    }
    file_size = ftell(fp);
    if (file_size < 0)
    {
        printf("[ERR] load_root_from_file: ftell failed\n");
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0)
    {
        printf("[ERR] load_root_from_file: fseek to start failed\n");
        fclose(fp);
        return NULL;
    }

    printf("[DBG] load_root_from_file: file size = %ld bytes\n", file_size);

    content = (char *)malloc((size_t)file_size + 1);
    if (content == NULL)
    {
        printf("[ERR] load_root_from_file: malloc failed\n");
        fclose(fp);
        return NULL;
    }

    read_size = fread(content, 1, (size_t)file_size, fp);
    fclose(fp);
    content[read_size] = '\0';
    printf("[DBG] load_root_from_file: read %zu bytes\n", read_size);

    root = cJSON_Parse(content);
    free(content);
    if (root == NULL)
    {
        printf("[ERR] load_root_from_file: cJSON_Parse failed\n");
        return NULL;
    }
    printf("[DBG] load_root_from_file: JSON parsed successfully\n");
    return root;
}

int device_binding_config_save(const char *user_id, const char *wifi_ssid,
                               const cJSON *mqtt_obj)
{
    cJSON *root;
    cJSON *mqtt_copy;
    char *json_text;
    int ret;

    printf("[DBG] device_binding_config_save: ENTER\n");

    if (user_id == NULL || wifi_ssid == NULL || user_id[0] == '\0' || wifi_ssid[0] == '\0')
    {
        printf("[ERR] device_binding_config_save: invalid parameters (user_id=%p, wifi_ssid=%p)\n",
               (void *)user_id, (void *)wifi_ssid);
        if (user_id) printf("[ERR]   user_id: '%s'\n", user_id);
        if (wifi_ssid) printf("[ERR]   wifi_ssid: '%s'\n", wifi_ssid);
        return -1;
    }
    printf("[DBG] device_binding_config_save: user_id='%s', wifi_ssid='%s'\n", user_id, wifi_ssid);

    printf("[DBG] device_binding_config_save: creating root JSON...\n");
    root = cJSON_CreateObject();
    if (root == NULL)
    {
        printf("[ERR] device_binding_config_save: cJSON_CreateObject failed\n");
        return -1;
    }
    printf("[DBG] device_binding_config_save: root JSON created\n");

    cJSON_AddNumberToObject(root, "version", 1);
    cJSON_AddStringToObject(root, "user_id", user_id);
    cJSON_AddStringToObject(root, "wifi_ssid", wifi_ssid);
    cJSON_AddNumberToObject(root, "updated_at", (double)time(NULL));
    printf("[DBG] device_binding_config_save: basic fields added\n");

    printf("[DBG] device_binding_config_save: processing mqtt_obj...\n");
    if (mqtt_obj != NULL && cJSON_IsObject(mqtt_obj))
    {
        printf("[DBG] device_binding_config_save: mqtt_obj is valid, duplicating...\n");
        mqtt_copy = cJSON_Duplicate((cJSON *)mqtt_obj, 1);
        if (mqtt_copy == NULL)
        {
            printf("[ERR] device_binding_config_save: cJSON_Duplicate failed\n");
            cJSON_Delete(root);
            return -1;
        }
        printf("[DBG] device_binding_config_save: mqtt_obj duplicated\n");
    }
    else
    {
        printf("[DBG] device_binding_config_save: mqtt_obj is NULL or not an object, creating empty\n");
        mqtt_copy = cJSON_CreateObject();
        if (mqtt_copy == NULL)
        {
            printf("[ERR] device_binding_config_save: cJSON_CreateObject for mqtt failed\n");
            cJSON_Delete(root);
            return -1;
        }
        printf("[DBG] device_binding_config_save: empty mqtt object created\n");
    }
    cJSON_AddItemToObject(root, "mqtt", mqtt_copy);
    printf("[DBG] device_binding_config_save: mqtt added to root\n");

    printf("[DBG] device_binding_config_save: converting JSON to string...\n");
    json_text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (json_text == NULL)
    {
        printf("[ERR] device_binding_config_save: cJSON_PrintUnformatted failed\n");
        return -1;
    }
    printf("[DBG] device_binding_config_save: JSON string:\n%s\n", json_text);

    printf("[DBG] device_binding_config_save: calling write_text_file_direct...\n");
    // ret = write_text_file_atomic(DEVICE_BINDING_CONFIG_PATH,
    //                              DEVICE_BINDING_CONFIG_TMP,
    //                              json_text);
    ret = write_text_file_direct(DEVICE_BINDING_CONFIG_PATH, json_text);
    cJSON_free(json_text);

    if (ret == 0)
    {
        printf("[DBG] device_binding_config_save: SUCCESS\n");
    }
    else
    {
        printf("[ERR] device_binding_config_save: write_text_file_atomic returned %d\n", ret);
    }
    return ret;
}

int device_binding_config_load(device_binding_config_t *config)
{
    cJSON *root;
    cJSON *item;
    cJSON *mqtt_obj;
    char *mqtt_text;

    printf("[DBG] device_binding_config_load: ENTER\n");
    if (config == NULL)
    {
        printf("[ERR] device_binding_config_load: config is NULL\n");
        return -1;
    }

    memset(config, 0, sizeof(*config));

    root = load_root_from_file();
    if (root == NULL)
    {
        printf("[ERR] device_binding_config_load: load_root_from_file failed\n");
        return -1;
    }

    item = cJSON_GetObjectItemCaseSensitive(root, "user_id");
    if (cJSON_IsString(item) && item->valuestring != NULL)
    {
        strncpy(config->user_id, item->valuestring, sizeof(config->user_id) - 1);
        printf("[DBG] device_binding_config_load: user_id='%s'\n", config->user_id);
    }

    item = cJSON_GetObjectItemCaseSensitive(root, "wifi_ssid");
    if (cJSON_IsString(item) && item->valuestring != NULL)
    {
        strncpy(config->wifi_ssid, item->valuestring, sizeof(config->wifi_ssid) - 1);
        printf("[DBG] device_binding_config_load: wifi_ssid='%s'\n", config->wifi_ssid);
    }

    item = cJSON_GetObjectItemCaseSensitive(root, "updated_at");
    if (cJSON_IsNumber(item))
    {
        config->updated_at = (time_t)item->valuedouble;
        printf("[DBG] device_binding_config_load: updated_at=%ld\n", (long)config->updated_at);
    }

    mqtt_obj = cJSON_GetObjectItemCaseSensitive(root, "mqtt");
    if (mqtt_obj != NULL && cJSON_IsObject(mqtt_obj))
    {
        mqtt_text = cJSON_PrintUnformatted(mqtt_obj);
        if (mqtt_text != NULL)
        {
            strncpy(config->mqtt_json, mqtt_text, sizeof(config->mqtt_json) - 1);
            config->mqtt_configured = (strcmp(config->mqtt_json, "{}") != 0);
            printf("[DBG] device_binding_config_load: mqtt_json='%s'\n", config->mqtt_json);
            cJSON_free(mqtt_text);
        }
    }

    cJSON_Delete(root);
    printf("[DBG] device_binding_config_load: SUCCESS\n");
    return 0;
}
