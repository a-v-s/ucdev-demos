################################################################################
# Set the MCU
################################################################################
MCU = STM32F103xB

################################################################################
# Set Output Filename 
################################################################################
TARGET = spi_demo

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../../ucdev


UCGLIB_DIR = $(UCDEV_ROOT)/ext/ucglib/csrc
U8G2_DIR = $(UCDEV_ROOT)/ext/u8g2/csrc

################################################################################
# Sources
################################################################################
C_SOURCES += main.c
#C_SOURCES += dis.c
C_SOURCES += dis_u8g.c

#C_SOURCES += radio/si443x.c
#C_SOURCES += radio/si443x_transport.c



C_SOURCES += $(LIBHALGLUE_ROOT)/hal/stm32f1/clocksetup.c
C_SOURCES += $(LIBHALGLUE_ROOT)/common/arm_cpuid.c
C_SOURCES += $(LIBHALGLUE_ROOT)/common/bshal_delay.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_gpio_stm32.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_spim_stm32f1.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/stm32/bshal_i2cm_stm32f1.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_i2cm.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/bshal_spim.c
#C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/ucg_spi.c

C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/u8x8_gpio_delay.c
C_SOURCES += $(LIBHALGLUE_ROOT)/bshal/extlib/u8x8_spi.c


#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc52x_transport.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc52x.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc66x_transport.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/drivers/rc66x.c
#C_SOURCES += $(UCDEV_ROOT)/lib/bsrfid/cards/picc.c


################################################################################
# Includes
################################################################################
C_INCLUDES += $(UCGLIB_DIR) 
C_INCLUDES += $(U8G2_DIR)


C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/extlib/
C_INCLUDES += $(LIBHALGLUE_ROOT)/common/
C_INCLUDES += $(LIBHALGLUE_ROOT)/bshal/

#C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/
#C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/drivers/
#C_INCLUDES += $(UCDEV_ROOT)/lib/bsrfid/cards


# link script
LDSCRIPT = $(shell tr '[:upper:]' '[:lower:]' <<< $(MCU)).ld

LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/ucg/debug/arm_m3  -lucg

# u8g2 library
CFLAGS += -DU8X8_WITH_USER_PTR
LDFLAGS +=  -L$(UCDEV_ROOT)/build/staticlib/u8g2/debug/arm_m3  -lu8g2

LDFLAGS += -u _printf_float

MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk
  
