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
	if (a >= b)
		return a - b;
	else
		return b - a;
}
/* 判断是否到达指定位置 */
static uint8_t deviceInThere(uint32_t ID, uint32_t there)
{
	MOVECMD dir;
	uint32_t pos;
	dir = ID == ID_XIAOCHE ? XiaoChe_Now_Direction : DaLiang_Now_Direction;
	pos = ID == ID_XIAOCHE ? XiaoChe_Now_Position : DaLiang_Now_Position;

	if (ReadStatus(ID_XIAOCHE))
	{
		if (dir == stop)
		{
			if (abs_int(pos, there) < PRECISION)
			{
				return 1;
			}
			else
				return 0;	//距离远
		}
		else
			return 0;	//不是静止状态
	}
	else
		return 0;	//通信问题
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
	if (Scan_Times == 0)	//1-判断当前时间是否大于起始时间，小于结束时间
	{
		if (CompareTime(scanStartTime.Hour, scanStartTime.Minute,
			scanStopTime.Hour, scanStopTime.Minute,
			RTC_TimeStruct.RTC_Hours, RTC_TimeStruct.RTC_Minutes))
			scan_flag = 1;
		else
			return 0;
	}
	else	//2-判断当前时间是否大于下次扫描时间，启动扫描
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
		if (Scan_Part())//启动扫描
		{
			Scan_Day = RTC_DateStruct.RTC_Date;	//当日扫描日期
			Scan_Times++;											//记录扫描次数
			nextScanTime.Hour = RTC_TimeStruct.RTC_Hours;	//记录下次扫描时间		
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
		if (Scan_Part())//启动扫描
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
	uint8_t wait;
	uint32_t timeout;

	PCBreakFlag = 0;
	//1-小车和大梁归零
	DebugMsg("发送小车和大梁归原点指令\r\n");
	if (MotorToZero(ID_XIAOCHE) == 0)
	{
		DebugMsg("和小车通信失败\r\n");
		return 0;
	}
	if (MotorToZero(ID_DALIANG) == 0)
	{
		DebugMsg("和大梁通信失败\r\n");
		return 0;
	}

	//2-查询等待大梁和小车到达零点
	DebugMsg("查询等待大梁和小车到达零点\r\n");
	timeout = 0;
	wait = 0x11;
	while (wait)
	{
		if (wait & 0x10)		//判断小车位置
		{
			if (deviceInThere(ID_XIAOCHE, 0))
				wait &= 0xEF;
		}

		if (wait & 0x01)		//判断大梁位置
		{
			if (deviceInThere(ID_DALIANG, 0))
				wait &= 0xFE;
		}
		delay_ms(1000);
		timeout++;
		DebugNum(timeout);	//输出等待时间
		if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
		{
			DebugMsg("等待超时返回\r\n");
			return 0;
		}
		if (pcCheck(wifi))
		{
			if (PCBreakFlag)
			{
				MotorMove(ID_XIAOCHE, stop);
				MotorMove(ID_DALIANG, stop);
				return 0;
			}
		}
	}
	return 1;
}

/* -----------------------------------------------------------------------------
判断扫描计划任务
----------------------------------------------------------------------------- */
extern void PlanTask(void)
{
	switch (scanRepeatStyle)
	{
	case everyDay://每天
		ScanNextTimes();
		break;

	case once://只一次
		ScanOneTimes();
		break;

	case workDay://工作日（周一至周五）
		if ((RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Monday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Tuesday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Wednesday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Thursday) ||
			(RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Friday))
			ScanNextTimes();
		break;

	case monday://周一
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Monday)
			ScanNextTimes();
		break;

	case tuesday://周二
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Tuesday)
			ScanNextTimes();
		break;

	case wednesday://周三
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Wednesday)
			ScanNextTimes();
		break;

	case thursday://周四
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Thursday)
			ScanNextTimes();
		break;

	case friday://周五
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Friday)
			ScanNextTimes();
		break;

	case saturday://周六
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Saturday)
			ScanNextTimes();
		break;

	case sunday://周日
		if (RTC_DateStruct.RTC_WeekDay == RTC_Weekday_Sunday)
			ScanNextTimes();
		break;

	case noneRepeat://不扫描，清除自动扫描任务

		break;
	}
}

