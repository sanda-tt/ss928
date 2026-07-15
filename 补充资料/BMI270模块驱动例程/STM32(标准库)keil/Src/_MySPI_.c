#include "_MySPI_.h"
#include "generic.h"//主要装Delay函数

#define My_SPI_Delay_us() ;

static void MySPI_GPIO_Config(){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    //MISO上拉
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //CS,SCK, MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    #if SPI_MODE == 0 || SPI_MODE == 1
        // Mode0/1：CPOL=0，SCK空闲低电平
        MYSPI_SCK_WRITE(0);
    #elif SPI_MODE == 2 || SPI_MODE == 3
        // Mode2/3：CPOL=1，SCK空闲高电平
        MYSPI_SCK_WRITE(1);
    #endif
    MYSPI_CS_WRITE(1);
}

void MySPI_Init(void){
    MySPI_GPIO_Config();//GPIO初始化
}

// 单字节收发
uint8_t My_SPI_ReadWriteByte(uint8_t data){
    uint8_t temp = 0;
    uint8_t i = 0;

    #if SPI_MODE == 0
        // CPOL=0、CPHA=0：空闲低，第1个边沿（上升沿）采样
        for(i = 0; i < 8; i++){
            // 1. 输出MOSI（高位先行）
            if(data & (0x80 >> i)){
                MYSPI_MOSI_WRITE(1);
            }else{
                MYSPI_MOSI_WRITE(0);
            }
            My_SPI_Delay_us();// 保持数据稳定

            // 2. SCK上升沿（第1个边沿），采样MISO
            MYSPI_SCK_WRITE(1);
            My_SPI_Delay_us();
            if(MYSPI_MISO_Read()){
                temp |= (0x80 >> i);
            }
            My_SPI_Delay_us();

            // 3. SCK拉回空闲低电平，准备下一位
            MYSPI_SCK_WRITE(0);
            My_SPI_Delay_us();
        }
    #elif SPI_MODE == 1
        // CPOL=0、CPHA=1：空闲低，第2个边沿（下降沿）采样
        for(i = 0; i < 8; i++){
            // 1. SCK先置高（有效电平）
            MYSPI_SCK_WRITE(1);
            My_SPI_Delay_us();

            // 2. 输出MOSI（高位先行）
            if(data & (0x80 >> i)){
                MYSPI_MOSI_WRITE(1);
            }else{
                MYSPI_MOSI_WRITE(0);
            }
            My_SPI_Delay_us();

            // 3. SCK下降沿（第2个边沿），采样MISO
            MYSPI_SCK_WRITE(0);
            My_SPI_Delay_us();
            if(MYSPI_MISO_Read()){
                temp |= (0x80 >> i);
            }
            My_SPI_Delay_us();
        }
    #elif SPI_MODE == 2
        // CPOL=1、CPHA=0：空闲高，第1个边沿（下降沿）采样
        for(i = 0; i < 8; i++){
            // 1. 输出MOSI（高位先行）
            if(data & (0x80 >> i)){
                MYSPI_MOSI_WRITE(1);
            }else{
                MYSPI_MOSI_WRITE(0);
            }
            My_SPI_Delay_us();

            // 2. SCK下降沿（第1个边沿），采样MISO
            MYSPI_SCK_WRITE(0);
            My_SPI_Delay_us();
            if(MYSPI_MISO_Read()){
                temp |= (0x80 >> i);
            }
            My_SPI_Delay_us();

            // 3. SCK拉回空闲高电平，准备下一位
            MYSPI_SCK_WRITE(1);
            My_SPI_Delay_us();
        }
    #elif SPI_MODE == 3
        // CPOL=1、CPHA=1：空闲高，第2个边沿（上升沿）采样
        for(i = 0; i < 8; i++){
            // 1. SCK先置低（有效电平）
            MYSPI_SCK_WRITE(0);
            My_SPI_Delay_us();

            // 2. 输出MOSI（高位先行）
            if(data & (0x80 >> i)){
                MYSPI_MOSI_WRITE(1);
            }else{
                MYSPI_MOSI_WRITE(0);
            }
            My_SPI_Delay_us();

            // 3. SCK上升沿（第2个边沿），采样MISO
            MYSPI_SCK_WRITE(1);
            My_SPI_Delay_us();
            if(MYSPI_MISO_Read()){
                temp |= (0x80 >> i);
            }
            My_SPI_Delay_us();
        }
    #else
        // 异常模式返回0，防止垃圾数据
        temp = 0;
    #endif

    return temp;
}


