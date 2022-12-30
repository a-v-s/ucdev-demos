#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <esp_timer.h>

#include "rc52x_transport.h"
#include "rc52x.h"
#include "rc66x.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/spi.h"

#include "bshal_i2cm.h"
//#include "i2c.h"

#include "rom/ets_sys.h"



void i2c_init(int a, int b){}
void framebuffer_apply(){}
void draw_plain_background(){}
void display_init(void){}
void print(char *str, int line){
	printf("%d: %s\n", line, str);
}
void demo_loop(bs_pdc_t*, size_t);

//--
// TODO
void framebuffer_apply();
void draw_plain_background();
void display_init(void);
void print(char *str, int line);
//--

//--
void bshal_delay_us(uint32_t us) {
	ets_delay_us(us);
}
void bshal_delay_ms(uint32_t ms) {
	bshal_delay_us(ms * 1000);
}
//--

uint32_t get_time_us(void) {
	return esp_timer_get_time();
}

uint32_t get_time_ms(void) {
	return get_time_us() / 1000;
}

void delay_time_ms(uint32_t ms) {
	// BUSY WAITING
	uint32_t now = get_time_us();
	uint32_t until = now + (1000 * ms);
	while (get_time_us() < until)
		;
}

void rfid5_spi_init(rc52x_t *rc52x) {
	static bshal_spim_instance_t rfid_spi_config;
/*
	rfid_spi_config.frequency = 1000000; // SPI speed for MFRC522 = 10 MHz
	rfid_spi_config.bit_order = 0; //MSB
	rfid_spi_config.mode = 0;
	rfid_spi_config.hw_nr = HW_NR;

	rfid_spi_config.sck_pin = SPI_SCK;
	rfid_spi_config.miso_pin = SPI_MISO;
	rfid_spi_config.mosi_pin = SPI_MOSI;
	rfid_spi_config.cs_pin = SPI_CS;

	rfid_spi_config.rs_pin = -1;
*/
	rc52x->delay_ms = delay_time_ms;
	rc52x->get_time_ms = get_time_ms;
	bshal_spim_init(&rfid_spi_config);
	rc52x->transport_type = bshal_transport_spi;
	rc52x->transport_instance.spim = &rfid_spi_config;
}




/*
void rfid5_i2c_init(rc52x_t *rc52x) {
	rc52x->delay_ms = delay_time_ms;
	rc52x->get_time_ms = get_time_ms;
	rc52x->transport_type = bshal_transport_i2c;
	rc52x->transport_instance.i2cm = gp_i2c;
}
*/

void app_main(void) {

	//i2c_init(I2C_SDA, I2C_SCL);

	rc52x_t rc52x_spi;
	rfid5_spi_init(&rc52x_spi);

//	rc52x_t rc52x_i2c;
//	rfid5_i2c_init(&rc52x_i2c);


	display_init();

	draw_plain_background();
	bs_pdc_t pdcs[5];
	size_t pdc_count = 0;

	char str[32];
	uint8_t version;
/*
	version = -1;
	rc52x_get_chip_version(&rc52x_i2c, &version);
	sprintf(str, "VERSION %02X", version);
	if (version != 0xFF) {
		rc52x_init(&rc52x_i2c);
		pdcs[pdc_count] = rc52x_i2c;
		pdc_count++;
	}
	print(str, 1);
*/



	version = -1;
	rc52x_get_chip_version(&rc52x_spi, &version);
	sprintf(str, "VERSION %02X", version);
	if (version != 0xFF) {
		rc52x_init(&rc52x_spi);
		pdcs[pdc_count] = rc52x_spi;
		pdc_count++;
	}
	print(str, 2);


	print("PDCs:", 0);

	framebuffer_apply();

	demo_loop(pdcs, pdc_count);

}
