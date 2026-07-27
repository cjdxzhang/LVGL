#include "robot_tcp.h"
#include "app_log.h"
#include <errno.h>
#include <fcntl.h>     // fcntl(), O_NONBLOCK
#include <stdint.h>    // uint32_t
#include <sys/time.h>  // struct timeval

#define ROBOT_TCP_LOG_USER(...) APP_LOG_USER("ROBOT_TCP", __VA_ARGS__)
#define ROBOT_TCP_LOG_WARN(...) APP_LOG_WARN("ROBOT_TCP", __VA_ARGS__)
#define ROBOT_TCP_LOG_ERROR(...) APP_LOG_ERROR("ROBOT_TCP", __VA_ARGS__)

// ================= 全局变量 =================
static int g_tcp_socket = -1;
static pthread_t g_work_thread = 0;
static pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static RobotState_t g_robot_state = {0};
static int g_running = 0;
int g_battery_percent = 100;

static MapStations_t g_stations_cache = {0};  // 站点缓存
static pthread_mutex_t g_stations_mutex = PTHREAD_MUTEX_INITIALIZER;

static void log_tcp_payload(const char* direction, const char* data, int len) {
    if (!direction) {
        direction = "TCP";
    }

    if (!data || len <= 0) {
        ROBOT_TCP_LOG_WARN("%s 数据为空: len=%d", direction, len);
        return;
    }

    /* 默认不打印完整载荷，避免高频日志和业务数据扩散。 */
}

// ================= 内部辅助函数 =================

// 建立TCP连接（带超时机制）
static int tcp_connect(void) {
    struct sockaddr_in server_addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        ROBOT_TCP_LOG_ERROR("socket 创建失败: %s", strerror(errno));
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ROBOT_PORT);
    if (inet_pton(AF_INET, ROBOT_IP, &server_addr.sin_addr) != 1) {
        ROBOT_TCP_LOG_ERROR("机器人 IP 地址无效: %s", ROBOT_IP);
        close(sock);
        return -1;
    }

    // ── 使用非阻塞 connect + select() 实现超时 ──
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    int rc = connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc < 0 && errno != EINPROGRESS) {
        ROBOT_TCP_LOG_ERROR("连接机器人失败: address=%s:%d error=%s",
                            ROBOT_IP, ROBOT_PORT, strerror(errno));
        close(sock);
        return -1;
    }

    if (rc != 0) {
        // connect 正在进行中，用 select 等待结果
        struct timeval tv;
        tv.tv_sec = 5;  // 5 秒超时
        tv.tv_usec = 0;
        fd_set write_set;
        FD_ZERO(&write_set);
        FD_SET(sock, &write_set);

        int sel = select(sock + 1, NULL, &write_set, NULL, &tv);
        if (sel <= 0) {
            if (sel == 0) {
                ROBOT_TCP_LOG_WARN("连接机器人超时: address=%s:%d timeout=5s",
                                   ROBOT_IP, ROBOT_PORT);
            } else {
                ROBOT_TCP_LOG_ERROR("等待机器人连接失败: %s", strerror(errno));
            }
            close(sock);
            return -1;
        }

        // 检查 connect 是否真的成功
        int so_error = 0;
        socklen_t err_len = sizeof(so_error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &err_len);
        if (so_error != 0) {
            ROBOT_TCP_LOG_ERROR("连接机器人失败: address=%s:%d error=%s",
                                ROBOT_IP, ROBOT_PORT, strerror(so_error));
            close(sock);
            return -1;
        }
    }

    // 恢复阻塞模式
    fcntl(sock, F_SETFL, flags);

    // 设置接收超时，避免 recv 无限阻塞
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500ms
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    return sock;
}

