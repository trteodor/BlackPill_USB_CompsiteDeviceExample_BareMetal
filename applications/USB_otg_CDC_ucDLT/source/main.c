// #Hello world :)

#include "System.h"
#include <usbd_api.h>

#include "GPIO.h"

#define SYS_CLK_MH 72


/*DLT_Viewer jest wkurzajacy :( - Ale db dziala to dziadostwo :) */

#include "DLTuc.h"

/*I'am too lazy to create header file for CDC :( )*/
extern uint8_t VCP__IsOpenedtatus();
extern void EP1IN(); /*EP1IN skonczyl nadawac - CALL_BACK od stosu USB*/
static uint32_t MessageStartTime = 0;

void EP1IN()
{
      MessageStartTime = GetSysTime();
      DLTuc_MessageTransmitDone(); /*Inform ucDLTlib about message transmission end*/
}

// /*CallBacks used by ucDltLibrary section start..*/
void SysTickCallBack(uint32_t TickCount)
{
	DLTuc_UpdateTimeStampMs(TickCount); /*Inform ucDLTlib about TimeStamp*/

  if(GetSysTime() - 500 > MessageStartTime)
  {
    DLTuc_MessageTransmitDone(); /*Inform ucDLTlib about message transmission end*/
    /*Timeout - for ex. port disconnected*/
  }
}

void USB_VCP_ucDLT_DataTransmit(uint8_t *DltLogData, uint8_t Size)
{
  if( VCP__IsOpenedtatus() )
  {
        USBDwriteEx(ENDP1, DltLogData, Size);
  }
  else
  {
    /*Receiver don't exist...*/
    DLTuc_MessageTransmitDone(); /*Inform ucDLTlib about message transmission end*/
  }
}
/**/
  // void reenumerate()
  // {

  //   GPIO_InitTypeDef GPIO_InitStruct;
  //   GPIO_InitStruct.Pin = GPIO_PIN_12;
  //   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  //   GPIO_InitStruct.Pull = GPIO_NOPULL;
  //   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  //   GPIO_Init(GPIOA, &GPIO_InitStruct);
  // //reenumerate
  //   GPIO_PinReset(GPIOA, GPIO_PIN_12);
  //   DelayMs(500);
  //   GPIO_PinSet(GPIOA, GPIO_PIN_12);
  //   DelayMs(500);
  // }


int main(void) {
	PLL_Config84Mhz();
  ConfigSysTick1ms();
  // reenumerate(); //Force
	USBd_InitAll(SYS_CLK_MH);


 	DelayMs(2000); 

  while( VCP__IsOpenedtatus() == 0 )
  {
    /*Wait for port open*/
  }

  RegisterSysTickCallBack(SysTickCallBack);
  DLTuc_RegisterTransmitSerialDataCallback(USB_VCP_ucDLT_DataTransmit);
	/*Now ucDLTlib is ready to work!*/
	DEBUGL(DL_INFO, "DLT TESTS START!!!");

	DelayMs(100);
	/*LOG DROP TEST*/
	for(int i=0; i<20; i++)
	{
		DEBUGL(DL_INFO, "Log Drop testing0 :)  %d" , i);
		DEBUGL(DL_INFO, "Log Drop testing1 :)  %d" , i);
		DEBUGL(DL_INFO, "Log Drop testing2:)  %d" , i);
		DEBUGL(DL_INFO, "Log Drop testing3 :)  %d" , i);
		DEBUGL(DL_INFO, "Log Drop testing4 :)  %d" , i);
	}
		DEBUGL(DL_INFO, "Dropped log...  %d" , 5);
		DEBUGL(DL_FATAL, "Dropped log...   %d" , 5);
	DelayMs(100);

	while(1)
	{
		/*Send example Logs in loop...*/
		DEBUGL(DL_ERROR, "Hello DLT Again Arg1 %d Arg2 :%d" , 2565, 56);
		DEBUGFF(DL_FATAL, "GENERALLY DLT Again1");
		DEBUGF(DL_DEBUG, "AnotherTest DLT Again");
		DEBUGL(DL_VERBOSE, "AnotherTest2 DLT Again");
		DelayMs(1000);
		DEBUGL(DL_WARN, "Orange is sweet fruit");
		DelayMs(1000);
	}

  for(;;)
  {

  }

}
