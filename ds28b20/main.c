/*

 File: 		main.c
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2017, 2018, 2019, 2020 André van Schoubroeck <andre@blaatschaap.be>

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

#include <stdfix.h>

#include "bshal_spim.h"
#include "bshal_delay.h"
#include "bshal_i2cm.h"
#include "bshal_uart.h"
#include "bshal_gpio.h"

bshal_i2cm_instance_t *gp_i2c = NULL;

#define OW_ROM_MATCH    0x55
#define OW_ROM_SKIP     0xcc
#define OW_ROM_SEARCH   0xf0

#define DS28B20_TEMPERATURE_CONVERT 0x44

#define DS18X20_SCRATCHPAD_READ		0xBE
#define DS18X20_SCRATCHPAD_WRITE	0x4E

#define DS18X20_EEPROM_WRITE		0x48
#define DS18X20_EEPROM_READ			0xB8

#define DS28B20_POWER_STATUS		0xB4

#define DS18S20_FAMILY_CODE 0x10
#define DS18B20_FAMILY_CODE 0x28
#define DS1822_FAMILY_CODE 0x22


void HardFault_Handler(void) {
	while (1)
		;
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

void SystemClock_Config(void) {

#ifdef STM32F0
	ClockSetup_HSE8_SYS48();
#endif

#ifdef STM32F1
	ClockSetup_HSE8_SYS72();
#endif

#ifdef STM32F3
	ClockSetup_HSE8_SYS72();
#endif

#ifdef STM32F4
	SystemClock_HSE25_SYS84();
#endif

#ifdef STM32L0
	SystemClock_HSE8_SYS32();
#endif

#ifdef STM32L1
	SystemClock_HSE8_SYS48();
#endif
}


void dwt_init(void) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t dwt_get(void) {
	return DWT->CYCCNT;
}

uint32_t get_time_us() {
	return dwt_get() / (SystemCoreClock / 1000000);
}

typedef struct {
	uint64_t device_id;
	uint8_t collision_pos;
	uint8_t collision_id;
} ds28b20_t;

volatile static bool m_recvd = false;

void uart_cb(char *data, size_t size) {
	m_recvd = true;
}
static uint8_t receive_buffer[1];
bshal_uart_instance_t* uart_init(int bps) {
	static bshal_uart_instance_t bshal_uart_instance = { };
	bshal_uart_instance.bps = bps;
	bshal_uart_instance.data_bits = 8;
	bshal_uart_instance.parity = bshal_uart_parity_none;
	bshal_uart_instance.stop_bits = 1;

	bshal_uart_instance.fc = bshal_uart_flow_control_none;

	bshal_uart_instance.hw_nr = 2;  // UASRT 2
	bshal_uart_instance.cts_pin = -1; // No flow control
	bshal_uart_instance.rts_pin = -1; // No flow control
	bshal_uart_instance.rxd_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_3); // PA3
	bshal_uart_instance.txd_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_2); // PA2

	static bshal_uart_async_t bshal_uart_async;

	//  Configuration for the Async handler, to configure the synchronosation
	bshal_uart_async.callback = uart_cb; // Callback for the resulting NMEA string

	bshal_uart_async.receive_buffer = receive_buffer;
	bshal_uart_async.receive_buffer_len = sizeof(receive_buffer);
	bshal_uart_instance.async = &bshal_uart_async; // Asign the async handler to the uart instance

	bshal_stm32_uart_init(&bshal_uart_instance);
	return &bshal_uart_instance;
}

int ds28b20_send_bit_uart(bshal_uart_instance_t *uart, bool val) {
	uint8_t tmp = val ? 0xFF : 0x00;
	return bshal_stm32_uart_tx(uart, &tmp, 1);
}

int ds28x20_read_bit_uart(bshal_uart_instance_t *uart, bool *val) {
	uint8_t tmp = 0xFF;
	int result = 0;
	m_recvd = false;
	result = bshal_stm32_uart_tx(uart, &tmp, 1);
	//result = bshal_stm32_uart_rx(uart, &tmp, 1);
	//bshal_delay_ms(1);
	while (!m_recvd)
		;
	m_recvd = false;
	tmp = receive_buffer[0];
	*val = (tmp == 0xFF);
	return result;
}


int ds28x20_read_bit(uint8_t pin, bool *val) {
	bshal_gpio_write_pin(pin, false);
	bshal_delay_us(5);
	bshal_gpio_write_pin(pin, true);
	bshal_delay_us(15);
	*val = 1;
	int timeout = get_time_us() + 60;
	while (get_time_us() < timeout) {
		if (!bshal_gpio_read_pin(pin)) {
			*val = 0;
		}
	}
	return 0;
}

int ds28b20_send_bit(uint8_t pin, bool val) {
	if(val) {
		bshal_gpio_write_pin(pin, false);
		bshal_delay_us(5);
		bshal_gpio_write_pin(pin, true);
		bshal_delay_us(55);
	} else {
		bshal_gpio_write_pin(pin, false);
		bshal_delay_us(60);
		bshal_gpio_write_pin(pin, true);
	}
	return 0;
}


int ds28x20_write_byte(int pin, uint8_t val) {
	int result = 0;
	for (int i = 0; i < 8; i++) {
		result = ds28b20_send_bit(pin, val & (1 << i));
		if (result)
			return result;
	}
	return result;
}

int ds28x20_write_byte_uart(bshal_uart_instance_t *uart, uint8_t val) {
	int result = 0;
	for (int i = 0; i < 8; i++) {
		result = ds28b20_send_bit(uart, val & (1 << i));
		if (result)
			return result;
	}
	return result;
}

int ds28b20_read_byte(int pin, uint8_t *val) {
	int result = 0;
	*val = 0;
	bool bitval;
	for (int i = 0; i < 8; i++) {
		result = ds28x20_read_bit(pin, &bitval);
		if (result)
			return result;
		*val |= bitval << i;
	}
	return result;
}

int ds28b20_read_byte_uart(bshal_uart_instance_t *uart, uint8_t *val) {
	int result = 0;
	*val = 0;
	bool bitval;
	for (int i = 0; i < 8; i++) {
		result = ds28x20_read_bit(uart, &bitval);
		if (result)
			return result;
		*val |= bitval << i;
	}
	return result;
}



int ds28x20_reset(int pin){
	bshal_gpio_write_pin(pin, false);
	bshal_delay_us(480);
	bshal_gpio_write_pin(pin, true);
	bshal_delay_us(15);
	int timeout = 480 + get_time_us();
	bool presense = false;
	while (get_time_us() < timeout) {
		if (!bshal_gpio_read_pin(pin)) {
			presense = true;
		}
	}
	return -!presense;
}

int ds28x20_reset_uart() {
	uint8_t tmp;
	int result;
	bshal_uart_instance_t *uart = uart_init(9600);
	tmp = 0xF0;
	m_recvd = false;
	result = bshal_stm32_uart_tx(uart, &tmp, 1);
	if (result)
		return -12;
	//result = bshal_stm32_uart_rx(uart, &tmp, 1);
	//bshal_delay_ms(1);
	//bshal_delay_ms(1);
	while (!m_recvd)
		;
	tmp = receive_buffer[0];
	m_recvd = false;

	if (result)
		return -11;
	if (tmp == 0xF0)
		return -1;
	if (tmp == 0xFF)
		return -2;
	if (tmp == 0x00)
		return -3;
	return 0;
}

int ds28x20_scan_bus(ds28b20_t *ds28b20, size_t size) {
	int result;
	int entry = 0;

	while (entry < size) {
		//puts("------------------");

		if (entry && !ds28b20[entry].collision_pos)
			break;
		//printf("Entry %d  Colpos %d\n" , entry, ds28b20[entry].collision_pos);

		result = ds28x20_reset(3);
		if (result)
			return result;

		//bshal_uart_instance_t *uart = uart_init(115200);
		result = ds28x20_write_byte(3, OW_ROM_SEARCH);
		if (result)
			return result;

		for (int i = 0; i < 64; i++) {

			bool bits[2];
			ds28x20_read_bit(3, bits + 0);
			ds28x20_read_bit(3, bits + 1);
			if (bits[0] == bits[1]) {
				if (bits[0]) {
					// All devices found
					break;
				} else {
					//printf("Entry %d  Colpos %d CurCol %d\n" , entry, ds28b20[entry].collision_pos, i);
					// Collision
					if (i < ds28b20[entry].collision_pos) {
						// Collision already handled
						bits[0] = ds28b20[entry].device_id & (1 << i);
						//printf("Entry %d  Colpos %d CurCol %d Handled Bitval %d\n" , entry, ds28b20[entry].collision_pos, i,  bits[0]);
					} else if (i == ds28b20[entry].collision_pos) {
						// Current collision, part 2
						bits[0] = 1;
						//printf("Second pass for the current collision, setting bit to %d\n" , bits[0]);
						//printf("Entry %d  Colpos %d CurCol %d Pass 2  Bitval %d\n" , entry, ds28b20[entry].collision_pos, i,  bits[0]);
					} else {
						bits[0] = 0;
						//printf("Entry %d  Colpos %d CurCol %d Pass 1  Bitval %d\n" , entry, ds28b20[entry].collision_pos, i,  bits[0]);
						if ((entry + 1) < size) {
							for (int j = entry + 1; j < size; j++) {
								if (!ds28b20[j].collision_pos) {
									//printf("Entering collision for second pass at entry %d\n",j);
									ds28b20[j] = ds28b20[entry];
									ds28b20[j].collision_pos = i;
									ds28b20[j].collision_id++;
								break;
								}
							}
						}
					}
				}
			}
			ds28b20[entry].device_id |= (uint64_t) bits[0] << i;
			ds28b20_send_bit(3, bits[0]);
		}


		entry++;

	}
	for (int i = entry; i < size; i++)
		ds28b20[i].device_id = 0;
	return entry;
}

int ds28x20_convert(ds28b20_t *ds28b20) {
	int result;
	int pin = 3;
	result = ds28x20_reset(pin);
	if (result)
		return result;

	result = ds28x20_write_byte(pin, OW_ROM_MATCH);
	result = ds28x20_write_byte(pin, ds28b20->device_id);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>8);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>16);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>24);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>32);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>40);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>48);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>56);
	result = ds28x20_write_byte(pin, DS28B20_TEMPERATURE_CONVERT);
	bool readbit = 0;
	int begin = get_time_us();
	while(!readbit) ds28x20_read_bit(pin,&readbit);
	int end = get_time_us();
	printf("Conversion took %d ms\n",(end-begin)/1000);
	return result;
}

int ds28x20_read(ds28b20_t *ds28b20, float * temperature_f) {
	int result;
	int pin = 3;
	result = ds28x20_reset(3);
	if (result)
		return result;
	//bshal_uart_instance_t *pin = uart_init(115200);

	result = ds28x20_write_byte(pin, OW_ROM_MATCH);
	result = ds28x20_write_byte(pin, ds28b20->device_id);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>8);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>16);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>24);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>32);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>40);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>48);
	result = ds28x20_write_byte(pin, ds28b20->device_id>>56);
	result = ds28x20_write_byte(pin, DS18X20_SCRATCHPAD_READ);
	uint8_t scratchpad[9]={};
	for (int i = 0 ; i < sizeof(scratchpad); i++)
		result = ds28b20_read_byte(pin, scratchpad+i);
	uint16_t *temperature=(uint16_t *)scratchpad;

	switch (ds28b20->device_id&0xFF) {
	case DS18B20_FAMILY_CODE:
	case DS1822_FAMILY_CODE:
		//DS18B20
		*temperature_f = 0.0625f * (float)(*temperature);
		break;
	case DS18S20_FAMILY_CODE:
		//DS1820
		//DS18S20
		*temperature_f = 0.5f * (float)(*temperature);
	}

	return result;
}

int main(void) {
	SEGGER_RTT_Init();
	SystemClock_Config();
	SystemCoreClockUpdate();

	HAL_Init();

	bshal_delay_init();
	bshal_delay_us(10);
	dwt_init();

	gp_i2c = i2c_init();

	display_init();
	draw_background();
	print("DS18x20 DEMO", 1);
	framebuffer_apply();
	bshal_delay_ms(1000);

	int pin = 3;
	bshal_gpio_cfg_out(pin, opendrain, true);

	ds28b20_t ds28b20[8];
	while (1) {
		memset(ds28b20, 0, sizeof(ds28b20));
		int count = ds28x20_scan_bus(ds28b20, 8);
		printf("Found %d devices\n",count);
		if (count > 0) {
			for (int i = 0 ; i < count; i++) {
				printf("Found %d %08X%08X \n" ,i,  (int)(ds28b20[i].device_id >> 32),
						(int)(ds28b20[i].device_id));
				ds28x20_convert(ds28b20+i);
				float temp;
				ds28x20_read(ds28b20+i, &temp);
				printf("Temperature %2d.%02d\n", (int)temp, (int)(temp*100)%100);
				char buffer[32];
				snprintf(buffer,32,"%2d.%02d\n", (int)temp, (int)(temp*100)%100);
				print(buffer,i);

			}
			framebuffer_apply();
			draw_background();

		}
		//puts("------------------");
		//bshal_delay_ms(250);
	}
}