// 发送原始数据 (带\r\n\r\n)
static int send_data(int sock, const char* data, int len) {
    if (sock < 0 || !data) return -1;

    log_tcp_payload("TCP SEND", data, len);
    
    int sent = send(sock, data, len, 0);
    if (sent <= 0) {
        ROBOT_TCP_LOG_ERROR("发送 TCP 数据失败: %s", strerror(errno));
        return sent;
    }
    if (sent < len) {
        ROBOT_TCP_LOG_WARN("TCP 数据发送不完整: sent=%d expected=%d", sent, len);
    }

    // 发送结束符
    int end_sent = send(sock, "\r\n\r\n", 4, 0);
    if (end_sent <= 0) {
        ROBOT_TCP_LOG_ERROR("发送 TCP 结束符失败: %s", strerror(errno));
        return -1;
    }
    if (end_sent < 4) {
        ROBOT_TCP_LOG_WARN("TCP 结束符发送不完整: sent=%d expected=4", end_sent);
    }
    return (end_sent > 0) ? sent : -1;
}

// 接收完整包
// 协议格式: [4字节小端包长度][JSON字符串]\r\n\r\n
// 返回值：>0 成功收到完整包（返回JSON长度）；0 连接关闭；-1 错误；-2 超时（无数据可读）
static int recv_packet(int sock, char** buffer) {
    // ── 1. 读取 4 字节长度头 ──
    uint32_t packet_len = 0;
    int ret = recv(sock, &packet_len, 4, MSG_WAITALL);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            *buffer = NULL;
            return -2;  // 超时
        }
        ROBOT_TCP_LOG_ERROR("读取 TCP 长度头失败: %s", strerror(errno));
        return -1;
    }
    if (ret == 0) {
        ROBOT_TCP_LOG_WARN("机器人关闭 TCP 连接");
        *buffer = NULL;
        return 0;
    }

    // 合法性检查
    if (packet_len == 0 || packet_len > 10 * 1024 * 1024) {
        ROBOT_TCP_LOG_ERROR("TCP 数据包长度无效: %u", packet_len);
        return -1;
    }

    // ── 2. 读取完整包体（含末尾 \r\n\r\n）──
    char* full_buffer = (char*)malloc(packet_len + 1);
    if (!full_buffer) {
        ROBOT_TCP_LOG_ERROR("TCP 接收缓冲区分配失败: size=%u", packet_len + 1);
        return -1;
    }

    int total_read = 0;
    while (total_read < (int)packet_len) {
        ret = recv(sock, full_buffer + total_read, packet_len - total_read, 0);
        if (ret <= 0) {
            if (ret == 0) {
                ROBOT_TCP_LOG_WARN("机器人在读取 TCP 包体时关闭连接");
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                ROBOT_TCP_LOG_ERROR("读取 TCP 包体失败: %s", strerror(errno));
            }
            free(full_buffer);
            *buffer = NULL;
            return (ret == 0) ? 0 : -1;
        }
        total_read += ret;
    }
    full_buffer[packet_len] = '\0';

    // ── 3. 去掉末尾的 \r\n\r\n ──
    int json_len = (int)packet_len;
    while (json_len >= 4 &&
           full_buffer[json_len - 1] == '\n' &&
           full_buffer[json_len - 2] == '\r' &&
           full_buffer[json_len - 3] == '\n' &&
           full_buffer[json_len - 4] == '\r') {
        json_len -= 4;
    }
    full_buffer[json_len] = '\0';

    log_tcp_payload("TCP RECV", full_buffer, json_len);
    *buffer = full_buffer;
    return json_len;
}

