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

#include "driver/spi_master.h"

#include "bshal_i2cm.h"
#include "i2c.h"

#include "rom/ets_sys.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define HW_NR    HSPI_HOST

#define SPI_INT		35
#define SPI_RST		27
#define SPI_CS2		5
#define SPI_CS3		21

#define SPI_CS		2
#define SPI_MOSI	4
#define SPI_MISO	16
#define SPI_SCK		17

//#define UART_IO1
//#define UART_IO2
//#define UART_IO3
//#define UART_IO4

//#define UART_CTS
#define UART_TXD	18
#define UART_RXD	19
//#define UART_RTS

#define I2C_IO1		22
#define I2C_IO2		23
//#define I2C_IO3
//#define I2C_IO4

#define I2C_INT		39
#define I2C_RST		25
#define I2C_SDA		33
#define I2C_SCL		32


#elif defined CONFIG_IDF_TARGET_ESP32S3
#define HW_NR    SPI2_HOST

#define SPI_INT 	4
#define SPI_RST		5
#define SPI_CS2   	6
#define SPI_CS3		7

#define SPI_CS   	15
#define SPI_MOSI 	16
#define SPI_MISO 	17
#define SPI_SCK  	18

#define UART_IO1	38
#define UART_IO2	37
#define UART_IO3	36
#define UART_IO4	35

#define UART_CTS	8
#define UART_TXD	1
#define UART_RXD	2
#define UART_RTS	9

#define I2C_IO1		10
#define I2C_IO2		21
#define I2C_IO3		47
//#define I2C_IO4		21

#define I2C_INT		11
#define I2C_RST		12
#define I2C_SCL 	13
#define I2C_SDA 	14



#elif defined CONFIG_IDF_TARGET_ESP32C3 
#define HW_NR    SPI2_HOST

//#define SPI_INT
//#define SPI_RST
#define SPI_CS2   	2
#define SPI_CS3		3

#define SPI_CS   	9
#define SPI_MOSI 	0
#define SPI_MISO 	1
#define SPI_SCK  	10

//#define UART_IO1
//#define UART_IO2
//#define UART_IO3
//#define UART_IO4

//#define UART_CTS
#define UART_TXD	7
#define UART_RXD	6
//#define UART_RTS

//#define I2C_IO1
//#define I2C_IO2
//#define I2C_IO3
//#define I2C_IO4

//#define I2C_INT
//#define I2C_RST
#define I2C_SCL 	5
#define I2C_SDA 	4

#endif

void i2c_init(int, int);
void framebuffer_apply();
void draw_plain_background();
void display_init(void);
void print(char *str, int line);
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
/*
void bshal_delay_ms(uint32_t ms) {
	bshal_delay_us(ms * 1000);
}
*/
//--
void bshal_delay_ms(uint32_t ms) {
	vTaskDelay(ms);
}

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
	puts("rfid5_spi_init");
	static bshal_spim_instance_t rfid_spi_config;
	rfid_spi_config.frequency = 1000000; // SPI speed for MFRC522 = 10 MHz
	rfid_spi_config.bit_order = 0; //MSB
	rfid_spi_config.mode = 0;
	rfid_spi_config.hw_nr = HW_NR;

	rfid_spi_config.sck_pin = SPI_SCK;
	rfid_spi_config.miso_pin = SPI_MISO;
	rfid_spi_config.mosi_pin = SPI_MOSI;
	rfid_spi_config.cs_pin = SPI_CS;

	rfid_spi_config.rs_pin = -1;

	rc52x->delay_ms = delay_time_ms;
	rc52x->get_time_ms = get_time_ms;
	bshal_spim_init(&rfid_spi_config);
	rc52x->transport_type = bshal_transport_spi;
	rc52x->transport_instance.spim = &rfid_spi_config;
}

