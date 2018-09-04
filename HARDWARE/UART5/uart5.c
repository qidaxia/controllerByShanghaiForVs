#include "timer3.h"
#include "uart5.h"	

/*
PD2  Uart5-RX
PC12 Uart5-TX
*/

uint8_t Uart5_rcv_flag;					//串口5接收到数据
uint8_t UART5_RX_BUF[UART5_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
uint8_t UART5_BUF_Index; 

//串口5发送函数
void uart5_send_byte(uint8_t ch)
{ 	
	while((UART5->SR&0X40)==0);//循环发送,直到发送完毕   
	UART5->DR = (u8) ch;      
}

/*
初始化串口5
bound:波特率
*/
void uart5_init(u32 bound){
	 //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD ,ENABLE); //使能GPIOC和GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);//使能UART5时钟

	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource2,GPIO_AF_UART5); 	//GPIOD2复用为UART5-RX
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_UART5); //GPIOC12复用为UART5-TX

	//UART5端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 				//GPIOD2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 			//上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); 					//初始化PD2

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 				//GPIOC12
	GPIO_Init(GPIOC,&GPIO_InitStructure); 					//初始化PD2

	//UART5 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(UART5, &USART_InitStructure); 						//初始化串口5

	USART_Cmd(UART5, ENABLE);  //使能串口5 
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启相关中断

	//Uart5 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;		//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器、
}

/*
串口5中断服务程序
*/
void UART5_IRQHandler(void)                	
{
	u8 Res;

	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)  	//接收中断
	{
		Res =USART_ReceiveData(UART5);	//(UART5->DR);		//读取接收到的数据
		UART5_RX_BUF[UART5_BUF_Index++] = Res;
		Uart5_receive_timer = 0;	//串口5接收超时定时器，只要有数据收到，就清定时器
		Uart5_receive_on = 1;		//串口5接收定时开关
	} 
} 

