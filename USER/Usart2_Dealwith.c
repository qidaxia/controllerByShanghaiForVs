/*
串口2：WIFI串口接收到的数据处理

*/
#include "delay.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "uart5.h"
#include "led.h"
#include "beep.h"
#include "timer3.h"
#include "main.h"
#include "rtc.h"
#include "adc.h"

#define BAT_THRESHOLD   2200*4096/2/3000    //1501,低于2.2V报警 Vbat = D/4096*3000*2

//发送帧头
void UART2_SendHead(void)
{
	uart5_send_byte(COMM_HEAD1);
	uart5_send_byte(COMM_HEAD2);
	uart5_send_byte(COMM_HEAD3);
	uart5_send_byte(COMM_HEAD4);
	uart5_send_byte(COMM_HEAD5);
}

//发送帧尾
void UART2_SendEnd(void)
{
	uart5_send_byte(COMM_END1);
	uart5_send_byte(COMM_END2);
	uart5_send_byte(COMM_END3);
}

//发送正确执行反馈
void UART2_SendACK(void)
{
	UART5_SendHead();
	uart5_send_byte(0x14);
	UART5_SendEnd();
}

//发送错误接收反馈
void UART2_SendError(void)
{
	UART5_SendHead();
	uart5_send_byte(0x15);
	UART5_SendEnd();
}

//发送当前激光传感器距离
void UART2_SendLaser(void)
{
	UART5_SendHead();
	uart5_send_byte(0x17);
//	uart5_send_byte(Laser1_Value/256);
//	uart5_send_byte(Laser1_Value%256);
//	uart5_send_byte(Laser2_Value/256);
//	uart5_send_byte(Laser2_Value%256);
	UART5_SendEnd();
}


