#ifndef __USART3_H
#define __USART3_H
#include "stm32f4xx_conf.h"
#include "sys.h" 

#define USART3_REC_LEN  			20  	//定义最大接收字节数 20
extern uint8_t  Usart3_rcv_flag;			//串口3接收到数据
extern u8  USART3_RX_BUF[USART3_REC_LEN]; 	//接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 

void usart3_send_byte(uint8_t ch);			//串口3发送函数
void usart3_init(u32 bound);

#endif


