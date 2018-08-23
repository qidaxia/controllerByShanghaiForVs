#ifndef _TIMER3_H
#define _TIMER3_H
#include "sys.h"

#define	COMM_TIMEOUT	5		//5ms通信超时

extern uint16_t ledCount;			//闪烁二极管定时器

extern uint8_t  Usart2_rcv_flag;			//串口2接收到数据
extern uint16_t Usart2_receive_timer;	//串口2接收超时定时器
extern uint8_t  Usart2_receive_on;		//串口2接收定时开关

extern uint8_t  Uart5_rcv_flag;			  //串口5接收到数据
extern uint16_t Uart5_receive_timer;	//串口5接收超时定时器
extern uint8_t  Uart5_receive_on;		  //串口5接收定时开关

void TIM3_Int_Init(u16 arr,u16 psc);

#endif
