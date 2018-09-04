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
#include "rtc.h"
#include "adc.h"
#include "design.h"
#include "toSensor.h"

uint32_t x = 0;
void Test(void)
{
	uint8_t i;
	uint8_t wrdata[10] = {1, 1, 1, 2, 5, 6, 7, 8, 9, 10};
	//uint8_t wrdata2[10] = { 11,12,13,14,15,16,17,18,19,20 };
	uint8_t rddata[10];

	x = (wrdata[0] << 24) + (wrdata[1] << 16) + (wrdata[2] << 8) + wrdata[3];
	//while(1);

	//1-���Ա��ݼĴ���
	while (0)
	{
		//int8_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );//д���ݵ�����SRAM
		//write_to_backup_sram(wrdata,10,0);
		delay_ms(500);
		for (i = 0; i < 10; i++)
			rddata[i] = 0;
		//int8_t read_from_backup_sram(uint8_t *data, uint16_t bytes, uint16_t offset);//�ӱ���SRAM��ȡ����
		read_from_backup_sram(rddata, 10, 0);
		delay_ms(500);

		//int8_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );//д���ݵ�����SRAM
		//write_to_backup_sram(wrdata2,10,0);
		delay_ms(500);
		for (i = 0; i < 10; i++)
			rddata[i] = 0;
		//int8_t read_from_backup_sram(uint8_t *data, uint16_t bytes, uint16_t offset);//�ӱ���SRAM��ȡ����
		read_from_backup_sram(rddata, 10, 0);
		delay_ms(500);
	}

	//2-����LED��SPK
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

	//3-����5
	while (0)
	{
		uart5_send_byte('A');
		uart5_send_byte('B');
		uart5_send_byte('C');
		delay_ms(1000);
	}

	//4-����6
	while (0)
	{
		usart6_send_byte('A');
		delay_ms(500);
	}

	//5-С������
	while (0)
	{
		MotorMove(ID_XIAOCHE, 1); //��ǰ
		delay_ms(5000);
		MotorMove(ID_XIAOCHE, 3); //ֹͣ
		delay_ms(5000);
		MotorMove(ID_XIAOCHE, 2); //���
		delay_ms(5000);
		MotorMove(ID_XIAOCHE, 3); //ֹͣ
		delay_ms(5000);

		MotorMove(ID_DALIANG, 1); //��ǰ
		delay_ms(5000);
		MotorMove(ID_DALIANG, 3); //ֹͣ
		delay_ms(5000);
		MotorMove(ID_DALIANG, 2); //���
		delay_ms(5000);
		MotorMove(ID_DALIANG, 3); //ֹͣ
		delay_ms(5000);
	}

	while (0) //�ƶ���ָ��λ��
	{
		MotorToPosition(ID_XIAOCHE, 60);
		delay_ms(1000);
		MotorToPosition(ID_DALIANG, 60);
		while (1)
			;
	}

	while (0) //��������
	{
		i = MotorToZero(ID_XIAOCHE);
		if (i)
		{
			BEEP_ON;
			delay_ms(500);
			BEEP_OFF;
		}
		while (1)
			;
	}

	while (0) //��ȡ����λ��
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

		while (1)
	{
		SendBatteryVoltage(2);
	}
