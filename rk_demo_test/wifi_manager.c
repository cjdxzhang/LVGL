#include "wifi_manager.h"
#include "app_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include "lvgl.h"

#define WIFI_MANAGER_LOG_USER(...) APP_LOG_USER("WIFI_MANAGER", __VA_ARGS__)
#define WIFI_MANAGER_LOG_WARN(...) APP_LOG_WARN("WIFI_MANAGER", __VA_ARGS__)
#define WIFI_MANAGER_LOG_ERROR(...) APP_LOG_ERROR("WIFI_MANAGER", __VA_ARGS__)

// 最大扫描结果数量
#define MAX_SCAN_RESULTS 50
// SSID 最大长度
#define MAX_SSID_LEN 64
// 回调最大数量
#define MAX_CALLBACKS 5
#define WIFI_STATUS_POLL_INTERVAL_SEC 2

// 扫描结果缓存
static char scan_results[MAX_SCAN_RESULTS][MAX_SSID_LEN];
//static int scan_count = 0;
static pthread_mutex_t scan_mutex = PTHREAD_MUTEX_INITIALIZER;

#define WPA_SUPPLICANT_CONF "/etc/wpa_supplicant.conf"
#define WPA_CTRL_DIR "/var/run/wpa_supplicant"

// 当前连接状态
static bool current_connected = false;
static char current_ssid[MAX_SSID_LEN] = {0};
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t status_monitor_thread;
static bool status_monitor_started = false;
static bool status_query_failure_reported = false;

// 回调列表
static wifi_status_callback_t callbacks[MAX_CALLBACKS];
static void *callback_user_data[MAX_CALLBACKS];
static int callback_count = 0;
static pthread_mutex_t callback_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *ssid_list[50];
static int ssid_count = 0;

static void clear_scan_results_locked(void) {
    for (int i = 0; i < ssid_count; i++) {
        free(ssid_list[i]);
        ssid_list[i] = NULL;
    }
    ssid_count = 0;
}

// 辅助函数：执行命令并获取输出（简单封装）
static int exec_command(const char *cmd, char *output, size_t output_size) {
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) return -1;
    
    size_t total = 0;
    while (fgets(output + total, output_size - total, fp) != NULL) {
        total = strlen(output);
        if (total >= output_size - 1) break;
    }
    
    int status = pclose(fp);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

