################################################################################
# Set the MCU
################################################################################
MCU = NRF52832

################################################################################
# Set Output Filename 
################################################################################
TARGET = ws2812_demo

################################################################################
# Set the root directory of the ucdev enviorement 
################################################################################
UCDEV_ROOT=../../../../ucdev

LIBWS2812DMA_ROOT=$(UCDEV_ROOT)/lib/libws2812dma/

################################################################################
# Sources
################################################################################
C_SOURCES += main.c
C_SOURCES += ../demo.c

#C_SOURCES += $(LIBWS2812DMA_ROOT)/ws2812.c
C_SOURCES +=../../../../ucdev/lib/libws2812dma/ws2812.c
C_SOURCES += $(LIBWS2812DMA_ROOT)/drivers/ws2812_nrfx.c
C_SOURCES +=  $(UCDEV_ROOT)/lib/libhalglue/common/bshal_delay.c

C_INCLUDES += $(LIBWS2812DMA_ROOT)
C_INCLUDES += $(LIBWS2812DMA_ROOT)/drivers
C_INCLUDES += $(UCDEV_ROOT)/lib/libhalglue/bshal/
C_INCLUDES += $(UCDEV_ROOT)/lib/libhalglue/hal/
C_INCLUDES += $(UCDEV_ROOT)/lib/libhalglue/
C_INCLUDES += $(UCDEV_ROOT)/lib/libhalglue/common

LDSCRIPT = nrf52832_xxaa.ld

MK_DIR=$(UCDEV_ROOT)/build/make
include $(MK_DIR)/all.mk
