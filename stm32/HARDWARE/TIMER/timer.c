#include "timer.h"
#include "led.h"
#include "usart2.h"	 
#include "lcd.h"
#include "others.h"
#include "esp8266.h" 	

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}

void TIM3_IRQHandler(void)   //TIM3中断
{
	static u16 rlen=0;        //接收数据的长度
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 

		if(USART2_RX_STA&0X8000)		//接收到一次数据了
		{ 
			rlen=USART2_RX_STA&0X7FFF;	//得到本次接收到的数据长度
			USART2_RX_BUF[rlen]='\0';		//添加结束符 
			LCD_Fill(20+40,60,239,40+16,WHITE);
			LCD_ShowString(20+40,60,200,16,16,USART2_RX_BUF);	    //显示接收到的数据 
			USART2_RX_STA=0;    //接收标志位清零
			//atk_8266_at_response(0);      //将接收的数据发送到串口1
		}  
		if(rlen==16 && Check_state()==1)            //接收数据长度必须为16 且校检合格
			{Control_Drive();}	
	}
}

//校检数据
u8 Check_state(void)
{
	u8 res=1;
	if (((USART2_RX_BUF[0]-'0')+	(USART2_RX_BUF[1]-'0')+	(USART2_RX_BUF[2]-'0'))%2 != (USART2_RX_BUF[12]-'0'))
	{res=0;}
	if (((USART2_RX_BUF[3]-'0')+	(USART2_RX_BUF[4]-'0')+	(USART2_RX_BUF[5]-'0'))%2 != (USART2_RX_BUF[13]-'0'))
	{res=0;}
	if (((USART2_RX_BUF[6]-'0')+	(USART2_RX_BUF[7]-'0')+	(USART2_RX_BUF[8]-'0'))%2 != (USART2_RX_BUF[14]-'0'))
	{res=0;}
	if (((USART2_RX_BUF[9]-'0')+	(USART2_RX_BUF[10]-'0')+	(USART2_RX_BUF[11]-'0'))%2 != (USART2_RX_BUF[15]-'0'))
	{res=0;}
	return res;
}



