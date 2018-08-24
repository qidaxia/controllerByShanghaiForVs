#include "design.h"
#include "beep.h"
#include "PC_Dealwith.h"
#include "toSensor.h"
#include "init_Parameter.h"
#include "PlanTask.h"


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
	usart2_init(9600);
	return;
}

static void loraSendBuff(u8 *buf, u8 len);

static  void wifiSendBuff(u8 *buf, u8 len);

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

static void loraSendB(u8 dat)
{
	usart6_send_byte(dat);
}

static void wifiSendB(u8 dat)
{
	usart2_send_byte(dat);
	return;
}

static void loraSendBuff(u8 *buf, u8 len)
{
	u8 i = 0;
	for (i = 0; i < len; i++)
	{
		loraSendB(*(buf + i));
	}
}

static void wifiSendBuff(u8 *buf, u8 len)
{
	u8 i = 0;
	for (i = 0; i < len; i++)
	{
		wifiSendB(*(buf + i));
	}
}

extern void DebugMsg(uint8_t *str)
{
	if (!DebugFlag)
	{
		return;
	}
	SendBuff(lora, cmdIdDebug, 3);
	while (*str != 0)
	{
		usart6_send_byte(*str);
		str++;
	}
}

extern void DebugNum(uint32_t num)
{
	if (!DebugFlag)
	{
		return;
	}
	SendBuff(lora, cmdIdDebug, 3);
	printf("%d", num);
}

extern int fputc(int ch, FILE *f)
{
	while ((USART6->SR & 0X40) == 0);//循环发送,直到发送完毕   
	USART6->DR = (u8)ch;
	return ch;
}

extern u8 getReciveLen(DEVICE com)
{
	return com == wifi ? USART2_BUF_Index : Usart6_rcv_index;
}

extern void clearReciveBuf(DEVICE com)
{
	if (com == wifi)
	{
		USART2_BUF_Index = 0;
		USART2_RX_BUF[0] = 0;
	}
	else
	{
		Usart6_rcv_index = 0;
		USART6_RX_BUF[0] = 0;
	}

	return;
}


//寻找命令头，失败返回0xff,成功返回对应的索引（s所在的索引）
static u8 searchCMDHead(DEVICE com, u8 len)
{
	u8 i = 0;
	u8 headIndex = 0xff;
	u8 *dataStr = (void *)0;
	dataStr = com == wifi ? USART2_RX_BUF : USART6_RX_BUF;
	for (i = 0; i < len - 4; i++)
	{
		if (dataStr[i] == 's'&&dataStr[i + 1] == 't'&&dataStr[i + 2] == 'a'&&dataStr[i + 3] == 'r'&&dataStr[i + 4] == 't')
		{
			headIndex = i;
			break;
		}
	}
	return headIndex;
}

extern uint8_t * getCmdFrame(DEVICE com, uint8_t *cmd)
{
	u8 len = getReciveLen(com);
	u8 headIndex = 0xff;
	u8 *dataStr = (void *)0;

	*cmd = 0xff;
	if (len < 9)//数据不完整
	{
		return dataStr;
	}
	if (len > 100)//错误数据
	{
		clearReciveBuf(com);
		return dataStr;
	}
	dataStr = com == wifi ? USART2_RX_BUF : USART6_RX_BUF;

	//验证命令尾
	if (dataStr[len - 1] == 'd'&&dataStr[len - 2] == 'n'&&dataStr[len - 3] == 'e')
	{
		//寻找命令头
		headIndex = searchCMDHead(com, len);
		if (headIndex != 0xff)//找到了
		{
			BeepOne();
			*cmd = dataStr[headIndex + 5];
		}
		//数据不完整，或处理完成，则重新开始接收
		clearReciveBuf(com);
	}
	else//验证命令尾不通过，重新接收
	{
		clearReciveBuf(com);
	}
	return dataStr + headIndex + 5;
}

