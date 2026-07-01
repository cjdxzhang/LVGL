#ifndef CARD_H
#define CARD_H

#include "lvgl/lvgl.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void card_create_info(lv_obj_t * parent);

/* 单项更新接口 */
void card_set_temperature(int temp);
void card_set_time_min(uint16_t min);
void card_set_water_level(uint8_t level);
void card_set_water_liters(uint8_t liters);
void card_set_low_water(bool low);
void card_set_herb_icons(uint8_t herb1, uint8_t herb2);
void card_set_runtime_status(uint8_t massage_level, uint8_t heat,
                             uint8_t sterilize_on);
void card_clear_bucket_status(void);

/* 可选：自定义文本接口，后面如果你想显示别的字也能直接用 */
void card_set_water_text(const char * text);
void card_set_status_text(const char * text);
void card_exit_clean(void);
void card_set_clean_running(void);

void card_set_clean_pending(void);
void card_blink_temperature_confirm(void);

#ifdef __cplusplus
}
#endif

#endif /* CARD_H */
