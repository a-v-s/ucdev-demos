/*

 File: 		wireless_sensor.c
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2023 André van Schoubroeck <andre@blaatschaap.be>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 */

#include <stdbool.h>
#include <string.h>



#include "system.h"

// NB. On STM32F0, stdfix conflicts with
// STM32CubeF0/Drivers/CMSIS/Core/Include/cmsis_gcc.h
// It should be included after STM32 includes stm32f0xx.h (included by system.h)
#include <stdfix.h>
// Might need to verify this also holds for latest CMSIS, and switch to upstream

#include "bshal_spim.h"
#include "bshal_delay.h"
#include "bshal_i2cm.h"


#include "lm75b.h"
#include "sht3x.h"
#include "bh1750.h"
#include "hcd1080.h"
#include "si70xx.h"
#include "ccs811.h"
#include "bmp280.h"
#include "scd4x.h"


#include "sxv1.h"
#include "si4x3x.h"
#include "si4x6x.h"

#include "protocol.h"

#include "bshal_i2cm.h"

static bshal_i2cm_instance_t m_i2c;

bshal_i2cm_instance_t * i2c_init(void) {
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

int test(void) {
	bsradio_instance_t radio;
	/*
	int (*set_frequency)(bsradio_instance_t *bsradio,int kHz) ;
	int (*set_tx_power)(bsradio_instance_t *bsradio,int tx_power);
	int (*set_bitrate)(bsradio_instance_t *bsradio,int bps) ;
	int (*set_fdev)(bsradio_instance_t *bsradio,int hz) ;
	int (*set_bandwidth)(bsradio_instance_t *bsradio,int hz) ;
	int (*configure_packet)(bsradio_instance_t *bsradio) ;
	int (*set_network_id)(bsradio_instance_t *bsradio,char *sync_word, size_t size) ;
	int (*set_mode)(bsradio_instance_t *bsradio,bsradio_mode_t mode) ;
	int (*recv_packet)(bsradio_instance_t *bsradio ,bsradio_packet_t *p_packet) ;
	int (*send_packet)(bsradio_instance_t *bsradio, bsradio_packet_t *p_packet) ;
	 */
	radio.driver.set_frequency = sxv1_set_frequency;
	radio.driver.set_tx_power = sxv1_set_tx_power;
	radio.driver.set_bitrate = sxv1_set_bitrate;
	radio.driver.set_fdev = sxv1_set_fdev;
	radio.driver.set_bandwidth = sxv1_set_bandwidth;
	radio.driver.init = sxv1_init;
	radio.driver.set_network_id = sxv1_set_network_id;
	radio.driver.set_mode = sxv1_set_mode;
	radio.driver.recv_packet = sxv1_recv_packet;
	radio.driver.send_packet = sxv1_send_packet;
}

int main(){
	i2c_init();
	test();
	while(1);}
