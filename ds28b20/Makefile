################################################################################
# Set the MCU
################################################################################
MCU ?= STM32F103xB

################################################################################
# Set Output Filename 
################################################################################
TARGET = ds28b12_demo

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../ucdev


U8G2_DIR = $(UCDEV_ROOT)/ext/u8g2/csrc

################################################################################
# Sources
################################################################################
C_SOURCES += main.c

C_SOURCES += dis2.c
C_SOURCES += i2c.c

C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_uart_stm32.c


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

C_SOURCES += $(LIBHALGLUE_ROOT)/common/bshal_delay.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_gpio_stm32.c

C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_i2cm.c
#C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_spim.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/u8x8_i2c.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/u8x8_gpio_delay.c



#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc52x_transport.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc52x.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc66x_transport.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc66x.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/cards/picc.c


################################################################################
# Includes
################################################################################
C_INCLUDES += $(U8G2_DIR) 

C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/extlib/
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/

C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/
C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/drivers/
C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/cards

# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld

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

ifeq ($(SUBARCH), M4F)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m4f  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m4f  -lrtt
endif

ifeq ($(SUBARCH), RV32IMAC)
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/riscv_rv32imac  -lu8g2
	LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/riscv_rv32imac  -lrtt
endif