void Control_Drive(void)
{
	static char green_bit_atomi=0;    //绿色工作方块闪烁 雾化
	static char green_bit_pump=0;    //绿色 水泵
	static char green_bit_heat=0;    //绿色 电热片
	static char green_bit_fan=0;    //绿色 风扇
	
	static char red_bit_atomi=0;    // 红色报警方块  雾化
	static char red_bit_pump=0;    // 红色报警  水泵
	static char red_bit_heat=0;    // 红色报警  电热片
	static char red_bit_fan=0;    // 红色报警    风扇
	
	static char red_bit_DHT11=0;     //红色报警温湿度
	static char red_bit_WindSpeed=0;  //风速仪故障报警
	static char red_bit_Ccs=0;        //ccs811故障报警
	
	if(USART2_RX_BUF[0]=='1')            //控制蜂鸣器  红光
	{
		BEEP=!BEEP;
		RED_LED=1;
		LCD_ShowString(60+40,180,200,16,16,"ON ");	
	}
  else 
	{
		 BEEP=0;
	   RED_LED=0;
     LCD_ShowString(60+40,180,200,16,16,"OFF");		
	}
	
	if (USART2_RX_BUF[1]=='1' && USART2_RX_BUF[5]=='1')    //雾化开关和雾化报警同时打开
	{
		ATOMI=1;
		red_bit_atomi=!red_bit_atomi;
		if (red_bit_atomi==0)
		{LCD_Fill(40,200,56,200+16,RED);}
		else
		{LCD_Fill(40,200,56,200+16,WHITE);}
		 LCD_ShowString(60+48,200,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[1]=='1' && USART2_RX_BUF[5]=='0')              //控雾化片开关  
	{
			ATOMI=1;
			green_bit_atomi=!green_bit_atomi;
			if (green_bit_atomi==0)
			{LCD_Fill(40,200,56,200+16,GREEN);}
			else
			{LCD_Fill(40,200,56,200+16,WHITE);}
			 LCD_ShowString(60+48,200,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[1]=='0' && USART2_RX_BUF[5]=='1')              //控雾化片报警故障
	{
		red_bit_atomi=!red_bit_atomi;
		if (red_bit_atomi==0)
		{LCD_Fill(40,200,56,200+16,RED);}
		else
		{LCD_Fill(40,200,56,200+16,WHITE);}
		LCD_ShowString(60+48,200,200,16,16,"OFF");		
	}
	else
	{
		ATOMI=0;
		LCD_Fill(40,200,56,200+16,WHITE);
		LCD_ShowString(60+48,200,200,16,16,"OFF");		
	}
	
	if (USART2_RX_BUF[2]=='1' && USART2_RX_BUF[6]=='1')   //控制和报警同时打开水泵
	{
		PUMP=1;
		red_bit_pump=!red_bit_pump;
		if (red_bit_pump==0)
		{LCD_Fill(40,260,56,260+16,RED);}
		else
		{LCD_Fill(40,260,56,260+16,WHITE);}
		 LCD_ShowString(60+40,260,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[2]=='1' && USART2_RX_BUF[6]=='0')              //控水泵开关  
	{
			PUMP=1;
			green_bit_pump=!green_bit_pump;
			if (green_bit_pump==0)
			{LCD_Fill(40,260,56,260+16,GREEN);}
			else
			{LCD_Fill(40,260,56,260+16,WHITE);}
			 LCD_ShowString(60+40,260,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[2]=='0' && USART2_RX_BUF[6]=='1')              //控报警 水泵  故障
	{
			red_bit_pump=!red_bit_pump;
			if (red_bit_pump==0)
			{LCD_Fill(40,260,56,260+16,RED);}
			else
			{LCD_Fill(40,260,56,260+16,WHITE);}
			LCD_ShowString(60+40,260,200,16,16,"OFF");		
	}
	else 
	{
			PUMP=0;
			LCD_Fill(40,260,56,260+16,WHITE);
			LCD_ShowString(60+40,260,200,16,16,"OFF");		
	}
		
	
	if (USART2_RX_BUF[3]=='1' && USART2_RX_BUF[7]=='1')   // 电热打开 和报警 同时打开
	{
		 HEAT=1;
	   red_bit_heat=!red_bit_heat;
		 if (red_bit_heat==0)
		 {LCD_Fill(40,220,56,220+16,RED);}
	   else
		 {LCD_Fill(40,220,56,220+16,WHITE);}
		 LCD_ShowString(60+40,220,200,16,16,"ON ");			
	}
	else if(USART2_RX_BUF[3]=='1' && USART2_RX_BUF[7]=='0')              //控电热开关+橙色光
	{
			HEAT=1;
			ORG_LED=1;
			green_bit_heat=!green_bit_heat;
			if (green_bit_heat==0)
			{LCD_Fill(40,220,56,220+16,GREEN);}
			else
			{LCD_Fill(40,220,56,220+16,WHITE);}
			 LCD_ShowString(60+40,220,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[3]=='0' && USART2_RX_BUF[7]=='1')              //电热报警
		{
	   red_bit_heat=!red_bit_heat;
		 if (red_bit_heat==0)
		 {LCD_Fill(40,220,56,220+16,RED);}
	   else
		 {LCD_Fill(40,220,56,220+16,WHITE);}	
		 	LCD_ShowString(60+40,220,200,16,16,"OFF");		
		}
	else 
		{
			HEAT=0;
			ORG_LED=0;
			LCD_Fill(40,220,56,220+16,WHITE);
			LCD_ShowString(60+40,220,200,16,16,"OFF");		
		}
	
	if (USART2_RX_BUF[4]=='1' && USART2_RX_BUF[8]=='1')     //风扇报警和控制同时开
	{
		FAN=1;
		red_bit_fan=!red_bit_fan;
		if (red_bit_fan==0)
		{LCD_Fill(40,240,56,240+16,RED);}
		else
		{LCD_Fill(40,240,56,240+16,WHITE);}
		 LCD_ShowString(60+32,240,200,16,16,"ON ");		
	}		
	else if(USART2_RX_BUF[4]=='1' && USART2_RX_BUF[8]=='0')              //控风扇电机 FAN 
	{
			FAN=1;
			green_bit_fan=!green_bit_fan;
			if (green_bit_fan==0)
			{LCD_Fill(40,240,56,240+16,GREEN);}
			else
			{LCD_Fill(40,240,56,240+16,WHITE);}
			 LCD_ShowString(60+32,240,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[4]=='0' && USART2_RX_BUF[8]=='1')              //风扇报警
	{
			red_bit_fan=!red_bit_fan;
			if (red_bit_fan==0)
			{LCD_Fill(40,240,56,240+16,RED);}
			else
			{LCD_Fill(40,240,56,240+16,WHITE);}
			LCD_ShowString(60+32,240,200,16,16,"OFF");		
	}
	else 
	{
			FAN=0;
			LCD_Fill(40,240,56,240+16,WHITE);
			LCD_ShowString(60+32,240,200,16,16,"OFF");		
	}
	

	if(USART2_RX_BUF[9]=='1')              //DHT11故障报警
	{
		red_bit_DHT11=!red_bit_DHT11;
		if (red_bit_DHT11==0)
		{
			LCD_Fill(40,140,56,140+16,RED);
			LCD_Fill(40,160,56,160+16,RED);
		}
		else
		{
			LCD_Fill(40,140,56,140+16,WHITE);
			LCD_Fill(40,160,56,160+16,WHITE);
		}
	}
	else 
	{
			LCD_Fill(40,140,56,140+16,WHITE);
			LCD_Fill(40,160,56,160+16,WHITE);
	}

	if(USART2_RX_BUF[10]=='1')              //风速仪故障报警
	{
		red_bit_WindSpeed=!red_bit_WindSpeed;
		if (red_bit_WindSpeed==0)
		{LCD_Fill(40,80,56,80+16,RED);}
		else
		{LCD_Fill(40,80,56,80+16,WHITE);}
	}
	else 
		{LCD_Fill(40,80,56,80+16,WHITE);}
	
	if(USART2_RX_BUF[11]=='1')              //ccs811故障报警
	{
		red_bit_Ccs=!red_bit_Ccs;
		if (red_bit_Ccs==0)
		{
			LCD_Fill(40,100,56,100+16,RED);
			LCD_Fill(40,120,56,120+16,RED);
		}
		else
		{
			LCD_Fill(40,100,56,100+16,WHITE);
			LCD_Fill(40,120,56,120+16,WHITE);
		}
	}
	else 
		{
			LCD_Fill(40,100,56,100+16,WHITE);
			LCD_Fill(40,120,56,120+16,WHITE);
		}
}







