#include "bsp_MR20Lidar.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

//在线标志位
uint8_t MR20_Lidar_OnlineFlag = 0;

//数据就绪标志位
uint8_t mr20_data_ready = 0;

//MR20 CAN数据帧
CANmsgType_t mr20_can_msg = { 0 };

//DMA原始数据存放位置
uint8_t mr20lidar_buffer[userconfig_MR20LidarDMA_LEN];

//MR20雷达数据处理函数
void mr20lidar_RawDataHandle(uint16_t size)
{
	static uint8_t handle_sm = 0;  //数据处理状态机
	static uint8_t lastrecv = 0;   //上一次的数据状态
	static uint8_t usebuffer[10];  //临时保存数据数据
	static uint8_t cnt = 0 ;       //数据接收辅组计数器
	
	for( uint8_t i=0;i<size;i++ )
	{
		switch( handle_sm )
		{
			case 0:
				if(mr20lidar_buffer[i]==0xAA && lastrecv==0xAA)
				{
					handle_sm = 1;//接收到帧头
					cnt=0;
				}
				break;
			case 1:
				usebuffer[cnt++] = mr20lidar_buffer[i];
				if( cnt == 10 ) handle_sm=2,cnt=0; //数据部分结束
				break;
			case 2:
				cnt++;
				if( cnt==2 )
				{
					//帧尾正确,开始处理数据
					if( lastrecv==0x55 && mr20lidar_buffer[i]==0x55 )
					{
						mr20_can_msg.id = usebuffer[1]<<8 | usebuffer[0]; //帧id
						memcpy(mr20_can_msg.buffer,&usebuffer[2],8);      //帧数据
						
						//接收到完整数据,MR20上线
						MR20_Lidar_OnlineFlag=1;
						
						//数据就绪通知
						mr20_data_ready=1;
					}
					
					//清空所有状态重新接收
					cnt=0;
					handle_sm=0;
				}
				break;
			default:
				break;
		}
		
		lastrecv = mr20lidar_buffer[i];
	}
}

//MR20数组解析
void MR20_ObjAnalysis(MR20Object_t* obj,uint8_t *buffer) 
{
	//原始值解析
	obj->id = buffer[0];
	obj->Y_Dis = (buffer[1]*32 + (buffer[2]>>3))*0.1f - 500;
	obj->X_Dis = ((buffer[2]&0x07)*256 + buffer[3])*0.1f - 102.3f;
	obj->Y_Vel = ((buffer[4]<<2)+(buffer[5]>>6))*0.25f-128;
	obj->X_Vel = ((buffer[5]&0x3F)*8 + (buffer[6]>>5))*0.25f-64;
	obj->DynProp = buffer[6]&0x07;
	obj->RCS = buffer[7];
	
	//径向距离与速度
	obj->R_Dis = sqrtf(obj->Y_Dis*obj->Y_Dis + obj->X_Dis*obj->X_Dis);
	obj->angle = atanf(obj->X_Dis/obj->Y_Dis);
	obj->R_Vel = obj->Y_Vel*cosf(obj->angle ) + obj->X_Vel*sinf(obj->angle );
}

