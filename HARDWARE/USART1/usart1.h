#ifndef __USART1_H
#define __USART1_H
#include "stm32f4xx_conf.h"
#include "sys.h" 

#define USART1_REC_LEN  			20  	//定义最大接收字节数 20

extern u8  USART1_RX_BUF[USART1_REC_LEN];   //接收缓冲
void usart1_init(u32 bound);
void usart1_send_byte(uint8_t ch);			//串口1发送

#endif


