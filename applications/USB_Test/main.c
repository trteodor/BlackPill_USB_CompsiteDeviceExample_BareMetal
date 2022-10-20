#include "main.h"

int main(void)
{
	PLL_Config84Mhz();
	ConfigSysTick1ms();
	Led_Cfg();
	Uart2InitTransmitWithDMAand_ucDLTlib();

	while(1)
	{
		toggleLed();
		DelayMs(500);
	}
}



