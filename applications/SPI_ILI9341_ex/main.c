#include "main.h"

#include "lvgl.h"

#include "lv_examples.h"


static lv_disp_draw_buf_t disp_buf;
 //The second buffer is optional*/

#define ILI_SCR_HORIZONTAL 320
#define ILI_SCR_VERTICAL   240
#define BUFFOR_SCR_ROWS 10

 static lv_color_t buf_1[ILI_SCR_HORIZONTAL * BUFFOR_SCR_ROWS] ;
 static lv_color_t buf_2[ILI_SCR_HORIZONTAL * BUFFOR_SCR_ROWS] ; //__attribute__ ((section (".LvBufferSection")))
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 //DMA don't have acces to CCMRAM :(
 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 //I don't have so too much time on experiments
 static lv_disp_drv_t disp_drv;

int main(void)
{
	PLL_Config84Mhz();
	ConfigSysTick1ms();
	Led_Cfg();
	Uart2InitTransmitWithDMAand_ucDLTlib();

	ILI9341_SPI_Init();


	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, ILI_SCR_HORIZONTAL * BUFFOR_SCR_ROWS);
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = ILI9341_flushLv;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = ILI_SCR_HORIZONTAL;                 /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = ILI_SCR_VERTICAL;                 /*Set the vertical resolution in pixels*/
	lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/

	// lv_indev_drv_t indev_drv;
	// lv_indev_drv_init(&indev_drv);
	// indev_drv.type =LV_INDEV_TYPE_POINTER;
	// indev_drv.read_cb = lvXPT2064_Read;
	// lv_indev_drv_register(&indev_drv);
	lv_example_get_started_1();



#warning I like my own warning :) 

	while(1)
	{

		lv_task_handler();
		lv_tick_inc(5);
		DelayMs(5);
	}
}



