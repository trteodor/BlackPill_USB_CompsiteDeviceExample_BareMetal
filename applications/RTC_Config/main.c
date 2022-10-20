#include "main.h"
#include "RTC.h"

int main()
{
	PLL_Config84Mhz();
	ConfigSysTick1ms();
	EnablePwrDomain();
	Led_Cfg();
	Uart2InitTransmitWithDMAand_ucDLTlib();
	DEBUGL(DL_INFO,"Hello");

	// RCC -> CFGR |= 25 << 16;
	// RCC -> BDCR |= RCC_BDCR_RTCSEL;
	// RCC -> BDCR |= RCC_BDCR_RTCEN;

	RTC_Time X;
	X.am_pm = 0;
	X.hour = 21;
	X.min = 55;
	X.seconds = 23;

	RTC_Date Y;
	Y.day = 9;
	Y.month = 10;
	Y.year = 2022;
	Y.week_day = Monday;

	RTC_Init();
	RTC_Set_Date(&Y);
	RTC_Set_Time(&X, Time_Format_12Hour);
	RTC_Start();
	DelayMs(100);
	while(1)
	{
		RTC_Get_Date(&Y);
		RTC_Get_Time(&X);
		DEBUGL(DL_INFO,"Date: %d/%d/%d \r\n",Y.day,Y.month,Y.year);
		DEBUGL(DL_INFO,"Time: %d:%d:%d \r\n", X.hour, X.min, X.seconds);
		DelayMs(1000);
	}
}