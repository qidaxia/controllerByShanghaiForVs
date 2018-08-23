#include "JDQ.h" 

/*
低电平继电器动作
继电器1输出：PE10
继电器2输出：PE8
继电器3输出：PE14
继电器4输出：PE12
继电器5输出：PC6
继电器6输出：PG8
*/
void JDQ_Init(void)
{   
    GPIO_InitTypeDef  GPIO_InitStructure;

    //JDQ1-JDQ4
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIOE时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_12 |GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;    //下拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);            //初始化GPIO

    GPIO_SetBits(GPIOE,GPIO_Pin_8);                   //拉高
    GPIO_SetBits(GPIOE,GPIO_Pin_10);                  //拉高
    GPIO_SetBits(GPIOE,GPIO_Pin_12);                  //拉高
    GPIO_SetBits(GPIOE,GPIO_Pin_14);                  //拉高

    //JDQ5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;    //下拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);            //初始化GPIO

    GPIO_ResetBits(GPIOC,GPIO_Pin_6);                   //拉低

    //JDQ6
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;     //普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;    //下拉
    GPIO_Init(GPIOG, &GPIO_InitStructure);            //初始化GPIO

    GPIO_ResetBits(GPIOG,GPIO_Pin_8);                   //拉低  
}






