#include "sys.h"
#include "delay.h"
#include "usart2.h"
#include "usart6.h"
#include "led.h"
#include "beep.h"
#include "timer3.h"
#include "rtc.h"
#include "adc.h"
#include "PlanTask.h"
#include "init_Parameter.h"


int main(void)
{
	u8 oldLen = 0;
	uint8_t cmd = 0xff;
	uint8_t * dataPtr = (void *)0;
	DEVICE device = wifi;
	
	DebugFlag = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2


	My_RTC_Init();				//初始化RTC
	delay_init(168);			//延时初始化 
	wifiInit();
	loraInit();
	Adc_Init();
	LED_Init();		  			//初始化与LED连接的硬件接口
	BEEP_Init();					//蜂鸣器初始化
	TIM3_Int_Init(10 - 1, 8400 - 1);//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数10次为1ms

	init_Parameter();

	BreakFlag = 0;
	WorkStatus = 0;
	BeepOne();

	while (1)
	{
		//------------------接收完成数据处理
		if (getReciveLen(device) != 0 && oldLen == getReciveLen(device))
		{
			dataPtr = getCmdFrame(device, &cmd);
			handlerFrame(device, cmd, dataPtr);
			oldLen = 0;
			clearReciveBuf(device);
		}
		else
		{
			oldLen = getReciveLen(device);
		}
		if (Scan_SW)
		{
			PlanTask();
		}
		delay_ms(12);//不可修改

	}
}

