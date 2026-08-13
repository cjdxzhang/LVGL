#include "robot_chassis_framer.h"

#include <stdlib.h>
#include <string.h>

static const uint8_t g_frame_suffix[ROBOT_CHASSIS_FRAME_SUFFIX_SIZE] =
{
    '\r', '\n', '\r', '\n'
};

static uint32_t read_le32(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8U) |
           ((uint32_t)data[2] << 16U) |
           ((uint32_t)data[3] << 24U);
}

static int reserve_buffer(robot_chassis_framer_t *framer, size_t required)
{
    uint8_t *new_buffer;
    size_t new_capacity;
    const size_t limit = framer->max_frame_size + sizeof(uint32_t);

    if (required <= framer->capacity)
    {
        return ROBOT_CHASSIS_OK;
    }
    if (required > limit)
    {
        return ROBOT_CHASSIS_ERR_FRAME_TOO_LARGE;
    }

    new_capacity = framer->capacity == 0U ? 256U : framer->capacity;
    while (new_capacity < required)
    {
        if (new_capacity > limit / 2U)
        {
            new_capacity = limit;
            break;
        }
        new_capacity *= 2U;
    }

    new_buffer = realloc(framer->buffer, new_capacity);
    if (new_buffer == NULL)
    {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    framer->buffer = new_buffer;
    framer->capacity = new_capacity;
    return ROBOT_CHASSIS_OK;
}

static int emit_complete_frames(robot_chassis_framer_t *framer,
                                robot_chassis_frame_callback_t callback,
                                size_t *frame_count)
{
    while (framer->length >= sizeof(uint32_t))
    {
        const uint32_t body_length = read_le32(framer->buffer);
        size_t total_length;
        size_t json_length;
        int result;

        if (body_length <= ROBOT_CHASSIS_FRAME_SUFFIX_SIZE)
        {
            return ROBOT_CHASSIS_ERR_FRAME_INVALID;
        }
        if ((size_t)body_length > framer->max_frame_size)
        {
            return ROBOT_CHASSIS_ERR_FRAME_TOO_LARGE;
        }

        total_length = sizeof(uint32_t) + (size_t)body_length;
        if (framer->length < total_length)
        {
            return ROBOT_CHASSIS_OK;
        }
        if (memcmp(framer->buffer + total_length - ROBOT_CHASSIS_FRAME_SUFFIX_SIZE,
                   g_frame_suffix, ROBOT_CHASSIS_FRAME_SUFFIX_SIZE) != 0)
        {
            return ROBOT_CHASSIS_ERR_FRAME_INVALID;
        }

        json_length = (size_t)body_length - ROBOT_CHASSIS_FRAME_SUFFIX_SIZE;
        result = callback((const char *)framer->buffer + sizeof(uint32_t),
                          json_length);
        if (result != ROBOT_CHASSIS_OK)
        {
            return result;
        }

        framer->length -= total_length;
        if (framer->length != 0U)
        {
            memmove(framer->buffer, framer->buffer + total_length, framer->length);
        }
        (*frame_count)++;
    }
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_framer_init(robot_chassis_framer_t *framer, size_t max_frame_size)
{
    if (framer == NULL || max_frame_size <= ROBOT_CHASSIS_FRAME_SUFFIX_SIZE ||
            max_frame_size > SIZE_MAX - sizeof(uint32_t))
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }

    memset(framer, 0, sizeof(*framer));
    framer->max_frame_size = max_frame_size;
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_framer_reset(robot_chassis_framer_t *framer)
{
    if (framer != NULL)
    {
        framer->length = 0U;
    }
}

void robot_chassis_framer_deinit(robot_chassis_framer_t *framer)
{
    if (framer == NULL)
    {
        return;
    }

    free(framer->buffer);
    memset(framer, 0, sizeof(*framer));
}

int robot_chassis_framer_feed(robot_chassis_framer_t *framer, const uint8_t *data,
                              size_t length, robot_chassis_frame_callback_t callback,
                              size_t *frame_count)
{
    size_t offset = 0U;
    const size_t limit = framer != NULL
                         ? framer->max_frame_size + sizeof(uint32_t)
                         : 0U;

    if (framer == NULL || callback == NULL || frame_count == NULL ||
            (data == NULL && length != 0U) || framer->max_frame_size == 0U)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    *frame_count = 0U;

    while (offset < length)
    {
        const size_t available = limit - framer->length;
        const size_t remaining = length - offset;
        const size_t copy_length = remaining < available ? remaining : available;
        int result;

        if (copy_length == 0U)
        {
            result = emit_complete_frames(framer, callback, frame_count);
            if (result != ROBOT_CHASSIS_OK)
            {
                return result;
            }
            if (framer->length == limit)
            {
                return ROBOT_CHASSIS_ERR_FRAME_INVALID;
            }
            continue;
        }

        result = reserve_buffer(framer, framer->length + copy_length);
        if (result != ROBOT_CHASSIS_OK)
        {
            return result;
        }
        memcpy(framer->buffer + framer->length, data + offset, copy_length);
        framer->length += copy_length;
        offset += copy_length;

        result = emit_complete_frames(framer, callback, frame_count);
        if (result != ROBOT_CHASSIS_OK)
        {
            return result;
        }
    }

    return emit_complete_frames(framer, callback, frame_count);
}
