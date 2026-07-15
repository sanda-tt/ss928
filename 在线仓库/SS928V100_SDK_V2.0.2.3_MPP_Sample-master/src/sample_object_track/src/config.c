 /*
 * Copyright (c) CompanyNameMagicTag 2025-2025. All rights reserved.
 * Description: object track sample.
 * Author: Software Develop Team
 * Create: 2025-03-06
 */
#include "config.h"
#include <sys/stat.h>
#include <errno.h>
#include "securec.h"
#include "log.h"

ConfigEntry g_configEntries[MAX_CONFIG_SIZE];
int g_configCount = 0;

td_char *Trim(td_char *str)
{
    td_char *end;
    while (isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == 0) {
        return str;
    }
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    return str;
}

void RegisterHandler(const td_char *key, ConfigHandler handler, SAMPLE_ControlMode *usedMode, td_s32 modeSize)
{
    if (g_configCount >= MAX_CONFIG_SIZE) {
        return;
    }
    (td_void)memcpy_s(g_configEntries[g_configCount].key, sizeof(td_char) * MAX_KEY_LENGTH,
        key, sizeof(td_char) * MAX_KEY_LENGTH);
    g_configEntries[g_configCount].handler = handler;
    for (td_s32 i = 0; i < modeSize; i++) {
        g_configEntries[g_configCount].usedMode[i] = usedMode[i];
    }
    g_configEntries[g_configCount].modeSize = modeSize;
    g_configCount++;
}

ConfigHandler FindHandler(const td_char *key)
{
    for (int i = 0; i < g_configCount; i++) {
        if (strcmp(g_configEntries[i].key, key) == 0) {
            return g_configEntries[i].handler;
        }
    }
    return TD_NULL;
}

td_s32 IsNeedCheck(const td_char *key, const SAMPLE_ControlMode mode)
{
    SAMPLE_ControlMode *usedMode = TD_NULL;
    td_s32 usedModeSize = 0;
    for (int i = 0; i < g_configCount; i++) {
        if (strcmp(g_configEntries[i].key, key) == 0) {
            usedMode = g_configEntries[i].usedMode;
            usedModeSize = g_configEntries[i].modeSize;
        }
    }

    if (usedMode == TD_NULL) {
        return TD_FAILURE;
    }
    for (int i = 0; i < usedModeSize; i++) {
        if (mode == usedMode[i]) {
            return TD_SUCCESS;
        }
    }
    return TD_FAILURE;
}

td_s32 batchInputInfoHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    struct stat pathStat;
    if (stat(value, &pathStat) != 0) {
        return TD_FAILURE;
    }
    td_char realPath[PATH_MAX];
    if (realpath(value, realPath) == TD_NULL && errno != EACCES) {
        LOGE("invalid batch input info file: %s! %s\n", value, strerror(errno));
        return TD_FAILURE;
    }
    FILE *file = fopen(realPath, "r");
    if (!file) {
        LOGE("Unable to open %s.\n", realPath);
        return TD_FAILURE;
    }
    (td_void)fclose(file);
    (td_void)memcpy_s(configInfo->batchInfoFilePath, sizeof(td_char) * PATH_MAX,
        realPath, sizeof(td_char) * PATH_MAX);
    LOGI("batch_input_root_path: %s.\n", configInfo->batchInfoFilePath);
    return TD_SUCCESS;
}

td_s32 onlineUpdateInterHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    char *endPtr;
    errno = 0;
    unsigned long val = strtoul(value, &endPtr, 10);
    if (val > UINT_MAX || errno == ERANGE) {
        return TD_FAILURE;
    }
    if (*endPtr != '\0') {
        return TD_FAILURE;
    }
    configInfo->objectTrackAlgParam.onlineUpdateInter = (unsigned int)val;
    LOGI("online_update_inter: %d.\n", configInfo->objectTrackAlgParam.onlineUpdateInter);
    return TD_SUCCESS;
}

td_s32 searchFactorHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    char *endPtr;
    errno = 0;
    float val = strtof(value, &endPtr);
    if (errno == ERANGE || val > OBJECT_TRACK_MAX_ZOOMIN_FACTOR || val < OBJECT_TRACK_MIN_ZOOMIN_FACTOR) {
        return TD_FAILURE;
    }
    if (*endPtr != '\0') {
        return TD_FAILURE;
    }
    configInfo->objectTrackAlgParam.searchFactor = val;
    LOGI("search_factor: %f.\n", configInfo->objectTrackAlgParam.searchFactor);
    return TD_SUCCESS;
}

