#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "adc.h"
#include "others.h"
#include "CCS.h"
#include "IIC.h"
#include "math.h"
#include "dht22.h" 
#include "esp8266.h" 	
#include "usart2.h"	   
#include "timer.h"
/********************************************************************
硬件接法：
 STM32
SCL---PC12
SDA---PC11
aWAKE---PC1
RST-----3.3
VCC-----3.3
波特率115200
风速传感器
棕色线接地
ADC  PA1    蓝线
BEEP PC5
DHT22 PC4
ESP8266接线5根 RX-PA2
				TX-PA3
				EN-3.3V
继电器模块
PA4----雾化
PA5----水泵
PA6----电热片
PA7----风扇
PA11----橙色灯光
PA12----红色灯光
********************************************************************/
#define CCS811_Add  0x5A<<1
#define STATUS_REG 0x00
#define MEAS_MODE_REG 0x01
#define ALG_RESULT_DATA 0x02
#define ENV_DATA 0x05
#define NTC_REG 0x06
#define THRESHOLDS 0x10
#define BASELINE 0x11
#define HW_ID_REG 0x20
#define ERROR_ID_REG 0xE0
#define APP_START_REG 0xF4
#define SW_RESET 0xFF
#define CCS_811_ADDRESS 0x5A
#define GPIO_WAKE 0x5
#define DRIVE_MODE_IDLE 0x0
#define DRIVE_MODE_1SEC 0x10
#define DRIVE_MODE_10SEC 0x20
#define DRIVE_MODE_60SEC 0x30
#define INTERRUPT_DRIVEN 0x8
#define THRESHOLDS_ENABLED 0x4

#define DipID  "1"                            //节点号
u8 BUF[12];
u8 Information[10];
u8 temp=0x5a;
u8 MeasureMode,Status,Error_ID;
u16 CCS_eco2;                //存放二氧化碳数值
u16 CCS_tvoc;                //存放tvoc数值
u8  temp_Co2[5];           //二氧化碳
u8  temp_Tvoc[5];           //空气清洁度
u16 adcx; 
u8 temperature;  	    
u8 humidity;           
u8  tempT[4];           //温度
u8  tempH[3];           //湿度
u8  tempWind_Speed[3];    //风速m/s


void CCS_init(void);
void Get_ccs(void);
void Data_NumToStr(void);
void Check_Connect_State(void);   //检测连接状态 断开则重新连接

void Control_Drive(void);   //返回控制驱动函数
int main(void)
{ 
	float ADCtemp;
	
	u16 t=99;		//加速第一次获取链接状态
	char StringSend[100]={0};
	
	delay_init();	    	 //延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级	  
	uart_init(115200);	 	//串口初始化为115200
	USART2_Init(115200);  //初始化串口2波特率为115200
	LCD_Init();				//初始化液晶 
	LED_Init();         	//LED初始化	 
	Others_Init();         	//其他初始化 继电器 蜂鸣器模块
 	Adc_Init();		  		//ADC初始化	   
	CCS_init();        //初始化iic和各个引脚
	POINT_COLOR=BLUE;//设置字体为蓝色
	while(DHT22_Init())	//DHT22初始化	
	{
		LCD_ShowString(60,20,200,16,16,"DHT22 Error");
		delay_ms(200);
		LCD_Fill(60,20,180,20+16,WHITE);
 		delay_ms(200);
	}	
	LCD_ShowString(60,20,200,16,16,"DHT22 OK");
	ESP8266_RESTORE();         //恢复出厂设置
	ESP8266_ConnecWIFI();      //连接指定wifi
  ESP8266_ConnecPORT();   //连端口开透传

	LCD_ShowString(60,80,200,16,16,"Wind Speed:  m/s");	       
	LCD_ShowString(60,100,200,16,16,"CO2:");	      
	LCD_ShowString(60,120,200,16,16,"TVOC:");	 
 	LCD_ShowString(60,140,200,16,16,"Temp:   C");	 
 	LCD_ShowString(60,160,200,16,16,"Humi:  %");		
	LCD_ShowString(60,180,200,16,16,"Beep:");		
	LCD_ShowString(60,200,200,16,16,"Atomi:OFF");	   //雾化
	LCD_ShowString(60,220,200,16,16,"Heat:OFF");      //加热
	LCD_ShowString(60,240,200,16,16,"Fan:OFF");	     //风扇
	LCD_ShowString(60,260,200,16,16,"Pump:OFF");	     //水泵
	
	TIM3_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为500ms  
	while(1)
	{
		 //采集数据
		 Get_ccs();   //获取co2和tvoc
		 DHT22_Read_Data(&temperature,&humidity);	//读取温湿度值		
		 adcx=Get_Adc_Average(ADC_Channel_1,10);
		 ADCtemp=(float)adcx*(3.3/4096);         //电压值
		 adcx=ADCtemp*0.027*1000; 
		 Data_NumToStr();
		//发送数据 
		sprintf(StringSend,"%s#%s#%s#%s#%s#%s\r\n",DipID,tempT,tempH,tempWind_Speed,temp_Co2,temp_Tvoc); 
		u2_printf(StringSend);
		//显示数据
		LCD_ShowString(92,100,200,16,16,temp_Co2);	 
		LCD_ShowString(100,120,200,16,16,temp_Tvoc);	
		LCD_ShowString(60+40,140,200,16,16,tempT);	 
		LCD_ShowString(60+40,160,200,16,16,tempH);		
		LCD_ShowString(60+88,80,200,16,16,tempWind_Speed);			
		LCD_ShowString(20+40,40,200,16,16,(u8 *)StringSend);	   //显示发送的数据

		t++;
		delay_ms(300);
		if(t==100)                               //间隔50s检查一遍连接状态  断开则重新连接
		{
			Check_Connect_State();
			t=0;
		}
		delay_ms(150);	
	}								    
}	

