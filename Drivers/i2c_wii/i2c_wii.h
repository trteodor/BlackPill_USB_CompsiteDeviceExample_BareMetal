

#ifndef __I2C_WII_H__
#define __I2C_WII_H__

#include "stm32f4xx.h"

void I2C_Wii_Init(void);

void I2C_Wii_Master_Transmit(uint16_t DevAddr, uint8_t *pData, uint16_t Size);
void I2C_Wii_Master_Receive(uint16_t DevAddr, uint8_t Reg, uint8_t *pData, uint16_t Size);



#endif //__I2C_WII_H__

