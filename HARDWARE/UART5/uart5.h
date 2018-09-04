#ifndef __UART5_H
#define __UART5_H
#include "stm32f4xx_conf.h"
#include "sys.h" 

#define UART5_REC_LEN  			100  	//定义最大接收字节数

extern uint8_t  Uart5_rcv_flag;			//串口5接收到数据
extern u8  UART5_RX_BUF[UART5_REC_LEN]; //接收缓冲,最大UART5_REC_LEN个字节.末字节为换行符 
extern u8 UART5_BUF_Index; 
void uart5_init(u32 bound);

void uart5_send_byte(uint8_t ch);			//串口5发送函数

#endif