// ================= 内部：从 cJSON 对象提取机器人状态字段 =================
static void extract_state_from_json(cJSON* data_obj) {
    if (!data_obj || !cJSON_IsObject(data_obj)) return;

    /* ---- 解析位置 pos: "x,y,z" ---- */
    cJSON* pos_item = cJSON_GetObjectItem(data_obj, "pos");
    if (pos_item && pos_item->valuestring) {
        float x = 0, y = 0, z = 0;
        if (sscanf(pos_item->valuestring, "%f,%f,%f", &x, &y, &z) >= 2) {
            g_robot_state.pos.x = x;
            g_robot_state.pos.y = y;
            g_robot_state.pos.z = z;
        }
    }

    /* ---- 电量 ---- */
    cJSON* power_qty = cJSON_GetObjectItem(data_obj, "powquantity");
    if (power_qty) g_robot_state.battery_percent = (float)power_qty->valuedouble;

    cJSON* power_vol = cJSON_GetObjectItem(data_obj, "power");
    if (power_vol) g_robot_state.voltage = (float)power_vol->valuedouble;

    /* ---- 充电状态 ---- */
    cJSON* charging = cJSON_GetObjectItem(data_obj, "charging");
    if (charging) g_robot_state.is_charging = charging->valueint;

    /* ---- 导航状态 ---- */
    cJSON* innav = cJSON_GetObjectItem(data_obj, "innavmap");
    if (innav) g_robot_state.in_nav_map = innav->valueint;

    /* ---- 地图名称 ---- */
    cJSON* mapname = cJSON_GetObjectItem(data_obj, "mapname");
    if (mapname && mapname->valuestring) {
        strncpy(g_robot_state.map_name, mapname->valuestring, sizeof(g_robot_state.map_name) - 1);
    }

    /* ---- 急停状态 ---- */
    cJSON* emg = cJSON_GetObjectItem(data_obj, "emgStop");
    if (emg) g_robot_state.is_emg_stop = emg->valueint;

    /* ---- 解析站点列表 stationObj ---- */
    cJSON* station_obj = cJSON_GetObjectItem(data_obj, "stationObj");
    if (station_obj && cJSON_IsObject(station_obj)) {
        pthread_mutex_lock(&g_stations_mutex);

        // 释放旧的站点缓存
        if (g_stations_cache.stations) {
            free(g_stations_cache.stations);
            g_stations_cache.stations = NULL;
            g_stations_cache.station_count = 0;
        }

        // 获取当前地图名称
        char current_map[128] = {0};
        cJSON* mapname_item = cJSON_GetObjectItem(data_obj, "mapname");
        if (mapname_item && mapname_item->valuestring) {
            strncpy(current_map, mapname_item->valuestring, sizeof(current_map) - 1);
            strncpy(g_stations_cache.map_name, current_map, sizeof(g_stations_cache.map_name) - 1);
        }

        // 遍历所有地图的站点，只保存当前地图的站点
        cJSON* map_item = NULL;
        cJSON_ArrayForEach(map_item, station_obj) {
            const char* map_key = map_item->string;
            if (!map_key) continue;
            if (strcmp(map_key, current_map) != 0) continue;

            cJSON* stations_array = map_item;
            if (!stations_array || !cJSON_IsArray(stations_array)) continue;

            int count = cJSON_GetArraySize(stations_array);
            if (count == 0) continue;

            StationInfo_t* stations = (StationInfo_t*)calloc(count, sizeof(StationInfo_t));
            if (!stations) break;

            for (int i = 0; i < count; i++) {
                cJSON* station = cJSON_GetArrayItem(stations_array, i);
                if (!station) continue;

                cJSON* id = cJSON_GetObjectItem(station, "id");
                stations[i].id = (id && cJSON_IsNumber(id)) ? id->valueint : i;

                cJSON* name = cJSON_GetObjectItem(station, "name");
                if (name && cJSON_IsString(name)) {
                    strncpy(stations[i].name, name->valuestring, sizeof(stations[i].name) - 1);
                } else {
                    snprintf(stations[i].name, sizeof(stations[i].name), "Station_%d", i);
                }

                cJSON* pos = cJSON_GetObjectItem(station, "position");
                if (pos && cJSON_IsString(pos) && pos->valuestring) {
                    char temp[128];
                    strncpy(temp, pos->valuestring, sizeof(temp) - 1);
                    temp[sizeof(temp) - 1] = '\0';
                    char* token = strtok(temp, ",");
                    if (token) stations[i].x = atof(token);
                    token = strtok(NULL, ",");
                    if (token) stations[i].y = atof(token);
                    token = strtok(NULL, ",");
                    if (token) stations[i].z = atof(token);
                }

                cJSON* type = cJSON_GetObjectItem(station, "type");
                stations[i].type = (type && cJSON_IsNumber(type)) ? type->valueint : 1;

                cJSON* is_enable = cJSON_GetObjectItem(station, "isEnable");
                stations[i].is_enable = (is_enable && cJSON_IsNumber(is_enable)) ? is_enable->valueint : 1;
            }

            g_stations_cache.stations = stations;
            g_stations_cache.station_count = count;
        }

        pthread_mutex_unlock(&g_stations_mutex);
    }
}

