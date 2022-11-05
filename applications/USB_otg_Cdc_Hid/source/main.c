// #Hello world :)

#include "System.h"
#include <usbd_api.h>

#include "GPIO.h"

#define SYS_CLK_MH 72


  void reenumerate()
  {

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
  //reenumerate
    GPIO_PinReset(GPIOA, GPIO_PIN_12);
    DelayMs(500);
    GPIO_PinSet(GPIOA, GPIO_PIN_12);
    DelayMs(500);
  }


int main(void) {
	PLL_Config84Mhz();
  ConfigSysTick1ms();
  // reenumerate(); //Force
	USBd_InitAll(SYS_CLK_MH);
  
  while(1)
  {
     /* Actually nothing TODO: :) */
  }

}
