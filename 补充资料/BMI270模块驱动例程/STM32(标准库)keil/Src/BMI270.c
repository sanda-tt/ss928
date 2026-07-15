#include "BMI270.h"
#include "math.h"
#include "generic.h"//主要装Delay
#include "_MySPI_.h"
#include "_MyI2C_.h"
#include "BMI270_config.h"

i2cbus_struct bmi270_iic_bus;
// #define BMI270_WRITE_REG(reg,data) MYI2C_Write_Reg(&bmi270_iic_bus,reg,data)
// #define BMI270_READ_REG(reg) MYI2C_Read_Reg(&bmi270_iic_bus,reg)

uint8_t bmi270_read_reg(uint8_t reg){
    uint8_t dat[2];
    MySPI_ReadReg_Continue(reg, 2, dat);
    return dat[1];
}

/////////////////////////以下接口为方便移植/////////////////////////
#define BMI270_WRITE_REG(reg,data) MySPI_WriteReg(reg,data)
#define BMI270_READ_REG(reg) bmi270_read_reg(reg)
#define BMI270_READ_REG_CONTINUE(reg,len,buf) MySPI_ReadReg_Continue(reg,len,buf)
#define BMI270_WRITE_REG_CONTINUE(reg,len,buf) MySPI_WriteReg_Continue(reg,len,buf)
#define BMI270_DELAY_MS(x) Delay_ms(x)
//////////////////////////以下接口为方便移植///////////////////////

#define BMI270_CHIP_ID 0x00//ID

#define BMI270_ACC_X_LSB 0x0C
#define BMI270_ACC_X_MSB 0x0D
#define BMI270_ACC_Y_LSB 0x0E
#define BMI270_ACC_Y_MSB 0x0F
#define BMI270_ACC_Z_LSB 0x10
#define BMI270_ACC_Z_MSB 0x11
#define BMI270_GYRO_X_LSB 0x12
#define BMI270_GYRO_X_MSB 0x13
#define BMI270_GYRO_Y_LSB 0x14
#define BMI270_GYRO_Y_MSB 0x15
#define BMI270_GYRO_Z_LSB 0x16
#define BMI270_GYRO_Z_MSB 0x17
#define BMI270_INTERNAL_STATUS 0x21
#define BMI270_TEMP_LSB 0x22
#define BMI270_TEMP_MSB 0x23

#define BMI270_ACC_CONF 0x40
#define BMI270_ACC_RANGE 0x41//加速度计量程
#define BMI270_GYR_CONF 0x42
#define BMI270_GYR_RANGE 0x43//陀螺仪量程

#define BMI270_INIT_CTRL 0x59//初始化控制
#define BMI270_INIT_DATA 0x5E//初始化数据
#define BMI270_IF_CONF 0x6B//接口配置
#define BMI270_NV_CONF 0x70//是否是spi模式
#define BMI270_PWR_CONF 0x7C
#define BMI270_CMD 0x7D //配置寄存器

typedef enum{//加速度计量程
    ACC_RANGE_2G,
    ACC_RANGE_4G,  
    ACC_RANGE_8G,  
    ACC_RANGE_16G,  
}BMI270_ACC_RANGE_TypeDef;

typedef enum{//陀螺仪量程
    GYRO_RANGE_2000,
    GYRO_RANGE_1000,  
    GYRO_RANGE_500,  
    GYRO_RANGE_250,  
    GYRO_RANGE_125,  
}BMI270_GRY_RANGE_TypeDef;

typedef enum{//加速度计采样率
    ACC_RATE_25Hz = 0x06,  
    ACC_RATE_50Hz = 0x07,  
    ACC_RATE_100Hz = 0x08,  
    ACC_RATE_200Hz = 0x09,  
    ACC_RATE_400Hz = 0x0A,
    ACC_RATE_800Hz = 0x0B,
    ACC_RATE_1600Hz = 0x0C,
}BMI270_ACC_CONF_TypeDef;

typedef enum{//陀螺仪采样率
    GYRO_RATE_25Hz = 0x06,  
    GYRO_RATE_50Hz = 0x07,  
    GYRO_RATE_100Hz = 0x08,  
    GYRO_RATE_200Hz = 0x09,  
    GYRO_RATE_400Hz = 0x0A,
    GYRO_RATE_800Hz = 0x0B,
    GYRO_RATE_1600Hz = 0x0C,
    GYRO_RATE_3200Hz = 0x0D,
}BMI270_GRY_CNOF_TypeDef;

