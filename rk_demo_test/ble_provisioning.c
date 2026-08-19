#include "ble_provisioning.h"

#include <dbus/dbus.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cJSON.h"
#include "device_binding_config.h"
#include "wifi_manager.h"

#define BLE_APP_PATH               "/com/rk3506/provision"
#define BLE_SERVICE_PATH           BLE_APP_PATH "/service0"
#define BLE_CMD_CHAR_PATH          BLE_SERVICE_PATH "/char_cmd"
#define BLE_RSP_CHAR_PATH          BLE_SERVICE_PATH "/char_rsp"

#define BLE_PROVISION_SERVICE_UUID "8c1f2f5a-4c5d-4f3b-8a6c-7a4f8dc2b001"
#define BLE_CMD_CHAR_UUID          "8c1f2f5a-4c5d-4f3b-8a6c-7a4f8dc2b002"
#define BLE_RSP_CHAR_UUID          "8c1f2f5a-4c5d-4f3b-8a6c-7a4f8dc2b003"

#define BLE_CMD_MAX_LEN            1536
#define BLE_RSP_MAX_LEN            1024
#define BLE_REQUEST_ID_MAX         64

static DBusConnection *g_dbus_conn = NULL;
static pthread_t g_dbus_thread;
static pthread_mutex_t g_ble_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_command_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_rx_lock = PTHREAD_MUTEX_INITIALIZER;
static bool g_dbus_running = false;
static bool g_notify_enabled = false;
static bool g_ble_connected = false;
static uint8_t g_response_value[BLE_RSP_MAX_LEN];
static size_t g_response_len = 0;
static char g_command_buffer[BLE_CMD_MAX_LEN];
static size_t g_command_len = 0;

static int ble_get_default_adapter(char *adapter, size_t size)
{
    FILE *fp;

    if (adapter == NULL || size == 0)
    {
        return -1;
    }

    fp = popen("ls /sys/class/bluetooth/ 2>/dev/null | grep '^hci' | head -n1", "r");
    if (fp == NULL)
    {
        return -1;
    }
    if (fgets(adapter, size, fp) == NULL)
    {
        pclose(fp);
        return -1;
    }
    pclose(fp);

    adapter[strcspn(adapter, "\r\n")] = '\0';
    return (adapter[0] != '\0') ? 0 : -1;
}

static DBusHandlerResult ble_send_method_error(DBusConnection *conn,
        DBusMessage *msg,
        const char *name,
        const char *detail)
{
    DBusMessage *reply = dbus_message_new_error(msg, name, detail);
    if (reply != NULL)
    {
        dbus_connection_send(conn, reply, NULL);
        dbus_connection_flush(conn);
        dbus_message_unref(reply);
    }
    return DBUS_HANDLER_RESULT_HANDLED;
}

static void append_variant_string(DBusMessageIter *dict, const char *key,
                                  const char *value)
{
    DBusMessageIter entry;
    DBusMessageIter variant;
    const char *safe_value = (value != NULL) ? value : "";

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &safe_value);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_variant_bool(DBusMessageIter *dict, const char *key, bool value)
{
    DBusMessageIter entry;
    DBusMessageIter variant;
    dbus_bool_t dbus_value = value ? TRUE : FALSE;

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &dbus_value);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_variant_object_path(DBusMessageIter *dict, const char *key,
                                       const char *value)
{
    DBusMessageIter entry;
    DBusMessageIter variant;
    const char *path_value = value;

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, DBUS_TYPE_OBJECT_PATH_AS_STRING,
                                     &variant);
    dbus_message_iter_append_basic(&variant, DBUS_TYPE_OBJECT_PATH, &path_value);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_variant_string_array(DBusMessageIter *dict, const char *key,
                                        const char *const *values, size_t count)
{
    DBusMessageIter entry;
    DBusMessageIter variant;
    DBusMessageIter array;
    size_t i;

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "as", &variant);
    dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);
    for (i = 0; i < count; i++)
    {
        const char *value = values[i];
        dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &value);
    }
    dbus_message_iter_close_container(&variant, &array);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_variant_byte_array(DBusMessageIter *dict, const char *key,
                                      const uint8_t *value, size_t len)
{
    DBusMessageIter entry;
    DBusMessageIter variant;
    DBusMessageIter array;
    const uint8_t *data = value;

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, "ay", &variant);
    dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &array);
    if (len > 0)
    {
        dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE, &data, (int)len);
    }
    dbus_message_iter_close_container(&variant, &array);
    dbus_message_iter_close_container(&entry, &variant);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_service_properties(DBusMessageIter *dict)
{
    DBusMessageIter entry;
    DBusMessageIter props;
    DBusMessageIter includes;
    const char *iface = "org.bluez.GattService1";

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_ARRAY, "{sv}", &props);
    append_variant_string(&props, "UUID", BLE_PROVISION_SERVICE_UUID);
    append_variant_bool(&props, "Primary", true);

    {
        DBusMessageIter includes_entry;
        DBusMessageIter includes_variant;
        dbus_message_iter_open_container(&props, DBUS_TYPE_DICT_ENTRY, NULL, &includes_entry);
        {
            const char *prop = "Includes";
            dbus_message_iter_append_basic(&includes_entry, DBUS_TYPE_STRING, &prop);
        }
        dbus_message_iter_open_container(&includes_entry, DBUS_TYPE_VARIANT, "ao", &includes_variant);
        dbus_message_iter_open_container(&includes_variant, DBUS_TYPE_ARRAY,
                                         DBUS_TYPE_OBJECT_PATH_AS_STRING, &includes);
        dbus_message_iter_close_container(&includes_variant, &includes);
        dbus_message_iter_close_container(&includes_entry, &includes_variant);
        dbus_message_iter_close_container(&props, &includes_entry);
    }

    dbus_message_iter_close_container(&entry, &props);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_cmd_char_properties(DBusMessageIter *dict)
{
    DBusMessageIter entry;
    DBusMessageIter props;
    const char *iface = "org.bluez.GattCharacteristic1";
    static const char *const flags[] = {"write", "write-without-response"};

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_ARRAY, "{sv}", &props);
    append_variant_string(&props, "UUID", BLE_CMD_CHAR_UUID);
    append_variant_object_path(&props, "Service", BLE_SERVICE_PATH);
    append_variant_string_array(&props, "Flags", flags, sizeof(flags) / sizeof(flags[0]));
    dbus_message_iter_close_container(&entry, &props);
    dbus_message_iter_close_container(dict, &entry);
}

