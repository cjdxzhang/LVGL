#include "cleaning_animation.h"

#include <stddef.h>

LV_FONT_DECLARE(lv_font_SourceHanSansSC_Regular_25);
LV_FONT_DECLARE(lv_font_montserratMedium_20);

#define CLEANING_STAGE_COUNT       6u
#define CLEANING_INVALID_STAGE     0xffu
#define CLEANING_ROOT_WIDTH        580
#define CLEANING_ROOT_HEIGHT       390
#define CLEANING_LINE_X            78
#define CLEANING_LINE_Y            256
#define CLEANING_LINE_WIDTH        430
#define CLEANING_LINE_HEIGHT       6
#define CLEANING_STAGE_GAP         (CLEANING_LINE_WIDTH / (CLEANING_STAGE_COUNT - 1u))
#define CLEANING_NODE_SIZE         38
#define CLEANING_ACTIVE_DOT_SIZE   18
#define CLEANING_SHINE_WIDTH       28
#define CLEANING_SHINE_HEIGHT      10
#define CLEANING_SHINE_SPEED       120

static const char *const s_stage_texts[CLEANING_STAGE_COUNT] =
{
    "排水中",
    "喷淋中",
    "排水中",
    "喷淋中",
    "排水中",
    "烘干中"
};

static lv_obj_t *s_parent = NULL;
static lv_obj_t *s_root = NULL;
static lv_obj_t *s_stage_label = NULL;
static lv_obj_t *s_progress_fill = NULL;
static lv_obj_t *s_shine = NULL;
static lv_obj_t *s_nodes[CLEANING_STAGE_COUNT] = {NULL};
static lv_obj_t *s_node_labels[CLEANING_STAGE_COUNT] = {NULL};
static lv_obj_t *s_active_dots[CLEANING_STAGE_COUNT] = {NULL};
static uint8_t s_current_stage = CLEANING_INVALID_STAGE;

static bool cleaning_obj_is_valid(const lv_obj_t *obj)
{
    return obj != NULL && lv_obj_is_valid(obj);
}

static void cleaning_clear_handles(void)
{
    size_t i;

    s_parent = NULL;
    s_root = NULL;
    s_stage_label = NULL;
    s_progress_fill = NULL;
    s_shine = NULL;
    s_current_stage = CLEANING_INVALID_STAGE;

    for (i = 0; i < CLEANING_STAGE_COUNT; ++i)
    {
        s_nodes[i] = NULL;
        s_node_labels[i] = NULL;
        s_active_dots[i] = NULL;
    }
}

static void cleaning_root_delete_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_DELETE)
    {
        cleaning_clear_handles();
    }
}

static void cleaning_make_transparent(lv_obj_t *obj)
{
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
}

