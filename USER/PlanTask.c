#include "delay.h"
#include "usart2.h"
#include "usart6.h"
#include "led.h"
#include "beep.h"
#include "timer3.h"
#include "rtc.h"
#include "design.h"
#include "PlanTask.h"
#include "PC_Dealwith.h"
#include "init_Parameter.h"
#include "toSensor.h"

/* 求整型数绝对值 */
static uint32_t abs_int(uint32_t a, uint32_t b)
{
	return a >= b ? (a - b) : (b - a);
}
/* 判断是否到达指定位置 */
static uint8_t deviceInThere(uint32_t ID, uint32_t there)
{
	MOVECMD dir;
	uint32_t pos;

	if (ReadStatus(ID))
	{
		dir = ID == ID_XIAOCHE ? XiaoChe_Now_Direction : DaLiang_Now_Direction;
		pos = ID == ID_XIAOCHE ? XiaoChe_Now_Position : DaLiang_Now_Position;
		if (dir == stop)
		{
			if (abs_int(pos, there) < PRECISION)
			{
				return 1;
			}
			else
				return 0; //距离远
		}
		else
			return 0; //不是静止状态
	}
	else
		return 0; //通信问题
}

/* 判断一个时刻C是否位于A和B之间 */
static uint8_t CompareTime(uint8_t A_Hour, uint8_t A_Minute, uint8_t B_Hour, uint8_t B_Minute, uint8_t C_Hour, uint8_t C_Minute)
{
	uint16_t A = 0, B = 0, C = 0;
	A = (A_Hour << 8) + A_Minute;
	B = (B_Hour << 8) + B_Minute;
	C = (C_Hour << 8) + C_Minute;
	if ((C > A) && (C < B))
		return 1;
	else
		return 0;
}

/* -----------------------------------------------------------------------------
判断检测时间是否到达
----------------------------------------------------------------------------- */
static uint16_t ScanNextTimes(void)
{
	uint32_t t, t_hour, t_minute;
	uint8_t scan_flag;

	scan_flag = 0;
	if (Scan_Times == 0) //1-判断当前时间是否大于起始时间，小于结束时间
	{
		if (CompareTime(scanStartTime.Hour, scanStartTime.Minute,
						scanStopTime.Hour, scanStopTime.Minute,
						RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes))
			scan_flag = 1;
		else
			return 0;
	}
	else //2-判断当前时间是否大于下次扫描时间，启动扫描
	{
		if (CompareTime(nextScanTime.Hour, nextScanTime.Minute,
						scanStopTime.Hour, scanStopTime.Minute,
						RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes))
			scan_flag = 1;
		else
			return 0;
	}

	if (scan_flag)
	{
		if (Scan_Part()) //启动扫描
		{
			Scan_Day = RTC_DateStruct.RTC_Date;			  //当日扫描日期
			Scan_Times++;								  //记录扫描次数
			nextScanTime.Hour = RTC_TimeStruct.RTC_Hours; //记录下次扫描时间
			t = RTC_TimeStruct.RTC_Minutes + Scan_Interval;
			t_hour = t / 60;
			t_minute = t % 60;
			nextScanTime.Hour += t_hour;
			nextScanTime.Minute = t_minute;
			if (nextScanTime.Hour >= 24)
			{
				Scan_Times = 0;
			}
			return 1;
		}
		else
			return 0;
	}
	return 0;
}

/* -----------------------------------------------------------------------------
区域扫描一次
----------------------------------------------------------------------------- */
static uint8_t ScanOneTimes(void)
{
	uint8_t wr_buf[10];
	if (CompareTime(scanStartTime.Hour, scanStartTime.Minute,
					scanStopTime.Hour, scanStopTime.Minute,
					RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes))
	{
		if (Scan_Part()) //启动扫描
		{
			scanRepeatStyle = noneRepeat;
			wr_buf[0] = (uint8_t)scanRepeatStyle;
			write_to_backup_sram(wr_buf, 1, 76); //更改保存RAM任务重复方式
			return 1;
		}
		else
			return 0;
	}
	else
		return 0;
}