// 解析机器人状态JSON
// 处理两种格式:
//   a) 通用响应: {"p":{"data":{...},"er":0,"id":""},"t":42}
//   b) 主动推送: {"p":{"data":"{\"8\":\"{\\\"pos\\\":...}\"}"},"t":42}
static void parse_status_json(const char* json_str) {
    cJSON* root = cJSON_Parse(json_str);
    if (!root) return;

    cJSON* p_obj = cJSON_GetObjectItem(root, "p");
    if (!p_obj) {
        cJSON_Delete(root);
        return;
    }

    cJSON* data_obj = cJSON_GetObjectItem(p_obj, "data");
    if (!data_obj) {
        cJSON_Delete(root);
        return;
    }

    pthread_mutex_lock(&g_state_mutex);

    if (cJSON_IsObject(data_obj)) {
        // 格式 a) 直接对象
        extract_state_from_json(data_obj);
    } else if (cJSON_IsString(data_obj) && data_obj->valuestring) {
        // 格式 b) 字符串 – 需要进一步展开
        // 数据格式: {"8":"{\"pos\":...,\"powquantity\":...}"}
        cJSON* inner = cJSON_Parse(data_obj->valuestring);
        if (inner && cJSON_IsObject(inner)) {
            // 遍历所有 key (如 "8")，取第一个值
            cJSON* slot_item = NULL;
            cJSON_ArrayForEach(slot_item, inner) {
                if (cJSON_IsString(slot_item) && slot_item->valuestring) {
                    cJSON* actual_data = cJSON_Parse(slot_item->valuestring);
                    if (actual_data && cJSON_IsObject(actual_data)) {
                        extract_state_from_json(actual_data);
                    }
                    cJSON_Delete(actual_data);
                }
                break; // 只处理第一个 slot
            }
        }
        cJSON_Delete(inner);
    }

    pthread_mutex_unlock(&g_state_mutex);
    cJSON_Delete(root);
}

// ================= 新增：发送状态请求命令 =================
static int request_robot_status(void) {
    if (g_tcp_socket < 0) return -1;
    
    cJSON* cmd_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(cmd_obj, "t", CMD_GET_STATUS);
    cJSON* p_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(p_obj, "data", 1);
    cJSON_AddItemToObject(cmd_obj, "p", p_obj);
    
    char* render_str = cJSON_PrintUnformatted(cmd_obj);
    if (!render_str) {
        cJSON_Delete(cmd_obj);
        return -1;
    }
    
    int len = strlen(render_str);
    int sent = send_data(g_tcp_socket, render_str, len);
    free(render_str);
    cJSON_Delete(cmd_obj);
    
    return (sent > 0) ? 0 : -1;
}


// 发送通用命令 (带t和p)
static int send_command(int cmd_enum, const char* json_payload) {
    if (g_tcp_socket < 0) return -1;

    // 构建 { "t": cmd, "p": { ... } }
    cJSON* cmd_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(cmd_obj, "t", cmd_enum);
    
    if (json_payload && strlen(json_payload) > 0) {
        cJSON* p_obj = cJSON_Parse(json_payload);
        if (p_obj) {
            cJSON_AddItemToObject(cmd_obj, "p", p_obj);
        } else {
            cJSON_AddStringToObject(cmd_obj, "p", json_payload); // 如果已经是字符串
        }
    } else {
        cJSON_AddNullToObject(cmd_obj, "p");
    }

    char* render_str = cJSON_PrintUnformatted(cmd_obj);
    if (render_str) {
        int len = strlen(render_str);
        int sent = send_data(g_tcp_socket, render_str, len);
        free(render_str);
        cJSON_Delete(cmd_obj);
        return (sent > 0) ? 0 : -1;
    }
    
    cJSON_Delete(cmd_obj);
    return -1;
}

