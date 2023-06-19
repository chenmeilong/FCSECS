#include "dht22.h"
#include "delay.h"

    
//��λDHT22
void DHT22_Rst(void)	   
{      	
    DHT22_DQ_OUT=0; 	//����DQ
    delay_ms(20);    	//��������18ms
    DHT22_DQ_OUT=1; 	//DQ=1 
	delay_us(30);     	//��������20~40us
}
//�ȴ�DHT22�Ļ�Ӧ
//����1:δ��⵽DHT22�Ĵ���
//����0:����
u8 DHT22_Check(void) 	   
{   
	u8 retry=100;
   	while ((!DHT22_DQ_IN)&&retry)//DHT22������40~80us
	{
		retry--;
		delay_us(1);
	};	 
	if(retry<1) return 1;
	else retry=100;
    while (DHT22_DQ_IN&&retry)//DHT22���ͺ���ٴ�����40~80us
	{
		retry--;
		delay_us(1);
	};
	if(retry<1) 
		return 1;	    
	return 0;
}
//��DHT22��ȡһ��λ
//����ֵ��1/0
u8 DHT22_Read_Bit(void) 			 
{
 	u8 dat;
 	u8 retry=100;
	while((!DHT22_DQ_IN)&&retry)//�ȴ���Ϊ�ߵ�ƽ
	{
		retry--;
		delay_us(1);
	}
	retry=100;
	dat=0;
	delay_us(30);//�ȴ�30us
	if(DHT22_DQ_IN)	
		dat=1;	   
	while(DHT22_DQ_IN&&retry)//�ȴ���͵�ƽ
	{
		retry--;
		delay_us(1);
	}
	return dat;		
}
//��DHT22��ȡһ���ֽ�
//����ֵ������������
u8 DHT22_Read_Byte(void)    
{        
    u8 i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT22_Read_Bit();
    }						    
    return dat;
}
//��DHT22��ȡһ������
//temp:�¶�ֵ(��Χ:0~50��)
//humi:ʪ��ֵ(��Χ:20%~90%)
//����ֵ��0,����;1,��ȡʧ��
u8 DHT22_Read_Data(u8 *temp,u8 *humi)    
{        
 	u8 buf[5];
	u8 i;
	DHT22_Rst();
	if(DHT22_Check()==0)
	{
		for(i=0;i<5;i++)//��ȡ40λ����
		{
			buf[i]=DHT22_Read_Byte();
		}
		//У����ۼ�ʱ�����൱���ʻᳬ��8λ(��ֵ����1��char,����ǿ������ת����ֻ���ǵ�8λ)
		if((unsigned char)(buf[0]+buf[1]+buf[2]+buf[3])==buf[4])  
		{
			*humi=(buf[0]*256+buf[1])/10;
			*temp=(buf[2]*256+buf[3])/10;
		}
	}else return 1;
	return 0;	    
}




//��ʼ��DHT22��IO�� DQ ͬʱ���DHT22�Ĵ���
//����1:������
//����0:����    	 
u8 DHT22_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOC, &GPIO_InitStructure);		
	
	DHT22_Rst();
	return DHT22_Check();
}