/* -----------------------------------------------------------------------------
全面扫描
----------------------------------------------------------------------------- */
extern uint8_t Scan_Full(void)
{
	uint8_t wait, scan_row;
	uint32_t timeout;

	PCBreakFlag = 0;	//清楚PC终止标志
	DebugMsg("开始执行全面扫描\r\n");

	if (XYgoZero())
	{
		DebugMsg("小车和大梁回归原点成功！\r\n");
	}
	else
	{
		DebugMsg("小车和大梁回归原点失败！\r\n");
		return 0;
	}

	//1-发送大梁回第一列指令
	DebugMsg("发送大梁回第一列指令\r\n");
	if (MotorToPosition(ID_DALIANG, map.Column_First) == 0)
		return 0;

	//2-发送小车回起始行	指令
	DebugMsg("发送小车回起始行指令\r\n");
	if (MotorToPosition(ID_XIAOCHE, map.Row[0]) == 0)
		return 0;

	//3-查询等待大梁和小车到达指定位置
	DebugMsg("查询等待大梁和小车到达指定位置\r\n");
	timeout = 0;
	wait = 0x11;
	while (wait)
	{
		if (wait & 0x10)		//判断小车位置
		{
			if (deviceInThere(ID_XIAOCHE, map.Row[0]))
			{
				wait &= 0xEF;
			}
		}

		if (wait & 0x01)		//判断大梁位置
		{

			if (deviceInThere(ID_DALIANG, map.Column_First))
				wait &= 0xFE;
		}
		delay_ms(1000);
		timeout++;
		DebugNum(timeout);	//输出等待时间
		if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
		{
			DebugMsg("等待超时返回\r\n");
			return 0;
		}
		if (pcCheck(wifi))
		{
			if (PCBreakFlag)
			{
				MotorMove(ID_XIAOCHE, stop);
				MotorMove(ID_DALIANG, stop);
				return 0;
			}
		}
	}

	//开始扫描
	DebugMsg("开始扫描\r\n");
	scan_row = 0;
	while (scan_row < map.Plant_Row)
	{
		//4-输出5V信号
		DebugMsg("输出5V信号\r\n");
		SetXiaoChe_5V_Level(paulseStyle);
		delay_ms(1000);

		//5-大梁移动到最大列处
		DebugMsg("大梁移动中....\r\n");
		MotorToPosition(ID_DALIANG, map.Column_Last);
		timeout = 0;
		wait = 0x01;
		while (wait)
		{
			if (deviceInThere(ID_DALIANG, map.Column_Last)) wait = 0;
			delay_ms(1000);
			timeout++;
			DebugNum(timeout);	//输出等待时间
			if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
			{
				DebugMsg("等待超时返回\r\n");
				return 0;
			}
			if (pcCheck(wifi))
			{
				if (PCBreakFlag)
				{
					MotorMove(ID_XIAOCHE, stop);
					MotorMove(ID_DALIANG, stop);
					return 0;
				}
			}
		}
		//6-关闭5V信号
		DebugMsg("关闭5V信号\r\n");
		SetXiaoChe_0V_Level();
		delay_ms(1000);

		//7-大梁移动到起始列处
		DebugMsg("大梁移动到起始列处\r\n");
		MotorToPosition(ID_DALIANG, map.Column_First);
		timeout = 0;
		wait = 0x01;
		while (wait)
		{
			if (deviceInThere(ID_DALIANG, map.Column_First)) wait = 0;
			delay_ms(1000);
			timeout++;
			DebugNum(timeout);	//输出等待时间
			if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
			{
				DebugMsg("等待超时返回\r\n");
				return 0;
			}
			if (pcCheck(wifi))
			{
				if (PCBreakFlag)
				{
					MotorMove(ID_XIAOCHE, stop);
					MotorMove(ID_DALIANG, stop);
					return 0;
				}
			}
		}
		//8-小车移动到下一行
		scan_row++;
		if (scan_row < map.Plant_Row)	//如果到达最大行，则小车不移动
		{
			DebugMsg("小车移动到下一行\r\n");
			MotorToPosition(ID_XIAOCHE, map.Row[scan_row]);
			timeout = 0;
			wait = 0x01;
			while (wait)
			{
				if (deviceInThere(ID_XIAOCHE, map.Row[scan_row])) wait = 0;
				delay_ms(1000);
				timeout++;
				DebugNum(timeout);	//输出等待时间
				if (timeout > XIAOCHE_TIMEOUT)//10分钟不能到达指定位置，超时退出
				{
					DebugMsg("等待超时\r\n");
					return 0;
				}
				if (pcCheck(wifi))
				{
					if (PCBreakFlag)
					{
						MotorMove(ID_XIAOCHE, stop);
						MotorMove(ID_DALIANG, stop);
						return 0;
					}
				}
			}
		}
	}
	DebugMsg("全面扫描成功完成\r\n");
	BeepThree();
	return 1;
}

