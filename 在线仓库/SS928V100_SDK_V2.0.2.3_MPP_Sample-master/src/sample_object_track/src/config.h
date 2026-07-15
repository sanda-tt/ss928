 /*
 * Copyright (c) CompanyNameMagicTag 2025-2025. All rights reserved.
 * Description: object track sample.
 * Author: Software Develop Team
 * Create: 2025-03-06
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "ot_type.h"
#include "ot_object_track_define.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_LINE_LENGTH 256
#define MAX_KEY_LENGTH 64
#define MAX_VALUE_LENGTH 128
#define MAX_CONFIG_SIZE 100
#define OBJECT_TRACK_MAX_SCORE_TH 1.0
#define OBJECT_TRACK_MIN_ZOOMIN_FACTOR 1.0
#define OBJECT_TRACK_MAX_ZOOMIN_FACTOR 6.0

typedef struct {
    OT_OBJECTTRACK_BoxParam initBoxParam;
    OT_OBJECTTRACK_AlgParam objectTrackAlgParam;
    td_char trackFramePath[PATH_MAX];
    td_char batchInfoFilePath[PATH_MAX];
} SAMPLE_ConfigInfo;

// 回调函数类型
typedef td_s32 (*ConfigHandler)(const td_char *value, SAMPLE_ConfigInfo *configInfo);

typedef enum {
    NORM_MODE = 0,
    DEBUG_MODE,
    BATCH_MODE,
    BUTT_MODE,
} SAMPLE_ControlMode;

typedef struct {
    td_char key[MAX_KEY_LENGTH];
    ConfigHandler handler;
    SAMPLE_ControlMode usedMode[BUTT_MODE];
    td_s32 modeSize;
} ConfigEntry;

char *Trim(td_char *str);
void RegisterHandler(const td_char *key, ConfigHandler handler, SAMPLE_ControlMode *usedMode, td_s32 modeSize);
ConfigHandler FindHandler(const td_char *key);
void InitConfig(void);
td_s32 IsNeedCheck(const td_char *key, const SAMPLE_ControlMode mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif // CONFIG_H
