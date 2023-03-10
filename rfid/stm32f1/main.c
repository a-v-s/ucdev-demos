/*

 File: 		main.c
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2017 - 2023 André van Schoubroeck <andre@blaatschaap.be>

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

#include <string.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"



#include "rc52x_transport.h"
#include "rc52x.h"
//#include "rc66x_transport.h"
#include "rc66x.h"
//#include "pn53x.h"
#include "iso14443a.h"

#include "bshal_spim.h"
#include "bshal_i2cm.h"
#include "bshal_delay.h"

#include "i2c.h"

void print(char *str, int line);
void framebuffer_apply(void);
void draw_plain_background(void);

void SysTick_Handler() {
	HAL_IncTick();
}

void rfid5_spi_init(rc52x_t *rc52x) {
	static bshal_spim_instance_t rfid_spi_config;
	rfid_spi_config.frequency = 10000000; // SPI speed for MFRC522 = 10 MHz
	rfid_spi_config.bit_order = 0; //MSB
	rfid_spi_config.mode = 0;

	rfid_spi_config.hw_nr = 1;
	rfid_spi_config.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);
	rfid_spi_config.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	rfid_spi_config.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);

	rfid_spi_config.cs_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_4);
	rfid_spi_config.rs_pin = -1;

	rc52x->delay_ms = bshal_delay_ms;
	rc52x->get_time_ms = HAL_GetTick; // TODO should we provide our own time source?
	bshal_spim_init(&rfid_spi_config);
	rc52x->transport_type = bshal_transport_spi;
	rc52x->transport_instance.spim = &rfid_spi_config;
}

void rfid6_spi_init(rc66x_t *rc66x) {
	static bshal_spim_instance_t rfid_spi_config;
	rfid_spi_config.frequency = 10000000; // SPI speed for CLRC663 = 10 MHz
	rfid_spi_config.bit_order = 0; //MSB
	rfid_spi_config.mode = 0;

	rfid_spi_config.hw_nr = 1;
	rfid_spi_config.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);
	rfid_spi_config.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	rfid_spi_config.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);

	rfid_spi_config.cs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_1);
	rfid_spi_config.rs_pin = -1;

	rc66x->delay_ms = bshal_delay_ms;
	rc66x->get_time_ms = HAL_GetTick; // TODO should we provide our own time source?
	bshal_spim_init(&rfid_spi_config);
	rc66x->transport_type = bshal_transport_spi;
	rc66x->transport_instance.spim = &rfid_spi_config;
}

void rfid5_i2c_init(rc52x_t *rc52x) {
	rc52x->transport_type = bshal_transport_i2c;
	rc52x->transport_instance.i2cm = gp_i2c;
	rc52x->delay_ms = bshal_delay_ms;
	rc52x->get_time_ms = HAL_GetTick; // TODO should we provide our own time source?
	rc52x_init(rc52x);
}

int main() {
#ifdef SEMI
	initialise_monitor_handles();
	printf("Hello world!\n");
#endif
	HAL_Init();

	ClockSetup_HSE8_SYS72();
	SystemCoreClockUpdate();
	i2c_init();




	rc52x_t rc52x_spi;
	rfid5_spi_init(&rc52x_spi);

	rc52x_t rc52x_i2c;
	rfid5_i2c_init(&rc52x_i2c);

	rc66x_t rc66x_spi;
	rfid6_spi_init(&rc66x_spi);

	display_init();


	draw_plain_background();
	bs_pdc_t pdcs[5];
	size_t pdc_count = 0;

	char str[32];
	uint8_t version;
	version = -1;
	rc52x_get_chip_version(&rc52x_i2c, &version);
	sprintf(str, "VERSION %02X", version);
	if (version != 0xFF) {
		rc52x_init(&rc52x_i2c);
		pdcs[pdc_count] = rc52x_i2c;
		pdc_count++;
	}
	print(str, 1);

	version = -1;
	rc52x_get_chip_version(&rc52x_spi, &version);
	sprintf(str, "VERSION %02X", version);
	if (version != 0xFF) {
		rc52x_init(&rc52x_spi);
		pdcs[pdc_count] = rc52x_spi;
		pdc_count++;
	}
	print(str, 2);

	version = -1;
	rc66x_get_chip_version(&rc66x_spi, &version);
	sprintf(str, "VERSION %02X", version);
	print(str, 3);
	if (version != 0xFF) {
		rc66x_init(&rc66x_spi);
		pdcs[pdc_count] = rc66x_spi;
		pdc_count++;
	}

	print("PDCs:", 0);

	framebuffer_apply();

 	demo_loop(pdcs, pdc_count);

}
