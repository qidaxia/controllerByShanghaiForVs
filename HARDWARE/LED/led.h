#ifndef __LED_H
#define __LED_H
#include "sys.h"

//LED∂Àø⁄∂®“Â
#define LED1_ON 	GPIO_ResetBits(GPIOE,GPIO_Pin_8);	//PE8…Ë÷√µÕ£¨ª∆µ∆¡¡
#define LED1_OFF 	GPIO_SetBits(GPIOE,GPIO_Pin_8);		//PE8…Ë÷√∏ﬂ£¨ª∆µ∆√
#define LED1_BLINK	GPIO_ToggleBits(GPIOE,GPIO_Pin_8);//LED1ª∆µ∆…¡À∏

#define LED2_ON 	GPIO_ResetBits(GPIOE,GPIO_Pin_9);	//PE9…Ë÷√µÕ£¨¿∂µ∆¡¡
#define LED2_OFF 	GPIO_SetBits(GPIOE,GPIO_Pin_9);		//PE9…Ë÷√∏ﬂ£¨¿∂µ∆√
#define LED2_BLINK	GPIO_ToggleBits(GPIOE,GPIO_Pin_9);//LED2¿∂µ∆…¡À∏

void LED_Init(void);//≥ı ºªØ	

#endif
