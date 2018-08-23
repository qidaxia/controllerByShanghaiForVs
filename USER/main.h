#ifndef __MAIN_H
#define __MAIN_H	 
#include "sys.h" 
#include "stdio.h"

void Test(void);    //测试程序
extern RTC_TimeTypeDef RTC_TimeStruct;
extern RTC_DateTypeDef RTC_DateStruct;
extern uint8_t DebugFlag;
extern u8 Config_Flag;      //单片机已标识标志
extern u8 WorkStatus;       //工作状态

void init_Parameter(void);	//初始化运行参数

//和PC通信配置相关函数
#define	COMM_HEAD1	0x73	//start
#define	COMM_HEAD2	0x74
#define	COMM_HEAD3	0x61
#define	COMM_HEAD4	0x72
#define	COMM_HEAD5	0x74
#define COMM_END1	0x65	//end
#define COMM_END2	0x6E
#define COMM_END3	0x64
#define ID_XIAOCHE	100
#define ID_DALIANG  200
#define ID_DEBUG		300
#define BASEADDR_ROW	500
#define BASEADDR_COL_FIRST	59
#define BASEADDR_COL_LAST	63
#define BASEADDR_COLUMN_BIT	100

#define PRECISION	30		//30个脉冲误差
#define DALIANG_TIMEOUT	600		//大梁运动超时
#define XIAOCHE_TIMEOUT 200		//小车运动超时

u8 WIFI_Dealwith(DEVICE com);	//WIFI数据处理
uint8_t WIFI_Stop(uint8_t com);	//WIFI数据处理，急停数据
uint8_t PC_Stop(void);			//判断是否存在上位机中止指令,返回1则终止扫描

//植物扫描配置相关参数
#define CAMERATIME	1							//按位扫描相机暂停时间

extern u8 BreakFlag;              //中断执行标志
extern uint8_t paulseStyle;//脉冲输出方式


extern uint16_t Scan_Interval;		//扫描时间间隔，单位分钟


extern uint16_t Scan_Times;				//当日扫描次数
extern uint8_t	Scan_Day;					//扫描日期，日期变化，扫描次数清零

void SendBatteryVoltage(uint8_t com);//发送纽扣电池电压值
extern uint8_t  Scan_SW;						 //任务扫描开关
extern uint8_t  PCBreakFlag;			 //上位机终止当前任务

void PlanTask(void);			  //判断扫描计划任务
uint8_t Scan_Full(void);		//全面扫描 
uint8_t Scan_Part(void);		//区域扫描 
uint8_t Scan_Row(uint8_t row);//扫描指定行 
uint8_t Scan_Column(uint16_t column);//扫描指定列

//小车和大梁控制函数
extern volatile uint8_t XiaoChe_Now_Direction;	//0x01向前、0x02向后，0x03停止，04归零过程中
extern volatile uint32_t XiaoChe_Now_Position;
extern volatile uint8_t DaLiang_Now_Direction;
extern volatile uint32_t DaLiang_Now_Position;
extern uint8_t NowSpeed; //当前大梁电机速度
uint8_t MotorMove(uint8_t ID,uint8_t dir);
uint8_t MotorToPosition(uint8_t ID,uint32_t pos);
uint8_t MotorToZero(uint8_t ID);
uint8_t ReadStatus(uint8_t ID);
uint8_t SetXiaoChe_5V_Level(uint8_t levelmode);
uint8_t SetXiaoChe_0V_Level(void);
uint8_t CommTest(uint8_t ID);
uint8_t ChangeSpeed(uint8_t speed);

#endif


