
#include "i2c_wii.h"

#include "GPIO.h"

#define GPIO_AF4_I2C                 ((uint8_t)0x04)

void I2C_Wii_Init(void)
{
    	// Enable the I2C CLOCK and GPIO CLOCK
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  // enable I2C CLOCK
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;  // Enable GPIOB CLOCK
	

    // GPIO_InitTypeDef GPIO_InitStruct;
    // GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    // GPIO_InitStruct.Pull = GPIO_PULLUP;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // GPIO_InitStruct.Alternate = GPIO_AF4_I2C;
    // GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Configure the I2C PINs for ALternate Functions
	GPIOB->MODER |= (2<<16) | (2<<18);  // Bits (17:16)= 1:0 --> Alternate Function for Pin PB8; Bits (19:18)= 1:0 --> Alternate Function for Pin PB9
	GPIOB->OTYPER |= (1<<8) | (1<<9);  //  Bit8=1, Bit9=1  output open drain
	GPIOB->OSPEEDR |= (3<<16) | (3<<18);  // Bits (17:16)= 1:1 --> High Speed for PIN PB8; Bits (19:18)= 1:1 --> High Speed for PIN PB9
	GPIOB->PUPDR |= (1<<16) | (1<<18);  // Bits (17:16)= 0:1 --> Pull up for PIN PB8; Bits (19:18)= 0:1 --> pull up for PIN PB9
	GPIOB->AFR[1] |= (4<<0) | (4<<4);  // Bits (3:2:1:0) = 0:1:0:0 --> AF4 for pin PB8;  Bits (7:6:5:4) = 0:1:0:0 --> AF4 for pin PB9

	// Reset the I2C 
	I2C1->CR1 |= I2C_CR1_SWRST;
	I2C1->CR1 &= ~I2C_CR1_SWRST;
	
	I2C1->CR2 |= (42<<0);
	I2C1->CCR = 210<<0;
	I2C1->TRISE = 43; 
	I2C1->CR1 |= I2C_CR1_PE;  // Enable I2C
}

void I2C_Start (void)
{
	I2C1->CR1 |= I2C_CR1_ACK;  
	I2C1->CR1 |= I2C_CR1_START;  // Generate START
	while (!(I2C1->SR1 & I2C_SR1_SB));  // Wait fror SB bit to set
}

void I2C_Write (uint8_t data)
{
	while (!(I2C1->SR1 & I2C_SR1_TXE));  // wait for TXE bit to set
	I2C1->DR = data;
	while (!(I2C1->SR1 & I2C_SR1_BTF));  // wait for BTF bit to set
}

void I2C_Address (uint8_t Address)
{
	I2C1->DR = Address;
	while (!(I2C1->SR1 & I2C_SR1_ADDR));
	uint8_t temp = I2C1->SR1 | I2C1->SR2;
	(void)temp;
}

void I2C_Stop (void)
{
	I2C1->CR1 |= I2C_CR1_STOP;
}

void I2C_WriteMulti(uint8_t *data, uint8_t size)
{

	while (!(I2C1->SR1 & I2C_SR1_TXE));
	while (size)
	{
		while (!(I2C1->SR1 & I2C_SR1_TXE)); 
		I2C1->DR = (uint32_t )*data++;
		size--;
	}
	while (!(I2C1->SR1 & I2C_SR1_BTF));
}

void I2C_Read (uint8_t Address, uint8_t *buffer, uint8_t size)
{
	int remaining = size;
	
	if (size == 1)
	{
		I2C1->DR = Address;
		while (!(I2C1->SR1 & I2C_SR1_ADDR));
		I2C1->CR1 &= ~I2C_CR1_ACK;
		uint8_t temp = I2C1->SR1 | I2C1->SR2;
		(void)temp;
		I2C1->CR1 |= I2C_CR1_STOP;
		while (!(I2C1->SR1 & I2C_SR1_RXNE));
		buffer[size-remaining] = I2C1->DR;
	}	
	else 
	{
		I2C1->DR = Address;
		while (!(I2C1->SR1 & I2C_SR1_ADDR));
		uint8_t temp = I2C1->SR1 | I2C1->SR2;
		(void)temp;
		while (remaining>2)
		{
			while (!(I2C1->SR1 & I2C_SR1_RXNE));
			buffer[size-remaining] = I2C1->DR;
			I2C1->CR1 |= I2C_CR1_ACK;
			remaining--;
		}
		while (!(I2C1->SR1 & I2C_SR1_RXNE));
		buffer[size-remaining] = I2C1->DR;
		I2C1->CR1 &= ~I2C_CR1_ACK; 
		I2C1->CR1 |= I2C_CR1_STOP;
		remaining--;
		while (!(I2C1->SR1 & I2C_CR1_ENGC));
		buffer[size-remaining] = I2C1->DR;
	}	
}

// I2C_Wii_Master_Transmit(WII_CONTROLLER_ID, 0x00, 1);
void I2C_Wii_Master_Transmit(uint16_t DevAddr, uint8_t *pData, uint16_t Size)
{
	I2C_Start ();
	I2C_Address (DevAddr);
    I2C_WriteMulti(pData,Size);
	I2C_Stop ();
}

void I2C_Wii_Master_Receive(uint16_t DevAddr, uint8_t Reg, uint8_t *pData, uint16_t Size)
{
    I2C_Start ();
	I2C_Address (DevAddr);
	I2C_Write (Reg);
	I2C_Stop ();
	I2C_Start ();  // repeated start
	I2C_Read (DevAddr+0x01, pData, Size); //Addres +1 to read (Addres is shifted this (1) means read... )
	I2C_Stop ();
}