// 辅助函数：执行命令不关心输出
static int exec_simple(const char *cmd) {
    int status = system(cmd);
    if (status == -1) return -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static int ensure_wpa_config_has_ctrl_interface(void) {
    FILE *fp = fopen(WPA_SUPPLICANT_CONF, "r");
    char content[2048] = {0};
    size_t len = 0;
    bool has_ctrl_interface = false;

    if (fp != NULL) {
        len = fread(content, 1, sizeof(content) - 1, fp);
        fclose(fp);
        if (strstr(content, "ctrl_interface=") != NULL) {
            has_ctrl_interface = true;
        }
    }

    if (has_ctrl_interface) {
        return 0;
    }

    fp = fopen(WPA_SUPPLICANT_CONF, "w");
    if (fp == NULL) {
        return -1;
    }

    if (fprintf(fp, "ctrl_interface=%s\nap_scan=1\n", WPA_CTRL_DIR) < 0) {
        fclose(fp);
        return -1;
    }

    if (len > 0 && fwrite(content, 1, len, fp) != len) {
        fclose(fp);
        return -1;
    }

    return fclose(fp) == 0 ? 0 : -1;
}

static int wpa_cli_is_ready(void) {
    return exec_simple("wpa_cli -i wlan0 ping >/dev/null 2>&1") == 0;
}

static int write_wpa_config(FILE *fp, const char *ssid, const char *psk) {
    if (fprintf(fp,
                "ctrl_interface=%s\n"
                "update_config=1\n"
                "ap_scan=1\n"
                "network={\n"
                "    ssid=\"%s\"\n"
                "    psk=\"%s\"\n"
                "}\n",
                WPA_CTRL_DIR,
                ssid,
                psk) < 0) {
        return -1;
    }

    return 0;
}

// 触发所有回调
static void notify_status_change(bool connected, const char *ssid) {
    pthread_mutex_lock(&callback_mutex);
    for (int i = 0; i < callback_count; i++) {
        if (callbacks[i]) {
            callbacks[i](connected, ssid, callback_user_data[i]);
        }
    }
    pthread_mutex_unlock(&callback_mutex);
}

static bool parse_wpa_status(const char *status, char *ssid, size_t ssid_size) {
    bool connected = false;
    const char *line = status;

    if (ssid != NULL && ssid_size > 0) {
        ssid[0] = '\0';
    }

    while (line != NULL && line[0] != '\0') {
        const char *line_end = strchr(line, '\n');
        size_t line_len = line_end != NULL ? (size_t)(line_end - line) : strlen(line);

        if (line_len == strlen("wpa_state=COMPLETED") &&
            strncmp(line, "wpa_state=COMPLETED", line_len) == 0) {
            connected = true;
        } else if (strncmp(line, "ssid=", strlen("ssid=")) == 0 &&
                   ssid != NULL && ssid_size > 0) {
            size_t copy_len = line_len - strlen("ssid=");

            if (copy_len >= ssid_size) {
                copy_len = ssid_size - 1;
            }
            memcpy(ssid, line + strlen("ssid="), copy_len);
            ssid[copy_len] = '\0';
        }

        if (line_end == NULL) {
            break;
        }
        line = line_end + 1;
    }

    return connected;
}

// 更新当前连接状态
static void update_connection_status(void) {
    char buf[256] = {0};
    char notify_ssid[MAX_SSID_LEN] = {0};
    bool status_changed = false;
    int ret;

    ret = exec_command("wpa_cli -i wlan0 status 2>/dev/null", buf, sizeof(buf));
    if (ret != 0 && !status_query_failure_reported) {
        WIFI_MANAGER_LOG_WARN("查询 Wi-Fi 连接状态失败: ret=%d", ret);
        status_query_failure_reported = true;
    } else if (ret == 0 && status_query_failure_reported) {
        WIFI_MANAGER_LOG_USER("Wi-Fi 连接状态查询恢复");
        status_query_failure_reported = false;
    }
    
    char new_ssid[MAX_SSID_LEN] = {0};
    bool new_connected = parse_wpa_status(buf, new_ssid, sizeof(new_ssid));
    
    pthread_mutex_lock(&status_mutex);
    if (new_connected != current_connected || strcmp(new_ssid, current_ssid) != 0) {
        current_connected = new_connected;
        strncpy(current_ssid, new_ssid, MAX_SSID_LEN - 1);
        current_ssid[MAX_SSID_LEN - 1] = '\0';
        strncpy(notify_ssid, current_ssid, MAX_SSID_LEN - 1);
        status_changed = true;
    }
    pthread_mutex_unlock(&status_mutex);

    if (status_changed) {
        WIFI_MANAGER_LOG_USER("连接状态变化: connected=%d ssid=%s",
                              new_connected ? 1 : 0,
                              notify_ssid[0] != '\0' ? notify_ssid : "<none>");
        notify_status_change(new_connected, notify_ssid);
    }
}

static void *wifi_status_monitor(void *arg) {
    (void)arg;

    while (true) {
        sleep(WIFI_STATUS_POLL_INTERVAL_SEC);
        update_connection_status();
    }

    return NULL;
}

static void start_status_monitor(void) {
    int ret;

    if (status_monitor_started) {
        return;
    }

    ret = pthread_create(&status_monitor_thread, NULL, wifi_status_monitor, NULL);
    if (ret != 0) {
        WIFI_MANAGER_LOG_ERROR("创建 Wi-Fi 状态监控线程失败: %s", strerror(ret));
        return;
    }

    pthread_detach(status_monitor_thread);
    status_monitor_started = true;
    WIFI_MANAGER_LOG_USER("Wi-Fi 状态监控线程启动成功: interval=%ds",
                          WIFI_STATUS_POLL_INTERVAL_SEC);
}

// 检查 wpa_supplicant 是否运行，如果没有则启动
static void ensure_wpa_supplicant_running(void) {
    int ret;

    if (ensure_wpa_config_has_ctrl_interface() != 0) {
        WIFI_MANAGER_LOG_ERROR("准备 wpa_supplicant 配置失败: path=%s",
                               WPA_SUPPLICANT_CONF);
        return;
    }

    if (wpa_cli_is_ready()) {
        return;
    }

    WIFI_MANAGER_LOG_USER("wpa_supplicant 未运行，尝试启动");
    ret = exec_simple("mkdir -p /var/run/wpa_supplicant");
    if (ret != 0) {
        WIFI_MANAGER_LOG_ERROR("创建 wpa_supplicant 控制目录失败: ret=%d", ret);
    }
    exec_simple("killall wpa_supplicant >/dev/null 2>&1");
    ret = exec_simple("wpa_supplicant -B -i wlan0 -C /var/run/wpa_supplicant -c /etc/wpa_supplicant.conf >/dev/null 2>&1");
    if (ret != 0) {
        WIFI_MANAGER_LOG_ERROR("启动 wpa_supplicant 失败: ret=%d", ret);
    }
    sleep(2);
}

// 转义字符串中的特殊字符（用于 wpa_supplicant.conf）
static int escape_string(const char *input, char *output, size_t out_size) {
    if (!input || !output || out_size == 0) return -1;
    size_t in_idx = 0, out_idx = 0;
    while (input[in_idx] != '\0') {
        if (input[in_idx] == '\n' || input[in_idx] == '\r') return -1;
        if (input[in_idx] == '\\' || input[in_idx] == '"') {
            if (out_idx + 2 >= out_size) return -1;
            output[out_idx++] = '\\';
        } else if (out_idx + 1 >= out_size) {
            return -1;
        }
        output[out_idx++] = input[in_idx++];
    }
    output[out_idx] = '\0';
    return 0;
}

// ========== 公共 API 实现 ==========

void wifi_manager_init(void) {
    pthread_mutex_init(&scan_mutex, NULL);
    pthread_mutex_init(&callback_mutex, NULL);
    ensure_wpa_supplicant_running();
    update_connection_status();
    start_status_monitor();
    WIFI_MANAGER_LOG_USER("Wi-Fi 管理器初始化完成");
}

int wifi_manager_register_callback(wifi_status_callback_t callback, void *user_data) {
    if (!callback) {
        WIFI_MANAGER_LOG_ERROR("注册状态回调失败: callback 为空");
        return -1;
    }
    pthread_mutex_lock(&callback_mutex);
    if (callback_count >= MAX_CALLBACKS) {
        pthread_mutex_unlock(&callback_mutex);
        WIFI_MANAGER_LOG_WARN("注册状态回调失败: 已达到上限 %d", MAX_CALLBACKS);
        return -1;
    }
    callbacks[callback_count] = callback;
    callback_user_data[callback_count] = user_data;
    callback_count++;
    pthread_mutex_unlock(&callback_mutex);
    return 0;
}

int wifi_manager_scan_start(void) {
    FILE *fp;
    char line[256];
    int result_count;

    WIFI_MANAGER_LOG_USER("开始扫描 Wi-Fi");

    ensure_wpa_supplicant_running();

    /* 1. 触发扫描 */
    fp = popen("wpa_cli scan", "r");
    if (fp == NULL) {
        WIFI_MANAGER_LOG_ERROR("触发 Wi-Fi 扫描失败: %s", strerror(errno));
        return -1;
    }
    pclose(fp);

    /* 等待扫描完成 */
    lv_task_handler();
    lv_timer_handler();
    /* 从实际使用经验看，wpa_supplicant 需要几秒钟来收集扫描结果 */
    sleep(3);
    /* 2. 获取扫描结果
     * 关键：使用 -i 参数指定无线接口。你的设备可能是 wlan0 或 wlan1，
     * 请通过 SSH 登录板子后执行 `iwconfig` 命令确认 [citation:7]
     */
    fp = popen("wpa_cli -i wlan0 scan_results", "r");
    if (fp == NULL) {
        WIFI_MANAGER_LOG_ERROR("读取 Wi-Fi 扫描结果失败: %s", strerror(errno));
        return -1;
    }
    /* 3. 解析结果并提取 SSID */
    if (fgets(line, sizeof(line), fp) == NULL) {
        pclose(fp);
        WIFI_MANAGER_LOG_ERROR("Wi-Fi 扫描结果缺少表头");
        return -1;
    }

    pthread_mutex_lock(&scan_mutex);
    clear_scan_results_locked();
    while (fgets(line, sizeof(line), fp) != NULL && ssid_count < MAX_SCAN_RESULTS) {
        char *ssid_start = strrchr(line, '\t');
        if (ssid_start != NULL) {
            ssid_start++;
            char *ssid_end = strchr(ssid_start, '\n');
            if (ssid_end) *ssid_end = '\0';
            if (strlen(ssid_start) > 0) {
                ssid_list[ssid_count] = strdup(ssid_start);
                if (ssid_list[ssid_count] != NULL) {
                    ssid_count++;
                }
            }
        }
    }
    result_count = ssid_count;
    pthread_mutex_unlock(&scan_mutex);

    pclose(fp);
    WIFI_MANAGER_LOG_USER("Wi-Fi 扫描完成: count=%d", result_count);
    return 0;
}

int wifi_manager_get_scan_count(void) {
    pthread_mutex_lock(&scan_mutex);
    int cnt = ssid_count;
    pthread_mutex_unlock(&scan_mutex);
    return cnt;
}

int wifi_manager_get_scan_ssid(int index, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;
    pthread_mutex_lock(&scan_mutex);
    if (index < 0 || index >= ssid_count) {
        pthread_mutex_unlock(&scan_mutex);
        return -1;
    }
    strncpy(buffer, ssid_list[index], buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    pthread_mutex_unlock(&scan_mutex);
    return 0;
}

int wifi_manager_connect(const char *ssid, const char *psk) {
    int ret;

    if (!ssid || !psk || ssid[0] == '\0' || psk[0] == '\0') {
        WIFI_MANAGER_LOG_ERROR("连接 Wi-Fi 失败: SSID 或密码无效");
        return -1;
    }

    WIFI_MANAGER_LOG_USER("开始连接 Wi-Fi: ssid=%s", ssid);
    
    // 写入配置文件
    char escaped_ssid[256], escaped_psk[256];
    if (escape_string(ssid, escaped_ssid, sizeof(escaped_ssid)) != 0 ||
        escape_string(psk, escaped_psk, sizeof(escaped_psk)) != 0) {
        WIFI_MANAGER_LOG_ERROR("连接 Wi-Fi 失败: SSID 或密码包含非法字符");
        return -1;
    }
    
    FILE *fp = fopen("/tmp/wpa_tmp.conf", "w");
    if (!fp) {
        WIFI_MANAGER_LOG_ERROR("创建临时 Wi-Fi 配置失败: %s", strerror(errno));
        return -1;
    }
    if (write_wpa_config(fp, escaped_ssid, escaped_psk) != 0) {
        fclose(fp);
        WIFI_MANAGER_LOG_ERROR("写入临时 Wi-Fi 配置失败");
        return -1;
    }
    fclose(fp);
    
    // 替换配置文件
    ret = exec_simple("cp /tmp/wpa_tmp.conf /etc/wpa_supplicant.conf");
    exec_simple("rm /tmp/wpa_tmp.conf");
    if (ret != 0) {
        WIFI_MANAGER_LOG_ERROR("安装 Wi-Fi 配置失败: ret=%d", ret);
    }

    ensure_wpa_supplicant_running();
    
    // 重新加载配置
    ret = exec_simple("wpa_cli -i wlan0 reconfigure 2>/dev/null");
    if (ret != 0) {
        WIFI_MANAGER_LOG_ERROR("重新加载 Wi-Fi 配置失败: ret=%d", ret);
    }

    exec_simple("wpa_cli -i wlan0 disconnect 2>/dev/null");
    ret = exec_simple("wpa_cli -i wlan0 reconnect 2>/dev/null");
    if (ret != 0) {
        WIFI_MANAGER_LOG_ERROR("重新连接 Wi-Fi 失败: ret=%d", ret);
    }
    
    // 等待连接
    for (int i = 0; i < 15; i++) {
        bool connected;
        char connected_ssid[MAX_SSID_LEN] = {0};

        sleep(1);
        update_connection_status();
        pthread_mutex_lock(&status_mutex);
        connected = current_connected;
        strncpy(connected_ssid, current_ssid, sizeof(connected_ssid) - 1);
        pthread_mutex_unlock(&status_mutex);
        if (connected && strcmp(connected_ssid, ssid) == 0) {
            exec_simple("dhcpcd -n wlan0 >/dev/null 2>&1 || udhcpc -i wlan0 -b >/dev/null 2>&1");
            update_connection_status();
            WIFI_MANAGER_LOG_USER("连接 Wi-Fi 成功: ssid=%s", ssid);
            return 0;
        }
    }

    WIFI_MANAGER_LOG_WARN("连接 Wi-Fi 超时: ssid=%s timeout=15s", ssid);
    return -1;
}

bool wifi_manager_is_connected(void) {
    bool connected;

    update_connection_status();
    pthread_mutex_lock(&status_mutex);
    connected = current_connected;
    pthread_mutex_unlock(&status_mutex);
    return connected;
}

int wifi_manager_get_current_ssid(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;
    update_connection_status();
    pthread_mutex_lock(&status_mutex);
    strncpy(buffer, current_ssid, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    pthread_mutex_unlock(&status_mutex);
    return strlen(buffer) > 0 ? 0 : -1;
}

int wifi_manager_disconnect(void) {
    int ret = exec_simple("wpa_cli -i wlan0 disconnect 2>/dev/null");
    if (ret == 0) {
        pthread_mutex_lock(&status_mutex);
        current_connected = false;
        current_ssid[0] = '\0';
        pthread_mutex_unlock(&status_mutex);
        WIFI_MANAGER_LOG_USER("Wi-Fi 已断开");
        notify_status_change(false, "");
    } else {
        WIFI_MANAGER_LOG_ERROR("断开 Wi-Fi 失败: ret=%d", ret);
    }
    return ret;
}

int wifi_manager_persist_config(const char *ssid, const char *psk) {
    int ret;

    if (ssid == NULL || psk == NULL) {
        WIFI_MANAGER_LOG_ERROR("保存 Wi-Fi 配置失败: SSID 或密码为空");
        return -1;
    }

    // 先写入配置文件
    char escaped_ssid[256], escaped_psk[256];
    if (escape_string(ssid, escaped_ssid, sizeof(escaped_ssid)) != 0 ||
        escape_string(psk, escaped_psk, sizeof(escaped_psk)) != 0) {
        WIFI_MANAGER_LOG_ERROR("保存 Wi-Fi 配置失败: SSID 或密码包含非法字符");
        return -1;
    }
    
    FILE *fp = fopen("/etc/wpa_supplicant.conf", "w");
    if (!fp) {
        WIFI_MANAGER_LOG_ERROR("打开 Wi-Fi 配置失败: path=%s error=%s",
                               WPA_SUPPLICANT_CONF, strerror(errno));
        return -1;
    }
    if (write_wpa_config(fp, escaped_ssid, escaped_psk) != 0) {
        fclose(fp);
        WIFI_MANAGER_LOG_ERROR("写入 Wi-Fi 配置失败: path=%s",
                               WPA_SUPPLICANT_CONF);
        return -1;
    }
    fclose(fp);
    
    // 创建开机启动脚本
    const char *script_cmd = 
        "cat > /etc/init.d/S99wifi << 'EOF'\n"
        "#!/bin/sh\n"
        "case \"$1\" in\n"
        "    start)\n"
        "        sleep 2\n"
        "        mkdir -p /var/run/wpa_supplicant\n"
        "        wpa_supplicant -B -i wlan0 -C /var/run/wpa_supplicant -c /etc/wpa_supplicant.conf\n"
        "        sleep 3\n"
        "        udhcpc -i wlan0 -b\n"
        "        ;;\n"
        "    stop)\n"
        "        killall wpa_supplicant 2>/dev/null\n"
        "        killall udhcpc 2>/dev/null\n"
        "        ;;\n"
        "esac\n"
        "exit 0\n"
        "EOF\n"
        "chmod +x /etc/init.d/S99wifi\n";
    
    ret = exec_simple(script_cmd);
    if (ret != 0) {
        WIFI_MANAGER_LOG_ERROR("创建 Wi-Fi 自启动脚本失败: ret=%d", ret);
    } else {
        WIFI_MANAGER_LOG_USER("Wi-Fi 配置保存成功: ssid=%s", ssid);
    }
    return 0;
}
