#include "sys.h"
#include "delay.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "uart5.h"
#include "usart6.h"
#include "led.h"
#include "beep.h"
#include "timer3.h"
#include "main.h"
#include "rtc.h"
#include "adc.h"
#include "design.h"

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

int main(void)
{
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

	Usart2_rcv_flag = 0;		//串口2接收到数据
	Usart2_receive_timer = 0;
	Usart2_receive_on = 0;

	Uart5_rcv_flag = 0;			//串口5接收到数据
	Uart5_receive_timer = 0;	//串口5接收超时定时器
	Uart5_receive_on = 0;		//串口5接收定时开关

	BreakFlag = 0;
	WorkStatus = 0;
	BeepOne();

	//Test();
	while (1)
	{
		//1-处理串口WIFI通信
		if (Usart2_rcv_flag)
		{
			WorkStatus = 1;
			Usart2_rcv_flag = 0;
			WIFI_Dealwith(2);
			WorkStatus = 0;
		}

		//2-处理串口以太网通信
		if (Uart5_rcv_flag)
		{
			WorkStatus = 1;
			Uart5_rcv_flag = 0;
			WIFI_Dealwith(5);
			WorkStatus = 0;
		}

		//3-执行计划任务，如果时间满足，触发区域扫描开关
		if (Scan_SW)
			PlanTask();

	}
}

