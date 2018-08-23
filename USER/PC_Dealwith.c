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

/*
向计算机发送控制器配置数据
*/
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
	send_buf[12] = paulseStyle;					//脉冲方式
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


//单片机反馈 0x00:成功执行,0x01:扫描任务开始,0x02:扫描任务结束,0x03:未能正确执行
extern void SendBack(uint8_t com, ECHO echoCode)
{
	u8 echoBuf[2] = { 0x0B,0x00 };
	echoBuf[1] = echoCode;

	echoCode == IS_OK ? BeepOne() : BeepTwo();
	SendBuff((DEVICE)com, cmdStart, 5);
	SendBuff((DEVICE)com, echoBuf, 2);
	SendBuff((DEVICE)com, cmdEnd, 3);
}



/*  发送纽扣电池电压值  */
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

/*
发送坐标数值
*/
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



/* ------------------------------------------------------------------------------
WIFI数据处理
------------------------------------------------------------------------------ */
extern uint8_t WIFI_Dealwith(DEVICE com)
{
	uint8_t rcv_buf[UART5_REC_LEN], i, j;
	uint8_t write_ram_buf[50];
	uint32_t k;


	if (com == wifi)
	{
		for (i = 0; i < USART2_BUF_Index; i++)	rcv_buf[i] = USART2_RX_BUF[i];
		USART2_BUF_Index = 0;
	}
	else if (com == networkModul)
	{
		for (i = 0; i < UART5_BUF_Index; i++)	rcv_buf[i] = UART5_RX_BUF[i];
		UART5_BUF_Index = 0;
	}
	else
		return 0;

	if ((rcv_buf[0] == COMM_HEAD1) && (rcv_buf[2] == COMM_HEAD3) && (rcv_buf[4] == COMM_HEAD5))
	{
		switch (rcv_buf[5])
		{
			//1-设置日期和时间
		case 0x0A: //年(67),月(8),日(9),时(10),分(11),秒(12),星期(13)（周日为7）
					//RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)	
			PWR_BackupAccessCmd(ENABLE);
			RTC_Set_Date((rcv_buf[6] + (rcv_buf[7] << 8)) - 2000, rcv_buf[8], rcv_buf[9], rcv_buf[13]);
			//RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
			RTC_Set_Time(rcv_buf[10], rcv_buf[11], rcv_buf[12], RTC_H12_AM);
			PWR_BackupAccessCmd(DISABLE);
			SendBack(com, IS_OK);
			break;

			//2-地图设置（行数，列数）
		case 0x0C://行数(4个字节，LSB),列数(4个字节，LSB)
			write_ram_buf[0] = 0;
			write_ram_buf[1] = 0;
			write_ram_buf[2] = rcv_buf[7];
			write_ram_buf[3] = rcv_buf[6];
			map.Plant_Row = (rcv_buf[7] << 8) + rcv_buf[6];
			write_to_backup_sram(write_ram_buf, 4, 11); //行数

			write_ram_buf[0] = 0;
			write_ram_buf[1] = 0;
			write_ram_buf[2] = rcv_buf[11];
			write_ram_buf[3] = rcv_buf[10];
			map.Plant_Column = (rcv_buf[11] << 8) + rcv_buf[10];
			write_to_backup_sram(write_ram_buf, 4, 15); //列数
			SendBack(com, IS_OK);
			if (map.Plant_Column == 1)
				map.Column_Interval = 0;
			else if (map.Plant_Column > 1)
				map.Column_Interval = (map.Column_Last - map.Column_First) / (map.Plant_Column - 1);
			DebugMsg("列间距：");
			DebugNum(map.Column_Interval);
			DebugMsg("\r\n");
			break;

			//3-地图行标定，当前位置为指定行号的坐标
		case 0x01:
			if (ReadStatus(ID_XIAOCHE))
			{
				write_ram_buf[0] = XiaoChe_Now_Position >> 24;
				write_ram_buf[1] = XiaoChe_Now_Position >> 16;
				write_ram_buf[2] = XiaoChe_Now_Position >> 8;
				write_ram_buf[3] = XiaoChe_Now_Position;
				write_to_backup_sram(write_ram_buf, 4, BASEADDR_ROW + 4 * (rcv_buf[6] - 1)); //行坐标
				SendBackValue(com, XiaoChe_Now_Position);
			}
			else
			{
				SendBack(com, IS_ERROR);
			}
			break;

			//4-地图列标定，当前位置为起始列或最大列的坐标
		case 0x02:
			if (ReadStatus(ID_DALIANG))
			{
				write_ram_buf[0] = DaLiang_Now_Position >> 24;
				write_ram_buf[1] = DaLiang_Now_Position >> 16;
				write_ram_buf[2] = DaLiang_Now_Position >> 8;
				write_ram_buf[3] = DaLiang_Now_Position;

				if (rcv_buf[6] == 0x01)
				{
					map.Column_First = DaLiang_Now_Position;//起始列坐标
					write_to_backup_sram(write_ram_buf, 4, BASEADDR_COL_FIRST); //列数
					SendBackValue(com, map.Column_First);
					map.Column_Interval = (map.Column_Last - map.Column_First) / map.Plant_Column;
				}
				else if (rcv_buf[6] == 0x02)
				{
					map.Column_Last = DaLiang_Now_Position;//最大列坐标
					write_to_backup_sram(write_ram_buf, 4, BASEADDR_COL_LAST); //列数
					SendBackValue(com, map.Column_Last);
					map.Column_Interval = (map.Column_Last - map.Column_First) / map.Plant_Column;
				}
				else
				{
					SendBack(com, IS_ERROR);
				}
			}
			else
			{
				SendBack(com, IS_ERROR);
			}
			break;

			//5-动作控制
		case 0x0D:
			switch (rcv_buf[6])
			{
			case 0x00://前进（小车前进，Y变大）
				if (MotorMove(ID_XIAOCHE, forward))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x01://后退（小车后退，Y变小）
				if (MotorMove(ID_XIAOCHE, back))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x02://左（大梁后退，X变小）
				if (MotorMove(ID_DALIANG, back))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x03://右（大梁前进，X变大）
				if (MotorMove(ID_DALIANG, forward))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x04://停止（大梁小车全部停止）
				i = MotorMove(ID_XIAOCHE, stop);
				j = MotorMove(ID_DALIANG, stop);
				if ((i + j) < 2)
					SendBack(com, IS_ERROR);
				else
					SendBack(com, IS_OK);
				break;

			case 0x05://进入调试模式
				DebugFlag = 1;
				SendBack(com, IS_OK);
				break;

			case 0x06://退出调试模式
				DebugFlag = 0;
				SendBack(com, IS_OK);
				break;

			case 0x07://读取配置
				SendConfig(com);
				break;

			case 0x08://X大梁返回原点
				if (MotorToZero(ID_DALIANG))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x09://Y小车返回到原点
				if (MotorToZero(ID_XIAOCHE))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x0A://大梁电机高速
				if (ChangeSpeed(highSpeed))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x0B://大梁电机中速
				if (ChangeSpeed(middleSpeed))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x0C://大梁电机低速
				if (ChangeSpeed(lowSpeed))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;
			}
			break;

			//6-手动立即扫描控制
		case 0x0F:
			switch (rcv_buf[6])
			{
			case 0://立即开始一次扫描（按照上位机设置，有可能是行扫或区域扫）
				WorkStatus = 1;
				SendBack(com, IS_OK);
				Scan_Part();
				WorkStatus = 0;
				break;

			case 1://立即开始一次全扫描
				WorkStatus = 1;
				SendBack(com, IS_OK);
				Scan_Full();
				WorkStatus = 0;
				break;

			case 2://立即开始扫描指定行
				WorkStatus = 1;
				SendBack(com, IS_OK);
				Scan_Row(rcv_buf[7]);
				WorkStatus = 0;
				break;

			case 3://立即开始扫描指定列
				WorkStatus = 1;
				SendBack(com, IS_OK);
				k = rcv_buf[7] + (rcv_buf[8] << 8);
				Scan_Column(k);
				WorkStatus = 0;
				break;
			}
			break;

			//计划任务扫描控制参数
		case 0x0B:
			paulseStyle = rcv_buf[6];//脉冲输出方式
			scanStartTime.Hour = rcv_buf[7];				//起始时间：时
			scanStartTime.Minute = rcv_buf[8];				//起始时间：分
			scanStopTime.Hour = rcv_buf[9];					//结束时间：时
			scanStopTime.Minute = rcv_buf[10];				//结束时间：分
			Scan_Interval = rcv_buf[11] + (rcv_buf[12] << 8); //扫描时间间隔
			scanRepeatStyle = (ScanRepeatStyle)rcv_buf[15];	//扫描重复方式

			write_ram_buf[0] = paulseStyle;					//脉冲输出方式
			write_ram_buf[1] = scanStartTime.Hour;			//起始时间：时
			write_ram_buf[2] = scanStartTime.Minute;       //起始时间：分
			write_ram_buf[3] = scanStopTime.Hour;           //结束时间：时
			write_ram_buf[4] = scanStopTime.Minute;         //结束时间：分
			write_ram_buf[5] = rcv_buf[11];					 //扫描时间间隔
			write_ram_buf[6] = rcv_buf[12];
			write_ram_buf[7] = rcv_buf[13];
			write_ram_buf[8] = rcv_buf[14];
			write_ram_buf[9] = rcv_buf[15];					//扫描重复方式
			write_to_backup_sram(write_ram_buf, 10, 67); //写入数据RAM
			SendBack(com, IS_OK);
			break;

			//行扫描位设定
		case 0x03:
			for (i = 0; i < rcv_buf[7] + 1; i++) write_ram_buf[i] = rcv_buf[i + 7];	//数据长度rcv_buf[6]
			write_to_backup_sram(write_ram_buf, rcv_buf[7] + 1, BASEADDR_COLUMN_BIT + 40 * (rcv_buf[6] - 1)); //起始地址100,每行最多40字节
			delay_ms(20);			//延时等待上次操作完成
			SendBack(com, IS_OK);
			break;

			//读取纽扣电池电压值
		case 0x04:
			SendBatteryVoltage(com);
			break;
			//读取调试数据
		case 0x05:

			break;

			//扫描任务开关
		case 0x06:
			Scan_SW = rcv_buf[6]; 	//参数1：1允许扫描，0禁止扫描
			write_ram_buf[0] = rcv_buf[6];
			write_to_backup_sram(write_ram_buf, 1, 77); //自动扫描任务开关
			SendBack(com, IS_OK);
			break;
		}
	}
	return 1;
}

