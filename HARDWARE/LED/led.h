#ifndef __LED_H
#define __LED_H
#include "sys.h"

//LED端口定义
#define LED1_ON 	GPIO_ResetBits(GPIOE,GPIO_Pin_8);	//PE8设置低，黄灯亮
#define LED1_OFF 	GPIO_SetBits(GPIOE,GPIO_Pin_8);		//PE8设置高，黄灯灭
#define LED1_BLINK	GPIO_ToggleBits(GPIOE,GPIO_Pin_8);//LED1黄灯闪烁

#define LED2_ON 	GPIO_ResetBits(GPIOE,GPIO_Pin_9);	//PE9设置低，蓝灯亮
#define LED2_OFF 	GPIO_SetBits(GPIOE,GPIO_Pin_9);		//PE9设置高，蓝灯灭
#define LED2_BLINK	GPIO_ToggleBits(GPIOE,GPIO_Pin_9);//LED2蓝灯闪烁

void LED_Init(void);//初始化	

#endif
