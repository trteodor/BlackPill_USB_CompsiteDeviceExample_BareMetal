


SRC += applications/USB_otg_Cdc_Hid/source/main.c
SRC += applications/USB_otg_Cdc_Hid/source/usb_cdc_hid_.c

SRC += Drivers/GPIO/GPIO.c
SRC += Drivers/System/System.c
SRC += Drivers/UART_DLT/UART2_dlt.c
SRC += libraries/ucDLT/DLTuc_libFiles/DLTuc.c

ASM += StartUpAndLinkerScript/startup_stm32f401xc.s

INC_DIR += -IDrivers/CMSIS/Device/ST/STM32F4xx/Include
INC_DIR += -IDrivers/CMSIS/Core/Include
INC_DIR += -IDrivers/RCC
INC_DIR += -IDrivers/GPIO
INC_DIR += -IDrivers/System
INC_DIR += -IDrivers/UART_DLT
INC_DIR += -Ilibraries/ucDLT/DLTuc_libFiles

####################################################################################
USB__otg_Cdc_INC_DIR = -IDrivers/USBd_otg_fs/include
USB__otg_Cdc_SOURCES_DIR = $(wildcard Drivers/USBd_otg_fs/source/)
USB__otg_Cdc_SOURCES := $(foreach dir,$(USB__otg_Cdc_SOURCES_DIR),$(wildcard $(dir)*.c))
SRC += $(USB__otg_Cdc_SOURCES)
INC_DIR += $(USB__otg_Cdc_INC_DIR)
####################################################################################

# link script
LDSCRIPT = StartUpAndLinkerScript/STM32F401CCUx_FLASH.ld