static void append_rsp_char_properties(DBusMessageIter *dict)
{
    DBusMessageIter entry;
    DBusMessageIter props;
    bool notifying;
    size_t value_len;
    uint8_t value_copy[BLE_RSP_MAX_LEN];
    const char *iface = "org.bluez.GattCharacteristic1";
    static const char *const flags[] = {"read", "notify"};

    pthread_mutex_lock(&g_ble_lock);
    notifying = g_notify_enabled;
    value_len = g_response_len;
    if (value_len > 0)
    {
        memcpy(value_copy, g_response_value, value_len);
    }
    pthread_mutex_unlock(&g_ble_lock);

    dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
    dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &iface);
    dbus_message_iter_open_container(&entry, DBUS_TYPE_ARRAY, "{sv}", &props);
    append_variant_string(&props, "UUID", BLE_RSP_CHAR_UUID);
    append_variant_object_path(&props, "Service", BLE_SERVICE_PATH);
    append_variant_string_array(&props, "Flags", flags, sizeof(flags) / sizeof(flags[0]));
    append_variant_bool(&props, "Notifying", notifying);
    append_variant_byte_array(&props, "Value", value_copy, value_len);
    dbus_message_iter_close_container(&entry, &props);
    dbus_message_iter_close_container(dict, &entry);
}

static DBusHandlerResult ble_handle_get_managed_objects(DBusConnection *conn,
        DBusMessage *msg)
{
    DBusMessage *reply;
    DBusMessageIter iter;
    DBusMessageIter objects;

    reply = dbus_message_new_method_return(msg);
    if (reply == NULL)
    {
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_message_iter_init_append(reply, &iter);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{oa{sa{sv}}}", &objects);

    {
        DBusMessageIter obj_entry;
        DBusMessageIter iface_array;
        const char *path = BLE_SERVICE_PATH;

        dbus_message_iter_open_container(&objects, DBUS_TYPE_DICT_ENTRY, NULL, &obj_entry);
        dbus_message_iter_append_basic(&obj_entry, DBUS_TYPE_OBJECT_PATH, &path);
        dbus_message_iter_open_container(&obj_entry, DBUS_TYPE_ARRAY, "{sa{sv}}", &iface_array);
        append_service_properties(&iface_array);
        dbus_message_iter_close_container(&obj_entry, &iface_array);
        dbus_message_iter_close_container(&objects, &obj_entry);
    }

    {
        DBusMessageIter obj_entry;
        DBusMessageIter iface_array;
        const char *path = BLE_CMD_CHAR_PATH;

        dbus_message_iter_open_container(&objects, DBUS_TYPE_DICT_ENTRY, NULL, &obj_entry);
        dbus_message_iter_append_basic(&obj_entry, DBUS_TYPE_OBJECT_PATH, &path);
        dbus_message_iter_open_container(&obj_entry, DBUS_TYPE_ARRAY, "{sa{sv}}", &iface_array);
        append_cmd_char_properties(&iface_array);
        dbus_message_iter_close_container(&obj_entry, &iface_array);
        dbus_message_iter_close_container(&objects, &obj_entry);
    }

    {
        DBusMessageIter obj_entry;
        DBusMessageIter iface_array;
        const char *path = BLE_RSP_CHAR_PATH;

        dbus_message_iter_open_container(&objects, DBUS_TYPE_DICT_ENTRY, NULL, &obj_entry);
        dbus_message_iter_append_basic(&obj_entry, DBUS_TYPE_OBJECT_PATH, &path);
        dbus_message_iter_open_container(&obj_entry, DBUS_TYPE_ARRAY, "{sa{sv}}", &iface_array);
        append_rsp_char_properties(&iface_array);
        dbus_message_iter_close_container(&obj_entry, &iface_array);
        dbus_message_iter_close_container(&objects, &obj_entry);
    }

    dbus_message_iter_close_container(&iter, &objects);
    dbus_connection_send(conn, reply, NULL);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);
    return DBUS_HANDLER_RESULT_HANDLED;
}