/* -----------------------------------------------------------------------------
大梁和小车回归原点
-----------------------------------------------------------------------------  */
static uint8_t XYgoZero(void)
{
	uint8_t i, j;
	PCBreakFlag = 0;
	//1-小车和大梁归零
	DebugMsg("Car and bridge return to zero\r\n"); //发送小车和大梁归原点指令
	if (MotorToZero(ID_XIAOCHE) == 0)
	{
		DebugMsg("Fall to send message to car\r\n"); //和小车通信失败
		return 0;
	}
	if (MotorToZero(ID_DALIANG) == 0)
	{
		DebugMsg("Fall to send message to bridge\r\n"); //和大梁通信失败
		return 0;
	}
	//2-查询等待大梁和小车到达零点
	DebugMsg("Waiting for bridge and car to zero\r\n"); //查询等待大梁和小车到达零点
	i = (uint8_t)waitDeviceToThere(ID_XIAOCHE, 0);
	j = (uint8_t)waitDeviceToThere(ID_DALIANG, 0);

	return i & j;
}

//1:正常退出 RET_OK
//0：超时退出 RET_ERR
extern RETCODE waitDeviceToThere(uint32_t ID, uint32_t des)
{
	uint32_t timeout = 0;
	uint32_t waitTime;
	waitTime = ID == ID_DALIANG ? DALIANG_TIMEOUT : XIAOCHE_TIMEOUT;

	while (1) //等待
	{
		if (pcCheck(wifi) && PCBreakFlag)
		{
			MotorMove(ID_XIAOCHE, stop);
			MotorMove(ID_DALIANG, stop);
			DebugMsg("com stop\r\n"); //上位机终止
			return RET_ERR;
		}
		delay_ms(20);
		timeout++;
		if (timeout % 50 == 0) //check postion
		{
			if (deviceInThere(ID, des))
			{
				break;
			}
			if (timeout > waitTime * 50) //超时退出 600/60=10min
			{
				DebugMsg("Overtime\r\n"); //等待超时
				return RET_ERR;
			}
		}
	}
	return RET_OK;
}

/* -----------------------------------------------------------------------------
判断扫描计划任务
----------------------------------------------------------------------------- */
extern void PlanTask(void)
{
	switch (scanRepeatStyle)
	{
	case everyDay: //每天
		ScanNextTimes();
		break;

	case once: //只一次
		ScanOneTimes();
		break;

	case workDay: //工作日（周一至周五）
		if ((RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Monday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Tuesday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Wednesday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Thursday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Friday))
			ScanNextTimes();
		break;

	case monday: //周一
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Monday)
			ScanNextTimes();
		break;

	case tuesday: //周二
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Tuesday)
			ScanNextTimes();
		break;

	case wednesday: //周三
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Wednesday)
			ScanNextTimes();
		break;

	case thursday: //周四
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Thursday)
			ScanNextTimes();
		break;

	case friday: //周五
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Friday)
			ScanNextTimes();
		break;

	case saturday: //周六
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Saturday)
			ScanNextTimes();
		break;

	case sunday: //周日
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Sunday)
			ScanNextTimes();
		break;

	case noneRepeat: //不扫描，清除自动扫描任务

		break;
	}
}

/* -----------------------------------------------------------------------------
全面扫描
----------------------------------------------------------------------------- */
extern uint8_t Scan_Full(void)
{
	uint8_t scan_row;

	PCBreakFlag = 0;				  //清楚PC终止标志
	DebugMsg("start scan plan \r\n"); //开始执行全面扫描

	if (XYgoZero())
	{
		DebugMsg("Car and bridge return success\r\n"); //小车和大梁回归原点成功！
	}
	else
	{
		DebugMsg("Car and bridge fail to return\r\n"); //小车和大梁回归原点失败！
		return 0;
	}

	//1-发送大梁回第一列指令
	DebugMsg("Bridge return first column\r\n"); //发送大梁回第一列指令
	if (MotorToPosition(ID_DALIANG, map.Column_First) == 0)
		return 0;

	//2-发送小车回起始行	指令
	DebugMsg("Car return first column\r\n"); //发送小车回起始行指令
	if (MotorToPosition(ID_XIAOCHE, map.Row[0]) == 0)
		return 0;

	//3-查询等待大梁和小车到达指定位置
	DebugMsg("Waiting for aim position\r\n"); //查询等待大梁和小车到达指定位置
	if (waitDeviceToThere(ID_XIAOCHE, map.Row[0]) == RET_ERR || waitDeviceToThere(ID_DALIANG, map.Column_First) == RET_ERR)
	{
		return 0;
	}
	//开始扫描
	DebugMsg("Start scan plan\r\n"); //开始扫描
	scan_row = 0;
	while (scan_row < map.Plant_Row)
	{
		//4-输出5V信号
		DebugMsg("Set 5V signal\r\n"); //输出5V信号
		SetXiaoChe_5V_Level(paulseStyle);
		delay_ms(1000);
		//5-大梁移动到最大列处
		DebugMsg("Bridge moving...\r\n"); //大梁移动中...
		MotorToPosition(ID_DALIANG, map.Column_Last);
		if (waitDeviceToThere(ID_DALIANG, map.Column_Last) == RET_ERR)
		{
			return 0;
		}
		//6-关闭5V信号
		DebugMsg("Reset 5V signal\r\n"); //关闭5V信号
		SetXiaoChe_0V_Level();
		delay_ms(1000);
		//7-大梁移动到起始列处
		DebugMsg("Bridge return to first column\r\n"); //大梁移动到起始列处
		MotorToPosition(ID_DALIANG, map.Column_First);
		if (waitDeviceToThere(ID_DALIANG, map.Column_First) == RET_ERR)
		{
			return 0;
		}
		//8-小车移动到下一行
		scan_row++;
		if (scan_row < map.Plant_Row) //如果到达最大行，则小车不移动
		{
			DebugMsg("Car move to next column\r\n"); //小车移动到下一行
			MotorToPosition(ID_XIAOCHE, map.Row[scan_row]);
			if (waitDeviceToThere(ID_XIAOCHE, map.Row[scan_row]) == RET_ERR)
			{
				return 0;
			}
		}
	}
	DebugMsg("Scan plan success\r\n"); //全面扫描成功完成
	BeepThree();
	return 1;
}

