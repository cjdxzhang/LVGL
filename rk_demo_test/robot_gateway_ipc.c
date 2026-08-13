#include "robot_gateway_ipc.h"

#include "app_manager.h"
#include "cJSON.h"
#include "robot_chassis/robot_chassis_types.h"

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define ROBOT_GATEWAY_IPC_REQUEST_SIZE 1024U
#define ROBOT_GATEWAY_IPC_RESPONSE_SIZE 256U
#define ROBOT_GATEWAY_IPC_BACKLOG 4

static pthread_t g_ipc_thread;
static pthread_mutex_t g_ipc_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_ipc_listen_fd = -1;
static bool g_ipc_running = false;

static bool json_number_to_int(const cJSON *item, int *value)
{
    if (!cJSON_IsNumber(item) || item->valuedouble < (double)INT_MIN ||
            item->valuedouble > (double)INT_MAX)
    {
        return false;
    }
    *value = (int)item->valuedouble;
    return item->valuedouble == (double) * value;
}

static int send_all(int fd, const char *data, size_t length)
{
    size_t offset = 0U;

    while (offset < length)
    {
        ssize_t sent = send(fd, data + offset, length - offset, MSG_NOSIGNAL);

        if (sent < 0 && errno == EINTR)
        {
            continue;
        }
        if (sent <= 0)
        {
            return -1;
        }
        offset += (size_t)sent;
    }
    return 0;
}

static ssize_t receive_line(int fd, char *buffer, size_t size)
{
    size_t length = 0U;

    while (length + 1U < size)
    {
        ssize_t received = recv(fd, buffer + length, 1U, 0);

        if (received < 0 && errno == EINTR)
        {
            continue;
        }
        if (received <= 0)
        {
            break;
        }
        if (buffer[length] == '\n')
        {
            buffer[length] = '\0';
            return (ssize_t)length;
        }
        length++;
    }
    buffer[length] = '\0';
    if (length + 1U == size)
    {
        return -1;
    }
    return (ssize_t)length;
}

static int parse_request(const char *request, size_t length, int *request_id,
                         const char **map_name, cJSON **root_out)
{
    const char *parse_end = NULL;
    const cJSON *id;
    const cJSON *cmd;
    const cJSON *data;
    const cJSON *name;
    cJSON *root;

    root = cJSON_ParseWithLengthOpts(request, length, &parse_end, 0);
    if (root == NULL || parse_end != request + length ||
            !cJSON_IsObject(root))
    {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_JSON;
    }
    id = cJSON_GetObjectItemCaseSensitive(root, "id");
    cmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    data = cJSON_GetObjectItemCaseSensitive(root, "data");
    name = cJSON_IsObject(data) ?
           cJSON_GetObjectItemCaseSensitive(data, "mapname") : NULL;
    if (!json_number_to_int(id, request_id) || !cJSON_IsString(cmd) ||
            cmd->valuestring == NULL ||
            strcmp(cmd->valuestring,
                   "robot_chassis.request_backup_map") != 0 ||
            !cJSON_IsString(name) || name->valuestring == NULL)
    {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    *map_name = name->valuestring;
    *root_out = root;
    return ROBOT_CHASSIS_OK;
}

static int build_response(char *response, size_t size, int request_id,
                          int result)
{
    cJSON *root;
    char *json;
    int written;

    root = cJSON_CreateObject();
    if (root == NULL ||
            cJSON_AddNumberToObject(root, "id", request_id) == NULL ||
            cJSON_AddBoolToObject(root, "success",
                                  result == ROBOT_CHASSIS_OK) == NULL ||
            cJSON_AddNumberToObject(root, "result", result) == NULL)
    {
        cJSON_Delete(root);
        return -1;
    }
    json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (json == NULL)
    {
        return -1;
    }
    written = snprintf(response, size, "%s\n", json);
    cJSON_free(json);
    if (written < 0 || (size_t)written >= size)
    {
        return -1;
    }
    return written;
}

static void handle_client(int client_fd)
{
    char request[ROBOT_GATEWAY_IPC_REQUEST_SIZE];
    char response[ROBOT_GATEWAY_IPC_RESPONSE_SIZE];
    const char *map_name = NULL;
    cJSON *root = NULL;
    ssize_t request_length;
    int request_id = 0;
    int response_length;
    int result;

    request_length = receive_line(client_fd, request, sizeof(request));
    if (request_length <= 0)
    {
        return;
    }
    result = parse_request(request, (size_t)request_length, &request_id,
                           &map_name, &root);
    if (result == ROBOT_CHASSIS_OK)
    {
        result = app_manager_robot_backup_map(map_name);
    }
    cJSON_Delete(root);
    response_length = build_response(response, sizeof(response), request_id,
                                     result);
    if (response_length > 0)
    {
        (void)send_all(client_fd, response, (size_t)response_length);
    }
}

static void *ipc_thread_main(void *argument)
{
    (void)argument;
    for (;;)
    {
        int listen_fd;
        int client_fd;

        pthread_mutex_lock(&g_ipc_mutex);
        listen_fd = g_ipc_listen_fd;
        pthread_mutex_unlock(&g_ipc_mutex);
        if (listen_fd < 0)
        {
            break;
        }
        client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            pthread_mutex_lock(&g_ipc_mutex);
            if (!g_ipc_running)
            {
                pthread_mutex_unlock(&g_ipc_mutex);
                break;
            }
            pthread_mutex_unlock(&g_ipc_mutex);
            continue;
        }
        handle_client(client_fd);
        close(client_fd);
    }
    return NULL;
}

