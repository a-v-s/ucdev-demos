idf_component_register(
	SRCS 
		bsrfid/cards/picc.c
		bsrfid/drivers/rc52x.c
		bsrfid/drivers/rc52x_ref.c
		bsrfid/drivers/rc52x_transport.c
		bsrfid/drivers/rc66x.c
		bsrfid/drivers/rc66x_transport.c
		libhalglue/bshal/esp32/bshal_gpio_esp32.c
		libhalglue/bshal/esp32/bshal_i2cm_esp32.c
		libhalglue/bshal/esp32/bshal_spim_esp32.c

		libhalglue/bshal/extlib/u8x8_gpio_delay.c
		libhalglue/bshal/extlib/u8x8_i2c.c
		libhalglue/bshal/extlib/u8x8_spi.c
#		libhalglue/bshal/extlib/ucg_i2c.c
#		libhalglue/bshal/extlib/ucg_spi.c


	INCLUDE_DIRS 
		libhalglue/hal/
		libhalglue/common/
		libhalglue/bshal/
		libhalglue/bshal/esp32
		libhalglue/bshal/extlib
		bsrfid/drivers/
		bsrfid/cards/
	REQUIRES 
		driver
		u8g2
)

idf_build_set_property(COMPILE_OPTIONS "-Wno-error=unused-const-variable" APPEND)

