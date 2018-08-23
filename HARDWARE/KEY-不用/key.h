#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 

/*下面的方式是通过直接操作库函数方式读取IO*/
#define KEY3 		GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_7) //PG7
#define KEY4 		GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_6) //PG6
#define KEY5 		GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_5) //PG5
#define KEY6 		GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_4) //PG4

#define KEY3_PRES	3
#define KEY4_PRES	4
#define KEY5_PRES	5
#define KEY6_PRES	6

void KEY_Init(void);	//IO初始化
u8 KEY_Scan(u8);  		//按键扫描函数	

#endif
