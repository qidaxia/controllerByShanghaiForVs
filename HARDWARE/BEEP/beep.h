#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h" 

//BEEP端口定义,PA13

#define BEEP_ON     GPIO_SetBits(GPIOA,GPIO_Pin_1); 
#define BEEP_OFF    GPIO_ResetBits(GPIOA,GPIO_Pin_1); 
void BEEP_Init(void);//初始化
void BeepOne(void);//响一声
void BeepTwo(void);//响两声
void BeepThree(void);//响两长一短
#endif

















