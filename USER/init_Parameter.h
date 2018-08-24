#ifndef INITPARAMS_H
#define INITPARAMS_H
#include "design.h"


extern uint8_t DebugFlag;
extern u8 WorkStatus;					//工作状态
extern u8 BreakFlag;					//中断执行标志
extern uint8_t  PCBreakFlag;			//上位机终止当前任务
extern LEVELMODE paulseStyle;				//脉冲输出方式
extern uint16_t Scan_Interval;			//扫描时间间隔，单位分钟
extern uint16_t Scan_Times;				//当日扫描次数
extern uint8_t  Scan_SW;				//任务扫描开关
extern uint8_t	Scan_Day;				//扫描日期，日期变化，扫描次数清零


extern RTC_TimeTypeDef RTC_TimeStruct;
extern RTC_DateTypeDef RTC_DateStruct;

extern volatile MOVECMD XiaoChe_Now_Direction;	//0x01向前、0x02向后，0x03停止，04归零过程中
extern volatile uint32_t XiaoChe_Now_Position;
extern volatile MOVECMD DaLiang_Now_Direction;
extern volatile uint32_t DaLiang_Now_Position;

extern void init_Parameter(void);


#endif