static bool append_property_get_reply(DBusMessageIter *iter, const char *path,
                                      const char *prop_name)
{
    static const char *const cmd_flags[] = {"write", "write-without-response"};
    static const char *const rsp_flags[] = {"read", "notify"};
    bool notifying;
    size_t value_len;
    uint8_t value_copy[BLE_RSP_MAX_LEN];

    pthread_mutex_lock(&g_ble_lock);
    notifying = g_notify_enabled;
    value_len = g_response_len;
    if (value_len > 0)
    {
        memcpy(value_copy, g_response_value, value_len);
    }
    pthread_mutex_unlock(&g_ble_lock);

    if (strcmp(path, BLE_SERVICE_PATH) == 0)
    {
        if (strcmp(prop_name, "UUID") == 0)
        {
            DBusMessageIter variant;
            const char *value = BLE_PROVISION_SERVICE_UUID;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &value);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Primary") == 0)
        {
            DBusMessageIter variant;
            dbus_bool_t value = TRUE;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &value);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Includes") == 0)
        {
            DBusMessageIter variant;
            DBusMessageIter array;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "ao", &variant);
            dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, DBUS_TYPE_OBJECT_PATH_AS_STRING,
                                             &array);
            dbus_message_iter_close_container(&variant, &array);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
    }

    if (strcmp(path, BLE_CMD_CHAR_PATH) == 0)
    {
        if (strcmp(prop_name, "UUID") == 0)
        {
            DBusMessageIter variant;
            const char *value = BLE_CMD_CHAR_UUID;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &value);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Service") == 0)
        {
            DBusMessageIter variant;
            const char *value = BLE_SERVICE_PATH;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_OBJECT_PATH_AS_STRING,
                                             &variant);
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_OBJECT_PATH, &value);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Flags") == 0)
        {
            DBusMessageIter variant;
            DBusMessageIter array;
            size_t i;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "as", &variant);
            dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);
            for (i = 0; i < sizeof(cmd_flags) / sizeof(cmd_flags[0]); i++)
            {
                const char *value = cmd_flags[i];
                dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &value);
            }
            dbus_message_iter_close_container(&variant, &array);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
    }

    if (strcmp(path, BLE_RSP_CHAR_PATH) == 0)
    {
        if (strcmp(prop_name, "UUID") == 0)
        {
            DBusMessageIter variant;
            const char *value = BLE_RSP_CHAR_UUID;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant);
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &value);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Service") == 0)
        {
            DBusMessageIter variant;
            const char *value = BLE_SERVICE_PATH;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_OBJECT_PATH_AS_STRING,
                                             &variant);
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_OBJECT_PATH, &value);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Flags") == 0)
        {
            DBusMessageIter variant;
            DBusMessageIter array;
            size_t i;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "as", &variant);
            dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);
            for (i = 0; i < sizeof(rsp_flags) / sizeof(rsp_flags[0]); i++)
            {
                const char *value = rsp_flags[i];
                dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &value);
            }
            dbus_message_iter_close_container(&variant, &array);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Notifying") == 0)
        {
            DBusMessageIter variant;
            dbus_bool_t value = notifying ? TRUE : FALSE;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant);
            dbus_message_iter_append_basic(&variant, DBUS_TYPE_BOOLEAN, &value);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
        if (strcmp(prop_name, "Value") == 0)
        {
            DBusMessageIter variant;
            DBusMessageIter array;
            const uint8_t *data = value_copy;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "ay", &variant);
            dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &array);
            if (value_len > 0)
            {
                dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE, &data, (int)value_len);
            }
            dbus_message_iter_close_container(&variant, &array);
            dbus_message_iter_close_container(iter, &variant);
            return true;
        }
    }

    return false;
}

static DBusHandlerResult ble_handle_properties(DBusConnection *conn,
        DBusMessage *msg,
        const char *path)
{
    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_PROPERTIES, "GetAll"))
    {
        DBusMessage *reply;
        DBusMessageIter args;
        DBusMessageIter iter;
        DBusMessageIter dict;
        const char *iface = NULL;

        if (!dbus_message_iter_init(msg, &args) ||
                dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING)
        {
            return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS, "GetAll expects interface name");
        }
        dbus_message_iter_get_basic(&args, &iface);

        reply = dbus_message_new_method_return(msg);
        if (reply == NULL)
        {
            return DBUS_HANDLER_RESULT_NEED_MEMORY;
        }

        dbus_message_iter_init_append(reply, &iter);
        dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);
        if (strcmp(path, BLE_SERVICE_PATH) == 0 && strcmp(iface, "org.bluez.GattService1") == 0)
        {
            append_variant_string(&dict, "UUID", BLE_PROVISION_SERVICE_UUID);
            append_variant_bool(&dict, "Primary", true);
            {
                DBusMessageIter includes_entry;
                DBusMessageIter includes_variant;
                DBusMessageIter includes_array;
                const char *prop = "Includes";

                dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &includes_entry);
                dbus_message_iter_append_basic(&includes_entry, DBUS_TYPE_STRING, &prop);
                dbus_message_iter_open_container(&includes_entry, DBUS_TYPE_VARIANT, "ao", &includes_variant);
                dbus_message_iter_open_container(&includes_variant, DBUS_TYPE_ARRAY,
                                                 DBUS_TYPE_OBJECT_PATH_AS_STRING, &includes_array);
                dbus_message_iter_close_container(&includes_variant, &includes_array);
                dbus_message_iter_close_container(&includes_entry, &includes_variant);
                dbus_message_iter_close_container(&dict, &includes_entry);
            }
        }
        else if (strcmp(path, BLE_CMD_CHAR_PATH) == 0
                 && strcmp(iface, "org.bluez.GattCharacteristic1") == 0)
        {
            static const char *const flags[] = {"write", "write-without-response"};
            append_variant_string(&dict, "UUID", BLE_CMD_CHAR_UUID);
            append_variant_object_path(&dict, "Service", BLE_SERVICE_PATH);
            append_variant_string_array(&dict, "Flags", flags, sizeof(flags) / sizeof(flags[0]));
        }
        else if (strcmp(path, BLE_RSP_CHAR_PATH) == 0
                 && strcmp(iface, "org.bluez.GattCharacteristic1") == 0)
        {
            static const char *const flags[] = {"read", "notify"};
            bool notifying;
            size_t value_len;
            uint8_t value_copy[BLE_RSP_MAX_LEN];

            pthread_mutex_lock(&g_ble_lock);
            notifying = g_notify_enabled;
            value_len = g_response_len;
            if (value_len > 0)
            {
                memcpy(value_copy, g_response_value, value_len);
            }
            pthread_mutex_unlock(&g_ble_lock);

            append_variant_string(&dict, "UUID", BLE_RSP_CHAR_UUID);
            append_variant_object_path(&dict, "Service", BLE_SERVICE_PATH);
            append_variant_string_array(&dict, "Flags", flags, sizeof(flags) / sizeof(flags[0]));
            append_variant_bool(&dict, "Notifying", notifying);
            append_variant_byte_array(&dict, "Value", value_copy, value_len);
        }
        else
        {
            dbus_message_unref(reply);
            return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS,
                                         "Unsupported interface for GetAll");
        }
        dbus_message_iter_close_container(&iter, &dict);

        dbus_connection_send(conn, reply, NULL);
        dbus_connection_flush(conn);
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_PROPERTIES, "Get"))
    {
        DBusMessage *reply;
        DBusMessageIter args;
        DBusMessageIter iter;
        const char *iface = NULL;
        const char *prop_name = NULL;

        if (!dbus_message_iter_init(msg, &args) ||
                dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING)
        {
            return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS,
                                         "Get expects interface name and property name");
        }
        dbus_message_iter_get_basic(&args, &iface);
        dbus_message_iter_next(&args);
        if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING)
        {
            return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS, "Get expects property name");
        }
        dbus_message_iter_get_basic(&args, &prop_name);

        if ((strcmp(path, BLE_SERVICE_PATH) == 0 && strcmp(iface, "org.bluez.GattService1") != 0) ||
                ((strcmp(path, BLE_CMD_CHAR_PATH) == 0 || strcmp(path, BLE_RSP_CHAR_PATH) == 0) &&
                 strcmp(iface, "org.bluez.GattCharacteristic1") != 0))
        {
            return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS, "Unsupported interface for Get");
        }

        reply = dbus_message_new_method_return(msg);
        if (reply == NULL)
        {
            return DBUS_HANDLER_RESULT_NEED_MEMORY;
        }
        dbus_message_iter_init_append(reply, &iter);
        if (!append_property_get_reply(&iter, path, prop_name))
        {
            dbus_message_unref(reply);
            return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS, "Unsupported property");
        }
        dbus_connection_send(conn, reply, NULL);
        dbus_connection_flush(conn);
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static int ble_set_response_locked(const char *json_text)
{
    size_t len;

    if (json_text == NULL)
    {
        return -1;
    }

    len = strlen(json_text);
    if (len >= BLE_RSP_MAX_LEN)
    {
        return -1;
    }

    memcpy(g_response_value, json_text, len);
    g_response_value[len] = '\0';
    g_response_len = len;
    return 0;
}