extern void handlerFrame(DEVICE com, uint8_t cmd, uint8_t * dataStr)
{
	uint8_t write_ram_buf[50];
	u8 i = 0;
	if (cmd == 0xff || dataStr == (void *)0)
	{
		return;
	}
	switch (cmd)
	{
		//1-设置日期和时间
	case 0x0A: //年(67),月(8),日(9),时(10),分(11),秒(12),星期(13)（周日为7）
			   //RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)	
		PWR_BackupAccessCmd(ENABLE);
		RTC_Set_Date((*(dataStr + 1) + (*(dataStr + 2) << 8)) - 2000, *(dataStr + 3), *(dataStr + 4), *(dataStr + 8));
		//RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
		RTC_Set_Time(*(dataStr + 5), *(dataStr + 6), *(dataStr + 7), RTC_H12_AM);
		PWR_BackupAccessCmd(DISABLE);
		SendBack(com, IS_OK);
		break;

		//2-地图设置（行数，列数）
	case 0x0C://行数(4个字节，LSB),列数(4个字节，LSB)
		write_ram_buf[0] = 0;
		write_ram_buf[1] = 0;
		write_ram_buf[2] = *(dataStr + 2);
		write_ram_buf[3] = *(dataStr + 1);
		map.Plant_Row = (*(dataStr + 2) << 8) + *(dataStr + 1);
		write_to_backup_sram(write_ram_buf, 4, 11); //行数

		write_ram_buf[0] = 0;
		write_ram_buf[1] = 0;
		write_ram_buf[2] = *(dataStr + 6);
		write_ram_buf[3] = *(dataStr + 5);
		map.Plant_Column = (*(dataStr + 6) << 8) + *(dataStr + 5);
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
			write_to_backup_sram(write_ram_buf, 4, BASEADDR_ROW + 4 * (*(dataStr + 1) - 1)); //行坐标
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

			if (*(dataStr + 1) == 0x01)
			{
				map.Column_First = DaLiang_Now_Position;//起始列坐标
				write_to_backup_sram(write_ram_buf, 4, BASEADDR_COL_FIRST); //列数
				SendBackValue(com, map.Column_First);
				map.Column_Interval = (map.Column_Last - map.Column_First) / map.Plant_Column;
			}
			else if (*(dataStr + 1) == 0x02)
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
		switch (*(dataStr + 1))
		{
		case 0x00://前进（小车前进，Y变大）

			if (MotorMove(ID_XIAOCHE, forward) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x01://后退（小车后退，Y变小）
			if (MotorMove(ID_XIAOCHE, back) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x02://左（大梁后退，X变小）
			if (MotorMove(ID_DALIANG, back) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x03://右（大梁前进，X变大）
			if (MotorMove(ID_DALIANG, forward) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x04://停止（大梁小车全部停止）
			if ((uint8_t)MotorMove(ID_DALIANG, stop) + (uint8_t)MotorMove(ID_XIAOCHE, stop) < 2)
			{
				SendBack(com, IS_ERROR);
			}
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
			if (MotorToZero(ID_DALIANG) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x09://Y小车返回到原点
			if (MotorToZero(ID_XIAOCHE) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x0A://大梁电机高速
			if (ChangeSpeed(highSpeed) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x0B://大梁电机中速
			if (ChangeSpeed(middleSpeed) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;

		case 0x0C://大梁电机低速
			if (ChangeSpeed(lowSpeed) == RET_ERR)
				SendBack(com, IS_ERROR);
			break;
		}
		break;

		//6-手动立即扫描控制
	case 0x0F:
		WorkStatus = 1;
		SendBack(com, IS_OK);
		if (*(dataStr + 1) == 0)
		{
			Scan_Part();
		}
		else if (*(dataStr + 1) == 1)
		{
			Scan_Full();
		}
		else if (*(dataStr + 1) == 2)
		{
			Scan_Row(*(dataStr + 2));
		}
		else if (*(dataStr + 1) == 3)
		{
			Scan_Column(*(dataStr + 2) + (*(dataStr + 3) << 8));
		}
		WorkStatus = 0;
		break;

		//计划任务扫描控制参数
	case 0x0B:
		paulseStyle = (LEVELMODE)(*(dataStr + 1));//脉冲输出方式
		scanStartTime.Hour = *(dataStr + 2);				//起始时间：时
		scanStartTime.Minute = *(dataStr + 3);				//起始时间：分
		scanStopTime.Hour = *(dataStr + 4);					//结束时间：时
		scanStopTime.Minute = *(dataStr + 5);				//结束时间：分
		Scan_Interval = *(dataStr + 6) + (*(dataStr + 7) << 8); //扫描时间间隔
		scanRepeatStyle = (ScanRepeatStyle)*(dataStr + 10);	//扫描重复方式

		write_ram_buf[0] =(uint8_t) paulseStyle;					//脉冲输出方式
		write_ram_buf[1] = scanStartTime.Hour;			//起始时间：时
		write_ram_buf[2] = scanStartTime.Minute;       //起始时间：分
		write_ram_buf[3] = scanStopTime.Hour;           //结束时间：时
		write_ram_buf[4] = scanStopTime.Minute;         //结束时间：分
		write_ram_buf[5] = *(dataStr + 6);					 //扫描时间间隔
		write_ram_buf[6] = *(dataStr + 7);
		write_ram_buf[7] = *(dataStr + 8);
		write_ram_buf[8] = *(dataStr + 9);
		write_ram_buf[9] = *(dataStr + 10);					//扫描重复方式
		write_to_backup_sram(write_ram_buf, 10, 67); //写入数据RAM
		SendBack(com, IS_OK);
		break;

		//行扫描位设定
	case 0x03:
		for (i = 0; i < *(dataStr + 2) + 1; i++)
		{
			write_ram_buf[i] = *(dataStr + i + 2);	//数据长度rcv_buf[6]
		}
		write_to_backup_sram(write_ram_buf, *(dataStr + 2) + 1, BASEADDR_COLUMN_BIT + 40 * (*(dataStr + 1) - 1)); //起始地址100,每行最多40字节
		delay_ms(20);			//延时等待上次操作完成
		SendBack(com, IS_OK);
		break;

		//读取纽扣电池电压值
	case 0x04:
		SendBatteryVoltage(com);
		break;

		//上位机终止当前任务
	case 0x05:
		PCBreakFlag = 1;
		DebugMsg("上位机终止\r\n");
		break;

		//扫描任务开关
	case 0x06:
		Scan_SW = *(dataStr + 1); 	//参数1：1允许扫描，0禁止扫描
		write_ram_buf[0] = *(dataStr + 1);
		write_to_backup_sram(write_ram_buf, 1, 77); //自动扫描任务开关
		SendBack(com, IS_OK);
		break;
	}
	return;
}

