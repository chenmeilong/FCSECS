#ifndef __ESP8266_H
#define __ESP8266_H	 
#include "sys.h"
#include "lcd.h"
#include "usart2.h" 
#include "delay.h"	
#include "string.h" 
#include "usart.h"		
#include "stdlib.h"
#include "stdio.h"


void ESP8266_ConnecWIFI(void);
void ESP8266_ConnecPORT(void);
void ESP8266_RESTORE(void);         //÷ÿ∆Ù
void atk_8266_at_response(u8 mode);
u8* atk_8266_check_cmd(u8 *str);
u8 atk_8266_quit_trans(void);
u8 atk_8266_consta_check(void);
u8 atk_8266_state_check(void);
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
		 				    
#endif
