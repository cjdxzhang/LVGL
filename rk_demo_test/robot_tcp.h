#ifndef ROBOT_TCP_H
#define ROBOT_TCP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "cJSON.h"
#include <stdio.h> // 新增，用于文件操作


// ================= 枚举命令 =================
typedef enum
{
    CMD_MOVE = 12,
    CMD_GOTO_POINT = 13,
    CMD_SET_CHARGE = 20,
    CMD_EMG_STOP = 45,
    CMD_GET_STATUS = 42,
    CMD_START_MAP = 50,
    CMD_SAVE_MAP = 51,
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
    char map_name[128];
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
    float battery_percent;  // 查询时读取MCU电量百分比
    float voltage;          // 电压
    int   is_charging;      // 充电状态
    int   is_emg_stop;      // 急停状态
    int   in_nav_map;       // 是否在导航地图中 (1=是, 0=否)
    char  map_name[64];     // 当前地图
    RobotPosition_t pos;    // 当前坐标
} RobotState_t;

// ================= 模块接口 =================

/**
 * @brief 初始化代理驱动的机器人模块
 * @return 0 成功, -1 失败
 */
int RobotTcp_Init(void);

/**
 * @brief 兼容旧调用，当前为 no-op
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
 * @brief 返回基站（通过代理发送 t=20 指令）
 * @return 0 成功, 其他失败
 */
int RobotTcp_ReturnToBase(void);

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
 * @brief 兼容旧调用，代理模式下未实现状态查询
 * @return 0 成功, -1 失败
 */
int RobotTcp_RequestStatus(void);

/**
 * @brief 反初始化，释放资源
 */
void RobotTcp_Deinit(void);

// ================= 新增模块接口 =================

/**
 * @brief 兼容旧调用，代理模式下未实现
 */
int RobotTcp_StartMapping(void);

/**
 * @brief 兼容旧调用，代理模式下未实现
 * @param map_name 地图名称
 */
int RobotTcp_BackupMap(const char *map_name);

/**
 * @brief 通过 tcp_service 代理请求最新站点列表
 */
int RobotTcp_FetchAndSaveStations(void);

/**
 * @brief 按缓存索引导航到指定站点
 * @param station_index 站点在当前缓存中的索引
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

/* 供 tcp_service 代理通道集成使用 */
cJSON *RobotTcp_BuildStationsQueryRequest(void);
bool RobotTcp_IsProxyMessage(const cJSON *root);
int RobotTcp_HandleProxyMessage(cJSON *root);

// ================= 新增：从 tcp_service 更新位置 =================

/**
 * @brief 从 tcp_service 更新机器人位置（由 tcp_service 在收到位置更新时调用）
 * @param x X坐标
 * @param y Y坐标
 * @param z Z坐标（朝向）
 * @param battery_percent 已停用的底盘电量参数，仅保留接口兼容性
 * @param voltage 电压
 * @param vel_speed 线速度
 * @param vel_angle 角速度
 * @param emg_stop 急停状态 (0=正常, 1=急停)
 * @param inbuildmap 是否在建图 (0=否, 1=是)
 * @param innavmap 是否在导航地图中 (0=否, 1=是)
 * @param mapname 地图名称
 */
void RobotTcp_UpdatePosition(float x, float y, float z, int battery_percent,
                             float voltage, float vel_speed, float vel_angle,
                             int emg_stop, int inbuildmap, int innavmap,
                             const char *mapname);

#endif // ROBOT_TCP_H
