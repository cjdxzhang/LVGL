#include "robot_chassis_map.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#include "cJSON.h"

#define ROBOT_CHASSIS_GZIP_OVERHEAD_LIMIT (64U * 1024U)

static int base64_value(unsigned char character)
{
    if (character >= 'A' && character <= 'Z') {
        return (int)(character - 'A');
    }
    if (character >= 'a' && character <= 'z') {
        return (int)(character - 'a') + 26;
    }
    if (character >= '0' && character <= '9') {
        return (int)(character - '0') + 52;
    }
    if (character == '+') {
        return 62;
    }
    if (character == '/') {
        return 63;
    }
    return -1;
}

static int decode_base64(const char *input, uint8_t **output,
                         size_t *output_length)
{
    size_t input_length;
    size_t padding = 0U;
    size_t decoded_length;
    uint8_t *decoded;
    size_t input_offset;
    size_t output_offset = 0U;

    if (input == NULL || output == NULL || output_length == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    input_length = strlen(input);
    if (input_length == 0U || input_length % 4U != 0U) {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    if (input[input_length - 1U] == '=') {
        padding++;
    }
    if (input[input_length - 2U] == '=') {
        padding++;
    }
    for (input_offset = 0U; input_offset < input_length - padding;
         ++input_offset) {
        if (input[input_offset] == '=') {
            return ROBOT_CHASSIS_ERR_FORMAT;
        }
    }
    decoded_length = input_length / 4U * 3U - padding;
    decoded = malloc(decoded_length);
    if (decoded == NULL) {
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }

    for (input_offset = 0U; input_offset < input_length; input_offset += 4U) {
        int values[4];
        uint32_t block;
        size_t index;

        for (index = 0U; index < 4U; ++index) {
            const char character = input[input_offset + index];

            values[index] = character == '=' ? 0 :
                            base64_value((unsigned char)character);
            if (values[index] < 0) {
                free(decoded);
                return ROBOT_CHASSIS_ERR_FORMAT;
            }
        }
        block = ((uint32_t)values[0] << 18U) |
                ((uint32_t)values[1] << 12U) |
                ((uint32_t)values[2] << 6U) |
                (uint32_t)values[3];
        if (output_offset < decoded_length) {
            decoded[output_offset++] = (uint8_t)(block >> 16U);
        }
        if (output_offset < decoded_length) {
            decoded[output_offset++] = (uint8_t)(block >> 8U);
        }
        if (output_offset < decoded_length) {
            decoded[output_offset++] = (uint8_t)block;
        }
    }

    *output = decoded;
    *output_length = decoded_length;
    return ROBOT_CHASSIS_OK;
}

static int inflate_gzip(const uint8_t *compressed, size_t compressed_length,
                        uint8_t *output, size_t output_length)
{
    z_stream stream;
    int result;

    if (compressed_length > UINT_MAX || output_length > UINT_MAX) {
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    memset(&stream, 0, sizeof(stream));
    stream.next_in = (Bytef *)compressed;
    stream.avail_in = (uInt)compressed_length;
    stream.next_out = output;
    stream.avail_out = (uInt)output_length;

    result = inflateInit2(&stream, MAX_WBITS + 16);
    if (result != Z_OK) {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    result = inflate(&stream, Z_FINISH);
    if (result != Z_STREAM_END || stream.total_out != output_length ||
        stream.total_in != compressed_length) {
        inflateEnd(&stream);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    inflateEnd(&stream);
    return ROBOT_CHASSIS_OK;
}

static bool json_u32(const cJSON *item, uint32_t *value)
{
    uint32_t converted;

    if (!cJSON_IsNumber(item) || item->valuedouble <= 0.0 ||
        item->valuedouble > (double)UINT32_MAX) {
        return false;
    }
    converted = (uint32_t)item->valuedouble;
    if (item->valuedouble != (double)converted) {
        return false;
    }
    *value = converted;
    return true;
}

static uint32_t read_le32(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8U) |
           ((uint32_t)data[2] << 16U) |
           ((uint32_t)data[3] << 24U);
}

static int parse_map_envelope(const char *json, size_t length, cJSON **root_out,
                              const char **base64_out, uint32_t *width,
                              uint32_t *height, double *offx, double *offy,
                              double *resolution)
{
    cJSON *root;
    const char *parse_end = NULL;
    const cJSON *t;
    const cJSON *p;
    const cJSON *data;
    const cJSON *payload;
    int t_value;

    root = cJSON_ParseWithLengthOpts(json, length, &parse_end, 0);
    if (root == NULL) {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (parse_end < json + length && isspace((unsigned char)*parse_end)) {
        parse_end++;
    }
    t = cJSON_GetObjectItemCaseSensitive(root, "t");
    if (parse_end != json + length || !cJSON_IsObject(root) ||
        !cJSON_IsNumber(t) || t->valuedouble < (double)INT_MIN ||
        t->valuedouble > (double)INT_MAX) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    t_value = (int)t->valuedouble;
    if (t->valuedouble != (double)t_value ||
        t_value != ROBOT_CHASSIS_T_REALTIME_MAP) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    p = cJSON_GetObjectItemCaseSensitive(root, "p");
    data = cJSON_IsObject(p)
               ? cJSON_GetObjectItemCaseSensitive(p, "data")
               : NULL;
    payload = cJSON_IsObject(data)
                  ? cJSON_GetObjectItemCaseSensitive(data, "data")
                  : NULL;
    if (!cJSON_IsString(payload) || payload->valuestring == NULL ||
        !json_u32(cJSON_GetObjectItemCaseSensitive(data, "width"), width) ||
        !json_u32(cJSON_GetObjectItemCaseSensitive(data, "height"), height)) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }

    payload = cJSON_GetObjectItemCaseSensitive(data, "offx");
    if (!cJSON_IsNumber(payload) || !isfinite(payload->valuedouble)) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    *offx = payload->valuedouble;
    payload = cJSON_GetObjectItemCaseSensitive(data, "offy");
    if (!cJSON_IsNumber(payload) || !isfinite(payload->valuedouble)) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    *offy = payload->valuedouble;
    payload = cJSON_GetObjectItemCaseSensitive(data, "resolution");
    if (!cJSON_IsNumber(payload) || !isfinite(payload->valuedouble)) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    *resolution = payload->valuedouble;
    payload = cJSON_GetObjectItemCaseSensitive(data, "data");
    *base64_out = payload->valuestring;
    *root_out = root;
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_map_parse_grid(const char *json, size_t length,
                                 size_t max_cells,
                                 robot_chassis_grid_t *grid)
{
    robot_chassis_grid_t parsed = {0};
    cJSON *root = NULL;
    const char *base64 = NULL;
    uint32_t width;
    uint32_t height;
    double offx;
    double offy;
    double resolution;
    size_t cell_count;
    size_t expanded_length;
    uint8_t *compressed = NULL;
    size_t compressed_length = 0U;
    uint8_t *expanded = NULL;
    size_t index;
    int result;

    if (json == NULL || length == 0U || max_cells == 0U || grid == NULL) {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    result = parse_map_envelope(json, length, &root, &base64, &width, &height,
                                &offx, &offy, &resolution);
    if (result != ROBOT_CHASSIS_OK) {
        return result;
    }
    if (width == 0U || height == 0U) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    if ((size_t)width > SIZE_MAX / (size_t)height) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    cell_count = (size_t)width * (size_t)height;
    if (cell_count > max_cells || cell_count > SIZE_MAX - 8U) {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    expanded_length = cell_count + 8U;

    result = decode_base64(base64, &compressed, &compressed_length);
    if (result != ROBOT_CHASSIS_OK) {
        cJSON_Delete(root);
        return result;
    }
    if (compressed_length > expanded_length + ROBOT_CHASSIS_GZIP_OVERHEAD_LIMIT) {
        free(compressed);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    expanded = malloc(expanded_length);
    if (expanded == NULL) {
        free(compressed);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    result = inflate_gzip(compressed, compressed_length, expanded,
                          expanded_length);
    free(compressed);
    if (result != ROBOT_CHASSIS_OK ||
        read_le32(expanded) != width ||
        read_le32(expanded + 4U) != height) {
        free(expanded);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    for (index = 0U; index < cell_count; ++index) {
        const uint8_t cell = expanded[index + 8U];

        if (cell != 0U && cell != 100U && cell != 255U) {
            free(expanded);
            cJSON_Delete(root);
            return ROBOT_CHASSIS_ERR_FORMAT;
        }
    }

    parsed.cells = malloc(cell_count);
    if (parsed.cells == NULL) {
        free(expanded);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_NO_MEMORY;
    }
    memcpy(parsed.cells, expanded + 8U, cell_count);
    free(expanded);
    cJSON_Delete(root);

    parsed.width = width;
    parsed.height = height;
    parsed.cell_count = cell_count;
    parsed.origin_x = offy;
    parsed.origin_y = offx;
    parsed.resolution = resolution;
    parsed.coordinate_conversion_available = resolution > 0.0;
    free(grid->cells);
    *grid = parsed;
    return ROBOT_CHASSIS_OK;
}

void robot_chassis_map_grid_release(robot_chassis_grid_t *grid)
{
    if (grid == NULL) {
        return;
    }
    free(grid->cells);
    memset(grid, 0, sizeof(*grid));
}
