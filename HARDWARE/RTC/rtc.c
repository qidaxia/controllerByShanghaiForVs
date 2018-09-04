#include "rtc.h"
#include "led.h"
#include "delay.h"


//RTC时间设置
//hour,min,sec:小时,分钟,秒钟
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC日期设置
//year,month,date:年(0-99),月(1-12),日(0-31)
//week:星期(1-7,0,非法!)
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}

//RTC初始化
//返回值:0,初始化成功;
//       1,LSE开启失败;
//       2,进入初始化模式失败;
u8 My_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry=0X1FFF; 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);//启用备份SRAM时钟
	PWR_BackupRegulatorCmd(ENABLE); //启用备份SRAM的低功耗稳压器，以保持它在VBAT模式的内容
    
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)		//是否第一次配置?
	{
		RCC_LSEConfig(RCC_LSE_ON);//LSE 开启    
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
			{
			retry++;
			delay_ms(10);
			}
		if(retry==0)return 1;		//LSE 开启失败. 
			
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
		RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟 

		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTC同步分频系数(0~7FFF)
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC设置为,24小时格式
		RTC_Init(&RTC_InitStructure);
	 
		RTC_Set_Time(23,59,56,RTC_H12_AM);	//设置时间
		RTC_Set_Date(18,4,7,6);		//设置日期
	 
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//标记已经初始化过了
	} 
 
	return 0;
}

/*
设置闹钟，按日期闹，不关心日期和星期几，只和时间有关
日期取值0-31
*/
void RTC_Set_AlarmA(u8 date,u8 hour,u8 min,u8 sec)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmTypeInitStructure;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);//关闭闹钟A 

	RTC_TimeTypeInitStructure.RTC_Hours=hour;//小时
	RTC_TimeTypeInitStructure.RTC_Minutes=min;//分钟
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;//秒
	RTC_TimeTypeInitStructure.RTC_H12=RTC_H12_AM;
	RTC_AlarmTypeInitStructure.RTC_AlarmTime=RTC_TimeTypeInitStructure;

	RTC_AlarmTypeInitStructure.RTC_AlarmMask=RTC_AlarmMask_DateWeekDay; //不关心是星期几/每月哪一天，只和时分秒有关
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel=RTC_AlarmDateWeekDaySel_Date;//按日期闹
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay=date;//日期
	
	RTC_SetAlarm(RTC_Format_BIN,RTC_Alarm_A,&RTC_AlarmTypeInitStructure);


	RTC_ClearITPendingBit(RTC_IT_ALRA);//清除RTC闹钟A的标志
	EXTI_ClearITPendingBit(EXTI_Line17);//清除LINE17上的中断标志位 

	RTC_ITConfig(RTC_IT_ALRA,ENABLE);//开启闹钟A中断
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);//开启闹钟A 

	EXTI_InitStructure.EXTI_Line = EXTI_Line17;//LINE17
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE17
	EXTI_Init(&EXTI_InitStructure);//配置

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置
}

//周期性唤醒定时器设置  
/*wksel:  @ref RTC_Wakeup_Timer_Definitions
#define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
#define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
#define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
#define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
#define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
#define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)
*/
//cnt:自动重装载值.减到0,产生中断.
void RTC_Set_WakeUp(u32 wksel,u16 cnt)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	RTC_WakeUpCmd(DISABLE);//关闭WAKE UP

	RTC_WakeUpClockConfig(wksel);//唤醒时钟选择

	RTC_SetWakeUpCounter(cnt);//设置WAKE UP自动重装载寄存器

	RTC_ClearITPendingBit(RTC_IT_WUT); //清除RTC WAKE UP的标志
	EXTI_ClearITPendingBit(EXTI_Line22);//清除LINE22上的中断标志位 
	 
	RTC_ITConfig(RTC_IT_WUT,ENABLE);//开启WAKE UP 定时器中断
	RTC_WakeUpCmd( ENABLE);//开启WAKE UP 定时器　

	EXTI_InitStructure.EXTI_Line = EXTI_Line22;//LINE22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE22
	EXTI_Init(&EXTI_InitStructure);//配置
 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置
}