// ================= 发送心跳包 =================
// 心跳包格式特殊：使用 "cmd" 字段而非 "t" 字段
static int send_heartbeat(void) {
    if (g_tcp_socket < 0) return -1;

    const char* heartbeat_str = "{\"cmd\":\"heatbeat\"}";
    int len = strlen(heartbeat_str);
    int sent = send_data(g_tcp_socket, heartbeat_str, len);
    return (sent > 0) ? 0 : -1;
}

// ================= 工作线程函数 =================
static void* robot_tcp_task(void* arg) {
    (void)arg;
    char* recv_buffer = NULL;
    
    while (g_running) {
        // 1. 连接逻辑
        if (g_tcp_socket < 0) {
            g_tcp_socket = tcp_connect();
            if (g_tcp_socket < 0) {
                // 连接失败，等待后重试
                sleep(3);
                continue;
            }
            // 连接成功，可以发送通知给UI
            ROBOT_TCP_LOG_USER("已连接机器人: address=%s:%d", ROBOT_IP, ROBOT_PORT);

            //马上发送状态请求
            request_robot_status();
        }

        // 2. 接收数据处理
        int recv_len = recv_packet(g_tcp_socket, &recv_buffer);
        if (recv_len > 0) {
            // 尝试解析状态包 (t=42) 或其他响应
            parse_status_json(recv_buffer);
            free(recv_buffer);
            recv_buffer = NULL;
        } else if (recv_len == 0) {
            // 服务端关闭连接
            close(g_tcp_socket);
            g_tcp_socket = -1;
            continue;
        } else if (recv_len == -2) {
            // 超时，无数据可读 - 继续执行心跳和保活逻辑
        } else {
            // 真正的网络错误，关闭连接等待重连
            ROBOT_TCP_LOG_WARN("TCP 接收异常，准备重新连接");
            close(g_tcp_socket);
            g_tcp_socket = -1;
            continue;
        }

        // 3. 心跳与保活 (每15秒一次)
        // 这里简化处理，实际应使用定时器或记录上次发送时间
        // 每循环几次发送一次心跳
        static int heartbeat_counter = 0;
        // 每4次心跳，请求一次状态，同步电量显示
        static int request_counter = 0;
        
        if (g_tcp_socket >= 0) {
            // 每轮循环约 600ms (500ms recv超时 + 100ms usleep)，25次 ≈ 15秒
            if (++heartbeat_counter >= 25) {
                send_heartbeat(); // 心跳
                heartbeat_counter = 0;
                request_counter++;
                if (request_counter >= 4) {
                    request_counter = 0;
                    request_robot_status();

                    RobotState_t state;
                    RobotTcp_GetState(&state);
                    g_battery_percent = (int)state.battery_percent;  
                }
            }
        }

        // 4. 避免CPU空转
        usleep(100000); // 100ms
    }

    return NULL;
}

// ================= 对外接口实现 =================

int RobotTcp_Init(void) {
    // 初始化状态
    memset(&g_robot_state, 0, sizeof(g_robot_state));
    strcpy(g_robot_state.map_name, "unknown");
    g_tcp_socket = -1;
    g_running = 1;
    ROBOT_TCP_LOG_USER("初始化完成");
    return 0;
}

int RobotTcp_Start(void) {
    int ret;

    if (g_work_thread) {
        ROBOT_TCP_LOG_WARN("工作线程已启动，跳过重复调用");
        return -1;
    }

    ret = pthread_create(&g_work_thread, NULL, robot_tcp_task, NULL);
    if (ret != 0) {
        ROBOT_TCP_LOG_ERROR("创建工作线程失败: %s", strerror(ret));
        return -1;
    }

    ROBOT_TCP_LOG_USER("工作线程启动成功");
    return 0;
}

int RobotTcp_SendMove(float linear_speed, float angular_speed) {
    // 构建参数: {"angle": ang, "speed": spd}
    char payload[64];
    snprintf(payload, sizeof(payload), "{\"angle\":%f,\"speed\":%f}", angular_speed, linear_speed);
    return send_command(CMD_MOVE, payload);
}

