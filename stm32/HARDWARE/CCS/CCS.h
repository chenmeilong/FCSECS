#ifndef __CCS_H
#define __CCS_H	 
#include "sys.h"

#define  ON_CS()    {GPIO_ResetBits(GPIOC,GPIO_Pin_1);}//delay_us(1);
#define  OFF_CS()   {GPIO_SetBits(GPIOC,GPIO_Pin_1);}//delay_us(1);
void CCS_GPIO_init(void);
#endif

