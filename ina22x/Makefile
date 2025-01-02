################################################################################
# Set the MCU
################################################################################
MCU ?= STM32F103xB

################################################################################
# Set Output Filename 
################################################################################
TARGET = ina22x

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../ucdev

C_SOURCES += main.c
C_SOURCES += i2c.c

C_SOURCES += ${UCDEV_ROOT}/lib/bssensor/ina22x.c

C_INCLUDES += ${UCDEV_ROOT}/lib/bssensor

C_SOURCES += $(LIBHALGLUE_ROOT)/common/nosysfix.c

ifneq (,$(findstring RV32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f1.c
endif


ifneq (,$(findstring STM32F0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f0/clocksetup.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f0.c
endif

ifneq (,$(findstring STM32F1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f1.c
endif

ifneq (,$(findstring STM32F3,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f3/clocksetup.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f3.c
endif


ifneq (,$(findstring STM32F4,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f4/clocksetup.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f4.c
endif

ifneq (,$(findstring STM32L0,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l0/clocksetup.c
	C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32l0.c
endif

ifneq (,$(findstring STM32L1,$(MCU)))
	C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32l1/clocksetup.c
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