typedef struct
{
    uint8_t en_spi_mode;
    BMI270_GRY_RANGE_TypeDef gyro_range;//陀螺仪量程
    BMI270_GRY_CNOF_TypeDef gyro_rate;//陀螺仪采样率
    BMI270_ACC_RANGE_TypeDef acc_range;//加速度计量程
    BMI270_ACC_CONF_TypeDef acc_rate;//加速度计采样率
    uint8_t en_acc_filter_3dB;//加速度计3dB自动滤波
    uint8_t en_gyro_filter_3dB;//加速度计3dB自动滤波
}BMI270_Init_Typedef;

///////////////////////////////////////////////////////////////////////////////////
static float bmi270_gyroScale;  // 角速度量程缩放因子
static float bmi270_accelScale; // 加速度计量程缩放因子

//return 0:初始化成功 -1:初始化失败
static int8_t BMI270_register_init(BMI270_Init_Typedef* _this){
    float temp_acc_scale;  // 比例因子(加速度)
    float temp_groy_scale; // 比例因子(角速度)
    //计算刻度缩放因子
    switch (_this->acc_range) {
        case ACC_RANGE_2G:
            temp_acc_scale = 16384;
            break;
        case ACC_RANGE_4G:
            temp_acc_scale = 8192;
            break;
        case ACC_RANGE_8G:
            temp_acc_scale = 4096;
            break;
        case ACC_RANGE_16G:
            temp_acc_scale = 2048;
            break;
    }
    switch (_this->gyro_range) {
        case GYRO_RANGE_125:
            temp_groy_scale = 262.4;
            break;
        case GYRO_RANGE_250:
            temp_groy_scale = 131.2;
            break;
        case GYRO_RANGE_500:
            temp_groy_scale = 65.6;
            break;
        case GYRO_RANGE_1000:
            temp_groy_scale = 32.8;
            break;
        case GYRO_RANGE_2000:
            temp_groy_scale = 16.4;
            break;
    }
    bmi270_accelScale = 1.f / temp_acc_scale;
    bmi270_gyroScale  = 0.0174533f / temp_groy_scale;
    BMI270_DELAY_MS(100);
    BMI270_WRITE_REG(BMI270_PWR_CONF,0x00);//关闭省电模式
    BMI270_DELAY_MS(2);

    BMI270_WRITE_REG(BMI270_INIT_CTRL,0x00);//开始初始化控制
    BMI270_WRITE_REG_CONTINUE(BMI270_INIT_DATA,sizeof(bmi270_config_file),bmi270_config_file);//写入初始化数据
    BMI270_WRITE_REG(BMI270_INIT_CTRL,0x01);//结束初始化控制
    BMI270_DELAY_MS(40);

    //配置文件不生效，可能传感器损坏
    if(!BMI270_READ_REG(BMI270_INTERNAL_STATUS))
    {
        return -1;
    }
    BMI270_WRITE_REG(BMI270_CMD,0x0e);//打开加速度计，陀螺仪，温度计
    
    BMI270_WRITE_REG(BMI270_NV_CONF,_this->en_spi_mode);//写入是否是spi模式
    BMI270_WRITE_REG(BMI270_IF_CONF,0x00);//使用四线SPI

    BMI270_WRITE_REG(BMI270_GYR_RANGE,_this->gyro_range);//写入陀螺仪量程
    if(_this->en_gyro_filter_3dB){
        BMI270_WRITE_REG(BMI270_GYR_CONF,_this->gyro_rate|0x20|0x80);//写入陀螺仪采样率
    }else{
        BMI270_WRITE_REG(BMI270_GYR_CONF,_this->gyro_rate);//写入陀螺仪采样率
    }

    BMI270_WRITE_REG(BMI270_ACC_RANGE,_this->acc_range);//写入加速度计量程
    if(_this->en_acc_filter_3dB){
        BMI270_WRITE_REG(BMI270_ACC_CONF,_this->acc_rate|0x20|0x80);//打开滤波器自动滤波
    }else{
        BMI270_WRITE_REG(BMI270_ACC_CONF,_this->acc_rate);//写入加速度计采样率
    }
		BMI270_DELAY_MS(10);
		
		//ID读出错误
		if(BMI270_Get_ID() == 0xFF){
			return -1;
		}
    return 0;
}

