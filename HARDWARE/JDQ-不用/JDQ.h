#ifndef __JDQ_H
#define __JDQ_H	 
#include "sys.h" 

/*
低电平继电器动作
继电器1输出：PE10，大梁前进
继电器2输出：PE8 ，大梁后退
继电器3输出：PE14，小车前进
继电器4输出：PE12，小车后退
继电器5输出：PC6， 小车松闸，关闭继电器抱死
继电器6输出：PG8， 输出5V的脉冲信号
*/

#define JDQ1_ON    GPIO_ResetBits(GPIOE,GPIO_Pin_10); 
#define JDQ1_OFF   GPIO_SetBits(GPIOE,GPIO_Pin_10); 

#define JDQ2_ON    GPIO_ResetBits(GPIOE,GPIO_Pin_8); 
#define JDQ2_OFF   GPIO_SetBits(GPIOE,GPIO_Pin_8); 

#define JDQ3_ON    GPIO_ResetBits(GPIOE,GPIO_Pin_14); 
#define JDQ3_OFF   GPIO_SetBits(GPIOE,GPIO_Pin_14); 

#define JDQ4_ON    GPIO_ResetBits(GPIOE,GPIO_Pin_12); 
#define JDQ4_OFF   GPIO_SetBits(GPIOE,GPIO_Pin_12); 

#define JDQ5_ON   GPIO_SetBits(GPIOC,GPIO_Pin_6); 
#define JDQ5_OFF  GPIO_ResetBits(GPIOC,GPIO_Pin_6); 

#define JDQ6_ON   GPIO_SetBits(GPIOG,GPIO_Pin_8); 
#define JDQ6_OFF  GPIO_ResetBits(GPIOG,GPIO_Pin_8); 

void JDQ_Init(void);	//初始化
		 				    
#endif

















