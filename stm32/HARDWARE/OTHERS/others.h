#ifndef __OTHERS_H
#define __OTHERS_H	 
#include "sys.h"

//�˿ڶ���



#define BEEP PCout(5)	// BEEP,�������ӿ�		   
#define ATOMI PAout(4)	// PA4----��
#define PUMP PAout(5)	// PA5----ˮ��
#define HEAT PAout(6)	// PA6----����Ƭ
#define FAN PAout(7)	// PA7----����
#define ORG_LED PAout(11)	// PA11----��ɫ�ƹ�
#define RED_LED PAout(12)	// PA12---��ɫ�ƹ�   

void Others_Init(void);	//��ʼ��
		 				    
#endif