/* -----------------------------------------------------------------------------
区域扫描
----------------------------------------------------------------------------- */
extern uint8_t Scan_Part(void)
{
	uint32_t timeout, x;
	uint8_t wait, i, j, k, m, rd_buf[40];
	uint8_t scan_long, nowbyte;			//扫描字节个数
	uint16_t scan_num;	//扫描到的植物

	PCBreakFlag = 0;	//清楚PC终止标志
	DebugMsg("开始执行区域扫描任务\r\n");

	//2-回归原点
	if (XYgoZero())
	{
		DebugMsg("小车和大梁回归原点成功！\r\n");
	}
	else
	{
		DebugMsg("小车和大梁回归原点失败！\r\n");
		return 0;
	}

	//3-开始扫描
	DebugMsg("开始逐行扫描\r\n");
	for (i = 0; i < map.Plant_Row; i++)		//逐行扫描
	{
		//4-小车移动到扫描行
		DebugMsg("小车移动到第 "); 	DebugNum(i + 1); DebugMsg(" 行,");
		DebugMsg("   目标坐标："); 	DebugNum(map.Row[i]); DebugMsg("\r\n");
		MotorToPosition(ID_XIAOCHE, map.Row[i]);
		timeout = 0;
		wait = 0x01;
		while (wait)
		{
			if (deviceInThere(ID_XIAOCHE, map.Row[i]))
				wait = 0;
			delay_ms(1000);
			timeout++;
			DebugNum(timeout);	//输出等待时间
			if (timeout > XIAOCHE_TIMEOUT)//10分钟不能到达指定位置，超时退出
			{
				DebugMsg("等待超时\r\n");
				return 0;
			}
			if (pcCheck(wifi))
			{
				if (PCBreakFlag)
				{
					MotorMove(ID_XIAOCHE, stop);
					MotorMove(ID_DALIANG, stop);
					return 0;
				}
			}
		}

		//5-读取扫描位图
		for (m = 0; m < 40; m++) rd_buf[m] = 0;
		read_from_backup_sram(rd_buf, 40, 100 + 40 * i);
		scan_long = rd_buf[0];
		scan_num = 0;
		for (j = 0; j < scan_long; j++)
		{
			nowbyte = rd_buf[j + 1];
			for (k = 0; k < 8; k++)
			{
				if (nowbyte & 0x01)	//对应位为1则移动到该列，并停留，给出提示信号
				{
					//6-大梁移动到标志列
					x = map.Column_First + map.Column_Interval*(8 * j + k);
					DebugMsg("大梁移动目标： "); DebugNum(x); DebugMsg("\r\n");
					MotorToPosition(ID_DALIANG, x);
					timeout = 0;
					wait = 0x01;
					while (wait)
					{
						if (deviceInThere(ID_DALIANG, x))
							wait = 0;
						delay_ms(1000);
						timeout++;
						DebugNum(timeout);	//输出等待时间
						if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
						{
							DebugMsg("等待超时\r\n");
							return 0;
						}
						if (pcCheck(wifi))
						{
							if (PCBreakFlag)
							{
								MotorMove(ID_XIAOCHE, stop);
								MotorMove(ID_DALIANG, stop);
								return 0;
							}
						}
					}
					//7-给出5V信号并等待
					DebugMsg("找到扫描点位\r\n");
					SetXiaoChe_5V_Level(paulseStyle);
					delay_ms(CAMERATIME);
					SetXiaoChe_0V_Level();
					scan_num++;
					DebugMsg("本行已扫描目标 ");
					DebugNum(scan_num);
					DebugMsg(" 个\r\n");
				}
				nowbyte >>= 1;
			}
		}
	}

	//8-扫描完毕，响三声
	DebugMsg("区域扫描完成\r\n");
	BeepThree();
	return 1;
}

