#ifndef SCREEN_HOME_CLEAN_FLOW_H
#define SCREEN_HOME_CLEAN_FLOW_H

#include <stdbool.h>
#include <stdint.h>
#include "protocol.h"
#include "page_clean.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool active;
    uint8_t expected_status;
    uint8_t last_status;
} screen_home_clean_flow_t;

void screen_home_clean_flow_start(screen_home_clean_flow_t *flow);
void screen_home_clean_flow_stop(screen_home_clean_flow_t *flow);
bool screen_home_clean_flow_update(screen_home_clean_flow_t *flow,
                                   uint8_t base_main_status,
                                   page_clean_step_t *step);

#ifdef __cplusplus
}
#endif

#endif
