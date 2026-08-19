#ifndef ROBOT_TCP_H
#define ROBOT_TCP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "cJSON.h"
#include <stdio.h> // 新增，用于文件操作


#define MAP_SAVE_PATH       "/mnt/udisk/maps/"  // 地图保存路径 (根据实际挂载点修改)
#define STATION_SAVE_PATH   "/mnt/udisk/stations.json" // 站点信息保存路径

// ================= 配置定义 =================
#define ROBOT_IP            "192.168.31.7"  // 机器人IP
#define ROBOT_PORT          9970            // 机器人端口
#define HEARTBEAT_INTERVAL  15              // 心跳间隔(秒)

// ================= 枚举命令 =================
typedef enum
{
    CMD_HEARTBEAT = 0,  // 心跳
    CMD_GET_STATUS = 42,   // 获取状态 (t=42)
    CMD_MOVE = 12,         // 移动控制 (t=12)
    CMD_GOTO_POINT = 13,   // 去某点 (t=13)
    CMD_SET_CHARGE = 20,   // 设置充电 (t=20/21)
    CMD_EMG_STOP = 45,     // 急停 (t=45)
    CMD_START_MAP = 50,    // 开始地图保存 (t=50)
    CMD_SAVE_MAP = 51,     // 停止地图保存 (t=51)
    CMD_MAX
} RobotCommand_e;

// ================= 数据结构 =================
// 机器人位置信息
typedef struct
{
    float x;
    float y;
    float z; // 朝向
} RobotPosition_t;

// 单个站点信息
typedef struct
{
    int id;
    char name[64];
    float x;
    float y;
    float z;
    int type;           // 1=普通站点, 2=充电桩
    int is_enable;      // 1=启用, 0=禁用
} StationInfo_t;

// 地图站点列表
typedef struct
{
    char map_name[128];          // 地图文件名，如 "sim_map.yaml"
    StationInfo_t *stations;     // 站点数组
    int station_count;           // 站点数量
} MapStations_t;

// 机器人状态信息 (根据t=42响应简化)
typedef struct
{
    float battery_percent;  // 电量
    float voltage;          // 电压
    int   is_charging;      // 充电状态
    int   is_emg_stop;      // 急停状态
    int   in_nav_map;       // 是否在导航地图中 (1=是, 0=否)
    char  map_name[64];     // 当前地图
    RobotPosition_t pos;    // 当前坐标
} RobotState_t;

// 全局电量%
extern int g_battery_percent;

// ================= 模块接口 =================

/**
 * @brief 初始化机器人TCP客户端
 * @return 0 成功, -1 失败
 */
int RobotTcp_Init(void);

/**
 * @brief 启动后台线程（连接、心跳、接收）
 * @return 0 成功, -1 失败
 */
int RobotTcp_Start(void);

/**
 * @brief 发送移动指令 (线速度, 角速度)
 * @param linear_speed 线速度 (m/s)
 * @param angular_speed 角速度 (rad/s)
 * @return 0 成功
 */
int RobotTcp_SendMove(float linear_speed, float angular_speed);

/**
 * @brief 发送导航指令 (去往指定点)
 * @param x 目标X
 * @param y 目标Y
 * @param z 目标Z(朝向)
 * @return 0 成功, 其他失败
 */
int RobotTcp_SendGoto(float x, float y, float z);

/**
 * @brief 设置充电
 * @param enable 1:开始充电, 0:取消充电
 * @return 0 成功
 */
int RobotTcp_SetCharge(int enable);

/**
 * @brief 软急停
 * @param stop 1:急停, 0:复位
 * @return 0 成功
 */
int RobotTcp_EmergencyStop(int stop);

/**
 * @brief 获取最新的机器人状态 (线程安全)
 * @param state 输出状态结构体
 */
void RobotTcp_GetState(RobotState_t *state);

/**
 * @brief 主动向机器人发送 t=42 状态请求
 *        后台线程收到响应后会解析并更新内部状态缓存，
 *        随后可通过 RobotTcp_GetState() 获取最新数据。
 * @return 0 成功, -1 失败
 */
int RobotTcp_RequestStatus(void);

/**
 * @brief 反初始化，释放资源
 */
void RobotTcp_Deinit(void);

// ================= 新增模块接口 =================

/**
 * @brief 开始建图
 */
int RobotTcp_StartMapping(void);

/**
 * @brief 备份地图并保存到本地
 * @param map_name 地图名称
 */
int RobotTcp_BackupMap(const char *map_name);

/**
 * @brief 获取站点信息并保存到本地文件
 */
int RobotTcp_FetchAndSaveStations(void);

/**
 * @brief 导航到指定站点
 * @param station_index 站点在本地缓存文件中的索引
 */
int RobotTcp_GotoStation(int station_index);


// 新增函数：获取当前地图的站点列表
int RobotTcp_GetStationList(MapStations_t *out_stations);

// 新增函数：根据站点名称导航
int RobotTcp_GotoStationByName(const char *station_name);

// 新增函数：释放站点列表内存
void RobotTcp_FreeStationList(MapStations_t *stations);

// 新增函数：根据站点ID导航
int RobotTcp_GotoStationById(int station_id);

/**
 * @brief 获取站点坐标 (线程安全)
 * @param station_id 站点ID
 * @param[out] x 目标X坐标
 * @param[out] y 目标Y坐标
 * @param[out] z 目标Z坐标
 * @param[out] name 站点名称缓冲区 (可为NULL)
 * @param name_size name缓冲区大小 (name为NULL时忽略)
 * @return 0 找到, -1 未找到
 */
int RobotTcp_GetStationCoordsById(int station_id, float *x, float *y, float *z,
                                  char *name, int name_size);
#endif // ROBOT_TCP_H
