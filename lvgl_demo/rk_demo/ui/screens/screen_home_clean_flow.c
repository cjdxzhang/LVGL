#include "screen_home_clean_flow.h"
#include <string.h>

static bool step_for_status(uint8_t status, page_clean_step_t *step)
{
    if (step == NULL) {
        return false;
    }

    switch (status) {
        case BASE_STATUS_CLEAN_SPRAYING:
            *step = PAGE_CLEAN_STEP_SPRAY_1;
            return true;
        case BASE_STATUS_CLEAN_DRAINING_1:
            *step = PAGE_CLEAN_STEP_DRAIN_2;
            return true;
        case BASE_STATUS_RINSE_SPRAYING:
            *step = PAGE_CLEAN_STEP_SPRAY_2;
            return true;
        case BASE_STATUS_CLEAN_DRAINING_2:
            *step = PAGE_CLEAN_STEP_DRAIN_3;
            return true;
        case BASE_STATUS_CLEAN_DRYING:
            *step = PAGE_CLEAN_STEP_DRY;
            return true;
        default:
            return false;
    }
}

// 根据当前状态返回下一个状态，清洁流程是喷淋1-排水1-喷淋2-排水2-烘干-完成
static uint8_t next_status_after(uint8_t status)
{
    switch (status) {
        case BASE_STATUS_CLEAN_SPRAYING:
            return BASE_STATUS_CLEAN_DRAINING_1;
        case BASE_STATUS_CLEAN_DRAINING_1:
            return BASE_STATUS_RINSE_SPRAYING;
        case BASE_STATUS_RINSE_SPRAYING:
            return BASE_STATUS_CLEAN_DRAINING_2;
        case BASE_STATUS_CLEAN_DRAINING_2:
            return BASE_STATUS_CLEAN_DRYING;
        case BASE_STATUS_CLEAN_DRYING:
            return BASE_STATUS_STANDBY;
        default:
            return BASE_STATUS_CLEAN_SPRAYING;
    }
}

void screen_home_clean_flow_start(screen_home_clean_flow_t *flow)
{
    if (flow == NULL) {
        return;
    }

    memset(flow, 0, sizeof(*flow));
    flow->active = true;
    flow->expected_status = BASE_STATUS_CLEAN_SPRAYING;
}

void screen_home_clean_flow_stop(screen_home_clean_flow_t *flow)
{
    if (flow == NULL) {
        return;
    }

    memset(flow, 0, sizeof(*flow));
}

// 根据当前状态和清洁流程状态机更新清洁流程状态，并返回是否需要刷新页面步骤
bool screen_home_clean_flow_update(screen_home_clean_flow_t *flow,
                                   uint8_t base_main_status,
                                   page_clean_step_t *step)
{
    if (flow == NULL || !flow->active || step == NULL) {
        return false;
    }

    if (base_main_status == flow->last_status) {
        return false;
    }

    if (base_main_status != flow->expected_status) {
        return false;
    }

    if (base_main_status == BASE_STATUS_STANDBY) {
        screen_home_clean_flow_stop(flow);
        *step = PAGE_CLEAN_STEP_FINISH;
        return true;
    }

    if (!step_for_status(base_main_status, step)) {
        return false;
    }

    flow->last_status = base_main_status;
    flow->expected_status = next_status_after(base_main_status);

    return true;
}