static void ble_emit_response_notification_locked(void)
{
    DBusMessage *signal;
    DBusMessageIter iter;
    DBusMessageIter changed_iface;
    DBusMessageIter changed_props;
    DBusMessageIter invalidated;

    if (g_dbus_conn == NULL || !g_notify_enabled)
    {
        return;
    }

    signal = dbus_message_new_signal(BLE_RSP_CHAR_PATH,
                                     DBUS_INTERFACE_PROPERTIES,
                                     "PropertiesChanged");
    if (signal == NULL)
    {
        return;
    }

    dbus_message_iter_init_append(signal, &iter);
    {
        const char *iface = "org.bluez.GattCharacteristic1";
        dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &iface);
    }
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &changed_props);
    append_variant_byte_array(&changed_props, "Value", g_response_value, g_response_len);
    dbus_message_iter_close_container(&iter, &changed_props);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &invalidated);
    dbus_message_iter_close_container(&iter, &invalidated);

    dbus_connection_send(g_dbus_conn, signal, NULL);
    dbus_connection_flush(g_dbus_conn);
    dbus_message_unref(signal);
}

static void ble_publish_json_text(const char *json_text)
{
    static const char fallback[] =
        "{\"type\":\"response\",\"cmd\":\"internal\",\"ok\":false,"
        "\"code\":\"response_too_large\",\"message\":\"BLE response too large\"}";

    pthread_mutex_lock(&g_ble_lock);
    if (ble_set_response_locked(json_text) != 0)
    {
        ble_set_response_locked(fallback);
    }
    ble_emit_response_notification_locked();
    pthread_mutex_unlock(&g_ble_lock);
}

static void ble_publish_response(const char *cmd, const char *request_id,
                                 bool ok, const char *code,
                                 const char *message, cJSON *data_obj)
{
    cJSON *root;
    char *json_text;

    root = cJSON_CreateObject();
    if (root == NULL)
    {
        return;
    }

    cJSON_AddStringToObject(root, "type", "response");
    cJSON_AddStringToObject(root, "cmd", (cmd != NULL) ? cmd : "unknown");
    if (request_id != NULL && request_id[0] != '\0')
    {
        cJSON_AddStringToObject(root, "request_id", request_id);
    }
    cJSON_AddBoolToObject(root, "ok", ok);
    cJSON_AddStringToObject(root, "code", (code != NULL) ? code : (ok ? "OK" : "ERROR"));
    cJSON_AddStringToObject(root, "message", (message != NULL) ? message : "");
    if (data_obj != NULL)
    {
        cJSON_AddItemToObject(root, "data", data_obj);
    }
    else
    {
        cJSON_AddItemToObject(root, "data", cJSON_CreateObject());
    }

    json_text = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (json_text == NULL)
    {
        return;
    }

    ble_publish_json_text(json_text);
    cJSON_free(json_text);
}

static cJSON *ble_make_notify_ready_event(void)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        return NULL;
    }

    cJSON_AddStringToObject(root, "type", "event");
    cJSON_AddStringToObject(root, "event", "notify_ready");
    cJSON_AddNumberToObject(root, "protocol_version", 1);
    cJSON_AddStringToObject(root, "service_uuid", BLE_PROVISION_SERVICE_UUID);
    return root;
}

