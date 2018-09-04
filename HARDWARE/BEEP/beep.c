#include "beep.h" 
#include "delay.h"

/*
初始化PA1为输出口		    
BEEP IO初始化
*/
void BEEP_Init(void)
{   
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  //下拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);            //初始化GPIO
	
  GPIO_ResetBits(GPIOA,GPIO_Pin_1);                //蜂鸣器对应引脚GPIOA1
}

//响一声
void BeepOne(void)
{
	BEEP_ON;
	delay_ms(100);
	BEEP_OFF;
}

//响两声
void BeepTwo(void)
{
	BeepOne(); 
	delay_ms(500);
	BeepOne(); 
}

//响两长一短
void BeepThree(void)
{
	BEEP_ON;
	delay_ms(1000);
	BEEP_OFF; 
	delay_ms(300);
	BEEP_ON; 
	delay_ms(300);
	BEEP_OFF; 
	delay_ms(300);
	BEEP_ON; 
	delay_ms(1000);
	BEEP_OFF;  
}



