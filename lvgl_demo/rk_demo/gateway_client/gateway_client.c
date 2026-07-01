#include "gateway_client.h"

#include "cJSON.h"

#include <errno.h>
#include <lvgl/lvgl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define GATEWAY_CLIENT_SOCKET_PATH "/run/robot_gateway.sock"
#define GATEWAY_CLIENT_BUF_SIZE 1024

static int g_request_id = 1;

static int connect_gateway(void)
{
	int fd;
	struct sockaddr_un addr;
	// AF_UNIX表示本机进程间通信
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		LV_LOG_WARN("[GATEWAY_CLIENT] create socket failed: errno=%d", errno);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", GATEWAY_CLIENT_SOCKET_PATH);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LV_LOG_WARN("[GATEWAY_CLIENT] connect %s failed: errno=%d",
					GATEWAY_CLIENT_SOCKET_PATH, errno);
		close(fd);
		return -1;
	}

	return fd;
}

static int send_request(cJSON *root)
{
	char *json;
	char response_buf[GATEWAY_CLIENT_BUF_SIZE];
	ssize_t len;
	int fd;
	int ret = GATEWAY_CLIENT_ERR_RESPONSE;
	cJSON *response;
	const cJSON *ok;

	if (root == NULL) {
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	cJSON_AddNumberToObject(root, "id", g_request_id++);
	json = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	if (json == NULL) {
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	fd = connect_gateway();
	if (fd < 0) {
		cJSON_free(json);
		return GATEWAY_CLIENT_ERR_CONNECT;
	}

	if (dprintf(fd, "%s\n", json) < 0) {
		LV_LOG_WARN("[GATEWAY_CLIENT] send failed: errno=%d", errno);
		cJSON_free(json);
		close(fd);
		return GATEWAY_CLIENT_ERR_SEND;
	}
	cJSON_free(json);

	len = read(fd, response_buf, sizeof(response_buf) - 1);
	close(fd);
	if (len <= 0) {
		LV_LOG_WARN("[GATEWAY_CLIENT] recv failed: len=%d errno=%d", (int)len, errno);
		return GATEWAY_CLIENT_ERR_RECV;
	}

	response_buf[len] = '\0';
	response = cJSON_Parse(response_buf);
	if (response == NULL) {
		LV_LOG_WARN("[GATEWAY_CLIENT] parse response failed: %s", response_buf);
		return GATEWAY_CLIENT_ERR_RESPONSE;
	}

	ok = cJSON_GetObjectItemCaseSensitive(response, "ok");
	if (cJSON_IsBool(ok) && cJSON_IsTrue(ok)) {
		ret = GATEWAY_CLIENT_OK;
	}
	cJSON_Delete(response);
	return ret;
}

static int send_simple_command(const char *cmd)
{
	cJSON *root;

	if (cmd == NULL) {
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	root = cJSON_CreateObject();
	if (root == NULL) {
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	cJSON_AddStringToObject(root, "cmd", cmd);
	return send_request(root);
}

static int send_two_string_command(const char *cmd,
								   const char *key1, const char *value1,
								   const char *key2, const char *value2)
{
	cJSON *root;
	cJSON *data;

	if (cmd == NULL || key1 == NULL || value1 == NULL) {
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	root = cJSON_CreateObject();
	data = cJSON_CreateObject();
	if (root == NULL || data == NULL) {
		cJSON_Delete(root);
		cJSON_Delete(data);
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	cJSON_AddStringToObject(root, "cmd", cmd);
	cJSON_AddStringToObject(data, key1, value1);
	if (key2 != NULL && value2 != NULL) {
		cJSON_AddStringToObject(data, key2, value2);
	}
	cJSON_AddItemToObject(root, "data", data);
	return send_request(root);
}

static int send_bool_command(const char *cmd, const char *key, bool value)
{
	cJSON *root;
	cJSON *data;

	if (cmd == NULL || key == NULL) {
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	root = cJSON_CreateObject();
	data = cJSON_CreateObject();
	if (root == NULL || data == NULL) {
		cJSON_Delete(root);
		cJSON_Delete(data);
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	cJSON_AddStringToObject(root, "cmd", cmd);
	cJSON_AddBoolToObject(data, key, value ? 1 : 0);
	cJSON_AddItemToObject(root, "data", data);
	return send_request(root);
}

// 发送移动到指定位置的命令
int gateway_client_go_location(gateway_location_id_t id)
{
	cJSON *root;
	cJSON *data;

	if (id < 0 || id >= GATEWAY_LOCATION_COUNT) {
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	root = cJSON_CreateObject();
	data = cJSON_CreateObject();
	if (root == NULL || data == NULL) {
		cJSON_Delete(root);
		cJSON_Delete(data);
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	cJSON_AddStringToObject(root, "cmd", "robot_chassis.go_location");
	cJSON_AddNumberToObject(data, "location_id", id);
	cJSON_AddItemToObject(root, "data", data);
	return send_request(root);
}

int gateway_client_clear_navigation(void)
{
	// 清除导航
	return send_simple_command("robot_chassis.clear_navigation");
}

int gateway_client_return_base(void)
{
	return send_simple_command("robot_chassis.return_base");
}

int gateway_client_refresh_status(void)
{
	return send_simple_command("robot_chassis.refresh_status");
}

int gateway_client_start_mapping(void)
{
	return send_simple_command("robot_chassis.start_mapping");
}

int gateway_client_stop_mapping(void)
{
	return send_simple_command("robot_chassis.stop_mapping");
}

int gateway_client_save_map(const char *name, const char *alias)
{
	return send_two_string_command("robot_chassis.save_map",
								   "name", name,
								   "alias", alias);
}

int gateway_client_lock_map(const char *mapname)
{
	return send_two_string_command("robot_chassis.lock_map",
								   "mapname", mapname,
								   NULL, NULL);
}

int gateway_client_request_backup_map(const char *mapname)
{
	return send_two_string_command("robot_chassis.request_backup_map",
								   "mapname", mapname,
								   NULL, NULL);
}

int gateway_client_cancel_charge(void)
{
	return send_simple_command("robot_chassis.cancel_charge");
}

int gateway_client_heartbeat(void)
{
	return send_simple_command("robot_chassis.heartbeat");
}

int gateway_client_set_wheel_enable(bool enable)
{
	return send_bool_command("robot_chassis.set_wheel_enable", "enable", enable);
}

int gateway_client_manual_control(double speed, double angle)
{
	cJSON *root;
	cJSON *data;

	root = cJSON_CreateObject();
	data = cJSON_CreateObject();
	if (root == NULL || data == NULL) {
		cJSON_Delete(root);
		cJSON_Delete(data);
		return GATEWAY_CLIENT_ERR_PARAM;
	}

	cJSON_AddStringToObject(root, "cmd", "robot_chassis.manual_control");
	cJSON_AddNumberToObject(data, "speed", speed);
	cJSON_AddNumberToObject(data, "angle", angle);
	cJSON_AddItemToObject(root, "data", data);
	return send_request(root);
}