int RobotTcp_SendGoto(float x, float y, float z) {
    // 构建参数: {"data": {"x":x, "y":y, "z":z, "tolerance":0.05}}
    char payload[128];
    snprintf(payload, sizeof(payload), 
             "{\"data\":{\"x\":%f,\"y\":%f,\"z\":%f,\"tolerance\":0.05}}", 
             x, y, z);
    return send_command(CMD_GOTO_POINT, payload);
}

int RobotTcp_SetCharge(int enable) {
    return send_command(enable ? CMD_SET_CHARGE : 21, "{}"); // 21是取消充电
}

int RobotTcp_EmergencyStop(int stop) {
    char payload[32];
    snprintf(payload, sizeof(payload), "{\"data\":{\"stop\":%d}}", stop ? 1 : 0);
    return send_command(CMD_EMG_STOP, payload);
}

void RobotTcp_GetState(RobotState_t* state) {
    if (!state) return;
    pthread_mutex_lock(&g_state_mutex);
    memcpy(state, &g_robot_state, sizeof(RobotState_t));
    pthread_mutex_unlock(&g_state_mutex);
}

int RobotTcp_RequestStatus(void)
{
    return request_robot_status();
}

void RobotTcp_Deinit(void) {
    g_running = 0;
    if (g_work_thread) {
        pthread_join(g_work_thread, NULL);
        g_work_thread = 0;
    }
    if (g_tcp_socket >= 0) {
        close(g_tcp_socket);
        g_tcp_socket = -1;
    }
}


// ================= 内部辅助函数：文件操作 =================

// 将字符串内容保存到本地文件
static int save_string_to_file(const char* filepath, const char* content) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) {
        ROBOT_TCP_LOG_ERROR("打开文件失败: path=%s error=%s", filepath,
                            strerror(errno));
        return -1;
    }
    fprintf(fp, "%s", content);
    fclose(fp);
    ROBOT_TCP_LOG_USER("数据保存成功: path=%s", filepath);
    return 0;
}

// ================= 新增接口实现 =================

int RobotTcp_StartMapping(void) {
    // 发送开始建图指令，具体参数根据协议调整
    return send_command(CMD_START_MAP, "{}"); 
}

int RobotTcp_BackupMap(const char* map_name) {
    if (!map_name) return -1;
    
    // 1. 发送备份地图指令
    char payload[128];
    snprintf(payload, sizeof(payload), "{\"data\":{\"name\":\"%s\"}}", map_name);
    int ret = send_command(CMD_SAVE_MAP, payload);
    if (ret != 0) return ret;

    // 2. 假设底盘会将地图数据通过某个响应返回，或者你需要再次请求获取
    // 这里演示如何将获取到的地图JSON数据(假设在recv_buffer中)保存
    // 实际应用中，你需要在 parse_status_json 或专门的接收回调中拦截地图数据
    // 示例：save_string_to_file("/mnt/udisk/maps/my_map.json", recv_buffer);
    
    ROBOT_TCP_LOG_USER("地图备份指令已发送: map=%s", map_name);
    return 0;
}

