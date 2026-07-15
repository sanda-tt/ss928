#ifndef __EXTEND_HEADFIEL__MYSPI__H_
#define __EXTEND_HEADFIEL__MYSPI__H_
#include "stm32f10x.h"
//这里是软件spi,用来检测spi模式是否正确

#define SPI_MODE    2//SPI模式选择
//4线SPI配置
#define MYSPI_CS_WRITE(x)  GPIO_WriteBit(GPIOA,GPIO_Pin_4,(BitAction)x)
#define MYSPI_SCK_WRITE(x) GPIO_WriteBit(GPIOA,GPIO_Pin_5,(BitAction)x)
#define MYSPI_MOSI_WRITE(x) GPIO_WriteBit(GPIOA,GPIO_Pin_7,(BitAction)x)
#define MYSPI_MISO_Read()  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)

///////////////应用函数///////////////
void MySPI_Init(void); //初始化
uint8_t MySPI_ReadReg(uint8_t reg_addr);//里面已经reg_addr | 0x80
void MySPI_WriteReg(uint8_t reg_addr, uint8_t write_data);//里面已经reg_addr & 0x7F

void MySPI_WriteReg_Continue(uint8_t start_reg_addr, uint32_t write_len,const uint8_t *p_write_buf);//连续写寄存器
void MySPI_ReadReg_Continue(uint8_t start_reg_addr, uint32_t read_len, uint8_t *p_read_buf);//连续读寄存器

///////////////底层函数///////////////
uint8_t My_SPI_ReadWriteByte(uint8_t data);


#endif