//RTC闹钟中断服务函数
void RTC_Alarm_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET)//ALARM A中断?
	{
		RTC_ClearFlag(RTC_FLAG_ALRAF);//清除中断标志
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断线17的中断标志 											 
}

//RTC WAKE UP中断服务函数
void RTC_WKUP_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_WUTF)==SET)//WK_UP中断?
	{ 
		RTC_ClearFlag(RTC_FLAG_WUTF);	//清除中断标志
	}   
	EXTI_ClearITPendingBit(EXTI_Line22);//清除中断线22的中断标志 								
}

//写数据到备份SRAM
int8_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset ) 
{
    const uint16_t backup_size = 0x1000;
    uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
    uint16_t i;
    
    /* ERROR : the last byte is outside the backup SRAM region */
    if( bytes + offset >= backup_size ) 
    {
        return -1;
    }
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE); //disable backup domain write protection
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);     //set RCC->APB1ENR.pwren
    PWR_BackupAccessCmd(ENABLE);                            //set PWR->CR.dbp = 1;
    
    /** enable the backup regulator (used to maintain the backup SRAM content in
    * standby and Vbat modes). NOTE : this bit is not reset when the device
    * wakes up from standby, system reset or power reset. You can check that
    * the backup regulator is ready on PWR->CSR.brr, see rm p144 */
    
    PWR_BackupRegulatorCmd(ENABLE);  // set PWR->CSR.bre = 1;
    for( i = 0; i < bytes; i++ ) 
    {
        *(base_addr + offset + i) = *(data + i);
    }
    PWR_BackupAccessCmd(DISABLE);      // reset PWR->CR.dbp = 0;
    return 0;
}

//从备份SRAM读取数据
int8_t read_from_backup_sram(uint8_t *data, uint16_t bytes, uint16_t offset) 
{
    const uint16_t backup_size = 0x1000;
    uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
    uint16_t i;
    
    /* ERROR : the last byte is outside the backup SRAM region */
    if( bytes + offset >= backup_size ) 
    {
        return -1;
    }
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
    for( i = 0; i < bytes; i++ ) 
    {
        *(data + i) = *(base_addr + offset + i);
    }
    
    return 0;
}

//写备份RTC寄存器
int8_t write_to_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset ) 
{
    const uint16_t backup_size = 80;
    volatile uint32_t* base_addr = &(RTC->BKP0R);
    uint16_t i;
    
    if(bytes + offset >= backup_size ) 
    {
        /* ERROR : the last byte is outside the backup SRAM region */
        return -1;
    } 
    else if( offset % 4 || bytes % 4 ) 
    {
        /* ERROR: data start or num bytes are not word aligned */
        return -2;
    } 
    else 
    {
        bytes >>= 2;  /* divide by 4 because writing words */
    }
    
    /* disable backup domain write protection */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // set RCC->APB1ENR.pwren
    PWR_BackupAccessCmd(ENABLE);       // set PWR->CR.dbp = 1;
    for( i = 0; i < bytes; i++ ) 
    {
        *(base_addr + offset + i) = *(data + i);
    }
    PWR_BackupAccessCmd(DISABLE);      // reset PWR->CR.dbp = 0;
    // consider also disabling the power peripherial?
    return 0;
}

//读备份RTC寄存器
int8_t read_from_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset ) 
{
    const uint16_t backup_size = 80;
    volatile uint32_t* base_addr = &(RTC->BKP0R);
    uint16_t i;
    
    if( bytes + offset >= backup_size ) {
        /* ERROR : the last byte is outside the backup SRAM region */
        return -1;
    } 
    else if( offset % 4 || bytes % 4 ) 
    {
        /* ERROR: data start or num bytes are not word aligned */
        return -2;
    } 
    else 
    {
        bytes >>= 2;  /* divide by 4 because writing words */
    }
    
    /* read should be 32 bit aligned */
    for( i = 0; i < bytes; i++ ) 
    {
        *(data + i) = *(base_addr + offset + i);
    }
    return 0;
}


