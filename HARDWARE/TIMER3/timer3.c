/*
Timer3作为系统定时器，定时精度1ms
*/
#include "timer3.h"
#include "usart2.h"
#include "uart5.h"
#include "led.h"
#include "init_Parameter.h"

uint16_t ledCount = 0;			//闪烁二极管定时器
uint16_t Laser1_send_timer;		//激光模块1发送命令定时器
uint16_t Laser2_send_timer;		//激光模块2发送命令定时器
uint16_t Laser1_receive_timer;	//激光模块1接收命令定时器
uint16_t Laser2_receive_timer;	//激光模块2接收命令定时器

uint16_t Uart5_receive_timer;	//串口5接收超时定时器
uint8_t  Uart5_receive_on;		//串口5接收定时开关

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);             ///使能TIM3时钟

	TIM_TimeBaseInitStructure.TIM_Period = arr; 	                //自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;                    //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);              //初始化TIM3

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                        //允许定时器3更新中断
	TIM_Cmd(TIM3, ENABLE); //使能定时器3

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                   //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;      //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;             //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) //溢出中断
	{
		//1-呼吸灯闪烁
		if (++ledCount >= 500)	//500ms闪烁
		{
			ledCount = 0;
			LED1_BLINK;	//LED1黄灯闪烁
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

			//如果日期发生变化，则清除扫描次数
			if (Scan_Times != 0)
			{
				if (RTC_DateStruct.RTC_Date != Scan_Day)
				{
					Scan_Times = 0;	//当日扫描次数
				}
			}
		}

		////2-串口转WIFI通信模块，超时则一次数据接收强制结束
		//if(Usart2_receive_on)
		//{
		//	Usart2_receive_timer++;
		//	if(Usart2_receive_timer >= COMM_TIMEOUT)
		//	{
		//		Usart2_receive_on = 0;
		//		Usart2_receive_timer = 0;
		//		Usart2_rcv_flag = 1;
		//	}
		//}
		//
		////3-以太网通信模块，超时则一次数据接收强制结束
		//if(Uart5_receive_on)
		//{
		//	Uart5_receive_timer++;
		//	if(Uart5_receive_timer >= COMM_TIMEOUT)
		//	{
		//		Uart5_receive_on = 0;
		//		Uart5_receive_timer = 0;
		//		Uart5_rcv_flag = 1;
		//	}
		//}

	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除中断标志位
}
