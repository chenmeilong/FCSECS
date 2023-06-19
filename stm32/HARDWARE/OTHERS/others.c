#include "others.h"

void Others_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);	 //使能端口时钟ABC

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_11|GPIO_Pin_12;				 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);	 //根据参数初始化GPIOB.8
 GPIO_ResetBits(GPIOA,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_11|GPIO_Pin_12);// 初始输出0

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
 GPIO_Init(GPIOC, &GPIO_InitStructure);	 //根据参数初始化GPIOB.8
 GPIO_ResetBits(GPIOB,GPIO_Pin_5);//输出0，关闭蜂鸣器输出

}







