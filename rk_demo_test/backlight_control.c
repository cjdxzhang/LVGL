#include "backlight_control.h"

#include "app_log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLIGHT_LOG_USER(...) APP_LOG_USER("BACKLIGHT", __VA_ARGS__)
#define BACKLIGHT_LOG_ERROR(...) APP_LOG_ERROR("BACKLIGHT", __VA_ARGS__)

#define BACKLIGHT_DEFAULT_BRIGHTNESS 255

static const char *g_backlight_paths[] = {
	"/sys/devices/platform/backlight/backlight/backlight/brightness",
	"/sys/class/backlight/backlight/brightness"
};

static int g_backlight_fd = -1;
static int g_restore_brightness = BACKLIGHT_DEFAULT_BRIGHTNESS;
static const char *g_backlight_path = NULL;

static bool backlight_write_value(int value)
{
	char text[16];
	int length;
	ssize_t written;

	if (g_backlight_fd < 0) {
		return false;
	}

	length = snprintf(text, sizeof(text), "%d", value);
	if (length <= 0 || length >= (int)sizeof(text)) {
		BACKLIGHT_LOG_ERROR("生成背光亮度文本失败: value=%d", value);
		return false;
	}

	if (lseek(g_backlight_fd, 0, SEEK_SET) < 0) {
		BACKLIGHT_LOG_ERROR("定位背光节点失败: path=%s error=%s",
					    g_backlight_path, strerror(errno));
		return false;
	}

	written = write(g_backlight_fd, text, (size_t)length);
	if (written != length) {
		BACKLIGHT_LOG_ERROR("写入背光节点失败: path=%s value=%d written=%zd error=%s",
					    g_backlight_path, value, written, strerror(errno));
		return false;
	}
	return true;
}

static void backlight_read_current(void)
{
	char text[16] = {0};
	ssize_t length;
	long value;
	char *end = NULL;

	if (lseek(g_backlight_fd, 0, SEEK_SET) < 0) {
		return;
	}
	length = read(g_backlight_fd, text, sizeof(text) - 1u);
	if (length <= 0) {
		return;
	}

	errno = 0;
	value = strtol(text, &end, 10);
	if (errno == 0 && end != text && value > 0 && value <= 65535) {
		g_restore_brightness = (int)value;
	}
}

static bool backlight_open(void)
{
	size_t index;

	if (g_backlight_fd >= 0) {
		return true;
	}

	for (index = 0; index < sizeof(g_backlight_paths) / sizeof(g_backlight_paths[0]); index++) {
		g_backlight_fd = open(g_backlight_paths[index], O_RDWR | O_CLOEXEC);
		if (g_backlight_fd >= 0) {
			g_backlight_path = g_backlight_paths[index];
			backlight_read_current();
			BACKLIGHT_LOG_USER("已打开背光节点: path=%s restore=%d",
					   g_backlight_path, g_restore_brightness);
			return true;
		}
	}

	BACKLIGHT_LOG_ERROR("打开背光节点失败: error=%s", strerror(errno));
	return false;
}

bool backlight_control_sleep(void)
{
	if (!backlight_open()) {
		return false;
	}
	backlight_read_current();
	return backlight_write_value(0);
}

bool backlight_control_wake(void)
{
	if (!backlight_open()) {
		return false;
	}
	return backlight_write_value(g_restore_brightness);
}

void backlight_control_deinit(void)
{
	if (g_backlight_fd >= 0) {
		close(g_backlight_fd);
		g_backlight_fd = -1;
	}
	g_backlight_path = NULL;
}
