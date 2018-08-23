#include "design.h"

UartManageVarStruct loraManage;
UartManageVarStruct wifiManage;

TIME scanStartTime;
TIME scanStopTime;
TIME nextScanTime;

MAP map;

ScanRepeatStyle scanRepeatStyle;

u8 cmdStart[5] = { 's','t','a','r','t' };
u8 cmdEnd[3] = { 'e','n','d' };
u8 cmdIdCar[3] = { ID_XIAOCHE / 256,ID_XIAOCHE % 256,72 };
u8 cmdIdBridge[3] = { ID_DALIANG / 256,ID_DALIANG % 256,72 };
u8 cmdIdDebug[3] = { ID_DEBUG / 256,ID_DEBUG % 256,72 };

extern void loraInit(void)
{
	usart6_init(9600);
	return;
}

extern void wifiInit(void)
{
	usart2_init(115200);
	return;
}

extern void loraSendBuff(u8 *buf, u8 len);
extern void wifiSendBuff(u8 *buf, u8 len);
extern void SendBuff(DEVICE device, u8 *buf, u8 len)
{
	if (device == lora)
	{
		loraSendBuff(buf, len);
	}
	else if (device == wifi)
	{
		wifiSendBuff(buf, len);
	}

	return;
}

extern void loraClear(void)
{
	Usart6_rcv_index = 0;
	Usart6_rcv_buf[0] = 0;
}

extern void wifiClear(void)
{
	//Usart2_rcv_index = 0;
}

static void loraSendB(u8 dat)
{
	usart6_send_byte(dat);
}

static void wifiSendB(u8 dat)
{
	usart2_send_byte(dat);
	return;
}

extern void loraSendBuff(u8 *buf, u8 len)
{
	u8 i = 0;
	for (i = 0; i < len; i++)
	{
		loraSendB(*(buf + i));
	}
}

extern void wifiSendBuff(u8 *buf, u8 len)
{
	u8 i = 0;
	for (i = 0; i < len; i++)
	{
		wifiSendB(*(buf + i));
	}
}


extern void DebugMsg(uint8_t *str)
{
	delay_ms(30);
	SendBuff(lora, cmdIdDebug, 3);
	while (*str != 0)
	{
		usart6_send_byte(*str);
		str++;
	}
}

extern void DebugNum(uint32_t num)
{
	delay_ms(30);
	SendBuff(lora, cmdIdDebug, 3);
	printf("%d", num);
}

extern int fputc(int ch, FILE *f)
{
	while ((USART6->SR & 0X40) == 0);//循环发送,直到发送完毕   
	USART6->DR = (u8)ch;
	return ch;
}
