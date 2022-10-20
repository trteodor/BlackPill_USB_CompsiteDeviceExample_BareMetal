/*
 * TFT_ILI9341.c
 *
 *  Created on: 12.06, 2020
 *      Author: Teodor R
 */
#include "stm32f4xx.h"
#include "GPIO.h"
#include "ILI9341_SPI.h"
#include "System.h"
#include "stdbool.h"
#include "ucDLTlib.h"



volatile uint32_t flag_DMA_Stream_bsy;
#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

static const uint8_t initcmd[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2
  ILI9341_MADCTL  , 1, 0x48,             // Memory Access Control
  ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
  ILI9341_DISPON  , 0x80,                // Display on
  0x00                                   // End of list
};

/*********************************************************************************************************
***************************************** HW Depend fun section start ************************************
**********************************************************************************************************/

void SPI1_TransmitConfigDMA(void)
{

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pins : PBPin PBPin PBPin */
	GPIO_InitStruct.Pin = TFT_DC_Pin|TFT_RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PAPin PAPin */
	GPIO_InitStruct.Pin = TFT_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5; /*SPI1 pins*/
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Alternate = (0x0000005U);

	GPIO_Init(GPIOB, &GPIO_InitStruct);

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | (0u <<SPI_CR1_BR_Pos) | SPI_CR1_SSM;
	SPI1->CR1 |= SPI_CR1_SPE;
	SPI1->CR2 |= SPI_CR2_TXDMAEN;

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	NVIC_SetPriority(DMA2_Stream3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}



void Send_DMA_Data16(uint16_t* buff, uint16_t dataSize)
{
	SPI1->CR1 &= ~SPI_CR1_SPE; /*disable spi1*/

	SPI1->CR1 |= SPI_CR1_DFF; /*Data frame format 16bit*/

	ILI9341_DC_HIGH;
	ILI9341_CS_LOW;

	DMA2_Stream3->CR &= ~DMA_SxCR_EN; /*turn off dma transfer*/

	DMA2_Stream3->CR = (DMA_SxCR_CHSEL_0 | DMA_SxCR_CHSEL_1)  
		| DMA_SxCR_MINC | DMA_SxCR_TCIE | DMA_SxCR_DIR_0 
		| DMA_SxCR_MINC | DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0;

	DMA2_Stream3->NDTR = dataSize;
	DMA2_Stream3->PAR = (uint32_t)&SPI1->DR;
	DMA2_Stream3->M0AR = (uint32_t)buff;

	SPI1->CR1 |= SPI_CR1_SPE; /*enable spi1*/
	DMA2_Stream3->CR |= DMA_SxCR_EN; /*turn on DMA2_Stream3*/
	flag_DMA_Stream_bsy=1;
}
/********************************************/
void DMA_ILI9341_SPI_TransmitComplete_Callback();

void DMA2_Stream3_IRQHandler(void)
{
	DMA_ILI9341_SPI_TransmitComplete_Callback();
	SPI1->CR1 &= ~SPI_CR1_DFF;
	DMA2->LIFCR |= (DMA_LIFCR_CTCIF3);
	ILI9341_CS_HIGH;
	flag_DMA_Stream_bsy=0;
}
/******************************************************************************************************/
/*0!=SPI_SR_RXNE - Prawda, kontynuuj pętle*/
/*SPI_SR_RXNE!=SPI_SR_RXNE - Falsz, przewij pętle*/
void Send_Data8(uint8_t data) 
{
	while( (SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE ){}; /*Jesli SPI_SR_TXE aktywny - nie blokuj kontekstu*/
	SPI1->DR = data;
    while( (SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE ){}; /*Jesli SPI_SR_RXNE aktywny - nie blokuj kontekstu*/
	(void) SPI1->DR; /*Uniewaznij odebrane dane*/
	while( (SPI1->SR & SPI_SR_BSY) == SPI_SR_BSY){ };
}
/******************************************************************************************************/
static void TransmitConfigSPI(void)
{
	SPI1_TransmitConfigDMA();
}
/******************************************************************************************************/
static void ILI9341_Delay(uint32_t ms)
{
	DelayMs(ms);
}

/*********************************************************************************************************
***************************************** HW Depend section end ******************************************
**********************************************************************************************************/

//
// TFT Functions
//
static void ILI9341_SendToTFT(uint8_t *Byte, uint32_t Length)
{
		ILI9341_CS_LOW;
	    for(uint16_t index =0; index<Length; index++)
	    {
	        Send_Data8( Byte[index]);
	    }
	    ILI9341_CS_HIGH;
}



void ILI9341_SendCommand(uint8_t Command)
{
	// CS Low
#if (ILI9341_USE_CS == 1)
	ILI9341_CS_LOW;
#endif
	// DC to Command - DC to Low
	ILI9341_DC_LOW;

	// Send to TFT 1 byte
	ILI9341_SendToTFT(&Command, 1);
	// CS High
#if (ILI9341_USE_CS == 1)
	ILI9341_CS_HIGH;
#endif
}

void ILI9341_SendCommandAndData(uint8_t Command, uint8_t *Data, uint32_t Length)
{
#if (ILI9341_USE_CS == 1)
	ILI9341_CS_LOW;
#endif
	ILI9341_DC_LOW;
	ILI9341_SendToTFT(&Command, 1);
	ILI9341_DC_HIGH;
	ILI9341_SendToTFT(Data, Length);
#if (ILI9341_USE_CS == 1)
	ILI9341_CS_HIGH;
#endif
}

void ILI9341_SendData16(uint16_t Data)
{
#if (ILI9341_USE_CS == 1)
	ILI9341_CS_LOW;
#endif
	uint8_t tmp[2];
	tmp[0] = (Data>>8);
	tmp[1] = Data & 0xFF;
	ILI9341_DC_HIGH;	// Data mode
	ILI9341_SendToTFT(tmp, 2);
#if (ILI9341_USE_CS == 1)
	ILI9341_CS_HIGH;
#endif
}

void ILI9341_SetRotation(uint8_t Rotation)
{
	if(Rotation > 3)
		return;

	//
	// Set appropriate bits for Rotation
	//
	switch(Rotation)
	{
	case 0:
		Rotation = (MADCTL_MX | MADCTL_BGR);
		break;
	case 1:
		Rotation = (MADCTL_MV | MADCTL_BGR);
		break;
	case 2:
		Rotation = (MADCTL_MY | MADCTL_BGR);
		break;
	case 3:
		Rotation = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		break;
	}

	// Write indo MAD Control register our Rotation data
	ILI9341_SendCommandAndData(ILI9341_MADCTL, &Rotation, 1);
}
void ILI9341_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h)
{
	uint8_t DataToTransfer[4];
	// Calculate end ranges
	uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
	// Fulfill X's buffer
	DataToTransfer[0] = x1 >> 8;
	DataToTransfer[1] = x1 & 0xFF;
	DataToTransfer[2] = x2 >> 8;
	DataToTransfer[3] = x2 & 0xFF;
	// Push X's buffer
	ILI9341_SendCommandAndData(ILI9341_CASET, DataToTransfer, 4);
	// Fulfill Y's buffer
	DataToTransfer[0] = y1 >> 8;
	DataToTransfer[1] = y1 & 0xFF;
	DataToTransfer[2] = y2 >> 8;
	DataToTransfer[3] = y2 & 0xFF;
	// Push Y's buffer
	ILI9341_SendCommandAndData(ILI9341_PASET, DataToTransfer, 4);
}

void ILI9341_WritePixel(int16_t x, int16_t y, uint16_t color)
{
	uint8_t DataToTransfer[2];

	if ((x >= 0) && (x < ILI9341_TFTWIDTH) && (y >= 0) && (y < ILI9341_TFTHEIGHT))
	{
		// Set Window for 1x1 pixel
		ILI9341_SetAddrWindow(x, y, 1, 1);
		// Fulfill buffer with color
		DataToTransfer[0] = color >> 8;
		DataToTransfer[1] = color & 0xFF;
		// Push color bytes to RAM
		ILI9341_SendCommandAndData(ILI9341_RAMWR, DataToTransfer, 2);
	}
}
void ILI9341_ClearDisplay(uint16_t Color)
{
	// Set window for whole screen
	ILI9341_SetAddrWindow(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT);
	// Set RAM writing
	ILI9341_SendCommand(ILI9341_RAMWR);

	for(uint32_t i = 0; i < (ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT); i++)
	{
		// Send Color bytes
		ILI9341_SendData16(Color);
	}
}
void ILI9341_SPI_Init()
{

    uint8_t cmd, x, numArgs;
    const uint8_t *addr = initcmd;

	TransmitConfigSPI();

#if (ILI9341_USE_HW_RESET == 1)
	ILI9341_RST_LOW;
	ILI9341_Delay(10);
	ILI9341_RST_HIGH;
	ILI9341_Delay(10);
#else
	ILI9341_SendCommand(ILI9341_SWRESET); // Engage software reset
    ILI9341_Delay(150);
#endif

    while ((cmd = *(addr++)) > 0)
    {
      x = *(addr++);
      numArgs = x & 0x7F;
      // Push Init data
      ILI9341_SendCommandAndData(cmd, (uint8_t *)addr, numArgs);

      addr += numArgs;

      if (x & 0x80)
      {
    	  ILI9341_Delay(10);
      }
    }
    // Set selected Rotation
    ILI9341_SetRotation(ILI9341_ROTATION);
}

void ILI9341_DrawRectWithoutDMA(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t color)
{
	// Set window for whole screen
	ILI9341_SetAddrWindow(x1, y1, w, h);
	// Set RAM writing
	ILI9341_SendCommand(ILI9341_RAMWR);

	for(uint32_t i = 0; i < (w * h); i++)
	{
		// Send Color bytes
		ILI9341_SendData16(color);
	}
}

void ILI9341_fillRect(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t color) {
    uint16_t tbuf[w*h];
    ILI9341_SetAddrWindow(x1, y1,w,h);
	ILI9341_SendCommand(ILI9341_RAMWR);  //Before Write to RAM use this command!!!

	uint16_t tempi=0;
		for(tempi=0; tempi < (w*h); tempi++)
		{
			tbuf[tempi]=color;
	        // ILI9341_SendData16(color);
		}
       Send_DMA_Data16(tbuf,(w*h));

        while(flag_DMA_Stream_bsy == 1){}
}














void ILI9341_SetAddrWindowLv(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint8_t DataToTransfer[4];
	// Calculate end ranges
	// Fulfill X's buffer
	DataToTransfer[0] = x1 >> 8;
	DataToTransfer[1] = x1 & 0xFF;
	DataToTransfer[2] = x2 >> 8;
	DataToTransfer[3] = x2 & 0xFF;
	// Push X's buffer
	ILI9341_SendCommandAndData(ILI9341_CASET, DataToTransfer, 4);
	// Fulfill Y's buffer
	DataToTransfer[0] = y1 >> 8;
	DataToTransfer[1] = y1 & 0xFF;
	DataToTransfer[2] = y2 >> 8;
	DataToTransfer[3] = y2 & 0xFF;
	// Push Y's buffer
	ILI9341_SendCommandAndData(ILI9341_PASET, DataToTransfer, 4);
}

static lv_disp_drv_t *LastDriver;
uint16_t tbuf[100];


void ILI9341_flushLv(lv_disp_drv_t * drv, const lv_area_t * area,  lv_color_t * color_map)
{
	LastDriver=drv;
	/*ILI9341_SetAddrWindow((uint16_t)area->x1, (uint16_t)area->y1,
							(uint16_t)area->x2-(uint16_t)area->x1 +1, (uint16_t)area->y2-(uint16_t)area->y1 +1);*/
					//Bc my SetAddrWindow function accepts argument x,y,w,h so i fitted into it
												//Buy at the expense of performance
														//Use It to safe some flash memory
	ILI9341_SetAddrWindowLv((uint16_t)area->x1, (uint16_t)area->y1, (uint16_t)area->x2, (uint16_t)area->y2);
	ILI9341_SendCommand(ILI9341_RAMWR);
	uint32_t size = (area->x2 - area->x1 +1) * (area->y2 - area->y1 +1);
	Send_DMA_Data16( (uint16_t *)color_map, size);
}
void DMA_ILI9341_SPI_TransmitComplete_Callback()
{
	lv_disp_flush_ready(LastDriver);
}
