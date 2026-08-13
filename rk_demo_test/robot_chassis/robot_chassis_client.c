#include "robot_chassis_client.h"
#include "robot_chassis_log.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define ROBOT_CHASSIS_TRANSIENT_RETRY_LIMIT 1024U
#define ROBOT_CHASSIS_SEND_WAIT_MS 3000

static ssize_t system_send(int fd, const void *buffer, size_t length,
                           int flags)
{
    return send(fd, buffer, length, flags);
}

static int wait_for_event(int fd, short event, int timeout_ms)
{
    struct pollfd descriptor =
    {
        .fd = fd,
        .events = event
    };
    int result;

    do
    {
        result = poll(&descriptor, 1U, timeout_ms);
    }
    while (result < 0 && errno == EINTR);

    if (result == 0)
    {
        return ROBOT_CHASSIS_ERR_TIMEOUT;
    }
    if (result < 0)
    {
        return ROBOT_CHASSIS_ERR_IO;
    }
    if ((descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)
    {
        return ROBOT_CHASSIS_ERR_PEER_CLOSED;
    }
    if ((descriptor.revents & event) == 0)
    {
        return ROBOT_CHASSIS_ERR_IO;
    }
    return ROBOT_CHASSIS_OK;
}

static int send_all(int fd, const uint8_t *data, size_t length,
                    robot_chassis_send_fn_t send_fn,
                    bool wait_when_blocked)
{
    size_t offset = 0U;
    unsigned int transient_retries = 0U;

    if (fd < 0 || data == NULL || length == 0U || send_fn == NULL)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    while (offset < length)
    {
        const ssize_t sent = send_fn(fd, data + offset, length - offset,
                                     MSG_NOSIGNAL);
        if (sent > 0)
        {
            offset += (size_t)sent;
            transient_retries = 0U;
            continue;
        }
        if (sent == 0)
        {
            return ROBOT_CHASSIS_ERR_PEER_CLOSED;
        }
        if (errno == EINTR)
        {
            if (++transient_retries > ROBOT_CHASSIS_TRANSIENT_RETRY_LIMIT)
            {
                return ROBOT_CHASSIS_ERR_IO;
            }
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            int result;

            if (++transient_retries > ROBOT_CHASSIS_TRANSIENT_RETRY_LIMIT)
            {
                return ROBOT_CHASSIS_ERR_WOULD_BLOCK;
            }
            if (!wait_when_blocked)
            {
                continue;
            }
            result = wait_for_event(fd, POLLOUT, ROBOT_CHASSIS_SEND_WAIT_MS);
            if (result != ROBOT_CHASSIS_OK)
            {
                return result;
            }
            continue;
        }
        return ROBOT_CHASSIS_ERR_IO;
    }

    return ROBOT_CHASSIS_OK;
}

int robot_chassis_client_init(robot_chassis_client_t *client, const char *host,
                              uint16_t port)
{
    size_t host_length;

    if (client == NULL || host == NULL || port == 0U)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    host_length = strlen(host);
    if (host_length == 0U || host_length >= sizeof(client->host))
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    memset(client, 0, sizeof(*client));
    memcpy(client->host, host, host_length + 1U);
    client->port = port;
    client->fd = -1;
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_client_connect(robot_chassis_client_t *client, int timeout_ms)
{
    struct sockaddr_in address;
    int fd;
    int flags;
    int result;
    int socket_error = 0;
    socklen_t error_length = sizeof(socket_error);

    if (client == NULL || client->fd >= 0 || timeout_ms < 0)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        return ROBOT_CHASSIS_ERR_IO;
    }
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        close(fd);
        return ROBOT_CHASSIS_ERR_IO;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(client->port);
    if (inet_pton(AF_INET, client->host, &address.sin_addr) != 1)
    {
        close(fd);
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    result = connect(fd, (const struct sockaddr *)&address, sizeof(address));
    if (result < 0 && errno != EINPROGRESS)
    {
        close(fd);
        return ROBOT_CHASSIS_ERR_IO;
    }
    if (result < 0)
    {
        result = wait_for_event(fd, POLLOUT, timeout_ms);
        if (result != ROBOT_CHASSIS_OK)
        {
            close(fd);
            return result;
        }
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &socket_error, &error_length) < 0 ||
                socket_error != 0)
        {
            close(fd);
            return ROBOT_CHASSIS_ERR_IO;
        }
    }

    client->fd = fd;
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_client_close(robot_chassis_client_t *client)
{
    if (client == NULL || client->fd < 0)
    {
        return;
    }

    close(client->fd);
    client->fd = -1;
}

ssize_t robot_chassis_client_receive(robot_chassis_client_t *client, void *buffer,
                                     size_t length)
{
    ssize_t received;

    if (client == NULL || client->fd < 0 || buffer == NULL || length == 0U)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    do
    {
        received = recv(client->fd, buffer, length, 0);
    }
    while (received < 0 && errno == EINTR);

    if (received > 0)
    {
        return received;
    }
    if (received == 0)
    {
        return ROBOT_CHASSIS_ERR_PEER_CLOSED;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
        return ROBOT_CHASSIS_ERR_WOULD_BLOCK;
    }
    return ROBOT_CHASSIS_ERR_IO;
}

int robot_chassis_client_send_request(robot_chassis_client_t *client,
                                      const char *json, size_t length)
{
    static const uint8_t suffix[ROBOT_CHASSIS_FRAME_SUFFIX_SIZE] =
    {
        '\r', '\n', '\r', '\n'
    };
    uint8_t *request;
    size_t request_length;
    int result;

    if (client == NULL || client->fd < 0 || json == NULL || length == 0U ||
            length > SIZE_MAX - sizeof(suffix))
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    request_length = length + sizeof(suffix);
    request = malloc(request_length);
    if (request == NULL)
    {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    memcpy(request, json, length);
    memcpy(request + length, suffix, sizeof(suffix));

    result = send_all(client->fd, request, request_length, system_send, true);
    if (result == ROBOT_CHASSIS_OK)
    {
        robot_chassis_log_send(json, length);
    }
    else
    {
        robot_chassis_log_send_failed(json, length, result);
    }
    free(request);
    return result;
}

int robot_chassis_client_send_all_for_test(int fd, const uint8_t *data,
        size_t length,
        robot_chassis_send_fn_t send_fn)
{
    return send_all(fd, data, length, send_fn, false);
}
