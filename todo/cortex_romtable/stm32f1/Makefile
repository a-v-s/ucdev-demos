################################################################################
# Set the MCU
################################################################################
MCU = STM32F103xB

################################################################################
# Set Output Filename 
################################################################################
TARGET = romtable

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../..

################################################################################
# Sources
################################################################################
C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
C_SOURCES += $(LIBHALGLUE_ROOT)/common/arm_cpuid.c

C_SOURCES += main.c
C_SOURCES += cpuid.c
C_SOURCES += ../demo.c
C_SOURCES += $(LIBHALGLUE_ROOT)/common/serialnumber.c
C_SOURCES += $(LIBHLUSBD_ROOT)/drivers/usbd_stm.c
C_SOURCES += $(LIBHLUSBD_ROOT)/usbd.c
C_SOURCES += $(LIBHLUSBD_ROOT)/usbd_descriptors.c
C_SOURCES += $(LIBHLUSBD_ROOT)/ConvertUTF/ConvertUTF.c

################################################################################
# Includes
################################################################################
INC += .
INC += ..
INC += $(LIBHLUSBD_ROOT)/drivers/
INC += $(LIBHLUSBD_ROOT)/
INC += $(LIBHLUSBD_ROOT)/ConvertUTF
INC += $(LIBHALGLUE_ROOT)/common/
INC += $(LIBHALGLUE_INC) 
INC += $(LIBHALGLUE_INC)/stm32
INC += $(CUBEF1_ROOT)/Drivers/CMSIS/Device/ST/STM32F1xx/Include/
INC += $(CUBEF1_ROOT)/Drivers/CMSIS/Include/
INC += $(CUBEF1_HAL_INC_ROOT)

# C includes
C_INCLUDES =  $(INC)  

# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld

MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk
  
