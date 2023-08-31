################################################################################
# Set the MCU
################################################################################
MCU?=STM32F103xB
#MCU?=STM32F411xE

DIS?=ST7735

OPT = -O1 -g3

################################################################################
# Set Output Filename 
################################################################################
TARGET = dsmr

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../ucdev


U8G2_DIR = $(UCDEV_ROOT)/ext/u8g2/csrc
UCG_DIR = $(UCDEV_ROOT)/ext/ucglib/csrc

################################################################################
# Sources
################################################################################

C_SOURCES += ucglib_demo.c
C_SOURCES += dsmr.c


ifneq (,$(findstring RV32,$(MCU)))
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_uart_stm32.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_gpio_stm32.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c		
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f1.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f1.c
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/release_speed/riscv_rv32imac
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/ucg/release_speed/riscv_rv32imac

endif 

ifneq (,$(findstring STM32,$(MCU)))
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_uart_stm32.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_gpio_stm32.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32.c
	

	ifneq (,$(findstring F1,$(MCU)))
		C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c		
		C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f1.c
		C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f1.c
		
		LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/release_speed/arm_m3
		LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/ucg/release_speed/arm_m3
		
		#LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m3
		#LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/ucg/debug/arm_m3
	endif

	ifneq (,$(findstring F4,$(MCU)))
		C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f4/clocksetup.c		
		C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f4.c
		C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f4.c
		LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m4f
		LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/ucg/debug/arm_m4f
	endif

endif

	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/ucg_spi.c
	C_INCLUDES += $(UCG_DIR)
	CFLAGS += -DWITH_USER_PTR
	LDFLAGS +=  -lucg



C_SOURCES += $(LIBHALGLUE_ROOT)/common/arm_cpuid.c
C_SOURCES += $(LIBHALGLUE_ROOT)/common/bshal_delay.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_i2cm.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_spim.c





################################################################################
# Includes
################################################################################


C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/extlib/
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/


# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld



LDFLAGS += -u _printf_float


MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk
  
