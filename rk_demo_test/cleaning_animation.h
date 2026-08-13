#ifndef CLEANING_ANIMATION_H
#define CLEANING_ANIMATION_H

#include <stdint.h>
#include "lvgl.h"

/**
 * 在自清洁容器中创建六步流程动效。
 * 重复传入同一个有效父对象时不会重复创建。
 */
void cleaning_animation_create(lv_obj_t *parent);

/**
 * 更新当前自清洁步骤，stage 的有效范围为 0～5。
 */
void cleaning_animation_set_stage(uint8_t stage);

/**
 * 停止流光动画并删除该组件创建的全部对象。
 */
void cleaning_animation_cleanup(void);

#endif
