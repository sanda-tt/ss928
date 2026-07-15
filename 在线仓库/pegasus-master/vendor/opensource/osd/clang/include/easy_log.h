/*
 * Copyright (c) 2025 UncleRoderick@hotmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef    EASY_LOG_H
#define    EASY_LOG_H

#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define HL_RED      //"\033[1;31m" // 亮红
#define HL_GREEN    //"\033[1;32m" // 亮绿
#define HL_YEL      //"\033[1;33m" // 亮黄
#define BROWN       //"\033[0;33m" // 灰
#define WHITE       //"\033[1;37m" // 白
#define NORMAL      //"\033[0m"    // 恢复正常

enum LOG_LEVEL {
    LOG_LEVEL_OFF = 0,
    LOG_LEVEL_ERR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DBG,
    LOG_LEVEL_ALL,
};

static const int log_level = LOG_LEVEL_ALL;

#define LOGE(fmt, ...)  \
do { \
    time_t timeSec; \
    struct tm timeTm;   \
    timeSec = time(NULL);   \
    localtime_r(&timeSec, &timeTm); \
    struct timeval tv = {0}; \
    gettimeofday(&tv, NULL); \
    if (log_level >= LOG_LEVEL_ERR) \
        printf(HL_RED "%04d/%02d/%02d %02d:%02d:%02d.%04ld[%s line:%d]:" fmt "\n", \
            1900 + timeTm.tm_year, 1 + timeTm.tm_mon, timeTm.tm_mday, timeTm.tm_hour, timeTm.tm_min, \
            timeTm.tm_sec, (long int)tv.tv_usec/1000, __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
} \
while (0)

#define LOGW(fmt, ...) \
do { \
    time_t timeSec; \
    struct tm timeTm;   \
    timeSec = time(NULL);   \
    localtime_r(&timeSec, &timeTm); \
    struct timeval tv = {0}; \
    gettimeofday(&tv, NULL); \
    if (log_level >= LOG_LEVEL_ERR) \
        printf(HL_YEL "%04d/%02d/%02d %02d:%02d:%02d.%04ld[%s line:%d]:" fmt "\n", \
            1900 + timeTm.tm_year, 1 + timeTm.tm_mon, timeTm.tm_mday, timeTm.tm_hour, timeTm.tm_min, \
            timeTm.tm_sec, (long int)(tv.tv_usec/1000), __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
} \
while (0)

#define LOGI(fmt, ...) \
do { \
    time_t timeSec; \
    struct tm timeTm;   \
    timeSec = time(NULL);   \
    localtime_r(&timeSec, &timeTm); \
    struct timeval tv = {0}; \
    gettimeofday(&tv, NULL); \
    if (log_level >= LOG_LEVEL_ERR) \
        printf(WHITE "%04d/%02d/%02d %02d:%02d:%02d.%04ld[%s line:%d]:" fmt "\n", \
            1900 + timeTm.tm_year, 1 + timeTm.tm_mon, timeTm.tm_mday, timeTm.tm_hour, timeTm.tm_min, \
            timeTm.tm_sec, (long int)(tv.tv_usec/1000), __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
} \
while (0)

#define LOGD(fmt, ...) \
do { \
    time_t timeSec; \
    struct tm timeTm; \
    timeSec = time(NULL); \
    localtime_r(&timeSec, &timeTm); \
    struct timeval tv = {0}; \
    gettimeofday(&tv, NULL); \
    if (log_level >= LOG_LEVEL_ERR) \
        printf(HL_GREEN "%04d/%02d/%02d %02d:%02d:%02d.%04ld[%s line:%d]:" fmt "\n", \
            1900 + timeTm.tm_year, 1 + timeTm.tm_mon, timeTm.tm_mday, timeTm.tm_hour, timeTm.tm_min, \
            timeTm.tm_sec, (long int)(tv.tv_usec/1000), __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
} \
while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef */
