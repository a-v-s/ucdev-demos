################################################################################
# Set the MCU
################################################################################
MCU ?= STM32F103xB

################################################################################
# Set Output Filename 
################################################################################
TARGET = wireless_sensor

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################

C_SOURCES += wireless_sensor.c


UCDEV_ROOT=../../ucdev
U8G2_DIR = $(UCDEV_ROOT)/ext/u8g2/csrc
BSSENSOR_DIR = $(UCDEV_ROOT)/lib/bssensor
BSRADIO_DIR = $(UCDEV_ROOT)/lib/bsradio
BSPROT_DIR =  $(UCDEV_ROOT)/lib/bsprot

C_INCLUDES += $(BSSENSOR_DIR)
C_INCLUDES += $(BSRADIO_DIR)
C_INCLUDES += $(BSPROT_DIR)

C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/extlib/
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/
C_INCLUDES += $(LIBHALGLUE_ROOT)/hal/

C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/
C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/drivers/
C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/cards

C_INCLUDES += $(UCDEV_ROOT)/lib/spi_flash
C_SOURCES += $(UCDEV_ROOT)/lib/spi_flash/spi_flash.c

LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/rtt/debug/arm_m3  -lrtt
C_SOURCES +=  $(UCDEV_ROOT)/ext/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Syscalls/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//RTT/
C_INCLUDES += $(UCDEV_ROOT)/ext/RTT//Config/
# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld

MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk
