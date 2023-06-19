#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"


void TIM3_Int_Init(u16 arr,u16 psc); 
 u8 Check_state(void);      //Ð£¼ìÊý¾Ý
 void Control_Drive(void);
 
#endif