/*
WIFI数据处理
*/
u8 Usart2_WIFI_Dealwith(void)
{
    u16 adc_value;
    u8 bat_low;
    
	if( (UART5_RX_BUF[0] == COMM_HEAD1) && 
		(UART5_RX_BUF[1] == COMM_HEAD2) && 
		(UART5_RX_BUF[2] == COMM_HEAD3) && 
		(UART5_RX_BUF[3] == COMM_HEAD4) && 
		(UART5_RX_BUF[4] == COMM_HEAD5) )
	{
		switch(UART5_RX_BUF[5])
		{
			case 0x01:	//设置日期和时间
				RTC_Set_Date(UART5_RX_BUF[6],UART5_RX_BUF[7],UART5_RX_BUF[8],RTC_Weekday_Monday);   //RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
				RTC_Set_Time(UART5_RX_BUF[9],UART5_RX_BUF[10],0x00,RTC_H12_AM);	                    //RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
				UART2_SendACK();
			break;
			
			case 0x02:	//设置每天扫描时间
				RTC_Set_AlarmA(1,UART5_RX_BUF[6],UART5_RX_BUF[7],30);    //开启并设置闹钟，日期，时分秒
				UART2_SendACK();
			break;
			
			case 0x03:	//设定原点坐标
				X0 = UART5_RX_BUF[6]*256+UART5_RX_BUF[7];       //X原点坐标
				Y0 = UART5_RX_BUF[8]*256+UART5_RX_BUF[9];       //Y原点坐标
				Xmax = UART5_RX_BUF[10]*256+UART5_RX_BUF[11];   //X最大坐标
				Ymax = UART5_RX_BUF[12]*256+UART5_RX_BUF[13];   //Y最大坐标
				LineNum = UART5_RX_BUF[14];                     //行数

				RTC_WriteBackupRegister(RTC_BKP_DR4,X0);        //坐标原点X0大梁方向
				RTC_WriteBackupRegister(RTC_BKP_DR5,Y0);        //坐标原点Y0小车方向
				RTC_WriteBackupRegister(RTC_BKP_DR6,Xmax);      //对角坐标Xmax大梁方向
				RTC_WriteBackupRegister(RTC_BKP_DR7,Ymax);      //对角坐标Ymax小车方向
				RTC_WriteBackupRegister(RTC_BKP_DR8,LineNum);   //总行数

				RTC_WriteBackupRegister(RTC_BKP_DR9,0x5050);          //原点已标识
				Config_Flag = 1;
				
				UART5_SendHead();       //命令头 0x18 参数 命令尾
				uart5_send_byte(0x18);
				uart5_send_byte(0x01);
				UART5_SendEnd();
                
			break;
			
			case 0x04:	//大梁前进
				//JDQ2_OFF;
				delay_ms(300);
				//JDQ1_ON;
				UART2_SendACK();
			break;
			
			case 0x05:	//大梁后退
				//JDQ1_OFF;
				delay_ms(300);
				//JDQ2_ON;
				UART2_SendACK();
			break;
			
			case 0x06:	//大梁停止
				//JDQ1_OFF;
				//JDQ2_OFF;
				//JDQ3_OFF;
				//JDQ4_OFF;
				//JDQ5_OFF;   //上闸
				BreakFlag = 1;
				UART5_SendACK();
			break;
			
			case 0x07:	//小车前进
				//JDQ4_OFF;
				delay_ms(300);
				//JDQ3_ON;
				//JDQ5_ON;    //松闸
				UART5_SendACK();
			break;
			
			case 0x08:	//小车后退
				//JDQ3_OFF;
				delay_ms(300);
				//JDQ4_ON;
				//JDQ5_ON;    //松闸
				UART5_SendACK();
			break;
			
			case 0x09:	//小车停止
				//JDQ1_OFF;
				//JDQ2_OFF;
				//JDQ3_OFF;
				//JDQ4_OFF;
				//JDQ5_OFF;   //上闸
				BreakFlag = 1;
				UART2_SendACK();
			break;
			
			case 0x0A:	//未使用
				UART2_SendACK();
			break;
			
			case 0x0B:	//设置速度
				LowSpeed = UART5_RX_BUF[6]*256+UART5_RX_BUF[7];
				MidSpeed = UART5_RX_BUF[8]*256+UART5_RX_BUF[9];
				HighSpeed = UART5_RX_BUF[10]*256+UART5_RX_BUF[11];
				RTC_WriteBackupRegister(RTC_BKP_DR10,LowSpeed);     //低俗
				RTC_WriteBackupRegister(RTC_BKP_DR11,MidSpeed);     //中速
				RTC_WriteBackupRegister(RTC_BKP_DR12,HighSpeed);    //高速             
				UART2_SendACK();
			break;
			
			case 0x0C:	//切换到低速
				NowSpeed = 1;      //当前电机速度
				UART2_SendACK();
			break;
			
			case 0x0D:	//切换到中速
				NowSpeed = 2;      //当前电机速度
				UART2_SendACK();
			break;
			
			case 0x0E:	//切换到高速
				NowSpeed = 3;      //当前电机速度
				UART2_SendACK();
			break;
			
			case 0x0F:	//握手信号
				adc_value = Get_Adc_Average(9,5); //纽扣电池电压
				if(adc_value>=BAT_THRESHOLD)
						bat_low = 1;
				 else
						bat_low = 2;
            
				UART5_SendHead();
				uart5_send_byte(0x16);
				uart5_send_byte(Config_Flag);   //原点已标识标志
				uart5_send_byte(bat_low);   	//原点已标识标志
				uart5_send_byte(NowSpeed);      //当前电机速度
				UART5_SendEnd();
			break;

			case 0x10:	//开启/关闭定时任务
				RTC_AlarmCmd(RTC_Alarm_A,DISABLE);//关闭闹钟A
			break;
			
			case 0x11:	//扫描矩形区域
				ScanPart_X0 = UART5_RX_BUF[6]*256 + UART5_RX_BUF[7];    //起始点X坐标
				ScanPart_Xstep = UART5_RX_BUF[8]*256 + UART5_RX_BUF[9]; //行扫描长度
				ScanPart_StartLine = UART5_RX_BUF[10];                  //起始行
				ScanPart_LineNum = UART5_RX_BUF[11];                    //行数
				ScanPart_SW = 1;
			break;
			
			case 0x12:	//激光传感器开
//				LaserFB = 1;
//				Laser1_ON = 1;
//				Laser2_ON = 1;
				UART5_SendACK();
			break;
			
			case 0x13:	//激光传感器关
//				LaserFB = 0;
//				Laser1_ON = 0;
//				Laser2_ON = 0;
				usart2_send_byte('C');
				usart3_send_byte('C');
				UART2_SendACK();
			break;
            
			case 0x40:	//立即自动扫描一次
				PlanTask_SW = 1;
			break;
            
			case 0x41:	//读取单片机工作状态
				UART5_SendHead();
				uart5_send_byte(0x19);
        uart5_send_byte(WorkStatus); 
				UART2_SendEnd();
			break;           
            
      case 0x42:	//执行一次大梁x移动，调试指令
				X_goto = UART5_RX_BUF[6]*256 + UART5_RX_BUF[7]; //X目标坐标
				Xgoto_SW = 1;
				UART2_SendACK();
			break;
            
      case 0x43:	//执行一次小车y移动，调试指令
				Y_goto = UART5_RX_BUF[6]*256 + UART5_RX_BUF[7]; //Y目标坐标
				Ygoto_SW = 1;
				UART2_SendACK();
			break;
           
		}
	}
	else
	{
		UART2_SendError();	//校验错误，请求重发
	}
	return 1;
}
