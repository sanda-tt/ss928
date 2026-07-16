/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

#include "bsp_MR20Lidar.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//MR20处理任务
uint8_t MR20Lidar_task(void);

//定义毫米波雷达目标对象
MR20Object_t mr20_Obj;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

	//启动串口6,接收毫米波雷达数据
	//接线: PC6 --- 毫米波雷达RX引脚
	//      PC7 --- 毫米波雷达TX引脚
	//      GND --- GND
	HAL_UARTEx_ReceiveToIdle_DMA(&huart6,mr20lidar_buffer,userconfig_MR20LidarDMA_LEN);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if( mr20_data_ready ) //1帧数据解析完成时,mr20_data_ready会置1
	  {
		  mr20_data_ready=0;//等待下一帧数据处理
		  
		  //处理1帧数据,完成一次数据处理时, MR20Lidar_task 返回 1
		  uint8_t ready = MR20Lidar_task();
		  
		  //数据处理完毕,打印距离
		  if( ready )
		  {
			  #if 1 //数据打印模式  设置0切换波形模式
			  printf("id:%d\r\n",mr20_Obj.id);
			  printf("x_distance:%.2f\r\n",mr20_Obj.X_Dis);
			  printf("y_distance:%.2f\r\n",mr20_Obj.Y_Dis);
			  printf("x_speed:   %.2f\r\n",mr20_Obj.X_Vel);
			  printf("y_speed:   %.2f\r\n",mr20_Obj.Y_Vel);
			  printf("obj state:");
			  if( mr20_Obj.DynProp==0 )
				  printf(" stop\r\n");
			  else if( mr20_Obj.DynProp==1 )
				  printf(" coming\r\n");
			  else if( mr20_Obj.DynProp==2 )
				  printf(" going\r\n");
			  else if( mr20_Obj.DynProp==3 )
				  printf(" crossing\r\n");
			  printf("\r\n");
			  
			  #else //波形模式,显示X和Y的距离
	
			  printf("{B%.2f:%.2f:%.2f:%.2f}$",mr20_Obj.X_Dis,mr20_Obj.Y_Dis,mr20_Obj.X_Vel,mr20_Obj.Y_Vel);
			  
			  #endif
		  }
	  }
	  else
	  {
		  if( MR20_Lidar_OnlineFlag==0 )
		  {
			  printf("未检测到雷达\r\n");
			  HAL_Delay(500);
		  }
	  }
	  
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

//筛选MR20最近的测量距离
uint8_t MR20Lidar_task(void)
{
	uint8_t ready = 0;
	static uint8_t MR20HandleSm = 0;
	static uint8_t PointNum = 0; //需要处理的点云数
	static uint8_t HandleNum = 0;//已处理的点云数量
	static float MinDistance = 1000;
	static uint8_t MinBuffer[8];
	
	//接收CAN数据变量
	CANmsgType_t msg;
	
	//读取数据
	memcpy(&msg,&mr20_can_msg,sizeof(CANmsgType_t));
	
	switch( MR20HandleSm )
	{
		case 0:
			if( msg.id == 0x60A ) 
			{
				PointNum = msg.buffer[0];         //本次需要处理的点云数据量
				MinDistance = 1000;
				if( PointNum!=0 ) MR20HandleSm=1,HandleNum=0; //仅有数据需要处理时切换状态
			}
			break;
		case 1:
			if( msg.id == 0x60B )
			{
				HandleNum++;
				
				//找出本次最小距离的点云
				float DistLong = (msg.buffer[1]*32 + (msg.buffer[2]>>3))*0.1f - 500;
				if( MinDistance > DistLong )
				{
					MinDistance = DistLong;
					memcpy(MinBuffer,msg.buffer,8);
				}
				
				//数据处理完毕
				if( HandleNum==PointNum ) 
				{
					//使用最小距离赋值
					MR20_ObjAnalysis(&mr20_Obj,MinBuffer); 
					
					//数据处理完成,恢复所有状态
					HandleNum=0;
					PointNum=0;
					MinDistance = 1000;
					MR20HandleSm=0;
					memset(MinBuffer,0,8);
					
					//处理完毕,返回1
					ready=1;
				}
			}

			break;
		default:
			break;
	}
	
	return ready;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

UART_HandleTypeDef *DebugSerial = &huart1;

//printf函数实现
int fputc(int ch,FILE* stream)
{
	while( HAL_OK != HAL_UART_Transmit(DebugSerial,(const uint8_t *)&ch,1,100));
	return ch;
}


//DMA传输完成与半完成中断入口函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if( huart == &huart6 && HAL_UART_STATE_READY == huart->RxState ) 
	{	
		//处理串口原始数据
		mr20lidar_RawDataHandle(Size);

		//重新启动DMA搬运
		HAL_UARTEx_ReceiveToIdle_DMA(&huart6,mr20lidar_buffer,200);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
