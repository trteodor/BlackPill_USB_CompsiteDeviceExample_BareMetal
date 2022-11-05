#ifndef _USBD_CORE_H
#define _USBD_CORE_H 1

#include <usb_def.h>

/** USB device core system API **/

int USBD_InitDevApp(void);
void USBDreset(usb_speed_t);
void USBDsuspend(void);
void USBDwakeup(void);
void USBDsof(uint16_t);
void USBDtransfer(uint8_t, usb_pid_t);

#endif
