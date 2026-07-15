#ifndef _WRAPPERNCNN_H__
#define _WRAPPERNCNN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "detectobjs.h"
/*
JPG or PNG chang to yuv420sp W=640 scale
*/
int ncnn_convertimg_yolov5s(const char* jpg, const char* yuvpath);
int ncnn_result_yolov5(float *src,unsigned int len, stYolov5Objs* pOut);

int ncnn_result_yolov8(float *src,unsigned int len, stYolov5Objs* pOut);

#ifdef __cplusplus
}
#endif
#endif