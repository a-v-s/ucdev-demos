################################################################################
# Set the MCU
################################################################################
MCU = STM32F072xB

################################################################################
# Set Output Filename 
################################################################################
TARGET = usb_demo

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../..

################################################################################
# Sources
################################################################################
C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f0/clocksetup.c

C_SOURCES += main.c
C_SOURCES += ../demo.c
C_SOURCES += $(LIBHALGLUE_ROOT)/common/serialnumber.c
C_SOURCES += $(LIBHLUSBD_ROOT)/drivers/usbd_stm.c
C_SOURCES += $(LIBHLUSBD_ROOT)/usbd.c
C_SOURCES += $(LIBHLUSBD_ROOT)/usbd_descriptors.c
C_SOURCES += $(LIBHLUSBD_ROOT)/ConvertUTF/ConvertUTF.c

################################################################################
# Includes
################################################################################
C_INCLUDES += . 
C_INCLUDES += ..
C_INCLUDES += $(LIBHLUSBD_ROOT)/drivers/
C_INCLUDES += $(LIBHLUSBD_ROOT)/
C_INCLUDES += $(LIBHLUSBD_ROOT)/ConvertUTF
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_INC) 
C_INCLUDES += $(LIBHALGLUE_INC)/stm32
C_INCLUDES += $(CUBEF0_ROOT)/Drivers/CMSIS/Device/ST/STM32F0xx/Include/
C_INCLUDES += $(CUBEF0_ROOT)/Drivers/CMSIS/Include/
C_INCLUDES += $(CUBEF0_HAL_INC_ROOT)

# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld

MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk
  
