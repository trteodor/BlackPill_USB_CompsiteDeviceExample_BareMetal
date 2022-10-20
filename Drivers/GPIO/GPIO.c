
#include "GPIO.h"
#include "stm32f4xx.h"



void GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
  uint32_t position;
  uint32_t ioposition = 0x00U;
  uint32_t iocurrent = 0x00U;
  uint32_t temp = 0x00U;

  for(position = 0U; position < GPIO_NUMBER; position++)
  {
    ioposition = 0x01U << position;
    iocurrent = (uint32_t)(GPIO_Init->Pin) & ioposition;

    if(iocurrent == ioposition)
    {
      if(((GPIO_Init->Mode & GPIO_MODE) == MODE_OUTPUT) || \
          (GPIO_Init->Mode & GPIO_MODE) == MODE_AF)
      {
        temp = GPIOx->OSPEEDR; 
        temp &= ~(GPIO_OSPEEDER_OSPEEDR0 << (position * 2U));
        temp |= (GPIO_Init->Speed << (position * 2U));
        GPIOx->OSPEEDR = temp;
        temp = GPIOx->OTYPER;
        temp &= ~(GPIO_OTYPER_OT_0 << position) ;
        temp |= (((GPIO_Init->Mode & OUTPUT_TYPE) >> OUTPUT_TYPE_Pos) << position);
        GPIOx->OTYPER = temp;
       }

      if((GPIO_Init->Mode & GPIO_MODE) != MODE_ANALOG)
      {
        temp = GPIOx->PUPDR;
        temp &= ~(GPIO_PUPDR_PUPDR0 << (position * 2U));
        temp |= ((GPIO_Init->Pull) << (position * 2U));
        GPIOx->PUPDR = temp;
      }
      if((GPIO_Init->Mode & GPIO_MODE) == MODE_AF)
      {
        temp = GPIOx->AFR[position >> 3U];
        temp &= ~(0xFU << ((uint32_t)(position & 0x07U) * 4U)) ;
        temp |= ((uint32_t)(GPIO_Init->Alternate) << (((uint32_t)position & 0x07U) * 4U));
        GPIOx->AFR[position >> 3U] = temp;
      }
      temp = GPIOx->MODER;
      temp &= ~(GPIO_MODER_MODER0 << (position * 2U));
      temp |= ((GPIO_Init->Mode & GPIO_MODE) << (position * 2U));
      GPIOx->MODER = temp;
    }
  }
}


void tooglePIN(GPIO_TypeDef *GPIOx, GPIO_PinMask_t GPIO_Pin)
{
  uint32_t odr;
  odr = GPIOx->ODR;
  GPIOx->BSRR = ((odr & GPIO_Pin) << GPIO_NUMBER) | (~odr & GPIO_Pin); //taki trick
}


void GPIO_PinSet(GPIO_TypeDef *GPIOx, GPIO_PinMask_t GPIO_Pin)
{
  GPIOx->BSRR = GPIO_Pin;
}

void GPIO_PinReset(GPIO_TypeDef *GPIOx, GPIO_PinMask_t GPIO_Pin)
{
  uint32_t odr;
  odr = GPIOx->ODR;
  GPIOx->BSRR = ((odr & GPIO_Pin) << GPIO_NUMBER);
}

void Led_Cfg(void)
{
	/*tooglePIN(GPIOC,Psm13);*/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : PBPin PBPin PBPin */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void toggleLed(void)
{
	tooglePIN(GPIOC,Psm13);
}