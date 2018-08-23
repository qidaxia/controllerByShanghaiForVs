#include "key.h"
#include "delay.h" 

/*
按键初始化函数
KEY1 -- PC6
KEY2 -- PG8
KEY3 -- PG7		小车后退中断，对应JDQ4
KEY4 -- PG6		小车前进中断，对应JDQ3
KEY5 -- PG5		大梁后退中断，对应JDQ2
KEY6 -- PG4		大梁前进中断，对应JDQ1
*/
void KEY_Init(void)
{
	
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIOC,GPIOG时钟
 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; //KEY3-6对应引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;        //普通输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  //100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
    GPIO_Init(GPIOG, &GPIO_InitStructure);

}

/*按键处理函数
返回按键值
mode:0,不支持连续按;1,支持连续按;
0，没有任何按键按下
3，KEY3按下 
4，KEY4按下
5，KEY5按下 
6，KEY6按下 
注意此函数有响应优先级,KEY1>KEY2>KEY3>KEY4>KEY5>KEY6
mode=0不支持连续按，mode=1支持连续按
*/
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1; //按键按松开标志
	if(mode)key_up=1;   //支持连按
    
	if(key_up&&(KEY3==0||KEY4==0||KEY5==0||KEY6==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY3==0)return 3;
		else if(KEY4==0)return 4;
		else if(KEY5==0)return 5;
		else if(KEY6==0)return 6;
	}
    else if(KEY3==1 && KEY4==1 && KEY5==1 && KEY6==1)
    {
        key_up=1;
 	}
    
 	return 0;// 无按键按下
}

