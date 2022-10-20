#include "main.h"

int main(void)
{
	PLL_Config84Mhz();
	ConfigSysTick1ms();
	Led_Cfg();
	toggleLed();
//	Uart2InitTransmitWithDMAand_ucDLTlib();

	localUSB_TEMPORARYinit();

	while(1)
	{
		gintsts_handler();
		// toggleLed();
		// DelayMs(500);
	}
}



