#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lvgl.h"
#include "gui_guider.h"

#define SYSTEM_MODE_NAME_LEN 32
#define SYSTEM_MODE_MAX_COUNT 16
#define SYSTEM_LOCATION_NAME_LEN 32
#define SYSTEM_LOCATION_MAX_COUNT 16

typedef struct {
	char name[SYSTEM_MODE_NAME_LEN];
	int icon_id;
	uint32_t time_sec;
	uint8_t water_level;
	int16_t temperature;
	bool use_drug1;
	bool use_drug2;
	int position;
} system_mode_t;

typedef struct {
	lv_coord_t x;
	lv_coord_t y;
} system_location_coord_t;

typedef struct {
	char name[SYSTEM_LOCATION_NAME_LEN];
	int index;
	int default_mode_index;
	system_location_coord_t coord;
} system_location_t;

extern system_location_t g_location_settings[SYSTEM_LOCATION_MAX_COUNT];
extern size_t g_location_count;
extern int current_selected_location_index;

/** 模式属性 */
extern int16_t last_link_status; /* 最近一次连接状态: 0x00=基站和桶已断开, 0x01=基站和桶已连接 */
extern bool has_mcu_status_report; /* 是否已收到有效 MCU 状态帧，避免默认 LINK_STATUS 误触发页面切换 */
extern int16_t temp_set; /* 温度设置值，单位 °C（可用乘 100 的整数表示） */
extern uint32_t timer_set; /* 定时设置值，单位 sec */
extern uint32_t remaining_seconds; /* 桶体本地倒计时剩余秒数，不参与A6编码 */
extern int16_t current_temp; /* 桶体离开基站后的实际温度，单位 °C */
extern int16_t current_water_level; /* 当前水位，范围 0-3 */
extern int water_level;   /* 目标水位挡位，范围 0-3 */
extern bool use_drug1;          /* 是否使用药物1 */
extern bool use_drug2;        /* 是否使用药物2 */
extern bool is_self_cleaning; /* 是否正在自清洁 */

/** 从串口协议帧解析出的桶/底座状态 */
extern uint8_t bucket_main_status;
extern uint8_t base_main_status;

/**工作时属性 */
extern bool constant_temperature; /* 恒温 */
extern bool sterilization; /* 除菌 */
extern int massage_intensity; /* 按摩强度 */

/**定位属性 */
extern int current_location; /* 0--基站 1，2,3,4,5.....当前定位索引 */
extern int going_to_location; /* 目标定位索引 */

/**记录某个页面是否已经初始化 */
extern bool _is_index_page_initialized;
extern bool _is_setting_page_initialized;
extern bool _is_location_page_initialized;
extern bool _is_working_page_initialized;
extern bool _is_conf_mode_page_initialized;
extern bool _is_conf_location_page_initialized;
extern bool _is_conf_mode_detail_page_initialized;
extern bool _is_preparing_page_initialized;

/**全局计时，回到基站开始计时 */
extern lv_timer_t *g_auto_drain_timer;

/**全局变量，记录是否正在排水 */
extern bool g_is_draining;

size_t system_mode_get_count(void);
extern int current_selected_mode_index;
const system_mode_t *system_mode_get(size_t index);



int system_location_save();

// 初始化 系统管理模块
void system_manager_init(lv_ui *ui);
void system_ui_schedule_link_status(uint8_t link_status);
//	将指定模式的设置应用到当前运行时环境
void apply_mode_to_runtime(lv_event_t *e);

//定位页面点击定位按钮
void location_grid_btn_clicked(lv_event_t *e);

//首页，根据当前模式设置载如模式按钮，并定义模式按钮事件，加载完模式按钮后，增加自清洁，药浴1,药浴2,定位，设置5个按钮
void index_page_init(lv_ui *ui);
//定位页面，根据当前的定位设置，载入定位按钮，并定义事件
void location_page_init(lv_ui *ui);
//工作页面，初始化右边按钮群，并定义事件
void working_page_init(lv_ui *ui);
void bucket_temp_update_actual_from_mcu(int16_t temp);
//模式设置界面列出当前所有模式 + 新增按钮，点击进入编辑界面，编辑界面有保存和删除按钮
void conf_mode_init_listall(lv_ui *ui);
//定位设置界面初始化
void conf_location_init(lv_ui *ui);

//编辑界面初始化，显示模式的详细设置
void conf_mode_detail_init(lv_ui *ui);
//准备页面初始化，显示当前模式的准备状态
void preparing_page_init(lv_ui *ui);

void plus_clicked(lv_event_t *e);
void minus_clicked(lv_event_t *e);
void timer_clicked(lv_event_t *e);
void water_clicked(lv_event_t *e);
void play_clicked(lv_event_t *e);

void action_temp_plus(lv_ui *ui);
void action_temp_minus(lv_ui *ui);
void action_time_loop(lv_ui *ui);
void action_water_loop(lv_ui *ui);
void action_use_drug1_toggle(lv_event_t *e);
void action_use_drug2_toggle(lv_event_t *e);
void action_clean_clicked(lv_event_t *e);

void update_temp_set(lv_ui *ui);
void update_timer_set(lv_ui *ui);
void apply_water_level_ui(int level);
void apply_use_drug_ui(bool use_drug1, bool use_drug2);

void preparing_page_cleanup(void);
/**暂停工作，根据当前定位，返回到前一个页面 */
void preparing_page_stop(lv_event_t *e);

void working_bind_play_pause_buttons(lv_obj_t *pause_btn, lv_obj_t *play_btn);

void working_plus_clicked(lv_event_t *e);
void working_minus_clicked(lv_event_t *e);
void working_timer_clicked(lv_event_t *e);
void working_constant_temperature_toggle(lv_event_t *e);
void working_sterilization_toggle(lv_event_t *e);
void working_massage_intensity_clicked(lv_event_t *e);
void working_show_return_dialog(lv_event_t *e);
void working_pause_clicked(lv_event_t *e);
void working_play_clicked(lv_event_t *e);

/**点击模式的恢复默认按钮 */
void conf_mode_detail_restore_default_clicked(lv_event_t *e);
/**点击模式的删除按钮 */
void conf_mode_detail_delete_clicked(lv_event_t *e);
//int system_mode_delete(size_t index);
/**点击模式的保存按钮 */
void conf_mode_detail_save_clicked(lv_event_t *e);
#endif
