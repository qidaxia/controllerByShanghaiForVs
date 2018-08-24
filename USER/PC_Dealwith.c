/*
串口模块数据处理
可以处理串口2的WIFI数据和串口5的以太网数据

*/
#include "delay.h"
#include "usart2.h"
#include "uart5.h"
#include "led.h"
#include "beep.h"
#include "timer3.h"
#include "rtc.h"
#include "adc.h"
#include "design.h"
#include "PC_Dealwith.h"
#include "init_Parameter.h"
#include "toSensor.h"
#include "PlanTask.h"


extern void SendConfig(uint8_t com)
{
	uint8_t send_buf[50];

	send_buf[0] = 0x0C;		//命令码
	send_buf[1] = RTC_DateStruct.RTC_Year + 2000;		//年4位，LSB
	send_buf[2] = (RTC_DateStruct.RTC_Year + 2000) >> 8;
	send_buf[3] = 0;
	send_buf[4] = 0;
	send_buf[5] = RTC_DateStruct.RTC_Month;			//月
	send_buf[6] = RTC_DateStruct.RTC_Date;			//日
	send_buf[7] = RTC_TimeStruct.RTC_Hours;			//时
	send_buf[8] = RTC_TimeStruct.RTC_Minutes;		//分
	send_buf[9] = RTC_TimeStruct.RTC_Seconds;		//秒
	send_buf[10] = RTC_DateStruct.RTC_WeekDay;	//星期
	send_buf[11] = 0x20;
	send_buf[12] = (uint8_t)paulseStyle;					//脉冲方式
	send_buf[13] = 0x20;
	send_buf[14] = scanStartTime.Hour;								 //起始时间：时
	send_buf[15] = scanStartTime.Minute;							 //起始时间：分
	send_buf[16] = scanStopTime.Hour;							 //结束时间：时
	send_buf[17] = scanStopTime.Minute;							 //结束时间：分
	send_buf[18] = Scan_Interval;							 //扫描间隔(4个字节 LSB)
	send_buf[19] = Scan_Interval >> 8;
	send_buf[20] = 0;
	send_buf[21] = 0;
	send_buf[22] = (uint8_t)scanRepeatStyle;							//扫描重复方式	
	send_buf[23] = 0x20;
	send_buf[24] = map.Plant_Row;										//行数(4个字节，LSB)
	send_buf[25] = map.Plant_Row >> 8;
	send_buf[26] = 0;
	send_buf[27] = 0;
	send_buf[28] = 0xFF;										   //预留(4个字节，LSB)
	send_buf[29] = 0xFF;
	send_buf[30] = 0xFF;
	send_buf[31] = 0xFF;
	send_buf[32] = map.Plant_Column;								//列数(4个字节，LSB)
	send_buf[33] = map.Plant_Column >> 8;
	send_buf[34] = 0;
	send_buf[35] = 0;

	SendBuff((DEVICE)com, cmdStart, 5);
	SendBuff((DEVICE)com, send_buf, 36);
	SendBuff((DEVICE)com, cmdEnd, 3);
}

extern void SendBack(uint8_t com, ECHO echoCode)
{
	u8 echoBuf[2] = { 0x0B,0x00 };
	echoBuf[1] = echoCode;

	echoCode == IS_OK ? BeepOne() : BeepTwo();
	SendBuff((DEVICE)com, cmdStart, 5);
	SendBuff((DEVICE)com, echoBuf, 2);
	SendBuff((DEVICE)com, cmdEnd, 3);
}

extern void SendBatteryVoltage(uint8_t com)
{
	u16 adc_value, bat1000;
	u8 bat_low;
	u8 batBuff[4] = { 0x0D,0x00,0x00,0x00 };
	float batv;

	adc_value = Get_Adc_Average(0, 5); //纽扣电池电压
	//batv = adc_value/4096*3300*2;//基准电压3.3V,放大1000倍，电阻二分压
	batv = adc_value * 6600 / 4096;
	bat_low = (batv >= BAT_THRESHOLD) ? 1 : 2;
	bat1000 = batv;

	batBuff[1] = bat_low;
	batBuff[2] = bat1000;
	batBuff[3] = bat1000 >> 8;

	SendBuff((DEVICE)com, cmdStart, 5);
	SendBuff((DEVICE)com, batBuff, 4);
	SendBuff((DEVICE)com, cmdEnd, 3);
}

extern void SendBackValue(uint8_t com, uint32_t d)
{
	u8 tempBuf[5] = { 0x0A };
	tempBuf[1] = d;
	tempBuf[2] = d >> 8;
	tempBuf[3] = d >> 16;
	tempBuf[4] = d >> 24;

	SendBuff((DEVICE)com, cmdStart, 5);
	SendBuff((DEVICE)com, tempBuf, 5);
	SendBuff((DEVICE)com, cmdEnd, 3);
}

extern uint8_t pcCheck(DEVICE com)
{
	uint8_t cmd = 0xff;
	uint8_t * dataPtr = (void *)0;

	dataPtr = getCmdFrame(com, &cmd);
	if (cmd == 0xff)
	{
		return 0;
	}
	handlerFrame(com, cmd, dataPtr);
	clearReciveBuf(com);
	return 1;
}
