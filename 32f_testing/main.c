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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "system.h"

#include "bshal_spim.h"
#include "bshal_delay.h"
#include "bshal_i2cm.h"
#include "i2c.h"

#include "bh1750.h"
#include "ccs811.h"
#include "sht3x.h"
#include "bmp280.h"
#include "lm75b.h"
#include "si70xx.h"
#include "pcf8563.h"
#include "hcd1080.h"



#define SSD1306_index  0

#define BH1750_index   1
#define SHT3X_index    2
#define LM75B_index    3
#define HCD1080_index  4
#define CCS811_index   5
#define PCF8563_index  6
#define BMP280_index   7

int test_results[8][4];

static const char * sensor_name[] = {
"SSD1306  ",
"BH1750   ",
"SHT3X    ",
"LM75B    ",
"HCD1080  ",
"CCS811   ",
"PCF8563  ",
"BMP280   ",
};


#ifdef UART

// I guess we need nosys, but with it, riscv won't call _write
// without it, we need to implement a bunch of stuff, including _sbrk
// to make malloc work... but still no call to _write


#include "uart.h"
#include "stm32f1xx_hal_uart.h"
#include <stdio.h>
#include <stdlib.h>
UART_HandleTypeDef huart1;
void initialise_uart(){
    __HAL_RCC_USART1_CLK_ENABLE();

    /**
     * USART1 GPIO Configuration
     * PB6     ------> USART1_TX
     * PB7     ------> USART1_RX
     */
    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin =  GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


	GPIO_InitStruct.Pin =  GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}

__attribute__((used)) int _write  (int fd, char * ptr, int len) {
  HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, HAL_MAX_DELAY);
  return len;
}