static int ble_append_read_value(DBusMessageIter *iter)
{
    DBusMessageIter array;
    uint8_t value_copy[BLE_RSP_MAX_LEN];
    const uint8_t *data = value_copy;
    size_t value_len;

    pthread_mutex_lock(&g_ble_lock);
    value_len = g_response_len;
    if (value_len > 0)
    {
        memcpy(value_copy, g_response_value, value_len);
    }
    pthread_mutex_unlock(&g_ble_lock);

    dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &array);
    if (value_len > 0)
    {
        dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE, &data, (int)value_len);
    }
    dbus_message_iter_close_container(iter, &array);
    return 0;
}

static int ble_get_request_id(cJSON *root, char *buffer, size_t buffer_size)
{
    cJSON *item;

    if (buffer == NULL || buffer_size == 0)
    {
        return -1;
    }
    buffer[0] = '\0';

    item = cJSON_GetObjectItemCaseSensitive(root, "request_id");
    if (cJSON_IsString(item) && item->valuestring != NULL)
    {
        strncpy(buffer, item->valuestring, buffer_size - 1);
        return 0;
    }
    return -1;
}

static int ble_get_wifi_payload(cJSON *root, char *ssid, size_t ssid_size,
                                char *password, size_t password_size,
                                cJSON **mqtt_obj, char *user_id,
                                size_t user_id_size)
{
    cJSON *wifi_obj;
    cJSON *item;

    if (root == NULL || ssid == NULL || password == NULL || mqtt_obj == NULL || user_id == NULL)
    {
        return -1;
    }

    wifi_obj = cJSON_GetObjectItemCaseSensitive(root, "wifi");
    if (!cJSON_IsObject(wifi_obj))
    {
        return -1;
    }

    item = cJSON_GetObjectItemCaseSensitive(wifi_obj, "ssid");
    if (!cJSON_IsString(item) || item->valuestring == NULL || item->valuestring[0] == '\0')
    {
        return -1;
    }
    strncpy(ssid, item->valuestring, ssid_size - 1);

    item = cJSON_GetObjectItemCaseSensitive(wifi_obj, "password");
    if (!cJSON_IsString(item) || item->valuestring == NULL || item->valuestring[0] == '\0')
    {
        return -1;
    }
    strncpy(password, item->valuestring, password_size - 1);

    item = cJSON_GetObjectItemCaseSensitive(root, "user_id");
    if (!cJSON_IsString(item) || item->valuestring == NULL || item->valuestring[0] == '\0')
    {
        return -1;
    }
    strncpy(user_id, item->valuestring, user_id_size - 1);

    *mqtt_obj = cJSON_GetObjectItemCaseSensitive(root, "mqtt");
    if (!cJSON_IsObject(*mqtt_obj))
    {
        return -1;
    }

    return 0;
}

static cJSON *ble_make_status_payload(const char *ip_override)
{
    cJSON *data;
    device_binding_config_t config;
    char ssid[64] = {0};
    char ip[64] = {0};
    bool connected;

    data = cJSON_CreateObject();
    if (data == NULL)
    {
        return NULL;
    }

    connected = wifi_manager_is_connected();
    if (wifi_manager_get_current_ssid(ssid, sizeof(ssid)) != 0
            && device_binding_config_load(&config) == 0)
    {
        strncpy(ssid, config.wifi_ssid, sizeof(ssid) - 1);
    }

    if (ip_override != NULL && ip_override[0] != '\0')
    {
        strncpy(ip, ip_override, sizeof(ip) - 1);
    }
    else
    {
        wifi_manager_get_current_ipv4(ip, sizeof(ip));
    }

    cJSON_AddNumberToObject(data, "protocol_version", 1);
    cJSON_AddBoolToObject(data, "wifi_connected", connected);
    cJSON_AddStringToObject(data, "ssid", ssid);
    cJSON_AddStringToObject(data, "ip", ip);
    cJSON_AddStringToObject(data, "ble_name", "rk3506");

    if (device_binding_config_load(&config) == 0)
    {
        cJSON_AddStringToObject(data, "user_id", config.user_id);
        cJSON_AddBoolToObject(data, "mqtt_configured", config.mqtt_configured);
        cJSON_AddNumberToObject(data, "updated_at", (double)config.updated_at);
    }
    else
    {
        cJSON_AddStringToObject(data, "user_id", "");
        cJSON_AddBoolToObject(data, "mqtt_configured", false);
        cJSON_AddNumberToObject(data, "updated_at", 0);
    }

    return data;
}

static void ble_handle_provision_command(cJSON *root)
{
    char request_id[BLE_REQUEST_ID_MAX] = {0};
    char ssid[DEVICE_BINDING_WIFI_SSID_MAX] = {0};
    char password[128] = {0};
    char user_id[DEVICE_BINDING_USER_ID_MAX] = {0};
    char ip[64] = {0};
    cJSON *mqtt_obj = NULL;
    cJSON *data;

    ble_get_request_id(root, request_id, sizeof(request_id));
    if (ble_get_wifi_payload(root, ssid, sizeof(ssid), password, sizeof(password),
                             &mqtt_obj, user_id, sizeof(user_id)) != 0)
    {
        ble_publish_response("provision", request_id, false, "invalid_payload",
                             "wifi/user_id/mqtt payload is required", NULL);
        return;
    }

    if (wifi_manager_connect(ssid, password) != 0)
    {
        ble_publish_response("provision", request_id, false, "wifi_connect_failed",
                             "Failed to connect WiFi or obtain IP address", NULL);
        return;
    }

    if (wifi_manager_persist_config(ssid, password) != 0)
    {
        ble_publish_response("provision", request_id, false, "wifi_persist_failed",
                             "WiFi connected but failed to persist config", NULL);
        return;
    }

    if (device_binding_config_save(user_id, ssid, mqtt_obj) != 0)
    {
        ble_publish_response("provision", request_id, false, "binding_persist_failed",
                             "WiFi connected but failed to save binding config", NULL);
        return;
    }

    wifi_manager_get_current_ipv4(ip, sizeof(ip));
    data = ble_make_status_payload(ip);
    if (data != NULL)
    {
        cJSON_AddBoolToObject(data, "binding_saved", true);
    }
    ble_publish_response("provision", request_id, true, "OK",
                         "Provisioning completed", data);
}

