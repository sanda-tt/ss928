#include "stm32f10x.h"
#include "generic.h"
#include "Serial.h"
#include "BMI270.h"
#include "Timer.h"

/*示例(使用定时器的方法读取数据)
硬件接线:
USART  				TX A9   RX A10
BMI270(SPI)  		SCK A5    SDI A7  SDO A6  CS A4
*/

//i2cbus_struct iic_test_bus;
BMI270 bmi270_ins; // 创建一个BMI270结构体对象

//中断函数(注意中断函数的执行速度应该比BMI270的速度快)
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) { // 定时器中断的方式
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);    // clear flag
				BMI270_Get_Angle_Plus(&bmi270_ins);//获取角度
    }
}

////////////////////////串口显示数据//////////////////////////////
int main(void)
{
	Serial_Init(USART1,115200);
	if(BMI270_Init(&bmi270_ins) == -1){//初始化失败
		printf("BMI270_Init Error\n");
	}else{//初始化成功
		printf("BMI270_Init Success\n");
	}
	Delay_ms(2000);
	
	Timer_IT_Init_Hz(TIM3,200);//开始定时器中断（控制周期200Hz，应该比BMI270的采集速度高）
	while (1)
	{
		// printf("%d,%d,%d,%d,%d,%d\n", bmi270_ins.acc_x,bmi270_ins.acc_y,bmi270_ins.acc_z,bmi270_ins.gyro_x,bmi270_ins.gyro_y,bmi270_ins.gyro_z);
		// printf("%.2f\n", BMI270_get_real_temp(&bmi270_ins));//获取实际温度
		printf("%.2f,%.2f,%.2f\n", bmi270_ins.pitch,bmi270_ins.roll,bmi270_ins.yaw);//打印出欧拉角
		
		//获取BMI270的ID
		//printf("%d\n",BMI270_Get_ID());
		//Delay_ms(100);
	}
}
