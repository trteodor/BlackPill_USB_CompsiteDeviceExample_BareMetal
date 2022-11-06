


SRC += applications/USB_otg_CDC_ucDLT_Wii/source/main.c
SRC += applications/USB_otg_CDC_ucDLT_Wii/source/USB_ucDLT_Wii.c

SRC += Drivers/GPIO/GPIO.c
SRC += Drivers/System/System.c
SRC += Drivers/UART_DLT/UART2_dlt.c
SRC += Drivers/i2c_wii/i2c_wii.c
SRC += libraries/ucDLT/DLTuc_libFiles/DLTuc.c
SRC += Drivers/WII_gamecontroller/wii_cc.c

ASM += StartUpAndLinkerScript/startup_stm32f401xc.s

INC_DIR += -IDrivers/CMSIS/Device/ST/STM32F4xx/Include
INC_DIR += -IDrivers/CMSIS/Core/Include
INC_DIR += -IDrivers/RCC
INC_DIR += -IDrivers/GPIO
INC_DIR += -IDrivers/System
INC_DIR += -IDrivers/UART_DLT
INC_DIR += -IDrivers/i2c_wii
INC_DIR += -Ilibraries/ucDLT/DLTuc_libFiles
INC_DIR += -IDrivers/WII_gamecontroller

####################################################################################
USB__otg_Cdc_INC_DIR = -IDrivers/USBd_otg_fs/include
USB__otg_Cdc_SOURCES_DIR = $(wildcard Drivers/USBd_otg_fs/source/)
USB__otg_Cdc_SOURCES := $(foreach dir,$(USB__otg_Cdc_SOURCES_DIR),$(wildcard $(dir)*.c))
SRC += $(USB__otg_Cdc_SOURCES)
INC_DIR += $(USB__otg_Cdc_INC_DIR)
####################################################################################

# link script
LDSCRIPT = StartUpAndLinkerScript/STM32F401CCUx_FLASH.ld