static void cleaning_create_glow_dot(lv_obj_t *parent, lv_coord_t center_x,
                                     lv_coord_t center_y)
{
    lv_obj_t *wrap = lv_obj_create(parent);
    lv_obj_t *glow;
    lv_obj_t *ring;
    lv_obj_t *core;

    lv_obj_set_size(wrap, 18, 18);
    lv_obj_set_pos(wrap, center_x - 9, center_y - 9);
    cleaning_make_transparent(wrap);

    glow = lv_obj_create(wrap);
    lv_obj_set_size(glow, 13, 13);
    lv_obj_center(glow);
    lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(glow, 0, 0);
    lv_obj_set_style_bg_color(glow, lv_color_hex(0x72E0FF), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_30, 0);
    lv_obj_set_style_shadow_width(glow, 10, 0);
    lv_obj_set_style_shadow_spread(glow, 1, 0);
    lv_obj_set_style_shadow_color(glow, lv_color_hex(0x58CFFF), 0);
    lv_obj_set_style_shadow_opa(glow, LV_OPA_90, 0);
    lv_obj_set_style_pad_all(glow, 0, 0);
    lv_obj_clear_flag(glow, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    ring = lv_obj_create(wrap);
    lv_obj_set_size(ring, 12, 12);
    lv_obj_center(ring);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ring, 1, 0);
    lv_obj_set_style_border_color(ring, lv_color_hex(0x72E0FF), 0);
    lv_obj_set_style_border_opa(ring, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(ring, 0, 0);
    lv_obj_set_style_pad_all(ring, 0, 0);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    core = lv_obj_create(wrap);
    lv_obj_set_size(core, 6, 6);
    lv_obj_center(core);
    lv_obj_set_style_radius(core, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(core, 0, 0);
    lv_obj_set_style_bg_color(core, lv_color_hex(0x72E0FF), 0);
    lv_obj_set_style_bg_opa(core, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(core, 0, 0);
    lv_obj_set_style_pad_all(core, 0, 0);
    lv_obj_clear_flag(core, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
}

static void cleaning_create_progress_badge(lv_obj_t *parent)
{
    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_t *label;

    lv_obj_set_size(badge, 136, 42);
    lv_obj_align(badge, LV_ALIGN_BOTTOM_MID, 7, -14);
    lv_obj_set_style_radius(badge, 12, 0);
    lv_obj_set_style_bg_color(badge, lv_color_hex(0x173D5C), 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_40, 0);
    lv_obj_set_style_border_width(badge, 1, 0);
    lv_obj_set_style_border_color(badge, lv_color_hex(0x72E0FF), 0);
    lv_obj_set_style_border_opa(badge, LV_OPA_70, 0);
    lv_obj_set_style_shadow_width(badge, 8, 0);
    lv_obj_set_style_shadow_color(badge, lv_color_hex(0x58CFFF), 0);
    lv_obj_set_style_shadow_opa(badge, LV_OPA_40, 0);
    lv_obj_set_style_pad_all(badge, 0, 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    label = lv_label_create(badge);
    lv_label_set_text(label, "流程进度");
    lv_obj_set_style_text_font(label, &lv_font_SourceHanSansSC_Regular_25, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);
}

static void cleaning_shine_anim_cb(void *obj, int32_t x)
{
    lv_obj_t *shine = (lv_obj_t *)obj;

    if (cleaning_obj_is_valid(shine))
    {
        lv_obj_set_x(shine, x);
    }
}

static void cleaning_stop_shine(void)
{
    if (cleaning_obj_is_valid(s_shine))
    {
        lv_anim_del(s_shine, cleaning_shine_anim_cb);
        lv_obj_add_flag(s_shine, LV_OBJ_FLAG_HIDDEN);
    }
}

static void cleaning_start_shine(uint8_t stage)
{
    int32_t start_x;
    int32_t end_x;
    uint32_t duration;
    lv_anim_t animation;

    if (!cleaning_obj_is_valid(s_shine) || stage == 0u)
    {
        cleaning_stop_shine();
        return;
    }

    lv_anim_del(s_shine, cleaning_shine_anim_cb);

    start_x = CLEANING_LINE_X - (CLEANING_SHINE_WIDTH / 2);
    end_x = CLEANING_LINE_X + (int32_t)(stage * CLEANING_STAGE_GAP) -
            (CLEANING_SHINE_WIDTH / 2);
    duration = (uint32_t)(((end_x - start_x) * 1000) /
                          CLEANING_SHINE_SPEED);
    if (duration < 1500u)
    {
        duration = 1500u;
    }

    lv_obj_set_x(s_shine, start_x);
    lv_obj_clear_flag(s_shine, LV_OBJ_FLAG_HIDDEN);

    lv_anim_init(&animation);
    lv_anim_set_var(&animation, s_shine);
    lv_anim_set_values(&animation, start_x, end_x);
    lv_anim_set_time(&animation, duration);
    lv_anim_set_repeat_count(&animation, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_delay(&animation, 200);
    lv_anim_set_path_cb(&animation, lv_anim_path_linear);
    lv_anim_set_exec_cb(&animation, cleaning_shine_anim_cb);
    lv_anim_start(&animation);
}

static void cleaning_create_node(uint8_t index)
{
    int32_t center_x = CLEANING_LINE_X + (int32_t)(index * CLEANING_STAGE_GAP);
    lv_obj_t *node = lv_obj_create(s_root);
    lv_obj_t *label;
    lv_obj_t *active_dot;

    lv_obj_set_size(node, CLEANING_NODE_SIZE, CLEANING_NODE_SIZE);
    lv_obj_set_pos(node,
                   center_x - (CLEANING_NODE_SIZE / 2),
                   CLEANING_LINE_Y - (CLEANING_NODE_SIZE / 2) +
                   (CLEANING_LINE_HEIGHT / 2));
    lv_obj_set_style_radius(node, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(node, 2, 0);
    lv_obj_set_style_border_color(node, lv_color_hex(0x7691A8), 0);
    lv_obj_set_style_border_opa(node, LV_OPA_70, 0);
    lv_obj_set_style_bg_color(node, lv_color_hex(0x183A5A), 0);
    lv_obj_set_style_bg_opa(node, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(node, 0, 0);
    lv_obj_clear_flag(node, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    label = lv_label_create(node);
    lv_obj_set_style_text_font(label, &lv_font_montserratMedium_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    active_dot = lv_obj_create(node);
    lv_obj_set_size(active_dot, CLEANING_ACTIVE_DOT_SIZE, CLEANING_ACTIVE_DOT_SIZE);
    lv_obj_set_style_radius(active_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(active_dot, 2, 0);
    lv_obj_set_style_border_color(active_dot, lv_color_hex(0xDDF7FF), 0);
    lv_obj_set_style_bg_color(active_dot, lv_color_hex(0x68D5FF), 0);
    lv_obj_set_style_bg_opa(active_dot, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(active_dot, 12, 0);
    lv_obj_set_style_shadow_color(active_dot, lv_color_hex(0x68D5FF), 0);
    lv_obj_set_style_shadow_opa(active_dot, LV_OPA_80, 0);
    lv_obj_set_style_pad_all(active_dot, 0, 0);
    lv_obj_clear_flag(active_dot, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(active_dot);
    lv_obj_add_flag(active_dot, LV_OBJ_FLAG_HIDDEN);

    s_nodes[index] = node;
    s_node_labels[index] = label;
    s_active_dots[index] = active_dot;
}

void cleaning_animation_create(lv_obj_t *parent)
{
    lv_obj_t *line_background;
    uint8_t i;

    if (!cleaning_obj_is_valid(parent))
    {
        return;
    }

    if (cleaning_obj_is_valid(s_root) && s_parent == parent)
    {
        return;
    }

    cleaning_animation_cleanup();
    s_parent = parent;

    s_root = lv_obj_create(parent);
    lv_obj_set_pos(s_root, 0, 0);
    lv_obj_set_size(s_root, CLEANING_ROOT_WIDTH, CLEANING_ROOT_HEIGHT);
    cleaning_make_transparent(s_root);
    lv_obj_add_event_cb(s_root, cleaning_root_delete_cb, LV_EVENT_DELETE, NULL);

    cleaning_create_progress_badge(s_root);

    s_stage_label = lv_label_create(s_root);
    lv_obj_set_pos(s_stage_label, 0, 160);
    lv_obj_set_size(s_stage_label, CLEANING_ROOT_WIDTH, 52);
    lv_label_set_long_mode(s_stage_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(s_stage_label,
                               &lv_font_SourceHanSansSC_Regular_25,
                               0);
    lv_obj_set_style_text_color(s_stage_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(s_stage_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_transform_scale(s_stage_label, 384, 0);

    line_background = lv_obj_create(s_root);
    lv_obj_set_pos(line_background, CLEANING_LINE_X, CLEANING_LINE_Y);
    lv_obj_set_size(line_background, CLEANING_LINE_WIDTH, CLEANING_LINE_HEIGHT);
    lv_obj_set_style_radius(line_background, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(line_background, 0, 0);
    lv_obj_set_style_bg_color(line_background, lv_color_hex(0x6F879B), 0);
    lv_obj_set_style_bg_opa(line_background, LV_OPA_60, 0);
    lv_obj_set_style_pad_all(line_background, 0, 0);
    lv_obj_clear_flag(line_background, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    s_progress_fill = lv_obj_create(s_root);
    lv_obj_set_pos(s_progress_fill, CLEANING_LINE_X, CLEANING_LINE_Y);
    lv_obj_set_size(s_progress_fill, 0, CLEANING_LINE_HEIGHT);
    lv_obj_set_style_radius(s_progress_fill, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(s_progress_fill, 0, 0);
    lv_obj_set_style_bg_color(s_progress_fill, lv_color_hex(0x2CA9E8), 0);
    lv_obj_set_style_bg_grad_color(s_progress_fill, lv_color_hex(0x72E0FF), 0);
    lv_obj_set_style_bg_grad_dir(s_progress_fill, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(s_progress_fill, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(s_progress_fill, 8, 0);
    lv_obj_set_style_shadow_color(s_progress_fill, lv_color_hex(0x58CFFF), 0);
    lv_obj_set_style_shadow_opa(s_progress_fill, LV_OPA_70, 0);
    lv_obj_set_style_pad_all(s_progress_fill, 0, 0);
    lv_obj_clear_flag(s_progress_fill, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    s_shine = lv_obj_create(s_root);
    lv_obj_set_pos(s_shine,
                   CLEANING_LINE_X,
                   CLEANING_LINE_Y - ((CLEANING_SHINE_HEIGHT - CLEANING_LINE_HEIGHT) / 2));
    lv_obj_set_size(s_shine, CLEANING_SHINE_WIDTH, CLEANING_SHINE_HEIGHT);
    lv_obj_set_style_radius(s_shine, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(s_shine, 0, 0);
    lv_obj_set_style_bg_color(s_shine, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_grad_color(s_shine, lv_color_hex(0x72E0FF), 0);
    lv_obj_set_style_bg_grad_dir(s_shine, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_bg_opa(s_shine, LV_OPA_80, 0);
    lv_obj_set_style_shadow_width(s_shine, 12, 0);
    lv_obj_set_style_shadow_color(s_shine, lv_color_hex(0x72E0FF), 0);
    lv_obj_set_style_shadow_opa(s_shine, LV_OPA_70, 0);
    lv_obj_set_style_pad_all(s_shine, 0, 0);
    lv_obj_clear_flag(s_shine, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_shine, LV_OBJ_FLAG_HIDDEN);

    for (i = 0; i < CLEANING_STAGE_COUNT; ++i)
    {
        cleaning_create_node(i);
    }

    cleaning_create_glow_dot(s_root, 112, 354);
    cleaning_create_glow_dot(s_root, 152, 354);
    cleaning_create_glow_dot(s_root, 192, 354);
    cleaning_create_glow_dot(s_root, 390, 354);
    cleaning_create_glow_dot(s_root, 430, 354);
    cleaning_create_glow_dot(s_root, 470, 354);

    cleaning_animation_set_stage(0u);
}

void cleaning_animation_set_stage(uint8_t stage)
{
    uint8_t i;

    if (!cleaning_obj_is_valid(s_root) || stage >= CLEANING_STAGE_COUNT)
    {
        return;
    }

    if (s_current_stage == stage)
    {
        return;
    }
    s_current_stage = stage;

    lv_label_set_text(s_stage_label, s_stage_texts[stage]);
    lv_obj_set_width(s_progress_fill, (lv_coord_t)(stage * CLEANING_STAGE_GAP));

    for (i = 0; i < CLEANING_STAGE_COUNT; ++i)
    {
        lv_obj_t *node = s_nodes[i];
        lv_obj_t *label = s_node_labels[i];
        lv_obj_t *active_dot = s_active_dots[i];
        char number[4];

        if (!cleaning_obj_is_valid(node) || !cleaning_obj_is_valid(label))
        {
            continue;
        }

        if (cleaning_obj_is_valid(active_dot))
        {
            lv_obj_add_flag(active_dot, LV_OBJ_FLAG_HIDDEN);
        }

        if (i < stage)
        {
            lv_obj_set_style_border_color(node, lv_color_hex(0x72E0FF), 0);
            lv_obj_set_style_border_opa(node, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(node, lv_color_hex(0x245D82), 0);
            lv_obj_set_style_shadow_width(node, 10, 0);
            lv_obj_set_style_shadow_color(node, lv_color_hex(0x58CFFF), 0);
            lv_obj_set_style_shadow_opa(node, LV_OPA_60, 0);
            lv_label_set_text(label, LV_SYMBOL_OK);
        }
        else if (i == stage)
        {
            lv_obj_set_style_border_color(node, lv_color_hex(0xDDF7FF), 0);
            lv_obj_set_style_border_opa(node, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(node, lv_color_hex(0x2CA9E8), 0);
            lv_obj_set_style_shadow_width(node, 20, 0);
            lv_obj_set_style_shadow_color(node, lv_color_hex(0x68D5FF), 0);
            lv_obj_set_style_shadow_opa(node, LV_OPA_90, 0);
            lv_label_set_text(label, "");
            if (cleaning_obj_is_valid(active_dot))
            {
                lv_obj_clear_flag(active_dot, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(active_dot);
            }
        }
        else
        {
            lv_obj_set_style_border_color(node, lv_color_hex(0x7691A8), 0);
            lv_obj_set_style_border_opa(node, LV_OPA_70, 0);
            lv_obj_set_style_bg_color(node, lv_color_hex(0x183A5A), 0);
            lv_obj_set_style_shadow_width(node, 0, 0);
            lv_obj_set_style_shadow_opa(node, LV_OPA_TRANSP, 0);
            lv_snprintf(number, sizeof(number), "%u", (unsigned int)i + 1u);
            lv_label_set_text(label, number);
        }

        lv_obj_move_foreground(node);
    }

    cleaning_start_shine(stage);
}

void cleaning_animation_cleanup(void)
{
    lv_obj_t *root = s_root;

    cleaning_stop_shine();
    if (cleaning_obj_is_valid(root))
    {
        lv_obj_delete(root);
    }
    cleaning_clear_handles();
}