void rfid6_spi_init(rc52x_t *rc66x) {
	puts("rfid6_spi_init");
	static bshal_spim_instance_t rfid_spi_config;
	rfid_spi_config.frequency = 1000000; // SPI speed for MFRC522 = 10 MHz
	rfid_spi_config.bit_order = 0; //MSB
	rfid_spi_config.mode = 0;
	rfid_spi_config.hw_nr = HW_NR;

	rfid_spi_config.sck_pin = SPI_SCK;
	rfid_spi_config.miso_pin = SPI_MISO;
	rfid_spi_config.mosi_pin = SPI_MOSI;
	rfid_spi_config.cs_pin = SPI_CS2;

	rfid_spi_config.rs_pin = -1;

	rc66x->delay_ms = delay_time_ms;
	rc66x->get_time_ms = get_time_ms;
	bshal_spim_init(&rfid_spi_config);
	rc66x->transport_type = bshal_transport_spi;
	rc66x->transport_instance.spim = &rfid_spi_config;
}

void rfid5_i2c_init(rc52x_t *rc52x) {
	puts("rfid5_i2c_init");
	rc52x->delay_ms = delay_time_ms;
	rc52x->get_time_ms = get_time_ms;
	rc52x->transport_type = bshal_transport_i2c;
	rc52x->transport_instance.i2cm = gp_i2c;
}

void mfrc630_ISO15693_init(rc66x_t *rc66x);
uint16_t mfrc630_ISO15693_readTag(rc66x_t *rc66x, uint8_t* uid, int colpos);

void app_main(void) {

	i2c_init(I2C_SDA, I2C_SCL);

	rc52x_t rc52x_spi;
	rfid5_spi_init(&rc52x_spi);

	rc52x_t rc52x_i2c;
	rfid5_i2c_init(&rc52x_i2c);

#ifdef SPI_CS2
	rc66x_t rc66x_spi;
	rfid6_spi_init(&rc66x_spi);
#endif

	display_init();

	draw_plain_background();
	bs_pdc_t pdcs[5];
	size_t pdc_count = 0;

	char str[32];
	uint8_t version;
	version = -1;
	rc52x_get_chip_version(&rc52x_i2c, &version);
	sprintf(str, "VERSION %02X", version);
	if (version != 0xFF  && version != 0x00) {
		puts("rc52x_i2c");
		rc52x_init(&rc52x_i2c);
		pdcs[pdc_count] = rc52x_i2c;
		pdc_count++;
	}
	print(str, 1);

	version = -1;
	rc52x_get_chip_version(&rc52x_spi, &version);
	sprintf(str, "VERSION %02X", version);
	if (version != 0xFF  && version != 0x00) {
		puts("rc52x_spi");
		rc52x_init(&rc52x_spi);
		pdcs[pdc_count] = rc52x_spi;
		pdc_count++;
	}
	print(str, 2);

#ifdef SPI_CS2
	version = -1;
	rc66x_get_chip_version(&rc66x_spi, &version);
	sprintf(str, "VERSION %02X", version);
	print(str, 3);
	if (version != 0xFF  && version != 0x00) {
		puts("rc66x_spi");
		rc66x_init(&rc66x_spi);
		pdcs[pdc_count] = rc66x_spi;
		pdc_count++;

//		rc66x_test(&rc66x_spi);
		mfrc630_ISO15693_init(&rc66x_spi);

//		rc66x_test(&rc66x_spi);
		uint8_t buffer[16];
		mfrc630_ISO15693_readTag(&rc66x_spi,buffer, 0);

//		extern void test_read_block(rc66x_t *rc66x, uint8_t* instruction, int instr_len);
//		uint8_t read_instr[] = {0x02,0x20,0x00};
////		uint8_t read_instr[] = {0x36,0x01,0x00, 0x00};
//		test_read_block(&rc66x_spi, read_instr, sizeof(read_instr) );

		rc66x_init(&rc66x_spi);

	}
#endif
	print("PDCs:", 0);

	framebuffer_apply();

	puts("----");
	bshal_delay_ms(1000);

	demo_loop(pdcs, pdc_count);

}