td_s32 templateFactorHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    char *endPtr;
    errno = 0;
    float val = strtof(value, &endPtr);
    if (errno == ERANGE || val > OBJECT_TRACK_MAX_ZOOMIN_FACTOR || val < OBJECT_TRACK_MIN_ZOOMIN_FACTOR) {
        return TD_FAILURE;
    }
    if (*endPtr != '\0') {
        return TD_FAILURE;
    }
    configInfo->objectTrackAlgParam.templateFactor = val;
    LOGI("template_factor: %f.\n", configInfo->objectTrackAlgParam.templateFactor);
    return TD_SUCCESS;
}

td_s32 onlineUpdateThreshHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    char *endPtr;
    errno = 0;
    float val = strtof(value, &endPtr);
    if (errno == ERANGE || val >= OBJECT_TRACK_MAX_SCORE_TH || val < 0) {
        return TD_FAILURE;
    }
    if (*endPtr != '\0') {
        return TD_FAILURE;
    }
    configInfo->objectTrackAlgParam.onlineUpdateThresh = val;
    LOGI("online_update_thresh: %f.\n", configInfo->objectTrackAlgParam.onlineUpdateThresh);
    return TD_SUCCESS;
}

td_s32 imgSizeHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    int width, height;
    int itemsRead = sscanf_s(value, "%d,%d", &width, &height);
    if (itemsRead != 2) { // 2: width, height
        return TD_FAILURE;
    }
    configInfo->objectTrackAlgParam.imgHeight = height;
    configInfo->objectTrackAlgParam.imgWidth = width;
    LOGI("img_size: %d,%d.\n", configInfo->objectTrackAlgParam.imgWidth, configInfo->objectTrackAlgParam.imgHeight);
    return TD_SUCCESS;
}

td_s32 pixFormatHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    if (strcmp(value, "0") != 0 && strcmp(value, "1") != 0) {
        return TD_FAILURE;
    }
    configInfo->objectTrackAlgParam.pixFormat = atoi(value) == 0
        ? OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420 : OT_PIXEL_FORMAT_RGB_888;
    LOGI("pix_format: %d.\n", atoi(value));
    return TD_SUCCESS;
}

td_s32 initBoxHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    int x, y, width, height;
    int itemsRead = sscanf_s(value, "%d,%d,%d,%d", &x, &y, &width, &height);
    if (itemsRead != 4) { // 4: x, y, width, height
        return TD_FAILURE;
    }
    configInfo->initBoxParam.x = x;
    configInfo->initBoxParam.y = y;
    configInfo->initBoxParam.width = width;
    configInfo->initBoxParam.height = height;
    LOGI("init_box: %d,%d,%d,%d.\n", configInfo->initBoxParam.x, configInfo->initBoxParam.y,
        configInfo->initBoxParam.width, configInfo->initBoxParam.height);
    return TD_SUCCESS;
}

td_s32 inputPathHandler(const td_char *value, SAMPLE_ConfigInfo *configInfo)
{
    struct stat pathStat;
    if (stat(value, &pathStat) != 0 || !S_ISDIR(pathStat.st_mode)) {
        return TD_FAILURE;
    }
    td_char realPath[PATH_MAX];
    if (realpath(value, realPath) == TD_NULL && errno != EACCES) {
        LOGE("invalid input path: %s! %s\n", value, strerror(errno));
        return TD_FAILURE;
    }
    (td_void)memcpy_s(configInfo->trackFramePath, sizeof(td_char) * PATH_MAX,
        realPath, sizeof(td_char) * PATH_MAX);
    LOGI("%s: %s.\n", value, configInfo->trackFramePath);
    return TD_SUCCESS;
}

void InitConfig(void)
{
    SAMPLE_ControlMode all[] = {NORM_MODE, DEBUG_MODE, BATCH_MODE};
    SAMPLE_ControlMode single[] = {NORM_MODE, DEBUG_MODE};
    SAMPLE_ControlMode batch[] = {BATCH_MODE};
    RegisterHandler("init_box", initBoxHandler, single, sizeof(single) / sizeof(single[0]));
    RegisterHandler("input_path", inputPathHandler, single, sizeof(single) / sizeof(single[0]));
    RegisterHandler("pix_format", pixFormatHandler, all, sizeof(all) / sizeof(all[0]));
    RegisterHandler("img_size", imgSizeHandler, single, sizeof(single) / sizeof(single[0]));
    RegisterHandler("online_update_thresh", onlineUpdateThreshHandler, all, sizeof(all) / sizeof(all[0]));
    RegisterHandler("template_factor", templateFactorHandler, all, sizeof(all) / sizeof(all[0]));
    RegisterHandler("search_factor", searchFactorHandler, all, sizeof(all) / sizeof(all[0]));
    RegisterHandler("online_update_inter", onlineUpdateInterHandler, all, sizeof(all) / sizeof(all[0]));
    RegisterHandler("batch_input_root_path", inputPathHandler, batch, sizeof(batch) / sizeof(batch[0]));
    RegisterHandler("batch_info_file", batchInputInfoHandler, batch, sizeof(batch) / sizeof(batch[0]));
}
