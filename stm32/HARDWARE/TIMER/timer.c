#include "timer.h"
#include "led.h"
#include "usart2.h"	 
#include "lcd.h"
#include "others.h"
#include "esp8266.h" 	

//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM3, //TIM2
		TIM_IT_Update ,
		ENABLE  //ʹ��
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
							 
}

void TIM3_IRQHandler(void)   //TIM3�ж�
{
	static u16 rlen=0;        //�������ݵĳ���
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ 

		if(USART2_RX_STA&0X8000)		//���յ�һ��������
		{ 
			rlen=USART2_RX_STA&0X7FFF;	//�õ����ν��յ������ݳ���
			USART2_RX_BUF[rlen]='\0';		//��ӽ����� 
			LCD_Fill(20+40,60,239,40+16,WHITE);
			LCD_ShowString(20+40,60,200,16,16,USART2_RX_BUF);	    //��ʾ���յ������� 
			USART2_RX_STA=0;    //���ձ�־λ����
			//atk_8266_at_response(0);      //�����յ����ݷ��͵�����1
		}  
		if(rlen==16 && Check_state()==1)            //�������ݳ��ȱ���Ϊ16 ��У��ϸ�
			{Control_Drive();}	
	}
}

//У������
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
	static char green_bit_atomi=0;    //��ɫ����������˸ ��
	static char green_bit_pump=0;    //��ɫ ˮ��
	static char green_bit_heat=0;    //��ɫ ����Ƭ
	static char green_bit_fan=0;    //��ɫ ����
	
	static char red_bit_atomi=0;    // ��ɫ��������  ��
	static char red_bit_pump=0;    // ��ɫ����  ˮ��
	static char red_bit_heat=0;    // ��ɫ����  ����Ƭ
	static char red_bit_fan=0;    // ��ɫ����    ����
	
	static char red_bit_DHT11=0;     //��ɫ������ʪ��
	static char red_bit_WindSpeed=0;  //�����ǹ��ϱ���
	static char red_bit_Ccs=0;        //ccs811���ϱ���
	
	if(USART2_RX_BUF[0]=='1')            //���Ʒ�����  ���
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
	
	if (USART2_RX_BUF[1]=='1' && USART2_RX_BUF[5]=='1')    //�����غ�������ͬʱ��
	{
		ATOMI=1;
		red_bit_atomi=!red_bit_atomi;
		if (red_bit_atomi==0)
		{LCD_Fill(40,200,56,200+16,RED);}
		else
		{LCD_Fill(40,200,56,200+16,WHITE);}
		 LCD_ShowString(60+48,200,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[1]=='1' && USART2_RX_BUF[5]=='0')              //����Ƭ����  
	{
			ATOMI=1;
			green_bit_atomi=!green_bit_atomi;
			if (green_bit_atomi==0)
			{LCD_Fill(40,200,56,200+16,GREEN);}
			else
			{LCD_Fill(40,200,56,200+16,WHITE);}
			 LCD_ShowString(60+48,200,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[1]=='0' && USART2_RX_BUF[5]=='1')              //����Ƭ��������
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
	
	if (USART2_RX_BUF[2]=='1' && USART2_RX_BUF[6]=='1')   //���ƺͱ���ͬʱ��ˮ��
	{
		PUMP=1;
		red_bit_pump=!red_bit_pump;
		if (red_bit_pump==0)
		{LCD_Fill(40,260,56,260+16,RED);}
		else
		{LCD_Fill(40,260,56,260+16,WHITE);}
		 LCD_ShowString(60+40,260,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[2]=='1' && USART2_RX_BUF[6]=='0')              //��ˮ�ÿ���  
	{
			PUMP=1;
			green_bit_pump=!green_bit_pump;
			if (green_bit_pump==0)
			{LCD_Fill(40,260,56,260+16,GREEN);}
			else
			{LCD_Fill(40,260,56,260+16,WHITE);}
			 LCD_ShowString(60+40,260,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[2]=='0' && USART2_RX_BUF[6]=='1')              //�ر��� ˮ��  ����
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
		
	
	if (USART2_RX_BUF[3]=='1' && USART2_RX_BUF[7]=='1')   // ���ȴ� �ͱ��� ͬʱ��
	{
		 HEAT=1;
	   red_bit_heat=!red_bit_heat;
		 if (red_bit_heat==0)
		 {LCD_Fill(40,220,56,220+16,RED);}
	   else
		 {LCD_Fill(40,220,56,220+16,WHITE);}
		 LCD_ShowString(60+40,220,200,16,16,"ON ");			
	}
	else if(USART2_RX_BUF[3]=='1' && USART2_RX_BUF[7]=='0')              //�ص��ȿ���+��ɫ��
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
	else if(USART2_RX_BUF[3]=='0' && USART2_RX_BUF[7]=='1')              //���ȱ���
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
	
	if (USART2_RX_BUF[4]=='1' && USART2_RX_BUF[8]=='1')     //���ȱ����Ϳ���ͬʱ��
	{
		FAN=1;
		red_bit_fan=!red_bit_fan;
		if (red_bit_fan==0)
		{LCD_Fill(40,240,56,240+16,RED);}
		else
		{LCD_Fill(40,240,56,240+16,WHITE);}
		 LCD_ShowString(60+32,240,200,16,16,"ON ");		
	}		
	else if(USART2_RX_BUF[4]=='1' && USART2_RX_BUF[8]=='0')              //�ط��ȵ�� FAN 
	{
			FAN=1;
			green_bit_fan=!green_bit_fan;
			if (green_bit_fan==0)
			{LCD_Fill(40,240,56,240+16,GREEN);}
			else
			{LCD_Fill(40,240,56,240+16,WHITE);}
			 LCD_ShowString(60+32,240,200,16,16,"ON ");		
	}
	else if(USART2_RX_BUF[4]=='0' && USART2_RX_BUF[8]=='1')              //���ȱ���
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
	

	if(USART2_RX_BUF[9]=='1')              //DHT11���ϱ���
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

	if(USART2_RX_BUF[10]=='1')              //�����ǹ��ϱ���
	{
		red_bit_WindSpeed=!red_bit_WindSpeed;
		if (red_bit_WindSpeed==0)
		{LCD_Fill(40,80,56,80+16,RED);}
		else
		{LCD_Fill(40,80,56,80+16,WHITE);}
	}
	else 
		{LCD_Fill(40,80,56,80+16,WHITE);}
	
	if(USART2_RX_BUF[11]=='1')              //ccs811���ϱ���
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