/* -----------------------------------------------------------------------------
扫描指定行
----------------------------------------------------------------------------- */
extern uint8_t Scan_Row(uint8_t scan_row)
{
	uint8_t wait;
	uint32_t timeout;

	PCBreakFlag = 0;	//清楚PC终止标志
	DebugMsg("扫描指定行任务\r\n");
	//回归原点
	if (XYgoZero())
	{
		DebugMsg("小车和大梁回归原点成功！\r\n");
	}
	else
	{
		DebugMsg("小车和大梁回归原点失败！\r\n");
		return 0;
	}

	//1-发送大梁回起始列指令
	DebugMsg("发送大梁回起始列指令\r\n");
	if (MotorToPosition(ID_DALIANG, map.Column_First) == 0)
		return 0;

	//2-发送小车回起始行指令
	DebugMsg("发送小车到达第"); DebugNum(scan_row); DebugMsg("行指令\r\n");
	if (MotorToPosition(ID_XIAOCHE, map.Row[scan_row - 1]) == 0)
		return 0;

	//3-查询等待大梁和小车到达指定位置
	DebugMsg("查询等待大梁和小车到达指定位置\r\n");
	timeout = 0;
	wait = 0x11;
	while (wait)
	{
		if (wait & 0x10)		//判断小车位置
		{
			if (deviceInThere(ID_XIAOCHE, map.Row[scan_row - 1])) wait &= 0xEF;
		}

		if (wait & 0x01)		//判断大梁位置
		{
			if (deviceInThere(ID_DALIANG, map.Column_First)) wait &= 0xFE;
		}
		delay_ms(1000);
		timeout++;
		DebugNum(timeout);	//输出等待时间
		if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
		{
			DebugMsg("等待超时\r\n");
			return 0;
		}
		if (pcCheck(wifi))
		{
			if (PCBreakFlag)
			{
				MotorMove(ID_XIAOCHE, stop);
				MotorMove(ID_DALIANG, stop);
				return 0;
			}
		}
	}

	//4-输出5V信号
	DebugMsg("输出5V信号\r\n");
	SetXiaoChe_5V_Level(paulseStyle);
	delay_ms(1000);

	//5-大梁移动到最大列处
	DebugMsg("大梁移动中....\r\n");
	MotorToPosition(ID_DALIANG, map.Column_Last);
	timeout = 0;
	wait = 0x01;
	while (wait)
	{
		if (deviceInThere(ID_DALIANG, map.Column_Last)) wait = 0;
		delay_ms(1000);
		timeout++;
		DebugNum(timeout);	//输出等待时间
		if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
		{
			DebugMsg("等待超时\r\n");
			return 0;
		}
		if (pcCheck(wifi))
		{
			if (PCBreakFlag)
			{
				MotorMove(ID_XIAOCHE, stop);
				MotorMove(ID_DALIANG, stop);
				return 0;
			}
		}
	}

	//6-关闭5V信号
	DebugMsg("关闭5V信号\r\n");
	SetXiaoChe_0V_Level();
	delay_ms(1000);

	DebugMsg("扫描指定行任务完成\r\n");
	BeepThree();
	return 1;
}