static void ble_handle_get_ip_command(cJSON *root)
{
    char request_id[BLE_REQUEST_ID_MAX] = {0};
    char ip[64] = {0};
    cJSON *data;

    ble_get_request_id(root, request_id, sizeof(request_id));
    if (wifi_manager_get_current_ipv4(ip, sizeof(ip)) != 0)
    {
        data = ble_make_status_payload(NULL);
        ble_publish_response("get_ip", request_id, false, "no_ip",
                             "No IPv4 address available on wlan0", data);
        return;
    }

    data = ble_make_status_payload(ip);
    ble_publish_response("get_ip", request_id, true, "OK",
                         "Current IP address fetched", data);
}

static void ble_handle_get_status_command(cJSON *root)
{
    char request_id[BLE_REQUEST_ID_MAX] = {0};
    cJSON *data;

    ble_get_request_id(root, request_id, sizeof(request_id));
    data = ble_make_status_payload(NULL);
    ble_publish_response("get_status", request_id, true, "OK",
                         "Current device status fetched", data);
}

static void *ble_command_worker(void *arg)
{
    char *json_text = (char *)arg;
    cJSON *root;
    cJSON *cmd_item;

    pthread_mutex_lock(&g_command_lock);

    root = cJSON_Parse(json_text);
    free(json_text);
    if (root == NULL)
    {
        ble_publish_response("unknown", NULL, false, "bad_json",
                             "BLE command is not valid JSON", NULL);
        pthread_mutex_unlock(&g_command_lock);
        return NULL;
    }

    cmd_item = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    if (!cJSON_IsString(cmd_item) || cmd_item->valuestring == NULL)
    {
        ble_publish_response("unknown", NULL, false, "missing_cmd",
                             "BLE command must contain cmd", NULL);
        cJSON_Delete(root);
        pthread_mutex_unlock(&g_command_lock);
        return NULL;
    }

    if (strcmp(cmd_item->valuestring, "provision") == 0)
    {
        ble_handle_provision_command(root);
    }
    else if (strcmp(cmd_item->valuestring, "get_ip") == 0)
    {
        ble_handle_get_ip_command(root);
    }
    else if (strcmp(cmd_item->valuestring, "get_status") == 0)
    {
        ble_handle_get_status_command(root);
    }
    else
    {
        char request_id[BLE_REQUEST_ID_MAX] = {0};
        ble_get_request_id(root, request_id, sizeof(request_id));
        ble_publish_response(cmd_item->valuestring, request_id, false,
                             "unsupported_cmd", "Unsupported BLE command", NULL);
    }

    cJSON_Delete(root);
    pthread_mutex_unlock(&g_command_lock);
    return NULL;
}

