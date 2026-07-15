#ifndef __EXTEND_HEADFIEL__MYSPI__H_
#define __EXTEND_HEADFIEL__MYSPI__H_
#include "main.h"
//这里是软件spi,用来检测spi模式是否正确

#define SPI_MODE    3//SPI模式选择
//4线SPI配置
#define MYSPI_CS_WRITE(x)  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,x)
#define MYSPI_SCK_WRITE(x) HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,x)
#define MYSPI_MOSI_WRITE(x) HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,x)
#define MYSPI_MISO_Read()  HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)

///////////////应用函数///////////////
void MySPI_Init(void); //初始化
uint8_t MySPI_ReadReg(uint8_t reg_addr);//里面已经reg_addr | 0x80
void MySPI_WriteReg(uint8_t reg_addr, uint8_t write_data);//里面已经reg_addr & 0x7F

void MySPI_WriteReg_Continue(uint8_t start_reg_addr, uint32_t write_len,const uint8_t *p_write_buf);//连续写寄存器
void MySPI_ReadReg_Continue(uint8_t start_reg_addr, uint32_t read_len, uint8_t *p_read_buf);//连续读寄存器

///////////////底层函数///////////////
uint8_t My_SPI_ReadWriteByte(uint8_t data);


#endif
