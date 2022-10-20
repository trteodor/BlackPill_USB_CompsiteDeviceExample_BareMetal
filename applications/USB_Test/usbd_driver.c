#include "usbd_driver.h"
#include "string.h"
#include "GPIO.h"


#define USB_OTG_FS_DP_Pin GPIO_PIN_12
#define USB_OTG_FS_DP_Port GPIOA
#define USB_OTG_FS_DM_Pin GPIO_PIN_11
#define USB_OTG_FS_DM_Port GPIOA

#define GPIO_AF10_OTG_FS        ((uint8_t)0x0A)


static void initialize_gpio_pins()
{
	// Enables the clock for GPIOA.
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);

	// // Sets alternate function 12 for: PB14 (-), and PB15 (+).
	// MODIFY_REG(GPIOA->AFR[1],
	// 	GPIO_AFRH_AFSEL11 | GPIO_AFRH_AFSEL12,
	// 	_VAL2FLD(GPIO_AFRH_AFSEL11, 0xa) | _VAL2FLD(GPIO_AFRH_AFSEL12, 0xa)
	// );

	// // Configures USB pins (in GPIOB) to work in alternate function mode.
	// MODIFY_REG(GPIOA->MODER,
	// 	GPIO_MODER_MODER11 | GPIO_MODER_MODER12,
	// 	_VAL2FLD(GPIO_MODER_MODER11, 2) | _VAL2FLD(GPIO_MODER_MODER12, 2)
	// );

    GPIO_InitTypeDef GPIO_InitStruct;
	/*Configure GPIO pins : PBPin PBPin PBPin */
	GPIO_InitStruct.Pin = USB_OTG_FS_DP_Pin|USB_OTG_FS_DM_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

}

static void initialize_core()
{
	// Enables the clock for USB core.
	SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_OTGFSEN);

	while(!(READ_BIT(RCC->AHB2ENR,RCC_AHB2ENR_OTGFSEN)));

	for(volatile int i=0; i<100; i++);
	{

	}
	// Configures the USB core to run in device mode, and to use the embedded full-speed PHY.
	MODIFY_REG(USB_OTG_FS_GLOBAL->GUSBCFG,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | USB_OTG_GUSBCFG_TRDT,
		USB_OTG_GUSBCFG_FDMOD | USB_OTG_GUSBCFG_PHYSEL | _VAL2FLD(USB_OTG_GUSBCFG_TRDT, 0x06)
	);

	// Configures the device to run in full speed mode.
	MODIFY_REG(USB_OTG_FS_DEVICE->DCFG,
		USB_OTG_DCFG_DSPD,
		_VAL2FLD(USB_OTG_DCFG_DSPD, 0x03)
	);

	// Enables VBUS sensing device.
	SET_BIT(USB_OTG_FS_GLOBAL->GCCFG, USB_OTG_GCCFG_VBUSBSEN);

	// Unmasks the main USB core interrupts.
	SET_BIT(USB_OTG_FS_GLOBAL->GINTMSK,
		USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_SOFM | 
		
		USB_OTG_GUSBCFG_SRPCAP | USB_OTG_GUSBCFG_HNPCAP |
		
		USB_OTG_GINTMSK_USBSUSPM | USB_OTG_GINTMSK_WUIM | USB_OTG_GINTMSK_IEPINT |
		USB_OTG_GINTSTS_OEPINT | USB_OTG_GINTMSK_RXFLVLM
	);

	// Clears all pending core interrupts.
	WRITE_REG(USB_OTG_FS_GLOBAL->GINTSTS, 0xFFFFFFFF);

	// Unmasks USB global interrupt.
	SET_BIT(USB_OTG_FS_GLOBAL->GAHBCFG, USB_OTG_GAHBCFG_GINT);

	// Unmasks transfer completed interrupts for all endpoints.
	SET_BIT(USB_OTG_FS_DEVICE->DOEPMSK, USB_OTG_DOEPMSK_XFRCM);
	SET_BIT(USB_OTG_FS_DEVICE->DIEPMSK, USB_OTG_DIEPMSK_XFRCM);
}

/** \brief Connects the USB device to the bus.
 */
static void connect()
{
	// Powers the transceivers on.
    SET_BIT(USB_OTG_FS_GLOBAL->GCCFG, USB_OTG_GCCFG_PWRDWN);

	// Connects the device to the bus. no nwm co z tym czeba sprawdzic datasheeta
    CLEAR_BIT(USB_OTG_FS_DEVICE->DCTL, USB_OTG_DCTL_SDIS);
}

void localUSB_TEMPORARYinit(void)
{
    initialize_gpio_pins();
    initialize_core();
    connect();
}


/** \brief Handles the USB core interrupts.
 */
void gintsts_handler(void)
{
	volatile uint32_t gintsts = USB_OTG_FS_GLOBAL->GINTSTS;

	if (gintsts & USB_OTG_GINTSTS_USBRST)
	{
		//usbrst_handler();
		// Clears the interrupt.
		SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_USBRST);
	}
	else if (gintsts & USB_OTG_GINTSTS_ENUMDNE)
	{
		//enumdne_handler();
		// Clears the interrupt.
		SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_ENUMDNE);
	}
	else if (gintsts & USB_OTG_GINTSTS_RXFLVL)
	{
		//rxflvl_handler();
		// Clears the interrupt.
		SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_RXFLVL);
	}
	else if (gintsts & USB_OTG_GINTSTS_IEPINT)
	{
		//iepint_handler();
		// Clears the interrupt.
		SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_IEPINT);
	}
	else if (gintsts & USB_OTG_GINTSTS_OEPINT)
	{
		//oepint_handler();
		// Clears the interrupt.
		SET_BIT(USB_OTG_FS_GLOBAL->GINTSTS, USB_OTG_GINTSTS_OEPINT);
	}
}