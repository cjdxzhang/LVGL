#define _POSIX_C_SOURCE 200809L

#include "robot_chassis_transfer.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

#include "cJSON.h"
#include "robot_chassis_control.h"

#define ROBOT_CHASSIS_TRANSFER_IO_SIZE (32U * 1024U)

static int hex_value(char value)
{
    if (value >= '0' && value <= '9')
    {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f')
    {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F')
    {
        return value - 'A' + 10;
    }
    return -1;
}

int robot_chassis_hex_decode(const char *hex, size_t hex_length,
                             uint8_t *output, size_t output_capacity,
                             size_t *output_length)
{
    size_t index;

    if (hex == NULL || output == NULL || output_length == NULL ||
            (hex_length % 2U) != 0U || output_capacity < hex_length / 2U)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    for (index = 0U; index < hex_length; index += 2U)
    {
        const int high = hex_value(hex[index]);
        const int low = hex_value(hex[index + 1U]);

        if (high < 0 || low < 0)
        {
            *output_length = 0U;
            return ROBOT_CHASSIS_ERR_FORMAT;
        }
        output[index / 2U] = (uint8_t)((high << 4) | low);
    }
    *output_length = hex_length / 2U;
    return ROBOT_CHASSIS_OK;
}

static void backup_cleanup(robot_chassis_backup_transfer_t *transfer,
                           bool restart_required)
{
    if (transfer->file != NULL)
    {
        fclose(transfer->file);
        transfer->file = NULL;
    }
    if (transfer->temporary_path[0] != '\0')
    {
        (void)unlink(transfer->temporary_path);
    }
    transfer->kind = ROBOT_CHASSIS_TRANSFER_NONE;
    transfer->restart_required = restart_required;
}

void robot_chassis_backup_abort(robot_chassis_backup_transfer_t *transfer)
{
    if (transfer != NULL)
    {
        backup_cleanup(transfer, false);
    }
}

int robot_chassis_backup_begin(robot_chassis_backup_transfer_t *transfer,
                               const char *temporary_path,
                               const char *final_path)
{
    const size_t temporary_length = temporary_path != NULL ?
                                    strlen(temporary_path) : 0U;
    const size_t final_length = final_path != NULL ? strlen(final_path) : 0U;

    if (transfer == NULL || transfer->kind != ROBOT_CHASSIS_TRANSFER_NONE ||
            temporary_length == 0U || final_length == 0U ||
            temporary_length >= sizeof(transfer->temporary_path) ||
            final_length >= sizeof(transfer->final_path) ||
            strcmp(temporary_path, final_path) == 0)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    memset(transfer, 0, sizeof(*transfer));
    memcpy(transfer->temporary_path, temporary_path, temporary_length + 1U);
    memcpy(transfer->final_path, final_path, final_length + 1U);
    transfer->file = fopen(transfer->temporary_path, "wb");
    if (transfer->file == NULL)
    {
        memset(transfer, 0, sizeof(*transfer));
        return ROBOT_CHASSIS_ERR_IO;
    }
    transfer->kind = ROBOT_CHASSIS_TRANSFER_MAP_BACKUP;
    transfer->expected_step = 1U;
    return ROBOT_CHASSIS_OK;
}

static bool json_number_to_u32(const cJSON *item, uint32_t *value)
{
    if (!cJSON_IsNumber(item) || item->valuedouble < 0.0 ||
            item->valuedouble > (double)UINT32_MAX)
    {
        return false;
    }
    *value = (uint32_t)item->valuedouble;
    return item->valuedouble == (double) * value;
}

static bool parse_size(const cJSON *item, uint64_t *value)
{
    char *end = NULL;
    unsigned long long parsed;

    if (cJSON_IsNumber(item) && item->valuedouble >= 0.0 &&
            item->valuedouble <= (double)UINT64_MAX)
    {
        *value = (uint64_t)item->valuedouble;
        return item->valuedouble == (double) * value;
    }
    if (!cJSON_IsString(item) || item->valuestring == NULL ||
            item->valuestring[0] == '\0' || item->valuestring[0] == '-')
    {
        return false;
    }
    errno = 0;
    parsed = strtoull(item->valuestring, &end, 10);
    if (errno != 0 || end == item->valuestring || *end != '\0')
    {
        return false;
    }
    *value = (uint64_t)parsed;
    return true;
}

static bool valid_md5(const char *md5)
{
    size_t index;

    if (md5 == NULL || strlen(md5) != 32U)
    {
        return false;
    }
    for (index = 0U; index < 32U; ++index)
    {
        if (!isxdigit((unsigned char)md5[index]))
        {
            return false;
        }
    }
    return true;
}

static int calculate_file_md5(const char *path, char output[33])
{
    uint8_t buffer[ROBOT_CHASSIS_TRANSFER_IO_SIZE];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length = 0U;
    EVP_MD_CTX *context = NULL;
    FILE *file = NULL;
    size_t count;
    size_t index;
    int result = ROBOT_CHASSIS_ERR_IO;

    file = fopen(path, "rb");
    context = EVP_MD_CTX_new();
    if (file == NULL || context == NULL ||
            EVP_DigestInit_ex(context, EVP_md5(), NULL) != 1)
    {
        goto done;
    }
    while ((count = fread(buffer, 1U, sizeof(buffer), file)) > 0U)
    {
        if (EVP_DigestUpdate(context, buffer, count) != 1)
        {
            goto done;
        }
    }
    if (ferror(file) ||
            EVP_DigestFinal_ex(context, digest, &digest_length) != 1 ||
            digest_length != 16U)
    {
        goto done;
    }
    for (index = 0U; index < digest_length; ++index)
    {
        static const char digits[] = "0123456789abcdef";

        output[index * 2U] = digits[digest[index] >> 4U];
        output[index * 2U + 1U] = digits[digest[index] & 0x0fU];
    }
    output[32] = '\0';
    result = ROBOT_CHASSIS_OK;

done:
    if (file != NULL)
    {
        fclose(file);
    }
    EVP_MD_CTX_free(context);
    return result;
}

static int calculate_hex_data_md5(const char *hex, char output[33])
{
    uint8_t decoded[ROBOT_CHASSIS_TRANSFER_IO_SIZE];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length = 0U;
    EVP_MD_CTX *context = NULL;
    const size_t hex_length = strlen(hex);
    size_t offset = 0U;
    size_t index;
    int result = ROBOT_CHASSIS_ERR_IO;

    if ((hex_length % 2U) != 0U)
    {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    context = EVP_MD_CTX_new();
    if (context == NULL ||
            EVP_DigestInit_ex(context, EVP_md5(), NULL) != 1)
    {
        goto done;
    }
    while (offset < hex_length)
    {
        size_t part_length = hex_length - offset;
        size_t decoded_length = 0U;

        if (part_length > sizeof(decoded) * 2U)
        {
            part_length = sizeof(decoded) * 2U;
        }
        result = robot_chassis_hex_decode(hex + offset, part_length, decoded,
                                          sizeof(decoded), &decoded_length);
        if (result != ROBOT_CHASSIS_OK)
        {
            goto done;
        }
        if (EVP_DigestUpdate(context, decoded, decoded_length) != 1)
        {
            result = ROBOT_CHASSIS_ERR_IO;
            goto done;
        }
        offset += part_length;
    }
    if (EVP_DigestFinal_ex(context, digest, &digest_length) != 1 ||
            digest_length != 16U)
    {
        result = ROBOT_CHASSIS_ERR_IO;
        goto done;
    }
    for (index = 0U; index < digest_length; ++index)
    {
        static const char digits[] = "0123456789abcdef";

        output[index * 2U] = digits[digest[index] >> 4U];
        output[index * 2U + 1U] = digits[digest[index] & 0x0fU];
    }
    output[32] = '\0';
    result = ROBOT_CHASSIS_OK;

done:
    EVP_MD_CTX_free(context);
    return result;
}

static int verify_hex_data_md5(const char *hex, const char *expected_md5)
{
    char actual_md5[33];
    int result;

    result = calculate_hex_data_md5(hex, actual_md5);
    if (result != ROBOT_CHASSIS_OK)
    {
        return result;
    }
    return strcasecmp(actual_md5, expected_md5) == 0 ?
           ROBOT_CHASSIS_OK : ROBOT_CHASSIS_ERR_CHECKSUM;
}

static int write_hex_data(robot_chassis_backup_transfer_t *transfer,
                          const char *hex)
{
    uint8_t decoded[ROBOT_CHASSIS_TRANSFER_IO_SIZE];
    const size_t hex_length = strlen(hex);
    size_t offset = 0U;

    if ((hex_length % 2U) != 0U)
    {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    while (offset < hex_length)
    {
        size_t part_length = hex_length - offset;
        size_t decoded_length = 0U;
        int result;

        if (part_length > sizeof(decoded) * 2U)
        {
            part_length = sizeof(decoded) * 2U;
        }
        result = robot_chassis_hex_decode(hex + offset, part_length, decoded,
                                          sizeof(decoded), &decoded_length);
        if (result != ROBOT_CHASSIS_OK)
        {
            return result;
        }
        if (decoded_length > 0U &&
                fwrite(decoded, 1U, decoded_length, transfer->file) !=
                decoded_length)
        {
            return ROBOT_CHASSIS_ERR_IO;
        }
        if (UINT64_MAX - transfer->received_size < decoded_length)
        {
            return ROBOT_CHASSIS_ERR_LIMIT;
        }
        transfer->received_size += decoded_length;
        offset += part_length;
    }
    return ROBOT_CHASSIS_OK;
}

static int parse_backup_response(const char *json, size_t length,
                                 uint32_t expected_step, const cJSON **data,
                                 cJSON **root, cJSON **inner)
{
    const char *parse_end = NULL;
    const char *inner_end = NULL;
    const cJSON *t;
    const cJSON *p;
    const cJSON *er;
    const cJSON *encoded;
    uint32_t parsed_t;

    *root = cJSON_ParseWithLengthOpts(json, length, &parse_end, 0);
    if (*root == NULL)
    {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (parse_end < json + length && isspace((unsigned char)*parse_end))
    {
        parse_end++;
    }
    t = cJSON_GetObjectItemCaseSensitive(*root, "t");
    p = cJSON_GetObjectItemCaseSensitive(*root, "p");
    er = cJSON_IsObject(p) ? cJSON_GetObjectItemCaseSensitive(p, "er") : NULL;
    encoded = cJSON_IsObject(p) ?
              cJSON_GetObjectItemCaseSensitive(p, "data") : NULL;
    if (parse_end != json + length || !cJSON_IsObject(*root) ||
            !json_number_to_u32(t, &parsed_t) ||
            parsed_t != (expected_step == 1U ?
                         ROBOT_CHASSIS_T_MAP_BACKUP_BEGIN :
                         ROBOT_CHASSIS_T_MAP_BACKUP_NEXT) ||
            !cJSON_IsNumber(er) || er->valuedouble != 0.0 ||
            !cJSON_IsString(encoded) || encoded->valuestring == NULL)
    {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    *inner = cJSON_ParseWithOpts(encoded->valuestring, &inner_end, 0);
    if (*inner == NULL)
    {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (*inner_end != '\0' && isspace((unsigned char)*inner_end))
    {
        inner_end++;
    }
    if (*inner_end != '\0' || !cJSON_IsObject(*inner))
    {
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    *data = *inner;
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_backup_accept_response(
    robot_chassis_backup_transfer_t *transfer, const char *json, size_t length)
{
    const cJSON *data = NULL;
    const cJSON *file_data;
    const cJSON *md5;
    const cJSON *step_item;
    const cJSON *total_item;
    cJSON *root = NULL;
    cJSON *inner = NULL;
    uint32_t step;
    uint32_t step_total;
    int result;

    if (transfer == NULL || json == NULL || length == 0U ||
            transfer->kind != ROBOT_CHASSIS_TRANSFER_MAP_BACKUP ||
            transfer->file == NULL)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = parse_backup_response(json, length, transfer->expected_step,
                                   &data, &root, &inner);
    if (result != ROBOT_CHASSIS_OK)
    {
        goto failed;
    }
    file_data = cJSON_GetObjectItemCaseSensitive(data, "fileData");
    md5 = cJSON_GetObjectItemCaseSensitive(data, "md5");
    step_item = cJSON_GetObjectItemCaseSensitive(data, "step");
    total_item = cJSON_GetObjectItemCaseSensitive(data, "stepTotal");
    if (!cJSON_IsString(file_data) || file_data->valuestring == NULL ||
            !cJSON_IsString(md5) || !valid_md5(md5->valuestring) ||
            !json_number_to_u32(step_item, &step) ||
            !json_number_to_u32(total_item, &step_total) || step_total == 0U ||
            step != transfer->expected_step || step > step_total)
    {
        result = ROBOT_CHASSIS_ERR_FORMAT;
        goto failed;
    }
    if (step == 1U)
    {
        const cJSON *file_size =
            cJSON_GetObjectItemCaseSensitive(data, "fileSize");

        if (!parse_size(file_size, &transfer->expected_size) ||
                transfer->expected_size == 0U)
        {
            result = ROBOT_CHASSIS_ERR_FORMAT;
            goto failed;
        }
        transfer->step_total = step_total;
    }
    else if (transfer->step_total != step_total)
    {
        result = ROBOT_CHASSIS_ERR_FORMAT;
        goto failed;
    }
    result = verify_hex_data_md5(file_data->valuestring, md5->valuestring);
    if (result != ROBOT_CHASSIS_OK)
    {
        goto failed;
    }
    result = write_hex_data(transfer, file_data->valuestring);
    if (result != ROBOT_CHASSIS_OK)
    {
        goto failed;
    }

    if (step < step_total)
    {
        transfer->expected_step++;
        cJSON_Delete(inner);
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_WOULD_BLOCK;
    }
    if (transfer->received_size != transfer->expected_size)
    {
        result = ROBOT_CHASSIS_ERR_IO;
        goto failed;
    }
    if (fflush(transfer->file) != 0)
    {
        result = ROBOT_CHASSIS_ERR_IO;
        goto failed;
    }
    if (fclose(transfer->file) != 0)
    {
        transfer->file = NULL;
        result = ROBOT_CHASSIS_ERR_IO;
        goto failed;
    }
    transfer->file = NULL;
    if (rename(transfer->temporary_path, transfer->final_path) != 0)
    {
        result = ROBOT_CHASSIS_ERR_IO;
        goto failed;
    }
    transfer->kind = ROBOT_CHASSIS_TRANSFER_NONE;
    transfer->restart_required = false;
    cJSON_Delete(inner);
    cJSON_Delete(root);
    return ROBOT_CHASSIS_OK;

failed:
    cJSON_Delete(inner);
    cJSON_Delete(root);
    backup_cleanup(transfer, true);
    return result;
}


static void upgrade_close_file(robot_chassis_upgrade_transfer_t *transfer)
{
    if (transfer->file != NULL)
    {
        fclose(transfer->file);
        transfer->file = NULL;
    }
}

static int upgrade_fail(robot_chassis_upgrade_transfer_t *transfer, int error)
{
    transfer->retries++;
    fprintf(stderr,
            "底盘升级失败 文件名=%s 段号=%u MD5=%s 结果=%d 重试=%u\n",
            transfer->file_name, transfer->current_step,
            transfer->whole_file_md5, error, transfer->retries);
    if (transfer->retries < 3U)
    {
        return ROBOT_CHASSIS_ERR_WOULD_BLOCK;
    }
    upgrade_close_file(transfer);
    transfer->kind = ROBOT_CHASSIS_TRANSFER_NONE;
    transfer->stage = ROBOT_CHASSIS_UPGRADE_FAILED;
    return error;
}

int robot_chassis_upgrade_begin(robot_chassis_upgrade_transfer_t *transfer,
                                const char *file_path)
{
    const size_t raw_chunk_size = ROBOT_CHASSIS_UPGRADE_HEX_CHUNK_MAX / 2U;
    const char *file_name;
    off_t file_size;
    uint64_t step_total;

    if (transfer == NULL || file_path == NULL || file_path[0] == 0 ||
            strlen(file_path) >= sizeof(transfer->file_path) ||
            transfer->kind != ROBOT_CHASSIS_TRANSFER_NONE)
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    file_name = strrchr(file_path, 47);
    file_name = file_name == NULL ? file_path : file_name + 1;
    if (file_name[0] == 0 || strlen(file_name) >= sizeof(transfer->file_name))
    {
        return ROBOT_CHASSIS_ERR_INVALID_ARG;
    }
    memset(transfer, 0, sizeof(*transfer));
    transfer->file = fopen(file_path, "rb");
    if (transfer->file == NULL || fseeko(transfer->file, 0, SEEK_END) != 0 ||
            (file_size = ftello(transfer->file)) <= 0 ||
            fseeko(transfer->file, 0, SEEK_SET) != 0)
    {
        upgrade_close_file(transfer);
        return ROBOT_CHASSIS_ERR_IO;
    }
    transfer->file_size = (uint64_t)file_size;
    step_total = transfer->file_size / raw_chunk_size;
    if ((transfer->file_size % raw_chunk_size) != 0U)
    {
        step_total++;
    }
    if (step_total == 0U || step_total > UINT32_MAX ||
            calculate_file_md5(file_path, transfer->whole_file_md5) !=
            ROBOT_CHASSIS_OK)
    {
        upgrade_close_file(transfer);
        return ROBOT_CHASSIS_ERR_LIMIT;
    }
    memcpy(transfer->file_path, file_path, strlen(file_path) + 1U);
    memcpy(transfer->file_name, file_name, strlen(file_name) + 1U);
    transfer->kind = ROBOT_CHASSIS_TRANSFER_UPGRADE;
    transfer->step_total = (uint32_t)step_total;
    transfer->stage = ROBOT_CHASSIS_UPGRADE_WAIT_BEGIN;
    return ROBOT_CHASSIS_OK;
}

static int upgrade_build_chunk(robot_chassis_upgrade_transfer_t *transfer,
                               robot_chassis_command_t *command)
{
    const size_t raw_chunk_size = ROBOT_CHASSIS_UPGRADE_HEX_CHUNK_MAX / 2U;
    static const char digits[] = "0123456789abcdef";
    uint64_t offset;
    uint64_t chunk_size;
    uint8_t *raw;
    char *hex;
    size_t index;
    int result;

    if (transfer->current_step == 0U ||
            transfer->current_step > transfer->step_total)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    offset = (uint64_t)(transfer->current_step - 1U) * raw_chunk_size;
    chunk_size = transfer->file_size - offset;
    if (chunk_size > raw_chunk_size)
    {
        chunk_size = raw_chunk_size;
    }
    raw = malloc((size_t)chunk_size);
    hex = malloc((size_t)chunk_size * 2U + 1U);
    if (raw == NULL || hex == NULL ||
            fseeko(transfer->file, (off_t)offset, SEEK_SET) != 0 ||
            fread(raw, 1U, (size_t)chunk_size, transfer->file) != chunk_size ||
            ferror(transfer->file))
    {
        free(raw);
        free(hex);
        return ROBOT_CHASSIS_ERR_IO;
    }
    for (index = 0U; index < chunk_size; ++index)
    {
        hex[index * 2U] = digits[raw[index] >> 4U];
        hex[index * 2U + 1U] = digits[raw[index] & 0x0fU];
    }
    hex[chunk_size * 2U] = 0;
    result = robot_chassis_build_upgrade_chunk(hex, transfer->file_name,
             transfer->whole_file_md5,
             transfer->current_step, command);
    if (result == ROBOT_CHASSIS_OK)
    {
        fprintf(stderr,
                "底盘升级分段构造 文件名=%s 段号=%u 十六进制长度=%zu MD5=%s 结果=成功\n",
                transfer->file_name, transfer->current_step,
                (size_t)chunk_size * 2U, transfer->whole_file_md5);
    }
    free(raw);
    free(hex);
    return result;
}

int robot_chassis_upgrade_build_next(robot_chassis_upgrade_transfer_t *transfer,
                                     robot_chassis_command_t *command)
{
    if (transfer == NULL || command == NULL ||
            transfer->kind != ROBOT_CHASSIS_TRANSFER_UPGRADE ||
            transfer->file == NULL)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    switch (transfer->stage)
    {
    case ROBOT_CHASSIS_UPGRADE_WAIT_BEGIN:
        return robot_chassis_build_upgrade_begin(transfer->file_name,
                transfer->file_size,
                transfer->step_total,
                command);
    case ROBOT_CHASSIS_UPGRADE_WAIT_CHUNK:
        return upgrade_build_chunk(transfer, command);
    case ROBOT_CHASSIS_UPGRADE_WAIT_FINISH:
        return robot_chassis_build_upgrade_finish(transfer->file_name,
                transfer->whole_file_md5,
                command);
    case ROBOT_CHASSIS_UPGRADE_WAIT_EXTRACT:
        return robot_chassis_build_upgrade_extract(transfer->file_name,
                command);
    default:
        return ROBOT_CHASSIS_ERR_STATE;
    }
}

static int upgrade_parse_response(const char *json, size_t length,
                                  uint32_t expected_t, uint32_t expected_step,
                                  bool need_step)
{
    const char *parse_end = NULL;
    cJSON *root;
    const cJSON *t;
    const cJSON *p;
    const cJSON *er;
    const cJSON *step;
    uint32_t value;

    root = cJSON_ParseWithLengthOpts(json, length, &parse_end, 0);
    if (root == NULL)
    {
        return ROBOT_CHASSIS_ERR_JSON;
    }
    while (parse_end < json + length && isspace((unsigned char)*parse_end))
    {
        parse_end++;
    }
    t = cJSON_GetObjectItemCaseSensitive(root, "t");
    p = cJSON_GetObjectItemCaseSensitive(root, "p");
    er = cJSON_IsObject(p) ? cJSON_GetObjectItemCaseSensitive(p, "er") : NULL;
    step = cJSON_IsObject(p) ? cJSON_GetObjectItemCaseSensitive(p, "data") : NULL;
    if (parse_end != json + length || !cJSON_IsObject(root) ||
            !json_number_to_u32(t, &value) || value != expected_t ||
            !cJSON_IsNumber(er) || er->valuedouble != 0.0 ||
            (need_step && (!json_number_to_u32(step, &value) ||
                           value != expected_step)))
    {
        cJSON_Delete(root);
        return ROBOT_CHASSIS_ERR_FORMAT;
    }
    cJSON_Delete(root);
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_upgrade_accept_response(robot_chassis_upgrade_transfer_t *transfer,
        const char *json, size_t length)
{
    uint32_t expected_t;
    bool need_step = false;
    int result;

    if (transfer == NULL || json == NULL || length == 0U ||
            transfer->kind != ROBOT_CHASSIS_TRANSFER_UPGRADE ||
            transfer->file == NULL)
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    switch (transfer->stage)
    {
    case ROBOT_CHASSIS_UPGRADE_WAIT_BEGIN:
        expected_t = ROBOT_CHASSIS_T_UPGRADE_BEGIN;
        break;
    case ROBOT_CHASSIS_UPGRADE_WAIT_CHUNK:
        expected_t = ROBOT_CHASSIS_T_UPGRADE_CHUNK;
        need_step = true;
        break;
    case ROBOT_CHASSIS_UPGRADE_WAIT_FINISH:
        expected_t = ROBOT_CHASSIS_T_UPGRADE_FINISH;
        break;
    case ROBOT_CHASSIS_UPGRADE_WAIT_EXTRACT:
        expected_t = ROBOT_CHASSIS_T_UPGRADE_EXTRACT;
        break;
    default:
        return ROBOT_CHASSIS_ERR_STATE;
    }
    result = upgrade_parse_response(json, length, expected_t,
                                    transfer->current_step, need_step);
    if (result != ROBOT_CHASSIS_OK)
    {
        return upgrade_fail(transfer, result);
    }
    transfer->retries = 0U;
    if (transfer->stage == ROBOT_CHASSIS_UPGRADE_WAIT_BEGIN)
    {
        transfer->stage = ROBOT_CHASSIS_UPGRADE_WAIT_CHUNK;
        transfer->current_step = 1U;
    }
    else if (transfer->stage == ROBOT_CHASSIS_UPGRADE_WAIT_CHUNK)
    {
        if (transfer->current_step < transfer->step_total)
        {
            transfer->current_step++;
        }
        else
        {
            transfer->stage = ROBOT_CHASSIS_UPGRADE_WAIT_FINISH;
        }
    }
    else if (transfer->stage == ROBOT_CHASSIS_UPGRADE_WAIT_FINISH)
    {
        transfer->stage = ROBOT_CHASSIS_UPGRADE_WAIT_EXTRACT;
    }
    else
    {
        upgrade_close_file(transfer);
        transfer->kind = ROBOT_CHASSIS_TRANSFER_NONE;
        transfer->stage = ROBOT_CHASSIS_UPGRADE_SUCCEEDED;
    }
    return ROBOT_CHASSIS_OK;
}

int robot_chassis_upgrade_timeout(robot_chassis_upgrade_transfer_t *transfer)
{
    if (transfer == NULL || transfer->kind != ROBOT_CHASSIS_TRANSFER_UPGRADE ||
            transfer->file == NULL ||
            (transfer->stage != ROBOT_CHASSIS_UPGRADE_WAIT_BEGIN &&
             transfer->stage != ROBOT_CHASSIS_UPGRADE_WAIT_CHUNK &&
             transfer->stage != ROBOT_CHASSIS_UPGRADE_WAIT_FINISH &&
             transfer->stage != ROBOT_CHASSIS_UPGRADE_WAIT_EXTRACT))
    {
        return ROBOT_CHASSIS_ERR_STATE;
    }
    return upgrade_fail(transfer, ROBOT_CHASSIS_ERR_TIMEOUT);
}

void robot_chassis_upgrade_abort(robot_chassis_upgrade_transfer_t *transfer)
{
    if (transfer == NULL)
    {
        return;
    }
    upgrade_close_file(transfer);
    transfer->kind = ROBOT_CHASSIS_TRANSFER_NONE;
    transfer->stage = ROBOT_CHASSIS_UPGRADE_IDLE;
    transfer->retries = 0U;
}
