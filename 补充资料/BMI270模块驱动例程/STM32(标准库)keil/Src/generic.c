#include "stm32f10x.h"
#include "generic.h"

#define FreeRTOS_SysTick 0  //���û��FreeRTOS �������ó�0

void Delay_100ns(uint32_t n) {
    TIM_SetCounter(TIM2, 0);//��ʼ��ʱ
    while (n > 0) {
        while (TIM_GetCounter(TIM2) < 1);//��ʱ����
        TIM_SetCounter(TIM2, 0);//���¹���
        n--;
    }
}

void Delay_us(uint32_t xus)
{
    #if FreeRTOS_SysTick
        static uint8_t TIM_init_falg=0;
        if(xus == 0)return;
        if(TIM_init_falg==0){//����1ms��ʱ��,set 1us Timer
            //ʹ�ö�ʱ��2������freeRTOS������ʽ��ʱ
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
            TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
            TIM_TimeBaseStructure.TIM_Period = 65535;
            TIM_TimeBaseStructure.TIM_Prescaler = 72 -1;
            TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
            TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
            TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
            TIM_Cmd(TIM2, ENABLE);
            TIM_init_falg = 1;
        }else{
            TIM_SetCounter(TIM2,0);
            while (TIM_GetCounter(TIM2) <= xus);
        }
    #else
        if(xus==0)return;
        SysTick->LOAD = 72 * xus;				//���ö�ʱ����װֵ
        SysTick->VAL = 0x00;					//��յ�ǰ�����?
        SysTick->CTRL = 0x00000005;				//����ʱ��ԴΪHCLK��������ʱ��
        while(!(SysTick->CTRL & 0x00010000));	//�ȴ�������0
        SysTick->CTRL = 0x00000004;				//�رն�ʱ��
    #endif
}


void Delay_ms(uint32_t xms)
{
    if(xms<=0)return;
    while(xms--)
         Delay_us(1000);
}
 
void Delay_s(uint32_t xs)
{
	while(xs--)
		Delay_ms(1000);
} 

/*
GPIOx:choose your GPIO A~G
GPIO_Mode: @ref GPIOMode_TypeDef
GPIO_Speed: @ref GPIOSpeed_TypeDef
PINx:GPIO_pin_x    x(0~15)
*/
void Pin_init(GPIO_TypeDef* GPIOx,uint32_t GPIO_Pin_x,GPIOMode_TypeDef GPIO_MODE){
    GPIO_InitTypeDef GPIOInitStruct;
    GPIOInitStruct.GPIO_Mode=GPIO_MODE;
    GPIOInitStruct.GPIO_Speed=GPIO_Speed_50MHz;
    GPIOInitStruct.GPIO_Pin=GPIO_Pin_x;
 
    //open clock
    if(GPIOx==GPIOA)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    else if(GPIOx==GPIOB)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    else if(GPIOx==GPIOC)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    else if(GPIOx==GPIOD)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
    else if(GPIOx==GPIOE)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
    else if(GPIOx==GPIOF)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
    else if(GPIOx==GPIOG)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
   
    GPIO_Init(GPIOx,&GPIOInitStruct);
}

/*
Pin:choose your pin 0~15
Time:set your delay time /ms
*/
void PIN_Toggle(GPIO_TypeDef* GPIOx,uint32_t PINx){
    uint16_t currentState = GPIOx->IDR & PINx;
    if (currentState == PINx)
       GPIOx->ODR &= ~PINx;
    else
       GPIOx->ODR |= PINx;
}

