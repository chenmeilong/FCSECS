#include "esp8266.h"


//���Ӷ˿ں�:8080,�������޸�Ϊ�����˿�.
const u8* portnum="8080";		 
const u8* IPnum="172.20.10.4";		 

//WIFI STAģʽ,����Ҫȥ���ӵ�·�������߲���,��������Լ���·��������,�����޸�.
const u8* wifista_ssid="esp8266";			//·����SSID��
const u8* wifista_password="123456789"; 	//��������

char StringWIFI[100]={0};
char StringIP[100]={0};


u8 porttate=0;  //port����״̬


//��ȡATK-ESP8266ģ�������״̬
//����ֵ:0,δ����;1,���ӳɹ�.   //  ��������Ӧ�������ӳɹ�
u8 atk_8266_consta_check(void)
{
	u8 *p;
	u8 res;
	if(atk_8266_quit_trans())return 0;			//�˳�͸�� 
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//����AT+CIPSTATUSָ��,��ѯ����״̬
	p=atk_8266_check_cmd("+CIPSTATUS:"); 
	res=*p;		
	return res;
}

//��ȡATK-ESP8266ģ�������״̬
//����ֵ:    5��δ����wifi    2������wifi ��δ���Ӷ˿�      0 ��ʾ������port
u8 atk_8266_state_check(void)
{
	u8 *p;
	u8 res;
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//����AT+CIPSTATUSָ��,��ѯ����״̬
	p=atk_8266_check_cmd("STATUS:"); 
	res=*(p+7);
	return res;
}



//usmart֧�ֲ���
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART2_RX_STA;
//     1,����USART2_RX_STA;
void atk_8266_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]='\0';//��ӽ�����
		printf("uart2:%s",USART2_RX_BUF);	//���͵�����
		if(mode)USART2_RX_STA=0;
	} 
}

//ATK-ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]='\0';//��ӽ����� 
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}

//ATK-ESP8266�˳�͸��ģʽ
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
u8 atk_8266_quit_trans(void)
{
	while((USART2->SR&0X40)==0);	//�ȴ����Ϳ�
	USART2->DR='+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART2->SR&0X40)==0);	//�ȴ����Ϳ�
	USART2->DR='+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART2->SR&0X40)==0);	//�ȴ����Ϳ�
	USART2->DR='+';      
	delay_ms(500);					//�ȴ�500ms
	return atk_8266_send_cmd("AT","OK",20);//�˳�͸���ж�.
}

//��ATK-ESP8266��������
//cmd:���͵������ַ���
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(atk_8266_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);
					break;//�õ���Ч���� 
				}
					USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 


void ESP8266_ConnecWIFI(void)
{
	while(atk_8266_send_cmd("AT","OK",20))//���WIFIģ���Ƿ�����
	{
		atk_8266_quit_trans();//�˳�͸��
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //�ر�͸��ģʽ	
		LCD_ShowString(60,20,200,16,16,"NOT found ESP8266        ");
		delay_ms(800);
		LCD_Fill(60,20,200,40+16,WHITE);
	} 
	
	LCD_Fill(60,20,200,40+16,WHITE);
	LCD_ShowString(60,20,200,16,16,"ESP8266 Ready");	
	while(atk_8266_send_cmd("ATE0","OK",20));//�رջ���
	
	atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//����WIFI STAģʽ
	//�������ӵ���WIFI��������/���ܷ�ʽ/����,�⼸��������Ҫ�������Լ���·�������ý����޸�!! 
	sprintf(StringWIFI,"AT+CWJAP=\"%s\",\"%s\"",wifista_ssid,wifista_password);//�������߲���:ssid,����
	while(atk_8266_send_cmd((u8 *)StringWIFI,"WIFI GOT IP",300));					//����Ŀ��·����, ��������ֱ���ظ�
	LCD_Fill(60,20,200,40+16,WHITE);
	LCD_ShowString(60,20,200,16,16,"WIFI connected");	
}

void ESP8266_RESTORE(void)
{
	while(atk_8266_send_cmd("AT","OK",20))//���WIFIģ���Ƿ�����
	{
		atk_8266_quit_trans();//�˳�͸��
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //�ر�͸��ģʽ	
		LCD_ShowString(60,20,200,16,16,"NOT found ESP8266        ");
		delay_ms(800);
		LCD_Fill(60,20,200,40+16,WHITE);
	} 
	atk_8266_send_cmd("AT+RESTORE","OK",20);		//�ظ���������
	delay_ms(1000);         //��ʱ3S�ȴ������ɹ�
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
}

void ESP8266_ConnecPORT(void)
{
	porttate=0;
	sprintf(StringIP,"AT+CIPSTART=\"TCP\",\"%s\",%s",IPnum,portnum);    //����Ŀ��TCP������
	while(1)
	{
		if (porttate=='0' || porttate =='3')
		{break;}
		else
		{
			atk_8266_send_cmd((u8 *)StringIP,"OK",200);
	   	porttate=atk_8266_state_check();      //  2����wifi   4.�˿����Ӳ��ɹ�   0��3��ʾ���ӳ�
	    LCD_ShowString(60,20,200,16,16,"PORT connecting");	
	   	LCD_ShowChar(180,20,porttate,16,0); //�ں�����ʾ����״̬ 
		}
	}	
	LCD_ShowString(60,20,200,16,16,"PORT connected   ");	
	
	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //����ģʽΪ��͸��	
	atk_8266_send_cmd("AT+CIPSEND","OK",20);         //��ʼ͸��     ��û�гɹ�����͸��ģʽ   
	
	LCD_ShowString(20,40,200,16,16,"SEND:");	
	LCD_ShowString(20,60,200,16,16,"RECE:");	
	USART2_RX_STA=0;
}