int8_t BMI270_Init(BMI270 *_this){
    _this->q0 = 1.0f;
	_this->q1 = 0.0f;
	_this->q2 = 0.0f;
	_this->q3 = 0.0f;
    MySPI_Init();//初始化我自己的软件SPI。
    // MyI2C_Init(&bmi270_iic_bus,
    // GPIOB, GPIO_Pin_6,       //scl引脚配置
    // GPIOB, GPIO_Pin_7,       //sda引脚配置
    // BMI_270_IIC_ADDR, 5);

    BMI270_Init_Typedef BMI270_init_struture;
    BMI270_init_struture.en_spi_mode = 1;//是否使用SPI模式
    BMI270_init_struture.gyro_range = GYRO_RANGE_2000;//陀螺仪量程
    BMI270_init_struture.gyro_rate = GYRO_RATE_200Hz;//陀螺仪采样率
    BMI270_init_struture.acc_range = ACC_RANGE_16G;//加速度计量程
    BMI270_init_struture.acc_rate = ACC_RATE_200Hz;//加速度计采样率
    BMI270_init_struture.en_acc_filter_3dB = 1;//加速度计3dB滤波器
    BMI270_init_struture.en_gyro_filter_3dB = 1;//陀螺仪3dB滤波器
    return BMI270_register_init(&BMI270_init_struture);//初始化BMI270的寄存器
}

float BMI270_get_real_temp(BMI270 *_this){
    BMI270_Get_RawData(_this);
    _this->temp = 23.0f + (float)_this->rawTemp/512.0f;
    return  _this->temp;
}