int RobotTcp_FetchAndSaveStations(void) {
    // 先请求状态获取站点数据
    if (request_robot_status() != 0) {
        ROBOT_TCP_LOG_ERROR("获取站点前请求机器人状态失败");
        return -1;
    }
    
    // 等待解析完成
    usleep(500000);
    
    pthread_mutex_lock(&g_stations_mutex);
    
    if (g_stations_cache.station_count == 0 || !g_stations_cache.stations) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("未获取到站点数据");
        return -1;
    }
    
    // 构建JSON用于保存到文件
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "map_name", g_stations_cache.map_name);
    cJSON_AddNumberToObject(root, "station_count", g_stations_cache.station_count);
    
    cJSON* stations_array = cJSON_CreateArray();
    for (int i = 0; i < g_stations_cache.station_count; i++) {
        cJSON* station_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(station_obj, "id", g_stations_cache.stations[i].id);
        cJSON_AddStringToObject(station_obj, "name", g_stations_cache.stations[i].name);
        cJSON_AddNumberToObject(station_obj, "x", g_stations_cache.stations[i].x);
        cJSON_AddNumberToObject(station_obj, "y", g_stations_cache.stations[i].y);
        cJSON_AddNumberToObject(station_obj, "z", g_stations_cache.stations[i].z);
        cJSON_AddNumberToObject(station_obj, "type", g_stations_cache.stations[i].type);
        cJSON_AddNumberToObject(station_obj, "is_enable", g_stations_cache.stations[i].is_enable);
        cJSON_AddItemToArray(stations_array, station_obj);
    }
    cJSON_AddItemToObject(root, "stations", stations_array);
    
    char* json_str = cJSON_Print(root);
    cJSON_Delete(root);
    
    pthread_mutex_unlock(&g_stations_mutex);
    
    if (!json_str) return -1;
    
    // 保存到文件
    int ret = save_string_to_file(STATION_SAVE_PATH, json_str);
    free(json_str);
    
    return ret;
}

int RobotTcp_GotoStation(int station_index) {
    FILE* fp = fopen(STATION_SAVE_PATH, "r");
    if (!fp) {
        ROBOT_TCP_LOG_ERROR("打开站点文件失败: path=%s error=%s",
                            STATION_SAVE_PATH, strerror(errno));
        return -1;
    }

    // 1. 读取整个文件内容到内存
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* json_str = (char*)malloc(fsize + 1);
    if (!json_str) { fclose(fp); return -1; }
    fread(json_str, 1, fsize, fp);
    json_str[fsize] = '\0';
    fclose(fp);

    // 2. 解析JSON并提取指定索引的站点坐标
    int ret = -1;
    cJSON* root = cJSON_Parse(json_str);
    free(json_str); // 读取完毕立即释放

    if (root) {
        cJSON* stations = cJSON_GetObjectItem(root, "stations"); // 假设数组key是stations
        if (cJSON_IsArray(stations) && station_index < cJSON_GetArraySize(stations)) {
            cJSON* station = cJSON_GetArrayItem(stations, station_index);
            
            cJSON* x = cJSON_GetObjectItem(station, "x");
            cJSON* y = cJSON_GetObjectItem(station, "y");
            cJSON* z = cJSON_GetObjectItem(station, "z");

            if (x && y && z) {
                ROBOT_TCP_LOG_USER("按索引导航到站点: index=%d position=(%.2f, %.2f, %.2f)",
                                   station_index, x->valuedouble,
                                   y->valuedouble, z->valuedouble);
                ret = RobotTcp_SendGoto(x->valuedouble, y->valuedouble, z->valuedouble);
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

// ================= 新增：获取站点列表（从缓存中读取）=================
int RobotTcp_GetStationList(MapStations_t* out_stations) {
    if (!out_stations) return -1;
    
    // 先主动请求一次状态，确保获取最新站点数据
    if (request_robot_status() != 0) {
        ROBOT_TCP_LOG_ERROR("获取站点列表前请求机器人状态失败");
        return -1;
    }
    
    // 等待一下让响应有时间处理（实际应该用信号量或回调）
    usleep(500000);  // 等待500ms
    
    pthread_mutex_lock(&g_stations_mutex);
    
    if (g_stations_cache.station_count == 0 || !g_stations_cache.stations) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("当前地图没有站点数据");
        return -1;
    }
    
    // 复制数据给调用者
    out_stations->station_count = g_stations_cache.station_count;
    out_stations->stations = (StationInfo_t*)malloc(
        g_stations_cache.station_count * sizeof(StationInfo_t));
    
    if (!out_stations->stations) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_ERROR("站点列表内存分配失败: count=%d",
                            g_stations_cache.station_count);
        return -1;
    }
    
    memcpy(out_stations->stations, g_stations_cache.stations, 
           g_stations_cache.station_count * sizeof(StationInfo_t));
    strncpy(out_stations->map_name, g_stations_cache.map_name, 
            sizeof(out_stations->map_name)-1);
    
    pthread_mutex_unlock(&g_stations_mutex);
    
    ROBOT_TCP_LOG_USER("获取站点列表成功: count=%d map=%s",
                       out_stations->station_count, out_stations->map_name);
    return 0;
}

// ================= 新增：释放站点列表内存 =================
void RobotTcp_FreeStationList(MapStations_t* stations) {
    if (stations && stations->stations) {
        free(stations->stations);
        stations->stations = NULL;
        stations->station_count = 0;
    }
}

// ================= 新增：根据站点名称导航 =================
int RobotTcp_GotoStationByName(const char* station_name) {
    if (!station_name) return -1;
    
    pthread_mutex_lock(&g_stations_mutex);
    
    if (g_stations_cache.station_count == 0 || !g_stations_cache.stations) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("没有可用的站点缓存，无法按名称导航");
        return -1;
    }
    
    // 查找匹配的站点
    StationInfo_t* target = NULL;
    for (int i = 0; i < g_stations_cache.station_count; i++) {
        if (strcmp(g_stations_cache.stations[i].name, station_name) == 0) {
            target = &g_stations_cache.stations[i];
            break;
        }
    }
    
    if (!target) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("未找到站点: name=%s", station_name);
        return -1;
    }
    
    // 检查站点是否启用
    if (!target->is_enable) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("站点未启用: name=%s", station_name);
        return -1;
    }
    
    float x = target->x;
    float y = target->y;
    float z = target->z;
    
    pthread_mutex_unlock(&g_stations_mutex);
    
    ROBOT_TCP_LOG_USER("按名称导航到站点: name=%s position=(%.2f, %.2f, %.2f)",
                       station_name, x, y, z);
    return RobotTcp_SendGoto(x, y, z);
}

