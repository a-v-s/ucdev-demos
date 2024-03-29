################################################################################
# Set the MCU
################################################################################
MCU ?= STM32F103xB

#	Working so far
#	STM32F072xB		Blue Pill Variant
#	STM32F103xB		Blue Pill
#   STM32F401xC		Black Pill
#   STM32F411xE		Black Pill
#	STM32F303xC		Blue Pill with swapped chip

# 	TODO
#	STM32L152xC		Blue Pill with swapped chip 
#	STM32F030x8		Blue Pill Variant
#	STM32F030x6		Rabbit Pill
#
##	STM32L051xx		Green Pill (NOT PIN COMPATIBLE 3V3/GND)

# Silicon Labs
#  EFR32BG22C224F512GM32
#  EFR32BG22C112F352GM32


################################################################################
# Set Output Filename 
################################################################################
TARGET = i2c_demo

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../ucdev


U8G2_DIR = $(UCDEV_ROOT)/ext/u8g2/csrc

BSSENSOR_DIR = $(UCDEV_ROOT)/lib/bssensor

################################################################################
# Sources
################################################################################
C_SOURCES += main.c
C_SOURCES += identify.c

C_SOURCES += dis2.c
C_SOURCES += i2c.c

C_SOURCES += $(BSSENSOR_DIR)/lm75b.c
C_SOURCES += $(BSSENSOR_DIR)/sht3x.c
C_SOURCES += $(BSSENSOR_DIR)/bh1750.c
C_SOURCES += $(BSSENSOR_DIR)/pcf8563.c
C_SOURCES += $(BSSENSOR_DIR)/pcf8574.c
C_SOURCES += $(BSSENSOR_DIR)/si70xx.c
C_SOURCES += $(BSSENSOR_DIR)/hcd1080.c
C_SOURCES += $(BSSENSOR_DIR)/ccs811.c

C_SOURCES += $(BSSENSOR_DIR)/bmp280.c
C_SOURCES += $(BSSENSOR_DIR)/scd4x.c

C_SOURCES += kt0803.c
C_SOURCES += rda5807.c





ifneq (,$(findstring RV32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f1.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f1.c
endif


ifneq (,$(findstring STM32F0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f0/clocksetup.c
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f0.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f0.c
endif

ifneq (,$(findstring STM32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f1.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f1.c
	
	C_SOURCES += $(LIBHALGLUE_ROOT)/common/arm_cpuid.c
endif

ifneq (,$(findstring STM32F3,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f3/clocksetup.c
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f4.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f3.c
endif


ifneq (,$(findstring STM32F4,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f4/clocksetup.c
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f4.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f4.c
endif

ifneq (,$(findstring STM32L0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l0/clocksetup.c
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f0.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32l0.c
endif

ifneq (,$(findstring STM32L1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l1/clocksetup.c
#	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f1.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32l1.c
endif


# TODO: CPUID does not work on Cortex M0
# error: 'SCB_Type' has no member named 'PFR'
#C_SOURCES += $(LIBHALGLUE_ROOT)/common/arm_cpuid.c


ifneq (,$(findstring STM32,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_gpio_stm32.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32.c
	LDSCRIPT = $(shell echo $(MCU) | tr '[:upper:]' '[:lower:]' ).ld
endif

ifneq (,$(findstring RV32,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_gpio_stm32.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32.c
	LDSCRIPT = $(shell echo $(MCU) | tr '[:upper:]' '[:lower:]' ).ld
endif


ifneq (,$(findstring EFR32,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/gecko/bshal_i2cm_gecko.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/gecko/bshal_spim_gecko.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/gecko/bshal_gpio_gecko.c
	LDSCRIPT = $(shell echo $(SERIES) | tr '[:upper:]' '[:lower:]' ).ld
endif


C_SOURCES += $(LIBHALGLUE_ROOT)/common/bshal_delay.c

C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_i2cm.c
#C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_spim.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/u8x8_i2c.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/u8x8_gpio_delay.c




################################################################################
# Includes
################################################################################
C_INCLUDES += $(U8G2_DIR) 
C_INCLUDES += $(BSSENSOR_DIR)

C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/extlib/
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/
C_INCLUDES += $(LIBHALGLUE_ROOT)/hal/

C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/
C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/drivers/
C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/cards

# link script


# u8g2 library
CFLAGS += -DU8X8_WITH_USER_PTR

#LDFLAGS += -u _printf_float

C_SOURCES +=  $(UCDEV_ROOT)/ext/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Syscalls/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//RTT/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Config/

MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk
  







ifeq ($(SUBARCH), M0)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m0  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m0  -lrtt

endif

ifeq ($(SUBARCH), M0P)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m0p  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m0p  -lrtt
endif

ifeq ($(SUBARCH), M3)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m3  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m3  -lrtt
endif

ifeq ($(SUBARCH), M33)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m33  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m33  -lrtt
endif


ifeq ($(SUBARCH), M33F)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m33f  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m33f  -lrtt
endif


ifeq ($(SUBARCH), M4F)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m4f  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m4f  -lrtt
endif

ifeq ($(SUBARCH), RV32IMAC)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/riscv_rv32imac  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/riscv_rv32imac  -lrtt
endif

