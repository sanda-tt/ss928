 /*
 * Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
 * Description: log define.
 * Author: Software Develop Team
 * Create: 2024-11-20
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* log level define */
typedef enum {
    OT_SOT_LOG_LEVEL_FATAL_REF = 0, /* action must be taken immediately */
    OT_SOT_LOG_LEVEL_PRINT_REF,     /* print directly */
    OT_SOT_LOG_LEVEL_ERROR_REF,     /* error conditions */
    OT_SOT_LOG_LEVEL_WARNING_REF,   /* warning conditions */
    OT_SOT_LOG_LEVEL_INFO_REF,      /* informational */
    OT_SOT_LOG_LEVEL_DEBUG_REF,     /* debug-level */
    OT_SOT_LOG_LEVEL_BUTT_REF
} OT_SOT_LOG_Level;

static OT_SOT_LOG_Level g_level = OT_SOT_LOG_LEVEL_INFO_REF;

// log
#define TRACK_PRINTF(level, msg, ...) \
    do { \
        fprintf(stderr, "[\033[1;32m %s \033[m]:%s[%u]:" msg, level, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define TRACK_PRINTF_RED(level, msg, ...) \
    do { \
        fprintf(stderr, "[\033[0;31m %s \033[m]:%s[%u]:" msg, level, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)


#define TRACK_TRACE_DEBUG(msg, ...) TRACK_PRINTF("Debug", msg,  ##__VA_ARGS__)
#define TRACK_TRACE_INFO(msg, ...) TRACK_PRINTF("Info", msg,  ##__VA_ARGS__)

#define LOGE(msg, ...) TRACK_PRINTF_RED("Error", msg,  ##__VA_ARGS__)

#define LOGI(msg, ...) do { \
    if (g_level >= OT_SOT_LOG_LEVEL_INFO_REF) { \
        TRACK_TRACE_INFO(msg,  ##__VA_ARGS__); \
    } \
} while (0)

#define LOGD(msg, ...) do { \
    if (g_level >= OT_SOT_LOG_LEVEL_DEBUG_REF) { \
        TRACK_TRACE_INFO(msg,  ##__VA_ARGS__); \
    } \
} while (0)

/* ----------------------------------------------------------------- */
#define OT_SOT_RETURN_IF_PTR_NULL(p, errCode) do { \
    if ((p) == TD_NULL) { \
        LOGE("pointer[%s] is null\n", #p); \
        return (errCode); \
    } \
} while (0)

#define OT_SOT_RETURN_IF_EXPR_FALSE(expr, errCode) do { \
    if (!(expr)) { \
        LOGE("expr[%s] false\n", #expr); \
        return (errCode); \
    } \
} while (0)

#define OT_SOT_LOG_AND_RETURN_IF_EXPR_FALSE(expr, errCode, errString) do { \
    if (!(expr)) { \
        LOGE("[%s] failed\n", (errString)); \
        return (errCode); \
    } \
} while (0)

#define OT_SOT_LOG_AND_GOTO_IF_EXPR_FALSE(expr, label, errString) do { \
    if (!(expr)) { \
        LOGE("[%s] failed\n", (errString)); \
        goto label; \
    } \
} while (0)

#define OT_SOT_RETURN_IF_FAIL(ret, errCode) do { \
    if ((ret) != TD_SUCCESS) { \
        LOGE("Error Code: [0x%08X]\n\n", (ret)); \
        return (errCode); \
    } \
} while (0)

#define OT_SOT_LOG_IF_FAIL(ret, errString) do { \
        if ((ret) != TD_SUCCESS) { \
            LOGE("[%s] failed[0x%08X]\n", (errString), (ret)); \
        }                                                       \
    } while (0)

#define OT_SOT_LOG_AND_RETURN_IF_FAIL(ret, errCode, errString) do { \
    if ((ret) != TD_SUCCESS) { \
        LOGE("[%s] failed[0x%08X]\n", (errString), (ret)); \
        return (errCode); \
    } \
} while (0)

#define OT_SOT_LOG_IF_EXPR_FALSE(expr, errString) do { \
    if (!(expr)) { \
        LOGE("[%s] failed\n", (errString)); \
    } \
} while (0)

#define OT_SOT_LOG_ERRCODE_IF_EXPR_FALSE(expr, errCode, errString) do { \
    if (!(expr)) { \
            LOGE("[%s] failed[0x%08X]\n", (errString), (errCode)); \
        } \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef LOG_H */