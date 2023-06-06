################################################################################
# Set the MCU
################################################################################
MCU ?= NRF52832

################################################################################
# Set Output Filename 
################################################################################
TARGET = qdec

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../../ucdev

C_SOURCES += main.c


LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m4f  -lrtt

C_SOURCES +=  $(UCDEV_ROOT)/ext/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Syscalls/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//RTT/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Config/


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

