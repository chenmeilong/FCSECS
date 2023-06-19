#include "esp8266.h"


//连接端口号:8080,可自行修改为其他端口.
const u8* portnum="8080";		 
const u8* IPnum="172.20.10.4";		 

//WIFI STA模式,设置要去连接的路由器无线参数,请根据你自己的路由器设置,自行修改.
const u8* wifista_ssid="esp8266";			//路由器SSID号
const u8* wifista_password="123456789"; 	//连接密码

char StringWIFI[100]={0};
char StringIP[100]={0};


u8 porttate=0;  //port连接状态


//获取ATK-ESP8266模块的连接状态
//返回值:0,未连接;1,连接成功.   //  返回其他应该是连接成功
u8 atk_8266_consta_check(void)
{
	u8 *p;
	u8 res;
	if(atk_8266_quit_trans())return 0;			//退出透传 
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//发送AT+CIPSTATUS指令,查询连接状态
	p=atk_8266_check_cmd("+CIPSTATUS:"); 
	res=*p;		
	return res;
}

//获取ATK-ESP8266模块的连接状态
//返回值:    5是未连接wifi    2已连接wifi ，未连接端口      0 表示已连接port
u8 atk_8266_state_check(void)
{
	u8 *p;
	u8 res;
	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//发送AT+CIPSTATUS指令,查询连接状态
	p=atk_8266_check_cmd("STATUS:"); 
	res=*(p+7);
	return res;
}



//usmart支持部分
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART2_RX_STA;
//     1,清零USART2_RX_STA;
void atk_8266_at_response(u8 mode)
{
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]='\0';//添加结束符
		printf("uart2:%s",USART2_RX_BUF);	//发送到串口
		if(mode)USART2_RX_STA=0;
	} 
}

//ATK-ESP8266发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]='\0';//添加结束符 
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}

//ATK-ESP8266退出透传模式
//返回值:0,退出成功;
//       1,退出失败
u8 atk_8266_quit_trans(void)
{
	while((USART2->SR&0X40)==0);	//等待发送空
	USART2->DR='+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART2->SR&0X40)==0);	//等待发送空
	USART2->DR='+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART2->SR&0X40)==0);	//等待发送空
	USART2->DR='+';      
	delay_ms(500);					//等待500ms
	return atk_8266_send_cmd("AT","OK",20);//退出透传判断.
}

//向ATK-ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n",cmd);	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(atk_8266_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);
					break;//得到有效数据 
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
	while(atk_8266_send_cmd("AT","OK",20))//检查WIFI模块是否在线
	{
		atk_8266_quit_trans();//退出透传
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //关闭透传模式	
		LCD_ShowString(60,20,200,16,16,"NOT found ESP8266        ");
		delay_ms(800);
		LCD_Fill(60,20,200,40+16,WHITE);
	} 
	
	LCD_Fill(60,20,200,40+16,WHITE);
	LCD_ShowString(60,20,200,16,16,"ESP8266 Ready");	
	while(atk_8266_send_cmd("ATE0","OK",20));//关闭回显
	
	atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//设置WIFI STA模式
	//设置连接到的WIFI网络名称/加密方式/密码,这几个参数需要根据您自己的路由器设置进行修改!! 
	sprintf(StringWIFI,"AT+CWJAP=\"%s\",\"%s\"",wifista_ssid,wifista_password);//设置无线参数:ssid,密码
	while(atk_8266_send_cmd((u8 *)StringWIFI,"WIFI GOT IP",300));					//连接目标路由器, 发送命令直到回复
	LCD_Fill(60,20,200,40+16,WHITE);
	LCD_ShowString(60,20,200,16,16,"WIFI connected");	
}

void ESP8266_RESTORE(void)
{
	while(atk_8266_send_cmd("AT","OK",20))//检查WIFI模块是否在线
	{
		atk_8266_quit_trans();//退出透传
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //关闭透传模式	
		LCD_ShowString(60,20,200,16,16,"NOT found ESP8266        ");
		delay_ms(800);
		LCD_Fill(60,20,200,40+16,WHITE);
	} 
	atk_8266_send_cmd("AT+RESTORE","OK",20);		//回复出厂设置
	delay_ms(1000);         //延时3S等待重启成功
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
}

void ESP8266_ConnecPORT(void)
{
	porttate=0;
	sprintf(StringIP,"AT+CIPSTART=\"TCP\",\"%s\",%s",IPnum,portnum);    //配置目标TCP服务器
	while(1)
	{
		if (porttate=='0' || porttate =='3')
		{break;}
		else
		{
			atk_8266_send_cmd((u8 *)StringIP,"OK",200);
	   	porttate=atk_8266_state_check();      //  2连接wifi   4.端口连接不成功   0、3表示连接成
	    LCD_ShowString(60,20,200,16,16,"PORT connecting");	
	   	LCD_ShowChar(180,20,porttate,16,0); //在后面显示连接状态 
		}
	}	
	LCD_ShowString(60,20,200,16,16,"PORT connected   ");	
	
	
	atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //传输模式为：透传	
	atk_8266_send_cmd("AT+CIPSEND","OK",20);         //开始透传     并没有成功进入透传模式   
	
	LCD_ShowString(20,40,200,16,16,"SEND:");	
	LCD_ShowString(20,60,200,16,16,"RECE:");	
	USART2_RX_STA=0;
}

