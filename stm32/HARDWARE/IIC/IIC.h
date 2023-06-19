#ifndef _IIC_H
#define _IIC_H
#include "stm32f10x.h"
#define SCL_H         GPIOC->BSRR = GPIO_Pin_12
#define SCL_L         GPIOC->BRR  = GPIO_Pin_12
   
#define SDA_H         GPIOC->BSRR = GPIO_Pin_11
#define SDA_L         GPIOC->BRR  = GPIO_Pin_11

#define SCL_read      GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)
#define SDA_read      GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11)
void I2C_GPIO_Config(void);
void I2C_Stop(void);
u8 Single_WriteI2C_byte(u8 Slave_Address,u8 REG_Address,u8 data);
u8 Single_MWriteI2C_byte(u8 Slave_Address,u8 REG_Address,u8 const *data,u8 length);
u8 Single_ReadI2C(u8 Slave_Address,u8 REG_Address,u8 *REG_data,u8 length);
#endif