/* -----------------------------------------------------------------------------
区域扫描
----------------------------------------------------------------------------- */
extern uint8_t Scan_Part(void)
{
	uint32_t x;
	uint8_t i, j, k, m, rd_buf[40];
	uint8_t scan_long, nowbyte; //扫描字节个数
	uint16_t scan_num;			//扫描到的植物

	PCBreakFlag = 0;					  //清楚PC终止标志
	DebugMsg("Start scan plan,area\r\n"); //开始执行区域扫描任务

	//2-回归原点
	if (XYgoZero())
	{
		DebugMsg("Car and bridge return success\r\n"); //小车和大梁回归原点成功！
	}
	else
	{
		DebugMsg("Car and bridge fail to return\r\n"); //小车和大梁回归原点失败！
		return 0;
	}

	//3-开始扫描
	DebugMsg("Start scan column by column\r\n"); //开始逐行扫描
	for (i = 0; i < map.Plant_Row; i++)			 //逐行扫描
	{
		if (PCBreakFlag)
		{
			return 0;
		}
		//4-小车移动到扫描行
		DebugMsg("Car now position"); //小车移动到第
		DebugNum(i + 1);
		DebugMsg("column");		   // 行,
		DebugMsg("aim position:"); //   目标坐标：
		DebugNum(map.Row[i]);
		DebugMsg("\r\n");
		MotorToPosition(ID_XIAOCHE, map.Row[i]);
		if (waitDeviceToThere(ID_XIAOCHE, map.Row[i]) == RET_ERR)
		{
			return 0;
		}

		//5-读取扫描位图
		for (m = 0; m < 40; m++)
			rd_buf[m] = 0;
		read_from_backup_sram(rd_buf, 40, 100 + 40 * i);
		scan_long = rd_buf[0];
		scan_num = 0;
		for (j = 0; j < scan_long; j++)
		{
			nowbyte = rd_buf[j + 1];
			for (k = 0; k < 8; k++)
			{
				if (PCBreakFlag)
				{
					return 0;
				}
				if (nowbyte & 0x01) //对应位为1则移动到该列，并停留，给出提示信号
				{
					//6-大梁移动到标志列
					x = map.Column_First + map.Column_Interval * (8 * j + k);
					DebugMsg("Bridge aim position"); //大梁移动目标：
					DebugNum(x);
					DebugMsg("\r\n");
					MotorToPosition(ID_DALIANG, x);
					if (waitDeviceToThere(ID_DALIANG, x) == RET_ERR)
					{
						return 0;
					}
					//7-给出5V信号并等待
					DebugMsg("Find out aim scan position\r\n"); //找到扫描点位
					SetXiaoChe_5V_Level(paulseStyle);
					delay_ms(CAMERATIME);
					SetXiaoChe_0V_Level();
					scan_num++;
					DebugMsg("Scanned targets "); //本行已扫描目标
					DebugNum(scan_num);
					DebugMsg(" of current row\r\n"); // 个
				}
				nowbyte >>= 1;
			}
		}
	}

	//8-扫描完毕，响三声
	DebugMsg("Area scan success\r\n"); //区域扫描完成
	BeepThree();
	return 1;
}

/* -----------------------------------------------------------------------------
扫描指定行
----------------------------------------------------------------------------- */
extern uint8_t Scan_Row(uint8_t scan_row)
{

	PCBreakFlag = 0;					  //清楚PC终止标志
	DebugMsg("Scan select row plan\r\n"); //扫描指定行任务
	//回归原点
	if (XYgoZero())
	{
		DebugMsg("Car and bridge return success\r\n"); //小车和大梁回归原点成功！
	}
	else
	{
		DebugMsg("Car and bridge fail to return\r\n"); //小车和大梁回归原点失败！
		return 0;
	}

	//1-发送大梁回起始列指令
	DebugMsg("Bridge return first column\r\n"); //发送大梁回起始列指令
	if (MotorToPosition(ID_DALIANG, map.Column_First) == 0)
		return 0;

	//2-发送小车回起始行指令
	DebugMsg("Car arrive at "); //发送小车到达第
	DebugNum(scan_row);
	DebugMsg(" row\r\n");
	if (MotorToPosition(ID_XIAOCHE, map.Row[scan_row - 1]) == 0)
		return 0;

	//3-查询等待大梁和小车到达指定位置
	DebugMsg("Waiting for aim position\r\n"); //查询等待大梁和小车到达指定位置

	if (waitDeviceToThere(ID_XIAOCHE, map.Row[scan_row - 1]) == RET_ERR || waitDeviceToThere(ID_DALIANG, map.Column_First) == RET_ERR)
	{
		return 0;
	}

	//4-输出5V信号
	DebugMsg("Set 5V signal\r\n"); //输出5V信号
	SetXiaoChe_5V_Level(paulseStyle);
	delay_ms(1000);

	//5-大梁移动到最大列处
	DebugMsg("Bridge moving...\r\n"); //大梁移动中....
	MotorToPosition(ID_DALIANG, map.Column_Last);
	if (waitDeviceToThere(ID_DALIANG, map.Column_Last) == RET_ERR)
	{
		return 0;
	}

	//6-关闭5V信号
	DebugMsg("Reset 5V signal\r\n"); //关闭5V信号
	SetXiaoChe_0V_Level();
	delay_ms(1000);

	DebugMsg("Scan select row plan success\r\n"); //扫描指定行任务完成
	BeepThree();
	return 1;
}

/* -----------------------------------------------------------------------------
扫描指定列
----------------------------------------------------------------------------- */
extern uint8_t Scan_Column(uint16_t scan_column)
{
	uint32_t column_position;

	PCBreakFlag = 0;							   //清楚PC终止标志
	DebugMsg("Start scan select column plan\r\n"); //扫描指定列开始
	//回归原点
	if (XYgoZero())
	{
		DebugMsg("Car and bridge return success\r\n"); //小车和大梁回归原点成功！
	}
	else
	{
		DebugMsg("Car and bridge fail to return\r\n"); //小车和大梁回归原点失败！
		return 0;
	}
	//1-发送大梁回指定列指令
	DebugMsg("Bridge arrive at"); //发送大梁到达第
	DebugNum(scan_column);
	DebugMsg("column\r\n"); //列指令
	column_position = map.Column_First + map.Column_Interval * (scan_column - 1);
	if (MotorToPosition(ID_DALIANG, column_position) == 0)
		return 0;

	//2-发送小车回起始行指令
	DebugMsg("Car return first row\r\n"); //发送小车到达起始行指令
	if (MotorToPosition(ID_XIAOCHE, map.Row[0]) == 0)
		return 0;

	//3-查询等待大梁和小车到达指定位置
	DebugMsg("Waiting for aim position\r\n"); //查询等待大梁和小车到达指定位置
	if (waitDeviceToThere(ID_XIAOCHE, map.Row[0]) == RET_ERR || waitDeviceToThere(ID_DALIANG, column_position) == RET_ERR)
	{
		return 0;
	}

	//4-输出5V信号
	DebugMsg("Set 5V signal\r\n"); //输出5V信号
	SetXiaoChe_5V_Level(paulseStyle);
	delay_ms(1000);

	//5-小车移动到最大行处
	DebugMsg("Car moving...\r\n"); //小车移动中....
	MotorToPosition(ID_XIAOCHE, map.Row[map.Plant_Row - 1]);

	if (waitDeviceToThere(ID_XIAOCHE, map.Row[map.Plant_Row - 1]) == RET_ERR)
	{
		return 0;
	}
	//6-关闭5V信号
	DebugMsg("Reset 5V signal\r\n"); //关闭5V信号
	SetXiaoChe_0V_Level();
	delay_ms(1000);

	DebugMsg("Scan select column plan success\r\n"); //扫描指定列任务完成！
	BeepThree();
	return 1;
}
