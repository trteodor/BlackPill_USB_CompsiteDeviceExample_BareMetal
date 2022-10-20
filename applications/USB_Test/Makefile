

SRC +=  applications/USB_Test/main.c

SRC += Drivers/GPIO/GPIO.c
SRC += Drivers/System/System.c
SRC += Drivers/UART_DLT/UART2_dlt.c
SRC += libraries/ucDLT/DLTuc_libFiles/DLTuc.c


ASM += StartUpAndLinkerScript/startup_stm32f401xc.s

INC_DIR += -Iapplications/SPI_ILI9341_ex
INC_DIR += -IDrivers/CMSIS/Device/ST/STM32F4xx/Include
INC_DIR += -IDrivers/CMSIS/Core/Include
INC_DIR += -IDrivers/RCC
INC_DIR += -IDrivers/GPIO
INC_DIR += -IDrivers/System
INC_DIR += -IDrivers/UART_DLT
INC_DIR += -Ilibraries/ucDLT/DLTuc_libFiles

#usb
SRC +=  applications/USB_Test/usbd_driver.c

# link script
LDSCRIPT = StartUpAndLinkerScript/STM32F401CCUx_FLASH.ld