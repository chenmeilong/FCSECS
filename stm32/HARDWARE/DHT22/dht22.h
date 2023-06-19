#ifndef __DHT22_H
#define __DHT22_H 
#include "sys.h"   

//IO��������											   
#define	DHT22_DQ_OUT PCout(4) //���ݶ˿�	PC4
#define	DHT22_DQ_IN  PCin(4)  //���ݶ˿�	PC4

u8 DHT22_Init(void);			//��ʼ��DHT22
u8 DHT22_Read_Data(u8 *temp,u8 *humi);   //��ȡ��ʪ��
u8 DHT22_Read_Byte(void);		//����һ���ֽ�
u8 DHT22_Read_Bit(void);		//����һ��λ
u8 DHT22_Check(void);			//����Ƿ����DHT22
void DHT22_Rst(void);			//��λDHT22    
#endif
