#ifndef __OTHERS_H
#define __OTHERS_H	 
#include "sys.h"

//端口定义



#define BEEP PCout(5)	// BEEP,蜂鸣器接口		   
#define ATOMI PAout(4)	// PA4----雾化
#define PUMP PAout(5)	// PA5----水泵
#define HEAT PAout(6)	// PA6----电热片
#define FAN PAout(7)	// PA7----风扇
#define ORG_LED PAout(11)	// PA11----橙色灯光
#define RED_LED PAout(12)	// PA12---红色灯光   

void Others_Init(void);	//初始化
		 				    
#endif

