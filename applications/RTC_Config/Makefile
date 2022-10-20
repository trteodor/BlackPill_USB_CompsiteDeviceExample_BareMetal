
SRC +=  applications/RTC_Config/main.c

SRC += Drivers/GPIO/GPIO.c
SRC += Drivers/System/System.c
SRC += Drivers/UART_DLT/UART2_dlt.c
SRC += Drivers/ILI9341_SPI/ILI9341_SPI.c
SRC += Drivers/RTC/RTC.c
SRC += libraries/dlt_logs_mcu_lib_files/dlt_logs_mcu.c

ASM += StartUpAndLinkerScript/startup_stm32f401xc.s

INC_DIR += -Iapplications/SPI_ILI9341_ex
INC_DIR += -IDrivers/CMSIS/Device/ST/STM32F4xx/Include
INC_DIR += -IDrivers/CMSIS/Core/Include
INC_DIR += -IDrivers/ILI9341_SPI
INC_DIR += -IDrivers/RCC
INC_DIR += -IDrivers/GPIO
INC_DIR += -IDrivers/System
INC_DIR += -IDrivers/UART_DLT
INC_DIR += -IDrivers/RTC

INC_DIR += -Ilibraries/ucDLT/ucDLTlibFiles

# link script
LDSCRIPT = StartUpAndLinkerScript/STM32F401CCUx_FLASH.ld