#ifndef __USART6_H
#define __USART6_H
#include "stm32f4xx_conf.h"
#include "sys.h" 
#include "stdio.h"

#define USART6_RCV_LEN  			30  	//定义最大接收字节数 20

extern uint8_t USART6_RX_BUF[USART6_RCV_LEN];     //接收缓冲
extern uint8_t Usart6_rcv_index;

void usart6_init(u32 bound);
void usart6_send_byte(uint8_t ch);			//串口6发送

#endif


