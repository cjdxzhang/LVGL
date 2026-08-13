#include "cJSON.h"
#include "robot_gateway_ipc.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define ROBOT_GATEWAY_CLI_RESPONSE_SIZE 256U

static void print_usage(const char *program)
{
    const char *name = program == NULL ? "robot_gateway_cli" : program;

    fprintf(stderr, "用法：%s backup-map <地图文件名>\n", name);
}

static char *build_request(const char *map_name)
{
    cJSON *root;
    cJSON *data;
    char *request;

    root = cJSON_CreateObject();
    data = cJSON_CreateObject();
    if (root == NULL || data == NULL ||
            cJSON_AddNumberToObject(root, "id", 1) == NULL ||
            cJSON_AddStringToObject(root, "cmd",
                                    "robot_chassis.request_backup_map") == NULL ||
            cJSON_AddStringToObject(data, "mapname", map_name) == NULL)
    {
        cJSON_Delete(data);
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, "data", data);
    request = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return request;
}

static int connect_gateway(void)
{
    struct sockaddr_un address;
    int fd;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        fprintf(stderr, "创建 IPC socket 失败：%s\n", strerror(errno));
        return -1;
    }
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    (void)snprintf(address.sun_path, sizeof(address.sun_path), "%s",
                   ROBOT_GATEWAY_IPC_SOCKET_PATH);
    if (connect(fd, (const struct sockaddr *)&address, sizeof(address)) < 0)
    {
        fprintf(stderr, "连接 %s 失败：%s\n",
                ROBOT_GATEWAY_IPC_SOCKET_PATH, strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
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

static ssize_t receive_response(int fd, char *response, size_t size)
{
    size_t length = 0U;

    while (length + 1U < size)
    {
        ssize_t received = recv(fd, response + length,
                                size - length - 1U, 0);

        if (received < 0 && errno == EINTR)
        {
            continue;
        }
        if (received <= 0)
        {
            break;
        }
        length += (size_t)received;
        if (memchr(response, '\n', length) != NULL)
        {
            break;
        }
    }
    response[length] = '\0';
    return (ssize_t)length;
}

static bool response_succeeded(const char *response, size_t length)
{
    const char *parse_end = NULL;
    const cJSON *success;
    cJSON *root;
    bool succeeded;

    root = cJSON_ParseWithLengthOpts(response, length, &parse_end, 0);
    if (root == NULL || !cJSON_IsObject(root))
    {
        cJSON_Delete(root);
        return false;
    }
    success = cJSON_GetObjectItemCaseSensitive(root, "success");
    succeeded = cJSON_IsBool(success) && cJSON_IsTrue(success);
    cJSON_Delete(root);
    return succeeded;
}

int main(int argc, char **argv)
{
    char response[ROBOT_GATEWAY_CLI_RESPONSE_SIZE];
    char *request;
    ssize_t response_length;
    int fd;
    int result = 1;

    if (argc != 3 || strcmp(argv[1], "backup-map") != 0 ||
            argv[2][0] == '\0')
    {
        print_usage(argc > 0 ? argv[0] : NULL);
        return 1;
    }
    request = build_request(argv[2]);
    if (request == NULL)
    {
        fprintf(stderr, "构造 IPC 请求失败\n");
        return 2;
    }
    fd = connect_gateway();
    if (fd < 0)
    {
        cJSON_free(request);
        return 3;
    }
    if (send_all(fd, request, strlen(request)) != 0 ||
            send_all(fd, "\n", 1U) != 0)
    {
        fprintf(stderr, "发送 IPC 请求失败：%s\n", strerror(errno));
        goto out;
    }
    response_length = receive_response(fd, response, sizeof(response));
    if (response_length <= 0)
    {
        fprintf(stderr, "读取 IPC 响应失败\n");
        goto out;
    }
    fputs(response, stdout);
    if (response[response_length - 1] != '\n')
    {
        fputc('\n', stdout);
    }
    result = response_succeeded(response, (size_t)response_length) ? 0 : 4;

out:
    close(fd);
    cJSON_free(request);
    return result;
}