static DBusHandlerResult ble_handle_cmd_write(DBusConnection *conn,
        DBusMessage *msg)
{
    DBusMessageIter args;
    DBusMessageIter array;
    uint8_t *bytes = NULL;
    int len = 0;
    char *json_text = NULL;
    pthread_t worker;
    DBusMessage *reply;

    if (!dbus_message_iter_init(msg, &args) ||
            dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
    {
        return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS, "WriteValue expects byte array");
    }

    dbus_message_iter_recurse(&args, &array);
    if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_BYTE &&
            dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_INVALID)
    {
        return ble_send_method_error(conn, msg, DBUS_ERROR_INVALID_ARGS,
                                     "WriteValue expects byte array payload");
    }

    if (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_BYTE)
    {
        dbus_message_iter_get_fixed_array(&array, &bytes, &len);
    }

    if (len <= 0 || len >= BLE_CMD_MAX_LEN)
    {
        ble_publish_response("unknown", NULL, false, "invalid_length",
                             "BLE command payload length is invalid", NULL);
    }
    else
    {
        bool complete = false;
        size_t i;
        size_t start = 0;

        while (start < (size_t)len &&
                (bytes[start] == ' ' || bytes[start] == '\n' || bytes[start] == '\r' || bytes[start] == '\t'))
        {
            start++;
        }

        pthread_mutex_lock(&g_rx_lock);
        if (start < (size_t)len && bytes[start] == '{' && g_command_len > 0)
        {
            g_command_len = 0;
        }

        if (g_command_len + (size_t)len >= sizeof(g_command_buffer))
        {
            g_command_len = 0;
            pthread_mutex_unlock(&g_rx_lock);
            ble_publish_response("unknown", NULL, false, "payload_too_large",
                                 "BLE command payload exceeds buffer limit", NULL);
        }
        else
        {
            memcpy(g_command_buffer + g_command_len, bytes, (size_t)len);
            g_command_len += (size_t)len;
            g_command_buffer[g_command_len] = '\0';

            {
                int brace_depth = 0;
                bool in_string = false;
                bool escape = false;

                for (i = 0; i < g_command_len; i++)
                {
                    char ch = g_command_buffer[i];

                    if (escape)
                    {
                        escape = false;
                        continue;
                    }
                    if (ch == '\\')
                    {
                        escape = in_string;
                        continue;
                    }
                    if (ch == '"')
                    {
                        in_string = !in_string;
                        continue;
                    }
                    if (in_string)
                    {
                        continue;
                    }
                    if (ch == '{')
                    {
                        brace_depth++;
                    }
                    else if (ch == '}')
                    {
                        brace_depth--;
                        if (brace_depth == 0)
                        {
                            complete = true;
                        }
                    }
                }
            }

            if (complete)
            {
                json_text = strdup(g_command_buffer);
                g_command_len = 0;
            }
            else
            {
                pthread_mutex_unlock(&g_rx_lock);
            }
        }

        if (complete)
        {
            pthread_mutex_unlock(&g_rx_lock);
            if (json_text == NULL)
            {
                ble_publish_response("unknown", NULL, false, "oom",
                                     "Unable to allocate BLE command buffer", NULL);
            }
            else if (pthread_create(&worker, NULL, ble_command_worker, json_text) == 0)
            {
                pthread_detach(worker);
            }
            else
            {
                free(json_text);
                ble_publish_response("unknown", NULL, false, "worker_start_failed",
                                     "Failed to start BLE command worker", NULL);
            }
        }
    }

    reply = dbus_message_new_method_return(msg);
    if (reply == NULL)
    {
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    dbus_connection_send(conn, reply, NULL);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);
    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult ble_handle_rsp_read(DBusConnection *conn,
        DBusMessage *msg)
{
    DBusMessage *reply = dbus_message_new_method_return(msg);
    DBusMessageIter iter;

    if (reply == NULL)
    {
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }

    dbus_message_iter_init_append(reply, &iter);
    ble_append_read_value(&iter);
    dbus_connection_send(conn, reply, NULL);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);
    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult ble_handle_start_notify(DBusConnection *conn,
        DBusMessage *msg)
{
    DBusMessage *reply;
    cJSON *event;
    char *json_text;

    pthread_mutex_lock(&g_ble_lock);
    g_notify_enabled = true;
    g_ble_connected = true;
    pthread_mutex_unlock(&g_ble_lock);

    event = ble_make_notify_ready_event();
    if (event != NULL)
    {
        json_text = cJSON_PrintUnformatted(event);
        cJSON_Delete(event);
        if (json_text != NULL)
        {
            ble_publish_json_text(json_text);
            cJSON_free(json_text);
        }
    }

    reply = dbus_message_new_method_return(msg);
    if (reply == NULL)
    {
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    dbus_connection_send(conn, reply, NULL);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);
    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult ble_handle_stop_notify(DBusConnection *conn,
        DBusMessage *msg)
{
    DBusMessage *reply;

    pthread_mutex_lock(&g_ble_lock);
    g_notify_enabled = false;
    g_ble_connected = false;
    pthread_mutex_unlock(&g_ble_lock);

    reply = dbus_message_new_method_return(msg);
    if (reply == NULL)
    {
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    }
    dbus_connection_send(conn, reply, NULL);
    dbus_connection_flush(conn);
    dbus_message_unref(reply);
    return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult ble_root_handler(DBusConnection *conn,
        DBusMessage *msg,
        void *user_data)
{
    (void)user_data;

    if (dbus_message_is_method_call(msg,
                                    "org.freedesktop.DBus.ObjectManager",
                                    "GetManagedObjects"))
    {
        return ble_handle_get_managed_objects(conn, msg);
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult ble_service_handler(DBusConnection *conn,
        DBusMessage *msg,
        void *user_data)
{
    (void)user_data;
    return ble_handle_properties(conn, msg, BLE_SERVICE_PATH);
}

static DBusHandlerResult ble_cmd_char_handler(DBusConnection *conn,
        DBusMessage *msg,
        void *user_data)
{
    (void)user_data;

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_PROPERTIES, "Get") ||
            dbus_message_is_method_call(msg, DBUS_INTERFACE_PROPERTIES, "GetAll"))
    {
        return ble_handle_properties(conn, msg, BLE_CMD_CHAR_PATH);
    }
    if (dbus_message_is_method_call(msg, "org.bluez.GattCharacteristic1", "WriteValue"))
    {
        return ble_handle_cmd_write(conn, msg);
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusHandlerResult ble_rsp_char_handler(DBusConnection *conn,
        DBusMessage *msg,
        void *user_data)
{
    (void)user_data;

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_PROPERTIES, "Get") ||
            dbus_message_is_method_call(msg, DBUS_INTERFACE_PROPERTIES, "GetAll"))
    {
        return ble_handle_properties(conn, msg, BLE_RSP_CHAR_PATH);
    }
    if (dbus_message_is_method_call(msg, "org.bluez.GattCharacteristic1", "ReadValue"))
    {
        return ble_handle_rsp_read(conn, msg);
    }
    if (dbus_message_is_method_call(msg, "org.bluez.GattCharacteristic1", "StartNotify"))
    {
        return ble_handle_start_notify(conn, msg);
    }
    if (dbus_message_is_method_call(msg, "org.bluez.GattCharacteristic1", "StopNotify"))
    {
        return ble_handle_stop_notify(conn, msg);
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusObjectPathVTable g_root_vtable =
{
    .unregister_function = NULL,
    .message_function = ble_root_handler,
};

static DBusObjectPathVTable g_service_vtable =
{
    .unregister_function = NULL,
    .message_function = ble_service_handler,
};

static DBusObjectPathVTable g_cmd_char_vtable =
{
    .unregister_function = NULL,
    .message_function = ble_cmd_char_handler,
};

static DBusObjectPathVTable g_rsp_char_vtable =
{
    .unregister_function = NULL,
    .message_function = ble_rsp_char_handler,
};

static void *ble_dbus_dispatch_thread(void *arg)
{
    (void)arg;

    while (g_dbus_running && g_dbus_conn != NULL)
    {
        dbus_connection_read_write_dispatch(g_dbus_conn, 200);
    }
    return NULL;
}

static DBusMessage *ble_wait_for_method_reply(DBusPendingCall *pending,
        int timeout_ms,
        DBusError *err)
{
    struct timespec start;

    if (pending == NULL)
    {
        return NULL;
    }

    if (timeout_ms >= 0)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    while (!dbus_pending_call_get_completed(pending))
    {
        struct timespec now;
        long elapsed_ms;

        dbus_connection_read_write_dispatch(g_dbus_conn, 100);

        if (timeout_ms < 0)
        {
            continue;
        }

        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed_ms = (now.tv_sec - start.tv_sec) * 1000L +
                     (now.tv_nsec - start.tv_nsec) / 1000000L;
        if (elapsed_ms >= timeout_ms)
        {
            dbus_set_error(err, DBUS_ERROR_NO_REPLY,
                           "Timed out waiting for RegisterApplication reply");
            return NULL;
        }
    }

    return dbus_pending_call_steal_reply(pending);
}

static int ble_register_app(void)
{
    DBusMessage *msg;
    DBusMessage *reply;
    DBusPendingCall *pending = NULL;
    DBusMessageIter args;
    DBusMessageIter options;
    DBusError err;
    char adapter[32];
    char adapter_path[64];
    const char *app_path = BLE_APP_PATH;

    if (ble_get_default_adapter(adapter, sizeof(adapter)) != 0)
    {
        printf("BLE: no adapter found for GATT registration\n");
        return -1;
    }

    snprintf(adapter_path, sizeof(adapter_path), "/org/bluez/%s", adapter);

    msg = dbus_message_new_method_call("org.bluez",
                                       adapter_path,
                                       "org.bluez.GattManager1",
                                       "RegisterApplication");
    if (msg == NULL)
    {
        return -1;
    }

    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &app_path);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &options);
    dbus_message_iter_close_container(&args, &options);

    dbus_error_init(&err);
    if (!dbus_connection_send_with_reply(g_dbus_conn, msg, &pending, 5000) ||
            pending == NULL)
    {
        dbus_message_unref(msg);
        printf("BLE: RegisterApplication failed: unable to queue D-Bus call\n");
        return -1;
    }

    dbus_connection_flush(g_dbus_conn);
    dbus_message_unref(msg);

    reply = ble_wait_for_method_reply(pending, 5000, &err);
    dbus_pending_call_unref(pending);

    if (reply == NULL)
    {
        printf("BLE: RegisterApplication failed: %s\n",
               dbus_error_is_set(&err) ? err.message : "unknown error");
        dbus_error_free(&err);
        return -1;
    }

    if (dbus_set_error_from_message(&err, reply))
    {
        printf("BLE: RegisterApplication failed: %s\n", err.message);
        dbus_error_free(&err);
        dbus_message_unref(reply);
        return -1;
    }

    dbus_message_unref(reply);
    return 0;
}

int ble_provisioning_start(void)
{
    DBusError err;

    if (g_dbus_running)
    {
        return 0;
    }

    dbus_threads_init_default();
    dbus_error_init(&err);
    g_dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (g_dbus_conn == NULL)
    {
        printf("BLE: failed to connect system bus: %s\n",
               dbus_error_is_set(&err) ? err.message : "unknown error");
        dbus_error_free(&err);
        return -1;
    }
    dbus_connection_set_exit_on_disconnect(g_dbus_conn, FALSE);

    if (!dbus_connection_register_object_path(g_dbus_conn, BLE_APP_PATH, &g_root_vtable, NULL) ||
            !dbus_connection_register_object_path(g_dbus_conn, BLE_SERVICE_PATH, &g_service_vtable, NULL) ||
            !dbus_connection_register_object_path(g_dbus_conn, BLE_CMD_CHAR_PATH, &g_cmd_char_vtable, NULL) ||
            !dbus_connection_register_object_path(g_dbus_conn, BLE_RSP_CHAR_PATH, &g_rsp_char_vtable, NULL))
    {
        printf("BLE: failed to register GATT object paths\n");
        ble_provisioning_stop();
        return -1;
    }

    pthread_mutex_lock(&g_ble_lock);
    ble_set_response_locked("{\"type\":\"event\",\"event\":\"idle\",\"protocol_version\":1}");
    pthread_mutex_unlock(&g_ble_lock);

    if (ble_register_app() != 0)
    {
        ble_provisioning_stop();
        return -1;
    }

    g_dbus_running = true;
    if (pthread_create(&g_dbus_thread, NULL, ble_dbus_dispatch_thread, NULL) != 0)
    {
        printf("BLE: failed to create GATT D-Bus thread\n");
        ble_provisioning_stop();
        return -1;
    }

    printf("BLE: provisioning GATT service ready (%s)\n", BLE_PROVISION_SERVICE_UUID);
    return 0;
}

void ble_provisioning_stop(void)
{
    if (g_dbus_conn != NULL)
    {
        dbus_connection_unregister_object_path(g_dbus_conn, BLE_RSP_CHAR_PATH);
        dbus_connection_unregister_object_path(g_dbus_conn, BLE_CMD_CHAR_PATH);
        dbus_connection_unregister_object_path(g_dbus_conn, BLE_SERVICE_PATH);
        dbus_connection_unregister_object_path(g_dbus_conn, BLE_APP_PATH);
    }

    if (g_dbus_running)
    {
        g_dbus_running = false;
        pthread_join(g_dbus_thread, NULL);
    }

    pthread_mutex_lock(&g_ble_lock);
    g_notify_enabled = false;
    g_ble_connected = false;
    g_response_len = 0;
    pthread_mutex_unlock(&g_ble_lock);

    pthread_mutex_lock(&g_rx_lock);
    g_command_len = 0;
    pthread_mutex_unlock(&g_rx_lock);

    if (g_dbus_conn != NULL)
    {
        dbus_connection_unref(g_dbus_conn);
        g_dbus_conn = NULL;
    }
}

bool ble_provisioning_is_connected(void)
{
    bool connected;

    pthread_mutex_lock(&g_ble_lock);
    connected = g_ble_connected;
    pthread_mutex_unlock(&g_ble_lock);
    return connected;
}