// ================= 新增：根据站点ID导航 =================
int RobotTcp_GotoStationById(int station_id) {
    pthread_mutex_lock(&g_stations_mutex);
    
    if (g_stations_cache.station_count == 0 || !g_stations_cache.stations) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("没有可用的站点缓存，无法按 ID 导航");
        return -1;
    }
    
    // 查找匹配的站点
    StationInfo_t* target = NULL;
    for (int i = 0; i < g_stations_cache.station_count; i++) {
        if (g_stations_cache.stations[i].id == station_id) {
            target = &g_stations_cache.stations[i];
            break;
        }
    }
    
    if (!target) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("未找到站点: id=%d", station_id);
        return -1;
    }
    
    // 检查站点是否启用
    if (!target->is_enable) {
        pthread_mutex_unlock(&g_stations_mutex);
        ROBOT_TCP_LOG_WARN("站点未启用: id=%d", station_id);
        return -1;
    }
    
    float x = target->x;
    float y = target->y;
    float z = target->z;
    char name[64];
    strncpy(name, target->name, sizeof(name)-1);
    
    pthread_mutex_unlock(&g_stations_mutex);
    
    ROBOT_TCP_LOG_USER("按 ID 导航到站点: name=%s id=%d position=(%.2f, %.2f, %.2f)",
                       name, station_id, x, y, z);
    return RobotTcp_SendGoto(x, y, z);
}

// ================= 获取站点坐标 (线程安全) =================
int RobotTcp_GetStationCoordsById(int station_id, float *x, float *y, float *z,
                                   char *name, int name_size)
{
    pthread_mutex_lock(&g_stations_mutex);

    if (g_stations_cache.station_count == 0 || !g_stations_cache.stations) {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    StationInfo_t *target = NULL;
    for (int i = 0; i < g_stations_cache.station_count; i++) {
        if (g_stations_cache.stations[i].id == station_id) {
            target = &g_stations_cache.stations[i];
            break;
        }
    }

    if (!target) {
        pthread_mutex_unlock(&g_stations_mutex);
        return -1;
    }

    if (x) *x = target->x;
    if (y) *y = target->y;
    if (z) *z = target->z;
    if (name && name_size > 0) {
        strncpy(name, target->name, (size_t)name_size - 1);
        name[name_size - 1] = '\0';
    }

    pthread_mutex_unlock(&g_stations_mutex);
    return 0;
}