/* ------------------------------------------------------------------------------
WIFI数据处理,只响应紧急停止命令
------------------------------------------------------------------------------ */
extern uint8_t WIFI_Stop(uint8_t com)
{
	uint8_t rcv_buf[UART5_REC_LEN], i, j;
	uint8_t write_ram_buf[10];

	if (com == 2)
	{
		for (i = 0; i < USART2_BUF_Index; i++)	rcv_buf[i] = USART2_RX_BUF[i];
		USART2_BUF_Index = 0;
	}
	else if (com == 5)
	{
		for (i = 0; i < UART5_BUF_Index; i++)	rcv_buf[i] = UART5_RX_BUF[i];
		UART5_BUF_Index = 0;
	}
	else
		return 0;

	if ((rcv_buf[0] == COMM_HEAD1) && (rcv_buf[2] == COMM_HEAD3) && (rcv_buf[4] == COMM_HEAD5))
	{
		switch (rcv_buf[5])
		{
			//终止当前任务
		case 0x05:
			PCBreakFlag = 1;
			break;

			//扫描任务开关
		case 0x06:
			Scan_SW = rcv_buf[6]; 	//参数1：1允许扫描，0禁止扫描
			write_ram_buf[0] = rcv_buf[6];
			write_to_backup_sram(write_ram_buf, 1, 77); //自动扫描任务开关
			SendBack(com, IS_OK);
			break;

			//5-动作控制
		case 0x0D:
			switch (rcv_buf[6])
			{
			case 0x04://停止（大梁小车全部停止）
				i = MotorMove(ID_XIAOCHE, stop);
				j = MotorMove(ID_DALIANG, stop);
				if ((i + j) < 2)
					SendBack(com, IS_ERROR);
				else
					SendBack(com, IS_OK);
				break;

			case 0x0A://大梁电机高速
				if (ChangeSpeed(highSpeed))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);

				break;

			case 0x0B://大梁电机中速
				if (ChangeSpeed(middleSpeed))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;

			case 0x0C://大梁电机低速
				if (ChangeSpeed(lowSpeed))
					SendBack(com, IS_OK);
				else
					SendBack(com, IS_ERROR);
				break;
			}
			break;
		}
	}
	return 1;
}

/* ------------------------------------------------------------------------------
判断是否存在上位机中止指令,返回1终止扫描
------------------------------------------------------------------------------ */
extern uint8_t PC_Stop(void)
{
	//1-处理串口WIFI通信
	if (Usart2_rcv_flag)
	{
		Usart2_rcv_flag = 0;
		WIFI_Stop(wifi);
		return 1;
	}

	//2-处理串口以太网通信
	if (Uart5_rcv_flag)
	{
		Uart5_rcv_flag = 0;
		WIFI_Stop(networkModul);
		return 1;
	}
	return 0;
}

extern uint8_t Is_PCStop(DEVICE com)
{
	uint8_t cmd = 0xff;
	getCmdFrame(com, &cmd);
	if (cmd == 0x05)//0x0D,0x04
	{
		DebugMsg("上位机终止等待\r\n");
		PCBreakFlag = 1;
		return 1;
	}
	return 0;

}
