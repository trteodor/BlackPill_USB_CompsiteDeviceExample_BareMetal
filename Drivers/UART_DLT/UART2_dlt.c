#include "stm32f4xx.h"
#include "stdbool.h"

#include "DLTuc.h"
#include "System.h"

static volatile bool UART2_DMA_TransmitEndFlag = true;


void DMA1_Stream6_IRQHandler(void)
{	
	UART2_DMA_TransmitEndFlag = true;
	DMA1->HIFCR |= (DMA_HIFCR_CTCIF6);
	DLTuc_MessageTransmitDone();
}

void UART2_DMA_Transmit(uint8_t *Buff, uint8_t size)
{
	while(UART2_DMA_TransmitEndFlag != true)
	{
		/*Wait for transmit end...*/
	}

	DLTuc_UpdateTimeStampMs(GetSysTime() ); /*Not best way but for me it's inaf solution*/

	DMA1_Stream6->NDTR = size;
	DMA1_Stream6->PAR = (volatile long unsigned int)&USART2->DR;
	DMA1_Stream6->M0AR =  (volatile long unsigned int)Buff;
	UART2_DMA_TransmitEndFlag = false;
	DMA1_Stream6->CR |= DMA_SxCR_EN;
}


void Uart2InitTransmitWithDMAand_ucDLTlib(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER2_1;
	GPIOA->MODER |= GPIO_MODER_MODER3_1;
	GPIOA->AFR[0] |= 7ul << GPIO_AFRL_AFSEL2_Pos;
	GPIOA->AFR[0] |= 7ul << GPIO_AFRL_AFSEL3_Pos;
/*USART*/
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	USART2->SR = 0;
	USART2->BRR = 0x16c; /*115200 for 84Mhz*/
	USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_TXEIE | USART_CR1_RXNEIE | USART_CR1_UE;
	USART2->CR3 |= USART_CR3_DMAT;
	/*DMA_Cfg*/ 	/*DMA1 Channel 4, stream5 RX, stream6 TX*/
	/* Only transmit acutally is used and configured */
	RCC->AHB1ENR  |= RCC_AHB1ENR_DMA1EN;
	DMA1_Stream6->CR = DMA_SxCR_TCIE | DMA_SxCR_DIR_0 | DMA_SxCR_MINC | (4ul << DMA_SxCR_CHSEL_Pos);
	NVIC_SetPriority (DMA1_Stream6_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); 
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);

	/*Register Low Level Transmit function for ucDltLibrary*/
	DLTuc_RegisterTransmitSerialDataCallback(UART2_DMA_Transmit);
	/*Now ucDLTlib is ready to work!*/
}