const float bmi270_dt = 0.005f;
#define ICM20602_INTEGRAL_LIMIT  0.5f
// 快速平方根倒数算法
static inline float invSqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));  // 一次牛顿迭代
    return y;
}
static float constrain(float value, float min_val, float max_val) {
    if (value < min_val) return min_val;
    else if (value > max_val) return max_val;
    else return value;
}
void BMI270_Get_Angle_Plus(BMI270* _this) {
    static float Kp, Ki;
    static float integralX = 0.0f, integralY = 0.0f, integralZ = 0.0f;

    // 读取原始数据
    BMI270_Get_RawData(_this);

    // 转换为物理量
    float ax = (float)_this->acc_x * bmi270_accelScale;
    float ay = (float)_this->acc_y * bmi270_accelScale;
    float az = (float)_this->acc_z * bmi270_accelScale;
    float gx = (float)_this->gyro_x * bmi270_gyroScale;
    float gy = (float)_this->gyro_y * bmi270_gyroScale;
    float gz = (float)_this->gyro_z * bmi270_gyroScale;

	// 加速度幅值（用于自适应增益）
	float accMag = ax * ax + ay * ay + az * az;
    //值越大越相信加速度计
	Kp = 1.2f;
	Ki = 0.01f;
//	Kp = 0.1f;
//	Ki = 0.00001f;
	// 加速度计有效时校正姿态
	if (accMag > 0.01f) {
        float recipNorm = invSqrt(accMag);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // 计算重力方向误差
        float vx = 2.0f * (_this->q1 * _this->q3 - _this->q0 * _this->q2);
        float vy = 2.0f * (_this->q0 * _this->q1 + _this->q2 * _this->q3);
        float vz = _this->q0 * _this->q0 - _this->q1 * _this->q1 - _this->q2 * _this->q2 + _this->q3 * _this->q3;

        float ex = ay * vz - az * vy;
        float ey = az * vx - ax * vz;
        float ez = ax * vy - ay * vx;

        // 积分项校正
        if (Ki > 0.0f) {
            integralX += ex * bmi270_dt;
            integralY += ey * bmi270_dt;
            integralZ += ez * bmi270_dt;
            //积分限幅
            integralX = constrain(integralX, -ICM20602_INTEGRAL_LIMIT, ICM20602_INTEGRAL_LIMIT);
            integralY = constrain(integralY, -ICM20602_INTEGRAL_LIMIT, ICM20602_INTEGRAL_LIMIT);
            integralZ = constrain(integralZ, -ICM20602_INTEGRAL_LIMIT, ICM20602_INTEGRAL_LIMIT);

            gx += Ki * integralX;
            gy += Ki * integralY;
            gz += Ki * integralZ;
        }

        // 比例项校正
        gx += Kp * ex;
        gy += Kp * ey;
        gz += Kp * ez;
    }

    // 四元数更新（核心计算）
    float qDot0 = 0.5f * (-_this->q1 * gx - _this->q2 * gy - _this->q3 * gz);
    float qDot1 = 0.5f * (_this->q0 * gx + _this->q2 * gz - _this->q3 * gy);
    float qDot2 = 0.5f * (_this->q0 * gy - _this->q1 * gz + _this->q3 * gx);
    float qDot3 = 0.5f * (_this->q0 * gz + _this->q1 * gy - _this->q2 * gx);

    _this->q0 += qDot0 * bmi270_dt;
    _this->q1 += qDot1 * bmi270_dt;
    _this->q2 += qDot2 * bmi270_dt;
    _this->q3 += qDot3 * bmi270_dt;

    // 四元数归一化
    float norm = invSqrt(_this->q0 * _this->q0 + _this->q1 * _this->q1 + _this->q2 * _this->q2 + _this->q3 * _this->q3);
    _this->q0 *= norm;
    _this->q1 *= norm;
    _this->q2 *= norm;
    _this->q3 *= norm;

    // 计算欧拉角（四元数转欧拉角核心）
    _this->roll        = atan2f(2.0f * (_this->q0 * _this->q1 + _this->q2 * _this->q3), 1.0f - 2.0f * (_this->q1 * _this->q1 + _this->q2 * _this->q2)) * 57.29578f;
    _this->pitch       = asinf(2.0f * (_this->q0 * _this->q2 - _this->q3 * _this->q1)) * 57.29578f;
    float current_yaw = atan2f(2.0f * (_this->q0 * _this->q3 + _this->q1 * _this->q2), 1.0f - 2.0f * (_this->q2 * _this->q2 + _this->q3 * _this->q3)) * 57.29578f;
	
    //_this->yaw = current_yaw;
    //角度无限叠加(如果不需要则注释下面这里然后打开上面这个注释,这样就是0~180)
   static float unwrapped_yaw = 0.0f;
   static float last_yaw = 0.0f;
   static uint8_t first_run   = 1;
   if (first_run) {
       unwrapped_yaw = current_yaw;
       last_yaw = current_yaw;
       first_run     = 0;
   } else {
       float diff = current_yaw - last_yaw;

       if (diff > 180.0f) {
           diff -= 360.0f;
       } else if (diff < -180.0f) {
           diff += 360.0f;
       }
       unwrapped_yaw += diff;
       last_yaw = current_yaw;
   }

   _this->yaw = unwrapped_yaw; // 输出连续yaw角
}


//获取原始数据
void BMI270_Get_RawData(BMI270 *_this){
    uint8_t dat[7];
    BMI270_READ_REG_CONTINUE(BMI270_ACC_X_LSB, 7, dat);
    _this->acc_x = (int16_t)(((uint16_t)dat[2] << 8 | dat[1]));
    _this->acc_y = (int16_t)(((uint16_t)dat[4] << 8 | dat[3]));
    _this->acc_z = (int16_t)(((uint16_t)dat[6] << 8 | dat[5]));

    uint8_t dat2[7];
    BMI270_READ_REG_CONTINUE(BMI270_GYRO_X_LSB, 7, dat2);
    _this->gyro_x = (int16_t)(((uint16_t)dat2[2] << 8 | dat2[1]));
    _this->gyro_y = (int16_t)(((uint16_t)dat2[4] << 8 | dat2[3]));
    _this->gyro_z = (int16_t)(((uint16_t)dat2[6] << 8 | dat2[5]));
}

void BMI270_Get_Raw_temp(BMI270 *_this){
    uint8_t data[3];
    BMI270_READ_REG_CONTINUE(BMI270_TEMP_LSB,3,data);
    _this->rawTemp = (int16_t)(((uint16_t)data[2] << 8 | data[1]));
}

uint8_t BMI270_Get_ID(){
    return BMI270_READ_REG(BMI270_CHIP_ID);
}
