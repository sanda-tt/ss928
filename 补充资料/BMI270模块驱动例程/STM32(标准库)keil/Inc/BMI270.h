#ifndef __BMI270_H__
#define __BMI270_H__

//示例只支持SPI模式
#include "stdint.h"
#include "stdio.h"

/*
 * BMI270 Driver
 * Copyright (c) 2026 bilibili 小努班 UID:437280309
 * 
 * 本作品采用知识共享署名-非商业性使用 3.0 未本地化版本协议进行许可。
 * 详情请见：https://creativecommons.org/licenses/by-nc/3.0/
 * 
 * 允许非商业目的的使用、复制、修改和分发，但必须保留原作者署名，
 * 且不得用于商业用途。如需商业使用，请联系我获取授权。
 */

#define BMI_270_IIC_ADDR 0x69//IIC地址(SDO:1 为0x69,SDO:0 为0x68)

typedef struct
{
	//原始数据
    int16_t acc_x;      // X轴加速度原始数据
    int16_t acc_y;      // Y轴加速度原始数据
    int16_t acc_z;      // Z轴加速度原始数据
    int16_t gyro_x;     // X轴陀螺仪原始数据
    int16_t gyro_y;     // Y轴陀螺仪原始数据
    int16_t gyro_z;     // Z轴陀螺仪原始数据
    int16_t rawTemp;   // 温度传感器原始数据
    //返回的角度(欧拉角描述法)
    float yaw;       // 偏航角
    float roll;      // 翻滚角
    float pitch;     // 俯仰角
    float temp;      //实际温度值
    //四元素描述法
    float q0;
    float q1;
    float q2;
    float q3;
}BMI270;

////////////////常用函数//////////////
int8_t BMI270_Init(BMI270* _this);//传感器初始化
float BMI270_get_real_temp(BMI270 *_this);//获取实际温度值
void BMI270_Get_Angle_Plus(BMI270* _this) ;

//////////////底层函数//////////////
void BMI270_Get_RawData(BMI270 *_this);
void BMI270_Get_Raw_temp(BMI270 *_this);
uint8_t BMI270_Get_ID(void);//获取ID

#endif // __BMI270_H__
