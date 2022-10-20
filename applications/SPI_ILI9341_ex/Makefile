

SRC +=  applications/SPI_ILI9341_ex/main.c

SRC += Drivers/GPIO/GPIO.c
SRC += Drivers/System/System.c
SRC += Drivers/UART_DLT/UART2_dlt.c
SRC += Drivers/ILI9341_SPI/ILI9341_SPI.c
SRC += libraries/ucDLT/ucDLTlibFiles/ucDLTlib.c


ASM += StartUpAndLinkerScript/startup_stm32f401xc.s



INC_DIR += -Iapplications/SPI_ILI9341_ex
INC_DIR += -IDrivers/CMSIS/Device/ST/STM32F4xx/Include
INC_DIR += -IDrivers/CMSIS/Core/Include
INC_DIR += -IDrivers/ILI9341_SPI
INC_DIR += -IDrivers/RCC
INC_DIR += -IDrivers/GPIO
INC_DIR += -IDrivers/System
INC_DIR += -IDrivers/UART_DLT
INC_DIR += -Ilibraries/ucDLT/ucDLTlibFiles

lvgl_src_ex = $(wildcard libraries/lvgl/examples/)
lvgl_src_ex1 = $(wildcard libraries/lvgl/examples/get_started/)

lvgl_src = $(wildcard libraries/lvgl/src/*/)
lvgl_src1 = $(wildcard libraries/lvgl/src/*/*/)
lvgl_src2 = $(wildcard libraries/lvgl/src/*/*/*)
lvgl_src3 = $(wildcard libraries/lvgl/src/*/*/*/*)


DIRECTORIES_LVGL += $(sort $(dir $(wildcard $(lvgl_src_ex))))
#DIRECTORIES_LVGL += $(sort $(dir $(wildcard $(lvgl_src_ex1))))
DIRECTORIES_LVGL += $(sort $(dir $(wildcard $(lvgl_src))))
DIRECTORIES_LVGL += $(sort $(dir $(wildcard $(lvgl_src1))))
DIRECTORIES_LVGL += $(sort $(dir $(wildcard $(lvgl_src2))))
DIRECTORIES_LVGL += $(sort $(dir $(wildcard $(lvgl_src3))))

DIRECTORIES_LVGL_OUT = $(sort $(dir $(wildcard $(DIRECTORIES_LVGL))))

LVGL_SRCC := $(foreach dir,$(DIRECTORIES_LVGL_OUT),$(wildcard $(dir)*.c))
LVGL_SRCC += $(foreach dir,$(lvgl_src_ex1),$(wildcard $(dir)*.c))

LVGL_INC = $(foreach dir, $(DIRECTORIES_LVGL_OUT), $(addprefix -I, $(dir)))

#$(info OnlyDirs: $(LVGL_SRCC657))

SRC += $(LVGL_SRCC)
INC_DIR += $(LVGL_INC)

# link script
LDSCRIPT = StartUpAndLinkerScript/STM32F401CCUx_FLASH.ld