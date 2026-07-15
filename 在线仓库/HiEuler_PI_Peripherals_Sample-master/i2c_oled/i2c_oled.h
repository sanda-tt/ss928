#ifndef __I2C_OLED_H__
#define __I2C_OLED_H__

#include <linux/ioctl.h>

#define OLED_MAGIC 'G'

struct oled_frame
{
    unsigned char data[128*8];
};

#define OLED_CMD_FRAME _IOW(OLED_MAGIC,0,struct oled_frame)
#define OLED_CMD_CLEAR _IOW(OLED_MAGIC,1,unsigned char)
#define OLED_CMD_POWER _IOW(OLED_MAGIC,2,unsigned char)



#endif // !__I2C_OLED_H__
