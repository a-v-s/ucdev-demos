################################################################################
# Set the MCU
################################################################################
#MCU ?= STM32F103xB
MCU ?= STM32F103x6


#	STM32F072xB		Blue Pill Variant
#	STM32F103xB		Blue Pill
#   STM32F401xC		Black Pill
#   STM32F411xE		Black Pill
#	STM32F303xC		Blue Pill with swapped chip


#	STM32L152xC		Blue Pill with swapped chip 
#	STM32F030x8		Blue Pill Variant
#	STM32F030x6		Rabbit Pill
#
#	STM32L051xx		Green Pill (NOT PIN COMPATIBLE 3V3/GND)
################################################################################
# Set Output Filename 
################################################################################
TARGET = instruction_latency

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../ucdev

C_SOURCES += main.c
C_SOURCES += identify.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_gpio_stm32.c

C_SOURCES += $(LIBHALGLUE_ROOT)/common/arm_cpuid.c
C_SOURCES += $(LIBHALGLUE_ROOT)/common/bshal_delay.c


#C_SOURCES += SEGGER_RTT_V784/Syscalls/SEGGER_RTT_Syscalls_GCC.c
#C_SOURCES += SEGGER_RTT_V784/RTT/SEGGER_RTT.c
#C_SOURCES += SEGGER_RTT_V784/RTT/SEGGER_RTT_printf.c
#C_INCLUDES += SEGGER_RTT_V784/Syscalls/
#C_INCLUDES += SEGGER_RTT_V784/RTT/
#C_INCLUDES += SEGGER_RTT_V784/Config/


LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m3  -lrtt


# Putting the regarget code in a static library does not work, 
# But adding the source here does work. 
#LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/retarget_rtt/debug/arm_m3  -lretarget_rtt
C_SOURCES +=  $(UCDEV_ROOT)/ext/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Syscalls/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//RTT/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Config/

ASM_SOURCES += t_aux.s


ifneq (,$(findstring RV32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
endif


ifneq (,$(findstring STM32F0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f0/clocksetup.c
endif

ifneq (,$(findstring STM32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
endif

ifneq (,$(findstring STM32F3,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f3/clocksetup.c
endif

ifneq (,$(findstring STM32F4,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f4/clocksetup.c
endif

ifneq (,$(findstring STM32L0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l0/clocksetup.c
endif

ifneq (,$(findstring STM32L1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l1/clocksetup.c
endif





################################################################################
# Includes
################################################################################

C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/extlib/
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/


# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld

MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk

