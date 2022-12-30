# COMPONENT_OBJS := file1.o file2.o thing/filea.o thing/fileb.o anotherthing/main.o
# COMPONENT_SRCDIRS := . thing anotherthing


# Libhalglue Generic Abstractions
COMPONENT_SRCDIRS += libhalglue/bshal
COMPONENT_SRCDIRS += libhalglue/bshal/esp8266
COMPONENT_ADD_INCLUDEDIRS += libhalglue/bshal
COMPONENT_ADD_INCLUDEDIRS += libhalglue/hal
COMPONENT_OBJS += libhalglue/bshal/bshal_i2cm.o
COMPONENT_OBJS += libhalglue/bshal/bshal_spim.o
COMPONENT_OBJS += libhalglue/bshal/esp8266/bshal_spim_esp8622.o
#COMPONENT_OBJS += libhalglue/bshal/bshal_uart.o

# Libhalglue External Library Support 
COMPONENT_SRCDIRS += libhalglue/bshal/extlib
#COMPONENT_OBJS += libhalglue/bshal/extlib/u8x8_gpio_delay.o
#COMPONENT_OBJS += libhalglue/bshal/extlib/u8x8_i2c.o
#COMPONENT_OBJS += libhalglue/bshal/extlib/u8x8_spi.o
#COMPONENT_OBJS += libhalglue/bshal/extlib/ucg_i2c.o
#COMPONENT_OBJS += libhalglue/bshal/extlib/ucg_spi.o

# RFID Readers
COMPONENT_SRCDIRS += bsrfid/drivers
COMPONENT_ADD_INCLUDEDIRS += bsrfid/drivers
COMPONENT_ADD_INCLUDEDIRS += bsrfid/cards
#COMPONENT_OBJS += bsrfid/drivers/pn5180.o
#COMPONENT_OBJS += bsrfid/drivers/pn5180_transport.o
#COMPONENT_OBJS += bsrfid/drivers/pn53x.o
COMPONENT_OBJS += bsrfid/drivers/rc52x.o
COMPONENT_OBJS += bsrfid/drivers/rc52x_ref.o
COMPONENT_OBJS += bsrfid/drivers/rc52x_transport.o
#COMPONENT_OBJS += bsrfid/drivers/rc66x.o
#COMPONENT_OBJS += bsrfid/drivers/rc66x_transport.o
#COMPONENT_OBJS += bsrfid/drivers/thm3060.o


# RFID Cards
COMPONENT_ADD_INCLUDEDIRS +=  bsrfid/cards
COMPONENT_SRCDIRS += bsrfid/cards
#COMPONENT_OBJS += bsrfid/cards/mifare.o
#COMPONENT_OBJS += bsrfid/cards/ntag.o
COMPONENT_OBJS += bsrfid/cards/picc.o



COMPONENT_ADD_INCLUDEDIRS 	+= common
COMPONENT_SRCDIRS 			+= common
#COMPONENT_OBJS 				+= common/dis2.o
COMPONENT_OBJS 				+= common/rfid_demo.o