int robot_gateway_ipc_start(void)
{
    struct sockaddr_un address;
    int fd;
    int result;

    pthread_mutex_lock(&g_ipc_mutex);
    if (g_ipc_running)
    {
        pthread_mutex_unlock(&g_ipc_mutex);
        return 0;
    }
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        pthread_mutex_unlock(&g_ipc_mutex);
        return -1;
    }
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    (void)snprintf(address.sun_path, sizeof(address.sun_path), "%s",
                   ROBOT_GATEWAY_IPC_SOCKET_PATH);
    (void)unlink(ROBOT_GATEWAY_IPC_SOCKET_PATH);
    if (bind(fd, (const struct sockaddr *)&address, sizeof(address)) < 0 ||
            listen(fd, ROBOT_GATEWAY_IPC_BACKLOG) < 0)
    {
        close(fd);
        (void)unlink(ROBOT_GATEWAY_IPC_SOCKET_PATH);
        pthread_mutex_unlock(&g_ipc_mutex);
        return -1;
    }
    g_ipc_listen_fd = fd;
    g_ipc_running = true;
    result = pthread_create(&g_ipc_thread, NULL, ipc_thread_main, NULL);
    if (result != 0)
    {
        g_ipc_running = false;
        g_ipc_listen_fd = -1;
        close(fd);
        (void)unlink(ROBOT_GATEWAY_IPC_SOCKET_PATH);
        pthread_mutex_unlock(&g_ipc_mutex);
        return -1;
    }
    pthread_mutex_unlock(&g_ipc_mutex);
    return 0;
}

void robot_gateway_ipc_stop(void)
{
    int fd;
    bool was_running;

    pthread_mutex_lock(&g_ipc_mutex);
    was_running = g_ipc_running;
    fd = g_ipc_listen_fd;
    g_ipc_running = false;
    g_ipc_listen_fd = -1;
    pthread_mutex_unlock(&g_ipc_mutex);
    if (!was_running)
    {
        return;
    }
    if (fd >= 0)
    {
        (void)shutdown(fd, SHUT_RDWR);
        close(fd);
    }
    (void)pthread_join(g_ipc_thread, NULL);
    (void)unlink(ROBOT_GATEWAY_IPC_SOCKET_PATH);
}
