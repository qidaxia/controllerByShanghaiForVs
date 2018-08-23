#include "sys.h"
#include "delay.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "uart5.h"
#include "usart6.h"
#include "led.h"
#include "beep.h"
#include "timer3.h"
#include "main.h"
#include "design.h"

volatile uint8_t XiaoChe_Now_Direction;	//0x01向前、0x02向后，0x03停止
volatile uint32_t XiaoChe_Now_Position;
volatile uint8_t DaLiang_Now_Direction;	//0x01向前、0x02向后，0x03停止
volatile uint32_t DaLiang_Now_Position;

//指令间等待程序，两个指令间必须加延时，否则会被当做一次传输
void LoraWait(void)
{
	LED2_ON;        //蓝灯闪烁一次
	delay_ms(100);
	LED2_OFF;
}


/* -------------------------------------------------------------------------------
1- 电机控制命令,定点发送，小车ID=100，大梁ID=200
方向（0x01向前、0x02向后，0x03停止）
------------------------------------------------------------------------------- */
RETCODE waitLoraAck(void);

RETCODE MotorMove(uint8_t ID, uint8_t dir)
{
	u8 tempBuf[5] = { 0x00,0x00,72,0x01,0x00 };

	tempBuf[0] = ID / 256;
	tempBuf[1] = ID % 256;
	LoraWait();
	loraClear();
	if (ID!=ID_XIAOCHE&&ID!=ID_DALIANG)
	{
		return RET_ERR;
	}
	tempBuf[4] = dir;

	SendBuff(lora, tempBuf, 3);
	SendBuff(lora, cmdStart, 5);
	SendBuff(lora, tempBuf+3, 2);
	SendBuff(lora, cmdEnd, 3);
	return waitLoraAck();
}

RETCODE waitLoraAck(void)
{
	u8 i = 0;
	while (Usart6_rcv_index < 3)
	{
		delay_ms(2);
		i++;
		if (i >= 500)
		{
			return RET_ERR;
		}
	}
	if ((Usart6_rcv_buf[0] == 'A') && (Usart6_rcv_buf[2] == 'K'))
		return RET_OK;
	else
		return RET_ERR;
}

/* -------------------------------------------------------------------------------
2- 运动到指定位置,定点发送，小车ID=100，大梁ID=200
三字节位置数据，高字节在前，低字节在后，最高位+1，为避免使用0
------------------------------------------------------------------------------- */
RETCODE MotorToPosition(uint8_t ID, uint32_t pos)
{
	uint8_t check, send_times, send_buf[20];
	loraClear();
	if (ID != ID_XIAOCHE&&ID != ID_DALIANG)
	{
		return RET_ERR;
	}

	send_buf[0] = ID / 256;
	send_buf[1] = ID % 256;
	send_buf[2] = 72;
	send_buf[3] = COMM_HEAD1;
	send_buf[4] = COMM_HEAD2;
	send_buf[5] = COMM_HEAD3;
	send_buf[6] = COMM_HEAD4;
	send_buf[7] = COMM_HEAD5;
	send_buf[8] = 0x11;	//指令
	send_buf[9] = (pos >> 16) + 1;
	send_buf[10] = pos >> 8;
	send_buf[11] = pos;
	check = send_buf[9] + send_buf[10] + send_buf[11];
	send_buf[12] = check;
	send_buf[13] = COMM_END1;
	send_buf[14] = COMM_END2;
	send_buf[15] = COMM_END3;

	send_times = 5;
	while (send_times)		//超时或收到错误时，重发5次
	{
		LoraWait();
		loraClear();
		SendBuff(lora, send_buf, 16);

		if (waitLoraAck() == RET_OK)
		{
			return RET_OK;
		}
	}
	return RET_ERR;
}

/*
3- 运动到零点,定点发送，小车ID=100，大梁ID=200
*/
RETCODE MotorToZero(uint8_t ID)
{
	uint8_t send_buf[20], send_times;

	loraClear();

	if (ID != ID_XIAOCHE && ID != ID_DALIANG)
	{
		return RET_ERR;
	}

	send_buf[0] = ID / 256;
	send_buf[1] = ID % 256;
	send_buf[2] = 72;

	send_buf[3] = COMM_HEAD1;
	send_buf[4] = COMM_HEAD2;
	send_buf[5] = COMM_HEAD3;
	send_buf[6] = COMM_HEAD4;
	send_buf[7] = COMM_HEAD5;
	send_buf[8] = 0x02;	//指令
	send_buf[9] = COMM_END1;
	send_buf[10] = COMM_END2;
	send_buf[11] = COMM_END3;

	send_times = 5;
	while (send_times--)
	{
		LoraWait();
		SendBuff(lora, send_buf, 12);
		loraClear();
		if (waitLoraAck() == RET_OK)
		{
			return RET_OK;
		}
	}
	return RET_ERR;
}