void Check_Connect_State(void)
{
			u8 constate=0;	//连接状态
			u8 PORTtate=0;  //port连接状态
			constate=atk_8266_consta_check();//退出透传 得到连接状态   +CIPSTATUS:0,"TCP","172.20.10.3",8080,0  
			if(constate=='+')
			{
				LCD_ShowString(60,20,200,16,16,"Success        ");	
				atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //传输模式为：透传	
				atk_8266_send_cmd("AT+CIPSEND","OK",20);         //开始透传     并没有成功进入透传模式   
			}
			else 
			{
				LCD_ShowString(60,20,200,16,16,"ERROR         ");	
				PORTtate=atk_8266_state_check();   // 2连接wifi   4.端口连接不成功   0、3表示连接成   5表示没有连接wifi
				LCD_ShowChar(60+40,20,PORTtate,16,0); //在后面显示连接状态 
				if(PORTtate=='2' || PORTtate=='4')
				{
					ESP8266_ConnecPORT();   //连端口开透传
				}
				else if (PORTtate=='5')
				{
					ESP8266_ConnecWIFI();      //连接指定wifi
					ESP8266_ConnecPORT();   //连端口开透传
				}
			}
			USART2_RX_STA=0;
}



void Data_NumToStr(void)
{
		 temp_Co2[0]=CCS_eco2/1000%10+'0';
		 temp_Co2[1]=CCS_eco2/100%10+'0';
		 temp_Co2[2]=CCS_eco2/10%10+'0';
		 temp_Co2[3]=CCS_eco2%10+'0';
		 temp_Co2[4]='\0';
 		 temp_Tvoc[0]=CCS_tvoc/1000%10+'0';
		 temp_Tvoc[1]=CCS_tvoc/100%10+'0';
		 temp_Tvoc[2]=CCS_tvoc/10%10+'0';
		 temp_Tvoc[3]=CCS_tvoc%10+'0';
		 temp_Tvoc[4]='\0';

		 tempT[0]='+';
		 tempT[1]=temperature/10%10+'0';          //数字转字符串
	   tempT[2]=temperature%10+'0';
		 tempT[3]='\0';
	   tempH[0]=humidity/10%10+'0';
	   tempH[1]=humidity%10+'0';
		 tempH[2]='\0';
			 
		 tempWind_Speed[0]=adcx/10%10+'0';
	   tempWind_Speed[1]=adcx%10+'0';
		 tempWind_Speed[2]='\0';
}

void Get_ccs(void)
{
     ON_CS(); 					   	//CCS811  WAK引脚拉低
	   Single_ReadI2C(CCS811_Add,0x00,&Status,1);
	   Single_ReadI2C(CCS811_Add,0xE0,&Error_ID,1);
	   Single_ReadI2C(CCS811_Add,0x02,BUF,8);
		 Single_ReadI2C(CCS811_Add,0x20,Information,1);  //Read CCS's information  ,ID
	   OFF_CS(); 
	   CCS_eco2= (u16)BUF[0]*256+BUF[1];
	   CCS_tvoc= (u16)BUF[2]*256+BUF[3];
		 Information[0]=0;
}



void CCS_init(void)
{
	I2C_GPIO_Config();
	CCS_GPIO_init(); 
	delay_ms(100);		//等待模块初始化完成
	ON_CS(); 					   	//CCS811  WAK引脚拉低
	delay_ms(10); 
	Single_ReadI2C(CCS811_Add,0x20,Information,1);  //Read CCS's information  ,ID
  Single_ReadI2C(CCS811_Add,0x23,&Information[1],2);	//FW_Boot_Version  
	Single_ReadI2C(CCS811_Add,0x24,&Information[3],2); 	//FW_App_Version

  Single_ReadI2C(CCS811_Add,0x00,&Status,1);	   //Firstly the status register is read and the APP_VALID flag is checked.
	if(Status&0x10)	 Single_MWriteI2C_byte(CCS811_Add,0xF4,&temp,0);	//Used to transition the CCS811 state from boot to application mode, a write with no data is required.
	Single_ReadI2C(CCS811_Add,0x00,&Status,1);
	Single_ReadI2C(CCS811_Add,0x01,&MeasureMode,1);
  Single_WriteI2C_byte(CCS811_Add,0x01,0x10); //Write Measure Mode Register,sensor measurement every second,no interrupt
  OFF_CS(); 
	delay_ms(10);
}




