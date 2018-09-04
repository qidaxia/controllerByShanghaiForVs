#ifndef __USART2_H
#define __USART2_H
#include "stm32f4xx_conf.h"
#include "sys.h" 

#define USART2_REC_LEN  			100  	//定义最大接收字节数
//extern uint8_t  Usart2_rcv_flag;		//串口2接收到数据
extern u8  USART2_RX_BUF[USART2_REC_LEN];   //接收缓冲 
extern u8 USART2_BUF_Index; 
void usart2_init(u32 bound);
void usart2_send_byte(uint8_t ch);				//串口2发送函数
	
#endif


