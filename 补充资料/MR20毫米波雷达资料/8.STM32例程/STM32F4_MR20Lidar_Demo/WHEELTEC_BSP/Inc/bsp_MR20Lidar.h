#ifndef __BSP_MR20LIDAR_H
#define __BSP_MR20LIDAR_H

#include <stdint.h>

//MR20毫米波雷达识别目标结果体
typedef struct{
	uint8_t id;
	float Y_Dis;
	float X_Dis;
	float Y_Vel;
	float X_Vel;
	uint8_t DynProp;
	uint8_t RCS;
	float R_Dis;
	float R_Vel;
	float angle;
}MR20Object_t;

//CAN数据帧定义
typedef struct{
	uint32_t id;
	uint8_t buffer[8];
}CANmsgType_t;

#define userconfig_MR20LidarDMA_LEN 50
extern uint8_t mr20lidar_buffer[userconfig_MR20LidarDMA_LEN];
void MR20_ObjAnalysis(MR20Object_t* obj,uint8_t *buffer);
void mr20lidar_RawDataHandle(uint16_t size);


//在线标志位
extern uint8_t MR20_Lidar_OnlineFlag;

//数据就绪标志位
extern uint8_t mr20_data_ready ;

//MR20 CAN数据帧
extern CANmsgType_t mr20_can_msg;

#endif