/* -----------------------------------------------------------------------------
扫描指定列
----------------------------------------------------------------------------- */
extern uint8_t Scan_Column(uint16_t scan_column)
{
	uint8_t wait;
	uint32_t timeout, column_position;

	PCBreakFlag = 0;	//清楚PC终止标志
	DebugMsg("扫描指定列开始\r\n");
	//回归原点
	if (XYgoZero())
	{
		DebugMsg("小车和大梁回归原点成功！\r\n");
	}
	else
	{
		DebugMsg("小车和大梁回归原点失败！\r\n");
		return 0;
	}

	//1-发送大梁回指定列指令
	DebugMsg("发送大梁到达第"); DebugNum(scan_column); DebugMsg("列指令\r\n");
	column_position = map.Column_First + map.Column_Interval*(scan_column - 1);
	if (MotorToPosition(ID_DALIANG, column_position) == 0)return 0;

	//2-发送小车回起始行指令
	DebugMsg("发送小车到达起始行指令\r\n");
	if (MotorToPosition(ID_XIAOCHE, map.Row[0]) == 0)return 0;

	//3-查询等待大梁和小车到达指定位置
	DebugMsg("查询等待大梁和小车到达指定位置\r\n");
	timeout = 0;
	wait = 0x11;
	while (wait)
	{
		if (wait & 0x10)		//判断小车位置
		{
			if (deviceInThere(ID_XIAOCHE, map.Row[0]))
				wait &= 0xEF;
		}

		if (wait & 0x01)		//判断大梁位置
		{
			if (deviceInThere(ID_DALIANG, column_position))
				wait &= 0xFE;
		}
		delay_ms(1000);
		timeout++;
		DebugNum(timeout);	//输出等待时间
		if (timeout > DALIANG_TIMEOUT)//10分钟不能到达指定位置，超时退出
		{
			DebugMsg("等待超时\r\n");
			return 0;
		}
		if (pcCheck(wifi))
		{
			if (PCBreakFlag)
			{
				MotorMove(ID_XIAOCHE, stop);
				MotorMove(ID_DALIANG, stop);
				return 0;
			}
		}
	}

	//4-输出5V信号
	DebugMsg("输出5V信号\r\n");
	SetXiaoChe_5V_Level(paulseStyle);
	delay_ms(1000);

	//5-小车移动到最大行处
	DebugMsg("小车移动中....\r\n");
	MotorToPosition(ID_XIAOCHE, map.Row[map.Plant_Row - 1]);
	timeout = 0;
	wait = 0x01;
	while (wait)
	{

		if (deviceInThere(ID_XIAOCHE, map.Row[map.Plant_Row - 1]))
			wait = 0;
		delay_ms(1000);
		timeout++;
		DebugNum(timeout);	//输出等待时间
		if (timeout > XIAOCHE_TIMEOUT)//10分钟不能到达指定位置，超时退出
		{
			DebugMsg("等待超时\r\n");
			return 0;
		}
		if (pcCheck(wifi))
		{
			if (PCBreakFlag)
			{
				MotorMove(ID_XIAOCHE, stop);
				MotorMove(ID_DALIANG, stop);
				return 0;
			}
		}
	}

	//6-关闭5V信号
	DebugMsg("关闭5V信号\r\n");
	SetXiaoChe_0V_Level();
	delay_ms(1000);

	DebugMsg("扫描指定列任务完成！\r\n");
	BeepThree();
	return 1;
}