/************************* SPI寄存器操作函数 *************************/
/**
 * @brief  读取SPI从设备指定寄存器的值
 * @param  reg_addr: 8bit寄存器地址（待读取的寄存器地址）
 * @retval uint8_t:  读取到的寄存器对应数据
 */
uint8_t MySPI_ReadReg(uint8_t reg_addr)
{
    uint8_t reg_data = 0;
    
    MYSPI_CS_WRITE(0);
    
    // 注：若从设备要求地址带读标识（如最高位置1），需修改为 reg_addr | 0x80
    My_SPI_ReadWriteByte(reg_addr|0x80);
    
    reg_data = My_SPI_ReadWriteByte(0x00);
    
    MYSPI_CS_WRITE(1);
    
    return reg_data;
}

/**
 * @brief  向SPI从设备指定寄存器写入数据
 * @param  reg_addr: 8bit寄存器地址（待写入的寄存器地址）
 * @param  write_data: 8bit待写入的数据（要写入寄存器的内容）
 * @retval 无
 */
void MySPI_WriteReg(uint8_t reg_addr, uint8_t write_data)
{
    MYSPI_CS_WRITE(0);
    
    // 注：若从设备要求地址带写标识（如最高位置0），需修改为 reg_addr & 0x7F
    My_SPI_ReadWriteByte(reg_addr & 0x7F);
    
    My_SPI_ReadWriteByte(write_data);
    
    MYSPI_CS_WRITE(1);
}

void MySPI_WriteReg_Continue(uint8_t start_reg_addr, uint32_t write_len, const uint8_t *p_write_buf)
{
    // 1. 启动SPI通信（拉低CS，选中从设备）
    MYSPI_CS_WRITE(0);
    
    // 2. 发送起始寄存器地址（带写标识：最高位清0，与单个写寄存器逻辑一致）
    My_SPI_ReadWriteByte(start_reg_addr & 0x7F);
    
    // 3. 循环发送多字节数据，对应连续的寄存器
    for (uint16_t i = 0; i < write_len; i++)
    {
        My_SPI_ReadWriteByte(p_write_buf[i]);
        My_SPI_Delay_us(); // 延时保证数据传输稳定，兼容低速从设备
    }
    
    // 4. 结束SPI通信（拉高CS，释放从设备）
    MYSPI_CS_WRITE(1);
}


void MySPI_ReadReg_Continue(uint8_t start_reg_addr, uint32_t read_len,uint8_t *p_read_buf)
{
    // 1. 启动SPI通信（拉低CS，选中从设备）
    MYSPI_CS_WRITE(0);
    
    // 2. 发送起始寄存器地址（带读标识：最高位置1，与单个读寄存器逻辑一致）
    My_SPI_ReadWriteByte(start_reg_addr | 0x80);
    
    // 3. 循环接收多字节数据，存入缓冲区，对应连续的寄存器
    for (uint16_t i = 0; i < read_len; i++)
    {
        p_read_buf[i] = My_SPI_ReadWriteByte(0x00); // 发送空字节以产生SPI时钟，接收从设备数据
        My_SPI_Delay_us(); // 延时保证数据传输稳定，兼容低速从设备
    }
    
    // 4. 结束SPI通信（拉高CS，释放从设备）
    MYSPI_CS_WRITE(1);
}
