#include "stm32f4xx.h"
#include <stddef.h>

volatile uint32_t SysTime=0;
static void (*StSysTickCallBack)(uint32_t TickCount) = NULL;
void SysTick_Handler(void);

/**********************************************************************************************/
void ConfigSysTick1ms(void)
{ 
	/*Config systick IRQ period 1ms - but for PLL eequal 84Mhz, i'am too lazy to explain simple...*/
	SysTick_Config(10500ul);
	SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;
	NVIC_SetPriority(SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
}

/******************************************************************************************************************/
void SysTick_Handler(void)
{
	SysTime++;
	if(StSysTickCallBack != NULL)
	{
		StSysTickCallBack(SysTime);
	}
}
/******************************************************************************************************************/
uint32_t GetSysTime(void)
{
	return SysTime;
}
/******************************************************************************************************************/
void RegisterSysTickCallBack(void(*SysTickCallBack)(uint32_t TickCount))
{
	StSysTickCallBack = SysTickCallBack;
}
/******************************************************************************************************************/
void DelayMs(uint32_t Ticks)
{
	volatile uint32_t StartTicks = SysTime;

	while( (SysTime - StartTicks) < Ticks) {
		/*Waiting*/
	}
}
/**********************************************************************************************/
void PLL_Config84Mhz(void)
{
	/*zweryfikowano 2x - ma byc dobrze*/
	/*Na usb powinno isc tez 48Mhz :/ */
	RCC->CR &= ~RCC_CR_HSEBYP;
	RCC->CR |= RCC_CR_HSEON;
	RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE | 25ul << 0 /*M*/ | 336ul<<6 /*N*/ 
										| 1ul << 16 /*P*/ | 7ul << 24 /*Q */      
										| 1ul << 29; /*bit 29 by default is equal 1 */ 
	RCC->SSCGR = 500ul<<0 | 44ul<<13 | RCC_SSCGR_SSCGEN;
	while (!(RCC->CR & RCC_CR_HSERDY));
	RCC->CR |= RCC_CR_PLLON;

	RCC->CFGR = RCC_CFGR_PPRE1_DIV2;
	FLASH->ACR = FLASH_ACR_DCRST | FLASH_ACR_ICRST;
	FLASH->ACR = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_2WS;
	while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2WS);
	while (!(RCC->CR & RCC_CR_PLLRDY));
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	RCC->CR &= ~RCC_CR_HSION;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	__DSB();
	SYSCFG->CMPCR = SYSCFG_CMPCR_CMP_PD;
	while (!(SYSCFG->CMPCR & SYSCFG_CMPCR_READY));
}
/**********************************************************************************************/
void EnablePwrDomain(void)
{
	RCC -> APB1ENR |= RCC_APB1ENR_PWREN;
	PWR -> CR |= PWR_CR_DBP;
}
/**********************************************************************************************/
void EntryStandby(void)
{
	PWR->CR |= PWR_CR_LPDS; /*!< Low-Power Deepsleep (Turn Of Voltage regulator when DeepSleep) */
	PWR->CR |= PWR_CR_PDDS; /*(Set to Standby) !< If Set to fall in Standby */
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; /*(Set to Standby) Set For StopMode*/
	__WFI();
}
/**********************************************************************************************/
void EntryStopMode(void)
{
	PWR->CR |= PWR_CR_LPDS; /*!< Low-Power Deepsleep (Turn Of Voltage regulator when DeepSleep) */
	PWR->CR &= ~PWR_CR_PDDS; /*(Clean) !< If Set Power then fall in Standby */
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; /*(Set to Standby) Set For StopMode*/
	__WFI();
}
/**********************************************************************************************/
void EntryWFI(void)
{
	PWR->CR &= ~PWR_CR_LPDS; /*!< Low-Power Deepsleep (Turn Of Voltage regulator when DeepSleep) */
	PWR->CR &= ~PWR_CR_PDDS; /*(Clean) !< If Set Power then fall in Standby */
	SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk; /*Clear deep sleep bit*/
	__WFI();
}
/**********************************************************************************************/