int _read(int fd, char* ptr, int len) {
    HAL_StatusTypeDef hstatus;

	hstatus = HAL_UART_Receive(&huart1, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
	if (hstatus == HAL_OK)
		return 1;
	else
		//return EIO;
		return 0;

}





#endif

bshal_i2cm_instance_t *gp_i2c = NULL;

void HardFault_Handler(void) {
	while (1)
		;
}

#if defined __ARM_EABI__
void SysTick_Handler(void) {
	HAL_IncTick();
}
#endif

void SystemClock_Config(void) {

	ClockSetup_HSE8_SYS72();

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

char* getserialstring(void) {
	static char serialstring[25];
	char *serial = (char*) (0x1FFFF7E8);
	sprintf(serialstring, "%02X%02X%02X%02X"
			"%02X%02X%02X%02X"
			"%02X%02X%02X%02X", serial[0], serial[1], serial[2], serial[3],
			serial[4], serial[5], serial[6], serial[7], serial[8], serial[9],
			serial[10], serial[11]);
	return serialstring;
}

int test_and_recover(uint32_t speed) {
	if (0 == bshal_i2cm_isok(gp_i2c, SSD1306_ADDR)) {
		printf("I²C BUS OK\n");
		return -2;
	} else {
		printf("I²C BUS Failed, re-initialisng...\n");
		gp_i2c = i2c_init(speed);
		if (0 == bshal_i2cm_isok(gp_i2c, SSD1306_ADDR)) {
			printf("I²C Bus recovery OK\n");
			return -3;
		} else {
			printf("I²C Bus recovery failed, aboring tests...\n");
			while (1)
				;
		}
	}
	return -1;
}

void i2c_test(uint32_t speed) {

	printf("Running the I2C tests at %d kHz\n", speed / 1000);

	gp_i2c = i2c_init(speed);

	pcf8563_t pcf8563 = { 0 };
	bh1750_t bh1750 = { 0 };
	ccs811_t ccs811 = { 0 };
	bmp280_t bmp280 = { 0 };
	sht3x_t sht3x = { 0 };
	lm75b_t lm75b = { 0 };
	si70xx_t si70xx = { 0 };
	hcd1080_t hcd1080 = { 0 };


	printf("Running I²C addressing tests...\n");

	// Display
	printf("Testing SSD1306...     ");
	if (0 == bshal_i2cm_isok(gp_i2c, SSD1306_ADDR)) {
		printf("OK\n");
		test_results[SSD1306_index][(speed >100000)] = 1;

	} else {
		printf("Failed\n");
		printf("Further testing aborted\n");
		while (1)
			;
	}


	// Light sensor
	printf("Testing BH1750...      ");
	if (0 == bshal_i2cm_isok(gp_i2c, BH1750_ADDR)) {
		printf("OK\n");
		bh1750.addr = BH1750_ADDR;
		bh1750.p_i2c = gp_i2c;
		test_results[BH1750_index][(speed >100000)] = 1;
	} else {
		printf("Failed\n");
		test_results[BH1750_index][(speed >100000)] = test_and_recover(speed);
	}


// Temperature / Humidity
	printf("Testing SHT3X...       ");
	if (0 == bshal_i2cm_isok(gp_i2c, SHT3X_ADDR)) {
		sht3x.addr = SHT3X_ADDR;
		sht3x.p_i2c = gp_i2c;
		printf("OK\n");
		test_results[SHT3X_index][(speed >100000)] = 1;
	} else {
		printf("Failed\n");
		test_results[SHT3X_index][(speed >100000)] = test_and_recover(speed);
	}

	// Temperature
	printf("Testing LM75B...       ");
	if (0 == bshal_i2cm_isok(gp_i2c, LM75B_ADDR)) {
		printf("OK\n");
		lm75b.addr = LM75B_ADDR;
		lm75b.p_i2c = gp_i2c;
		test_results[LM75B_index][(speed >100000)] = 1;
	} else {
		printf("Failed\n");
		test_results[SHT3X_index][(speed >100000)] = test_and_recover(speed);
	}


	// Temperature / Humidity
	// Note: same module with different chips as on option
	// Verify the right one is inserted
	// The HCD1080 may cause trouble so that why this one
	// is chosen.

	// MH32F103 fails so hard, re-initialising the I2C bus
	// does not recover the problem. Reboot does,
	// more investigation needed whether this is software recoverable
	printf("Testing HCD1080...     ");
	if (0 == bshal_i2cm_isok(gp_i2c, HCD1080_ADDR)) {
		printf("OK\n");
		hcd1080.addr = HCD1080_ADDR;
		hcd1080.p_i2c = gp_i2c;
		test_results[HCD1080_index][(speed >100000)] = true;
	} else {
		printf("Failed\n");
		test_and_recover(speed);
	}

	// VOC sensor
	printf("Testing CCS811...      ");
	if (0 == bshal_i2cm_isok(gp_i2c, CCS811_ADDR)) {
		printf("OK\n");
		ccs811.addr = CCS811_I2C_ADDR1;
		ccs811.p_i2c = gp_i2c;
		test_results[CCS811_index][(speed >100000)] = 1;
	} else {
		printf("Failed\n");
		test_results[CCS811_index][(speed >100000)] = test_and_recover(speed);
	}

	// RTC
	printf("Testing PCF8563...     ");
	if (0 == bshal_i2cm_isok(gp_i2c, PCF8563_ADDR)) {
		printf("OK\n");
		test_results[PCF8563_index][(speed >100000)] = 1;
		pcf8563.addr = PCF8563_I2C_ADDR;
		pcf8563.p_i2c = gp_i2c;
	} else {
		printf("Failed\n");
		test_results[PCF8563_index][(speed >100000)] = test_and_recover(speed);
	}

	// TODO: BMP280, Air pressure
	printf("Testing BMP280...      ");
	if (0 == bshal_i2cm_isok(gp_i2c, BMP280_I2C_ADDR)) {
		printf("OK\n");
		bmp280.addr = BMP280_I2C_ADDR;
		bmp280.p_i2c = gp_i2c;
		test_results[BMP280_index][(speed >100000)] = 1;
	} else {
		printf("Failed\n");
		test_results[BMP280_index][(speed >100000)] = test_and_recover(speed);
	}

	//return; // for now

	printf("Running I²C communication tests...\n");


	printf("Testing BH1750...      ");
	if (bh1750.addr) {
		static int lux;
		if (0 == bh1750_measure_ambient_light(&bh1750, &lux)) {
			// meassure twice?
			bh1750_measure_ambient_light(&bh1750, &lux);
			printf("OK:    %6d lux\n", lux);
			test_results[BH1750_index][2+(speed >100000)] = 1;
		} else {
			printf("Failed\n");
			test_results[BH1750_index][2+(speed >100000)] = test_and_recover(speed);
		}
	} else {
		printf("Skipped\n");
	}

	printf("Testing SHT3X...       ");
	if (sht3x.addr) {
		float temperature_f;
		float huminity_f;
		if (0 == sht3x_get_humidity_float(&sht3x, &huminity_f)
				&& 0 == sht3x_get_temperature_C_float(&sht3x, &temperature_f)) {

			printf("OK:  %3d.%02d°C %3d.%02d%%  \n", (int) temperature_f % 999,
					abs((int) (100 * temperature_f)) % 100, (int) huminity_f,
					abs((int) (100 * huminity_f)) % 100);

			test_results[SHT3X_index][2+(speed >100000)] = 1;
		} else {
			printf("Failed\n");
			test_results[SHT3X_index][2+(speed >100000)] = test_and_recover(speed);
		}
	} else {
		printf("Skipped\n");
	}

	printf("Testing LM75B...       ");
	if (lm75b.addr ) {

		float temperature_f;
		if (lm75b_get_temperature_C_float(&lm75b, &temperature_f)) {
			printf("Failed\n");
			test_results[LM75B_index][2+(speed >100000)] = test_and_recover(speed);
		} else {
			printf("OK: LM75B:  %3d.%02d°C  \n", (int) temperature_f,
					(int) (100 * temperature_f) % 100);

			test_results[LM75B_index][2+(speed >100000)] = 1;
		}
	} else {
		printf("Skipped\n");
	}

	printf("Testing SSD1306...     ");
	if (0 == bshal_i2cm_isok(gp_i2c, SSD1306_ADDR)) {
		display_init();
		draw_background();
		char buff[16];
		sprintf(buff,"I2C TEST @ %d", speed);
		print(buff, 4);
		framebuffer_apply();
		puts("Check Display");

		test_results[SSD1306_index][2+(speed >100000)] = 1;
	} else {
		printf("Skipped\n");
	}



	printf("Testing HCD1080...     ");
	if (hcd1080.addr) {
		float temperature_f;
		hcd1080_get_temperature_C_float(&hcd1080, &temperature_f);
		float huminity_f;
		if (hcd1080_get_humidity_float(&hcd1080, &huminity_f)) {
			printf("Failed\n");
			test_results[HCD1080_index][2+(speed >100000)] = test_and_recover(speed);
		} else {
			printf("OK: %3d.%02d°C %3d.%02d%%  \n", (int) temperature_f % 999,
					abs((int) (100 * temperature_f)) % 100, (int) huminity_f,
					abs((int) (100 * huminity_f)) % 100);

			test_results[HCD1080_index][2+(speed >100000)] = 1;
		}
	} else {
		printf("Skipped\n");
	}

	printf("Testing CCS811...      ");

	if (ccs811.addr) {

		if (0 == ccs811_init(&ccs811)) {
			bshal_delay_ms(2500); // it is slow

			// I Swear this was failing on the HK32 befpre

			static uint16_t TVOC = 0;
			if (0 == css811_measure(&ccs811, NULL, &TVOC)) {
				printf("OK:    %4d ppb TVOC\n", TVOC);

				test_results[CCS811_index][2+(speed >100000)] = 1;
			} else {
				printf("Failed (meaure)\n");
				test_results[CCS811_index][2+(speed >100000)] =test_and_recover(speed);
			}
		} else {
			printf("Failed (init)\n");
			test_results[CCS811_index][2+(speed >100000)] =test_and_recover(speed);
		}
	} else {
		printf("Skipped\n");
	}

	printf("Testing PCF8523...     ");
	if (	pcf8563.addr) {

		pcf8563_time_t time = { 0 };
		if ( pcf8563_get_time(&pcf8563, &time) ) {
			test_results[PCF8563_index][2+(speed >100000)] = test_and_recover(speed);
		} else {
		char buff[32] = "OK:";
		//buff[0] = buff[1] = buff[2] = ' ';
		buff[3 + 0] = '2';
		buff[3 + 1] = '0' + time.months.century;
		buff[3 + 2] = '0' + time.years.tens;
		buff[3 + 3] = '0' + time.years.unit;
		buff[3 + 4] = '-';
		buff[3 + 5] = '0' + time.months.tens;
		buff[3 + 6] = '0' + time.months.unit;
		buff[3 + 7] = '-';
		buff[3 + 8] = '0' + time.days.tens;
		buff[3 + 9] = '0' + time.days.unit;
		buff[3 + 10] = ' ';
		buff[3 + 11] = '0' + time.hours.tens;
		buff[3 + 12] = '0' + time.hours.unit;
		buff[3 + 13] = ':';
		buff[3 + 14] = '0' + time.minutes.tens;
		buff[3 + 15] = '0' + time.minutes.unit;
		buff[3 + 16] = ':';
		buff[3 + 17] = '0' + time.seconds.tens;
		buff[3 + 18] = '0' + time.seconds.unit;
		buff[3 + 19] = 0;
		puts(buff);

		test_results[PCF8563_index][2+(speed >100000)] = 1;
		}
	} else {
		puts("Skipped");
	}

	printf("Testing BMP280...      ");
	if (bmp280.p_i2c) {

		if (bmp280_init(&bmp280)) {
			puts("Init Error");
			test_results[BMP280_index][2+(speed >100000)] = test_and_recover(speed);
		} else {
			float temperature, pressure;
			if (bmp280_measure_f(&bmp280, &temperature, &pressure)) {
				puts("Measure Error");
				test_results[BMP280_index][2+(speed >100000)] = test_and_recover(speed);
			} else {
				printf("OK: %3d.%02d°C %d hPa  \n",
						(int) temperature % 999,
						abs((int) (100 * temperature)) % 100,
						(int) pressure / 100);

				test_results[BMP280_index][2+(speed >100000)] = 1;
			}
		}

	} else {
		puts("Skipped");
	}

}

int main() {
	mcuid();

	SystemClock_Config();
	SystemCoreClockUpdate();

#ifdef SEMI
	initialise_monitor_handles();
#endif

#ifdef UART
	initialise_uart();
#endif

	HAL_Init();
	bshal_delay_init();

	printf("----------------------------------------\n");
	printf(" BlaatSchaap 32F103 I2C Testing         \n");
	printf("----------------------------------------\n");
	printf(" Core         : %s\n", cpuid());
	printf(" Guessed chip : %s\n", mcuid());
	printf(" Serial number: %s\n", getserialstring());
	printf("----------------------------------------\n");


	gp_i2c = i2c_init(100000);
	display_init();
	draw_background();
	char buff[16];
	int test = 0;
	while (1) {
		sprintf(buff,"TEST  %d", test);
		print(buff, 4);
		sprintf(buff,"TEST  %08X", &test);
		print(buff, 2);
		framebuffer_apply();
		puts("Check Display");
	}
	//while (1); } int main2(void){

//		display_init();
//		print("test",1);
	// ccs811 and hcd1080 are known issues on HK32 and GD32

	// First do the standard PIO variant.
	// Then we might add some IRQ and DMA variants

	printf("\n\nPlease note the test assumed the microcontroller under test\n");
	printf("is assumed to be placed into a BlaatSchaap PMOD Baseboard\n");
	printf("and connected to a BlaatSchaap I²C Modules board with all\n");
	printf("modules in place\n\n");

	printf("----------------------------------------\n");
	printf(" 100 kHz \n");
	printf("----------------------------------------\n");
	i2c_test(100000);
	printf("----------------------------------------\n");
	printf(" 400 kHz \n");
	printf("----------------------------------------\n");
	i2c_test(400000);

	printf("----------------------------------------\n");
	printf(" DONE \n");
	printf("----------------------------------------\n");

	bool errors_found = false;
	puts("");
	puts("| SENSOR    | A1 | A4 | D1 | D4 |");
	puts("| --------- | -- | -- | -- | -- |");
	for (int i = 0 ; i < 8 ; i++ ) {
		printf ("| %s | %2d | %2d | %2d | %2d |\n",
				sensor_name[i],
				test_results[i][0],
				test_results[i][1],
				test_results[i][2],
				test_results[i][3]);

			errors_found |=
					(test_results[i][0]!= 1) ||
					(test_results[i][1]!= 1) ||
					(test_results[i][2]!= 1) ||
					(test_results[i][3]!= 1);

	}
	puts("| --------- | -- | -- | -- | -- |");
	printf("| %-15s:%14s|\n",
			mcuid() , errors_found ? " ERRORS FOUND " : " all ok ");
	puts("| ----------------------------- |");
	puts("");
	puts("Legend:");
	puts("|  0 = TEST SKIPPED                | 1 = TEST PASSED             |");
	puts("| -2 = FAIL: I2C RESET NOT NEEDED  | -3 = FAIL: I2C RESET NEEDED |");
	puts("");




	int c;
	int t;
	while (1) {
//		c = getchar();
//		(void) c;
	}
}

