#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"
#include "JDQ.h"

/*
KEY1 -- PC6
KEY2 -- PG8
KEY3 -- PG7		小车后退中断，对应JDQ4
KEY4 -- PG6		小车前进中断，对应JDQ3
KEY5 -- PG5		大梁后退中断，对应JDQ2
KEY6 -- PG4		大梁前进中断，对应JDQ1
*/

//外部中断4服务程序
void EXTI4_IRQHandler(void)
{
	delay_ms(10);	//消抖
	if(KEY6 == 0)
	{
		JDQ1_OFF; 	//大梁前进继电器关闭 
	}		 
	 EXTI_ClearITPendingBit(EXTI_Line4); //清除LINE4上的中断标志位 
}	
//外部中断2服务程序
void EXTI9_5_IRQHandler(void)
{
	delay_ms(10);	//消抖
	
	if(KEY5 == 0)
	{
		JDQ2_OFF; 	//大梁前进继电器关闭 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line5); //清除LINE5上的中断标志位 	
	
	if(KEY4 == 0)
	{
		JDQ3_OFF; 	//大梁前进继电器关闭 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line6); //清除LINE6上的中断标志位 		
	
	if(KEY3 == 0)
	{
		JDQ4_OFF; 	//大梁前进继电器关闭 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line7); //清除LINE7上的中断标志位 	
}

	   
/*
外部中断初始化程序
*/
void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	KEY_Init(); //按键对应的IO口初始化
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource4);//PG4 连接到中断线4
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource5);//PG5 连接到中断线5
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource6);//PG6 连接到中断线6
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource7);//PG7 连接到中断线7
	
	/* 配置EXTI_Line0 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line4 | EXTI_Line5 | EXTI_Line6 | EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE
	EXTI_Init(&EXTI_InitStructure);//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//外部中断4
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//外部中断2
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置
	   
}
