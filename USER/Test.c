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
#include "rtc.h"
#include "adc.h"
#include "design.h"
uint32_t x = 0;
void Test(void)
{
	uint8_t i;
	uint8_t wrdata[10] = { 1,1,1,2,5,6,7,8,9,10 };
	//uint8_t wrdata2[10] = { 11,12,13,14,15,16,17,18,19,20 };
	uint8_t rddata[10];


	x = (wrdata[0] << 24) + (wrdata[1] << 16) + (wrdata[2] << 8) + wrdata[3];
	//while(1);

//1-测试备份寄存器
	while (0)
	{
		//int8_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );//写数据到备份SRAM
		//write_to_backup_sram(wrdata,10,0);
		delay_ms(500);
		for (i = 0; i < 10; i++)rddata[i] = 0;
		//int8_t read_from_backup_sram(uint8_t *data, uint16_t bytes, uint16_t offset);//从备份SRAM读取数据
		read_from_backup_sram(rddata, 10, 0);
		delay_ms(500);

		//int8_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );//写数据到备份SRAM
		//write_to_backup_sram(wrdata2,10,0);
		delay_ms(500);
		for (i = 0; i < 10; i++)rddata[i] = 0;
		//int8_t read_from_backup_sram(uint8_t *data, uint16_t bytes, uint16_t offset);//从备份SRAM读取数据
		read_from_backup_sram(rddata, 10, 0);
		delay_ms(500);
	}

	//2-测试LED和SPK
	while (0)
	{
		LED1_ON;
		LED2_ON;
		//BEEP_ON;
		delay_ms(1000);
		LED1_OFF;
		LED2_OFF;
		BEEP_OFF;
		delay_ms(1000);
	}

	//3-串口5
	while (0)
	{
		uart5_send_byte('A');
		uart5_send_byte('B');
		uart5_send_byte('C');
		delay_ms(1000);
	}

	//4-串口6
	while (0)
	{
		usart6_send_byte('A');
		delay_ms(500);
	}

	//5-小车测试
	while (0)
	{
		MotorMove(ID_XIAOCHE, 1);	//向前
		delay_ms(5000);
		MotorMove(ID_XIAOCHE, 3);	//停止
		delay_ms(5000);
		MotorMove(ID_XIAOCHE, 2);	//向后
		delay_ms(5000);
		MotorMove(ID_XIAOCHE, 3);	//停止
		delay_ms(5000);

		MotorMove(ID_DALIANG, 1);	//向前
		delay_ms(5000);
		MotorMove(ID_DALIANG, 3);	//停止
		delay_ms(5000);
		MotorMove(ID_DALIANG, 2);	//向后
		delay_ms(5000);
		MotorMove(ID_DALIANG, 3);	//停止
		delay_ms(5000);
	}

	while (0)	//移动到指定位置
	{
		MotorToPosition(ID_XIAOCHE, 60);
		delay_ms(1000);
		MotorToPosition(ID_DALIANG, 60);
		while (1);
	}

	while (0)	//坐标清零
	{
		i = MotorToZero(ID_XIAOCHE);
		if (i)
		{
			BEEP_ON;
			delay_ms(500);
			BEEP_OFF;
		}
		while (1);
	}

	while (0)	//读取坐标位置
	{
		i = ReadStatus(ID_XIAOCHE);
		if (i)
		{
			BEEP_ON;
			delay_ms(100);
			BEEP_OFF;
		}
		delay_ms(2000);
	}

	while (0)
	{
		i = SetXiaoChe_5V_Level(1);
		delay_ms(2000);
		i = SetXiaoChe_0V_Level();
		delay_ms(2000);
		i = SetXiaoChe_5V_Level(2);
		delay_ms(2000);
	}

	while (0)
	{
		i = ChangeSpeed(0);
		delay_ms(3000);
		i = ChangeSpeed(1);
		delay_ms(3000);
		i = ChangeSpeed(2);
		delay_ms(3000);
	}

	//纽扣电池电压检测
	while (1)
	{
		SendBatteryVoltage(2);
	}

	/*while (1)
	{
		DebugMsg("i的当前值为：");
		DebugNum(100);
		DebugMsg("\r\n");
		delay_ms(2000);
	}*/
}

