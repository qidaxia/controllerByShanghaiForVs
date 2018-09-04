#include "sys.h"
#include "delay.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "uart5.h"
#include "led.h"
#include "beep.h"
#include "timer3.h"
#include "rtc.h"
#include "design.h"
#include "init_Parameter.h"

uint8_t DebugFlag;   //调试模式
uint8_t WorkStatus;  //工作状态
uint8_t BreakFlag;   //中断执行标志
uint8_t PCBreakFlag; //上位机终止当前任务

LEVELMODE paulseStyle;  //脉冲输出方式
uint16_t Scan_Interval; //扫描时间间隔，单位分钟
uint16_t Scan_Times;	//当日扫描次数
uint8_t Scan_SW;		//任务扫描开关
uint8_t Scan_Day;		//扫描日期，日期变化，扫描次数清零

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

volatile MOVECMD XiaoChe_Now_Direction; //0x01向前、0x02向后，0x03停止
volatile uint32_t XiaoChe_Now_Position;
volatile MOVECMD DaLiang_Now_Direction; //0x01向前、0x02向后，0x03停止
volatile uint32_t DaLiang_Now_Position;

/*
初始化运行参数，并判断参数的有效性
RTC_BKP_DR0		RTC初始化标志，如果=0x5050表示已经初始化过了
*/

extern void init_Parameter(void)
{
	uint8_t rd_buf[50], i;

	//1-植物行数
	read_from_backup_sram(rd_buf, 4, 11);
	map.Plant_Row = rd_buf[3];
	DebugMsg("row of plants:");
	DebugNum(map.Plant_Row);
	DebugMsg("\r\n"); //植物行数

	//2-植物列数
	read_from_backup_sram(rd_buf, 4, 15);
	map.Plant_Column = (rd_buf[2] << 8) + rd_buf[3];
	DebugMsg("line of plants");
	DebugNum(map.Plant_Column);
	DebugMsg("\r\n"); //植物列数

	//3-植物行坐标
	for (i = 0; i < map.Plant_Row; i++)
	{
		read_from_backup_sram(rd_buf, 4, BASEADDR_ROW + 4 * i);
		map.Row[i] = (rd_buf[0] << 24) + (rd_buf[1] << 16) + (rd_buf[2] << 8) + rd_buf[3];
		DebugMsg(" coordinate ");
		DebugNum(i + 1);
		DebugMsg("row");
		DebugNum(map.Row[i]);
		DebugMsg("\r\n"); //第i+1行坐标
	}

	//4-首列坐标
	read_from_backup_sram(rd_buf, 4, BASEADDR_COL_FIRST);
	map.Column_First = (rd_buf[0] << 24) + (rd_buf[1] << 16) + (rd_buf[2] << 8) + rd_buf[3];
	DebugMsg("coordinate of first line"); //首列坐标：
	DebugNum(map.Column_First);
	DebugMsg("\r\n");

	//5-最大列坐标
	read_from_backup_sram(rd_buf, 4, BASEADDR_COL_LAST);
	map.Column_Last = (rd_buf[0] << 24) + (rd_buf[1] << 16) + (rd_buf[2] << 8) + rd_buf[3];
	DebugMsg("coordinate of last line"); //末列坐标：
	DebugNum(map.Column_Last);
	DebugMsg("\r\n");

	//6-计算列间隙
	if (map.Plant_Column == 1)
		map.Column_Interval = 0;
	else if (map.Plant_Column > 1)
		map.Column_Interval = (map.Column_Last - map.Column_First) / (map.Plant_Column - 1);
	DebugMsg("Interval of column"); //列间距：
	DebugNum(map.Column_Interval);
	DebugMsg("\r\n");

	//7-脉冲输出方式，扫描起始时间，扫描结束时间，扫描间隔，扫描重复方式
	read_from_backup_sram(rd_buf, 11, 67);
	paulseStyle = (LEVELMODE)rd_buf[0];													  //脉冲输出方式
	scanStartTime.Hour = rd_buf[1];														  //起始时间：时
	scanStartTime.Minute = rd_buf[2];													  //起始时间：分
	scanStopTime.Hour = rd_buf[3];														  //结束时间：时
	scanStopTime.Minute = rd_buf[4];													  //结束时间：分
	Scan_Interval = rd_buf[5] + (rd_buf[6] << 8) + (rd_buf[7] << 16) + (rd_buf[8] << 24); //扫描时间间隔
	scanRepeatStyle = (ScanRepeatStyle)rd_buf[9];										  //扫描重复方式
	Scan_SW = rd_buf[10];																  //任务总开关

	Scan_Times = 0;		   //当日扫描次数
	Scan_Day = 0;		   //当日扫描日期
	nextScanTime.Hour = 0; //下次扫描时间
	nextScanTime.Minute = 0;
}
