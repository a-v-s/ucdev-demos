/*
 * i2c.c
 *
 *  Created on: 8 aug. 2021
 *      Author: andre
 */

#include "i2c.h"

static bshal_i2cm_instance_t m_i2c;

#ifdef STM32
#include "stm32/bshal_gpio_stm32.h"
#include "stm32/bshal_i2cm_stm32.h"
#endif

// TODO something about GECKO include

bshal_i2cm_instance_t* i2c_init(void) {
#ifdef STM32
	m_i2c.sda_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_7);
	m_i2c.scl_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_6);
	m_i2c.hw_nr = 1;
#elif defined GECKO
//	m_i2c.sda_pin = 5;// bshal_gpio_encode_pin(gpioPortA, 5);
//	m_i2c.scl_pin = 4;// bshal_gpio_encode_pin(gpioPortA, 4);

	m_i2c.sda_pin = bshal_gpio_encode_pin(1, 2);
	m_i2c.scl_pin = bshal_gpio_encode_pin(1, 1);
	m_i2c.hw_nr = 0;
#endif

	m_i2c.speed_hz = 100000;
//	m_i2c.speed_hz = 400000;
	//m_i2c.speed_hz = 360000;
#ifdef STM32
	bshal_stm32_i2cm_init(&m_i2c);
#elif defined GECKO
	bshal_gecko_i2cm_init(&m_i2c);
#else
#error "Unsupported MCU"
#endif
	return &m_i2c;
}
