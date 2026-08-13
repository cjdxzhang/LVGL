#ifndef SELF_CLEAN_FLOW_H
#define SELF_CLEAN_FLOW_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    SELF_CLEAN_FLOW_EVENT_NONE = 0,
    SELF_CLEAN_FLOW_EVENT_STAGE_1,
    SELF_CLEAN_FLOW_EVENT_STAGE_2,
    SELF_CLEAN_FLOW_EVENT_STAGE_3,
    SELF_CLEAN_FLOW_EVENT_STAGE_4,
    SELF_CLEAN_FLOW_EVENT_STAGE_5,
    SELF_CLEAN_FLOW_EVENT_COMPLETE
} self_clean_flow_event_t;

typedef struct
{
    bool active;
    uint8_t expected_status;
    uint8_t last_status;
} self_clean_flow_t;

void self_clean_flow_start(self_clean_flow_t *flow);
void self_clean_flow_stop(self_clean_flow_t *flow);
bool self_clean_flow_update(self_clean_flow_t *flow, uint8_t base_main_status,
                            self_clean_flow_event_t *event);

#endif
