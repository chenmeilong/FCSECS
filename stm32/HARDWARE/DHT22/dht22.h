#ifndef __DHT22_H
#define __DHT22_H 
#include "sys.h"   

//IO操作函数											   
#define	DHT22_DQ_OUT PCout(4) //数据端口	PC4
#define	DHT22_DQ_IN  PCin(4)  //数据端口	PC4

u8 DHT22_Init(void);			//初始化DHT22
u8 DHT22_Read_Data(u8 *temp,u8 *humi);   //读取温湿度
u8 DHT22_Read_Byte(void);		//读出一个字节
u8 DHT22_Read_Bit(void);		//读出一个位
u8 DHT22_Check(void);			//检测是否存在DHT22
void DHT22_Rst(void);			//复位DHT22    
#endif