/* -------------------------------------------------------------------------------
4- 读取当前状态,定点发送，小车ID=100，大梁ID=200
------------------------------------------------------------------------------- */
RETCODE ReadStatus(uint8_t ID)
{
	uint8_t check;
	u8 tempBuf[4] = { 0x00,0x00,0x72,0x00 };
	uint8_t dir;
	uint32_t pos;

	if (ID != ID_DALIANG && ID != ID_XIAOCHE)
	{
		return RET_ERR;
	}

	loraClear();
	LoraWait();
	tempBuf[0] = ID / 256;
	tempBuf[1] = ID % 256;
	tempBuf[3] = 0x03;//状态读取指令
	SendBuff(lora, tempBuf, 3);

	SendBuff(lora, cmdStart, 5);
	SendBuff(lora, tempBuf + 3, 1);
	SendBuff(lora, cmdEnd, 3);


	Usart6_rcv_buf[0] = '0';
	delay_ms(1000);

	if ((Usart6_rcv_buf[0] == 's') && (Usart6_rcv_buf[1] == 't') && (Usart6_rcv_buf[12] == 'd'))
	{
		check = Usart6_rcv_buf[6] + Usart6_rcv_buf[7] + Usart6_rcv_buf[8];
		if (Usart6_rcv_buf[9] == check)	//判断校验
		{
			dir = Usart6_rcv_buf[5];
			pos = Usart6_rcv_buf[6] - 1;		//最高位+1，避免发送0时模块容易丢失
			pos <<= 8;
			pos += Usart6_rcv_buf[7];
			pos <<= 8;
			pos += Usart6_rcv_buf[8];

			if (ID == ID_XIAOCHE)
			{
				XiaoChe_Now_Direction = dir;
				XiaoChe_Now_Position = pos;
			}
			else
			{
				DaLiang_Now_Direction = dir;
				DaLiang_Now_Position = pos;
			}

			/* 发送小车和大梁位置 */
			DebugMsg("小车状态：");
			switch (XiaoChe_Now_Direction)
			{
			case 1:
				DebugMsg("前进，坐标：");
				break;
			case 2:
				DebugMsg("后退，坐标：");
				break;
			case 3:
				DebugMsg("静止，坐标：");
				break;

			}
			DebugNum(XiaoChe_Now_Position);
			DebugMsg("\r\n");

			DebugMsg("大梁状态：");
			switch (DaLiang_Now_Direction)
			{
			case 1:
				DebugMsg("前进，坐标：");
				break;
			case 2:
				DebugMsg("后退，坐标：");
				break;
			case 3:
				DebugMsg("静止，坐标：");
				break;
			}
			DebugNum(DaLiang_Now_Position);
			DebugMsg("\r\n");
			return RET_OK;
		}
		else
			return RET_ERR;
	}
	else
		return RET_ERR;
}

/* -------------------------------------------------------------------------------
5- 小车输出5V信号，只对小车有效
命令（04）+参数（1持续电平，2脉冲，3其它）
返回：ACK
------------------------------------------------------------------------------- */
uint8_t SetXiaoChe_5V_Level(uint8_t levelmode)
{
	u8 tempBuf[2] = { 0x00,0x00 };

	loraClear();
	LoraWait();
	tempBuf[0] = 0x04;
	tempBuf[1] = levelmode;

	SendBuff(lora, cmdIdCar, 3);
	SendBuff(lora, cmdStart, 5);
	SendBuff(lora, tempBuf, 2);
	SendBuff(lora, cmdEnd, 3);

	return waitLoraAck();
}

/* -------------------------------------------------------------------------------
6- 小车结束5V信号，只对小车有效
命令（07）+参数
返回：ACK
------------------------------------------------------------------------------- */
uint8_t SetXiaoChe_0V_Level(void)
{
	u8 tempBuf[1] = { 0x00 };

	loraClear();
	LoraWait();

	SendBuff(lora, cmdIdCar, 3);
	SendBuff(lora, cmdStart, 5);
	SendBuff(lora, tempBuf, 1);
	SendBuff(lora, cmdEnd, 3);

	return waitLoraAck();
}


/* -------------------------------------------------------------------------------
7- 切换速度（只对大梁有效）
 命令（06）+参数（0低速，1中速，2高速）
返回：ACK
------------------------------------------------------------------------------- */
uint8_t ChangeSpeed(SPEED speed)
{
	u8 tempBuf[2] = { 0x00,0x00 };
	loraClear();
	LoraWait();

	tempBuf[0] = 0x06;
	tempBuf[1] = (u8)speed;

	SendBuff(lora, cmdIdBridge, 3);
	SendBuff(lora, cmdStart, 5);
	SendBuff(lora, tempBuf, 2);
	SendBuff(lora, cmdEnd, 3);

	return waitLoraAck();
}

