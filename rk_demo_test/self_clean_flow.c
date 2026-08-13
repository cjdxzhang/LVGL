#include "self_clean_flow.h"

#include <string.h>

#define SELF_CLEAN_STATUS_STANDBY       3U
#define SELF_CLEAN_STATUS_SPRAY_1       7U
#define SELF_CLEAN_STATUS_DRAIN_1       8U
#define SELF_CLEAN_STATUS_SPRAY_2       9U
#define SELF_CLEAN_STATUS_DRAIN_2       10U
#define SELF_CLEAN_STATUS_DRY           11U
#define SELF_CLEAN_STATUS_INVALID       0xFFU

static bool event_for_status(uint8_t status, self_clean_flow_event_t *event)
{
    switch (status)
    {
    case SELF_CLEAN_STATUS_SPRAY_1:
        *event = SELF_CLEAN_FLOW_EVENT_STAGE_1;
        return true;
    case SELF_CLEAN_STATUS_DRAIN_1:
        *event = SELF_CLEAN_FLOW_EVENT_STAGE_2;
        return true;
    case SELF_CLEAN_STATUS_SPRAY_2:
        *event = SELF_CLEAN_FLOW_EVENT_STAGE_3;
        return true;
    case SELF_CLEAN_STATUS_DRAIN_2:
        *event = SELF_CLEAN_FLOW_EVENT_STAGE_4;
        return true;
    case SELF_CLEAN_STATUS_DRY:
        *event = SELF_CLEAN_FLOW_EVENT_STAGE_5;
        return true;
    default:
        return false;
    }
}

static uint8_t next_status_after(uint8_t status)
{
    switch (status)
    {
    case SELF_CLEAN_STATUS_SPRAY_1:
        return SELF_CLEAN_STATUS_DRAIN_1;
    case SELF_CLEAN_STATUS_DRAIN_1:
        return SELF_CLEAN_STATUS_SPRAY_2;
    case SELF_CLEAN_STATUS_SPRAY_2:
        return SELF_CLEAN_STATUS_DRAIN_2;
    case SELF_CLEAN_STATUS_DRAIN_2:
        return SELF_CLEAN_STATUS_DRY;
    case SELF_CLEAN_STATUS_DRY:
        return SELF_CLEAN_STATUS_STANDBY;
    default:
        return SELF_CLEAN_STATUS_SPRAY_1;
    }
}

void self_clean_flow_start(self_clean_flow_t *flow)
{
    if (flow == NULL)
    {
        return;
    }

    memset(flow, 0, sizeof(*flow));
    flow->active = true;
    flow->expected_status = SELF_CLEAN_STATUS_SPRAY_1;
    flow->last_status = SELF_CLEAN_STATUS_INVALID;
}

void self_clean_flow_stop(self_clean_flow_t *flow)
{
    if (flow == NULL)
    {
        return;
    }

    memset(flow, 0, sizeof(*flow));
}

bool self_clean_flow_update(self_clean_flow_t *flow, uint8_t base_main_status,
                            self_clean_flow_event_t *event)
{
    if (event != NULL)
    {
        *event = SELF_CLEAN_FLOW_EVENT_NONE;
    }
    if (flow == NULL || event == NULL || !flow->active)
    {
        return false;
    }
    if (base_main_status == flow->last_status ||
            base_main_status != flow->expected_status)
    {
        return false;
    }
    if (base_main_status == SELF_CLEAN_STATUS_STANDBY)
    {
        self_clean_flow_stop(flow);
        *event = SELF_CLEAN_FLOW_EVENT_COMPLETE;
        return true;
    }
    if (!event_for_status(base_main_status, event))
    {
        return false;
    }

    flow->last_status = base_main_status;
    flow->expected_status = next_status_after(base_main_status);
    return true;
}
