/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023.All rightd reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#ifndef __DATA_PROC_H__
#define __DATA_PROC_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define IDENTIFY_NUM 10
#define EEPROM_LEN 521
#define CLOUDPOINT_CALIB_PARA_LEN 12

typedef enum {
    DTOF_SUCCESS,
    // 内存申请失败
    DTOF_MALLOC_FAILED,
    // 内存拷贝失败
    DTOF_MEMCPY_FAILED,
    // 参数异常
    DTOF_PARA_INVALID,
    // 温度异常
    DTOF_TEMP_INVALID,
    // 空指针
    DOTF_POINT_INVALID,
    // 文件路径异常
    DOTF_FILE_PATH_INVALID,
    // 未初始化
    DOTF_NOT_INIT,
    DTOF_FAILURE,
} DtofError;

typedef struct {
    char *iniFile;
    char *spotFile;
    unsigned char dtofCalibPara[EEPROM_LEN]; // dtof标定参数
} DtofHandleConfig;

typedef struct DtofPixelPosition {
    int x;
    int y;
    int averageDepth;
} DtofPixelPosition;

typedef struct DtofComponent {
    DtofPixelPosition leftBottomPixel;
    DtofPixelPosition rightTopPixel;
    DtofPixelPosition centerPixel;
    int labelId;
} DtofComponent;

typedef struct DtofPointCloud {
    float x; // x坐标
    float y; // y坐标
    float z; // z坐标
} DtofPointCloud;

typedef struct {
    int switchFlag; // 1-> switch    0->  not switch
    int configFlag;
    unsigned short *distance;
    unsigned short *confidence;
    unsigned short *zDepth;
    unsigned short *labelImg;
    DtofPointCloud *pointCloud;
    float para[CLOUDPOINT_CALIB_PARA_LEN]; // 相机标定参数, 按照下标顺序依次为：内参(fx,fy,cx,cy)，畸变参数(k1,k2,p1,p2,k3,k4,k5,k6)
    DtofComponent component[IDENTIFY_NUM];
    int ifPseudorandom; // 0: 表示可信 1： 表示不可信
    int pseudorandomCount; // 伪随机过滤点数
} DtofOutput;

typedef struct {
    unsigned short *data; // 图像数据
    int dataLen;
    float thresholdProb;
    float temperature;
    unsigned int readTemperatureCount;
    unsigned int processor;
    DtofOutput dtofOutput; // 输出数据
} DtofHandle;

/*
 * \brief init dtof.
 * \param[in] config The path of config file and the calibration parameters of dtof.
 * \return the handle of dtof.
 */
DtofHandle *DtofInit(DtofHandleConfig *config);

/*
 * \brief process pixel data.
 * \param[in] handle the handle of dtof.
 */
DtofError DtofProcess(DtofHandle *handle);

/*
 * \brief Release related resources.
 * \param[in] handle the handle of dtof.
 */
void DtofDestory(DtofHandle *handle);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif