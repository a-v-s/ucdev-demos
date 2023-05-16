#include "system.h"
#include "bshal_spim.h"
#include "bshal_gpio.h"
#include "stm32/bshal_gpio_stm32.h"
#include <stdio.h>
#include "rfm69.h"
#include "si4x3x.h"

#if defined __ARM_EABI__
void SysTick_Handler(void) {
	HAL_IncTick();
}


void dwt_init(void) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	__DSB();
	DWT->CTRL &= !DWT_CTRL_CYCCNTENA_Msk;
	__DSB();
	DWT->CYCCNT = 0;
	__DSB();
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	__DSB();
}

uint32_t dwt_get(void) {
	return DWT->CYCCNT;
}

uint32_t get_time_ms(void) {
	uint32_t ticks = dwt_get();
	return ticks / (SystemCoreClock/1000);
}

uint32_t get_time_us(void) {
	uint32_t ticks = dwt_get();
	return ticks / (SystemCoreClock/1000000);
}

#else
void dwt_init(void) {
	timer_init();
}
#endif

extern bshal_spim_instance_t radio_spi_config;

#define SERIALNUMBER  *((uint32_t*) (0x1FFFF7F0))

void test() {
	uint8_t partno = 0;
	rfm69_read_reg(0x10, &partno);
	printf("Verification partno %02X\n", partno);
}

void radio_init(void) {
	SEGGER_RTT_Init();
	//radio_spi_config.frequency = 10000000;
	radio_spi_config.frequency = 1000000;
	radio_spi_config.bit_order = 0; //MSB
	radio_spi_config.mode = 0;

	radio_spi_config.hw_nr = 1;
	radio_spi_config.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);
	radio_spi_config.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	radio_spi_config.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);
	radio_spi_config.cs_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_4);
	radio_spi_config.rs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_10);

	bshal_spim_init(&radio_spi_config);

}

void SystemClock_Config(void) {
	ClockSetup_HSE8_SYS48();
}

void *gp_i2c = NULL;

int rfm69_init() {

	// reset is active high!!!!
	bshal_gpio_write_pin(radio_spi_config.rs_pin, 1);
	bshal_delay_ms(5);
	bshal_gpio_write_pin(radio_spi_config.rs_pin, 0);
	bshal_delay_ms(20);

	uint8_t partno = 0;
	rfm69_read_reg(0x10, &partno);

	printf("Verification partno %02X\n", partno);

	//  rfm69_set_frequency(915000);
	//rfm69_set_frequency(868000);
	rfm69_set_frequency(434000);

	/*
	 * Europe: 433 MHz Band:
	 * 433.050 MHz to 434.790 MHz:
	 * +10 dBm if bandwidth ≤ 25 kHz or duty cycle ≤ 10%
	 * otherwise 0 dBm

	 * Europe: 868 MHz band
	 * 863.000 to 870.000 MHz: +14 dBm
	 *		863,00 MHz to 865,00 MHz	≤ 0,1 % duty cycle
	 *		865,00 MHz to 868,00 MHz	≤ 1,0 % duty cycle
	 * 		868,00 MHz to 868,60 MHz	≤ 1,0 % duty cycle
	 * 		868,70 MHz to 869,20 MHz	≤ 0,1 % duty cycle
	 * 		869,40 MHz to 869,65 MHz	≤ 0.1 % duty cycle
	 * 		869,70 MHz to 870,00 MHz	≤ 1,0 % duty cycle
	 *
	 * 		869,40 MHz to 869,65 MHz:	500 mW = +27 dBm	≤ 10 % duty cycle
	 * 		869,70 MHz to 870,00 MHz:	  5 mW =  +7 dBm	 no duty cycle limit
	 *
	 * 		For any duty cycle limit, it says "or polite spectrum access"
	 * 		Not sure about the definition though.
	 * 		The dBm is "ERP"
	 *
	 *
	 *
	 * North America: 915 MHz band:
	 * 902 to 928 MHz:
	 * Single Frequency:	–1.23 dBm
	 * Frequency Hopping:	+24 dBm
	 * 		The dBm is "EIRP",
	 * 		ERP = EIRP - 2.15 dB
	 *
	 *
	 * How to these ERP or EIRP values relate to the settings I give to the module?

	 */

	rfm69_calibarte_rc();

	rfm69_configure_packet();

	rfm69_set_bitrate(12500);
	rfm69_set_fdev(12500);
	rfm69_set_bandwidth(25000);

	//rfm69_set_tx_power(17);
	//rfm69_set_tx_power(10);
	rfm69_set_tx_power(0);
	//rfm69_set_tx_power(-4);

	//rfm69_write_reg(RFM69_REG_RSSITHRESH, 0xE4);
	rfm69_write_reg(RFM69_REG_RSSITHRESH, 0xC4);

	rfm69_set_sync_word(0xdeadbeef);

//// for testing
//	rfm69_sync_config_t config;
//	config.fifo_fill_condition = 0;
//	config.sync_on = 1;
//	config.sync_size = 0; // size = sync_size + 1, thus 4
//	config.sync_tol = 0;
//
//	rfm69_write_reg(RFM69_REG_SYNCVALUE1, 0xDE);

	/*
	 The DAGC is enabled by setting RegTestDagc to 0x20 for low modulation index systems
	 (i.e. when AfcLowBetaOn=1, refer to section 3.4.16), and 0x30 for other systems.
	 It is recommended to always enable the DAGC.
	 */
	rfm69_write_reg(RFM69_REG_AFCCTRL, 0x00);
	rfm69_write_reg(RFM69_REG_TESTDAGC, 0x30);

	return 0;
}

int si4x3x_init() {

	bshal_gpio_write_pin(radio_spi_config.rs_pin, 1);
	bshal_delay_ms(5);
	bshal_gpio_write_pin(radio_spi_config.rs_pin, 0);
	bshal_delay_ms(20);

	uint8_t dt = 0;
	si4x3x_read_reg8(0x00, &dt);
	if (0b00001000 != dt) {
		print("BAD DEVICE", 5);
		framebuffer_apply();
		while (1)
			;
	}
	print("Si4432", 5);
	framebuffer_apply();

	//  si4x3x_set_frequency(915000);
//	si4x3x_set_frequency(868000);
//	si4x3x_set_frequency(868000);
	//si4x3x_set_frequency(434000);
	si4x3x_set_frequency(434012); // According to SDR I am 12 kHz too low

	si4x3x_configure_packet();

	si4x3x_set_bitrate(12500);
	si4x3x_set_fdev(12500);
	si4x3x_set_bandwidth(25000);
	si4x3x_update_clock_recovery();

	//si4x3x_set_tx_power(17);
	si4x3x_set_tx_power(10);
	//si4x3x_set_tx_power(0);
	//si4x3x_set_tx_power(-4);

	si4x3x_set_sync_word(0xdeadbeef);
	//si4x3x_set_sync_word(0xefbeadde);

	return 0;
}

void si4x3x_recv_test() {
	rfm69_air_packet_t packet = { 0 };
	char strbuff[32];

	si4x3x_init();
	print("RX Si4x3x", 5);
	framebuffer_apply();
	si4x3x_configure_packet();

	while (1) {

		if (!si4x3x_receive_request(&packet)) {
			printf("Packet Received\n");
			if (packet.header.size < 16)
				for (int i = 0; i < packet.header.size - sizeof(packet.header);
						i++)
					printf("%02X ", packet.data[i]);

			sprintf(strbuff, "RX %02X", packet.data[4]);
			print(strbuff, 1);
			framebuffer_apply();
			memset(&packet, 0, sizeof(packet));
			draw_plain_background();

		}
	}
}
void si4x3x_send_test() {
	rfm69_air_packet_t packet = { 0 };
	char strbuff[32];

	//		// Si4x3x test
	si4x3x_init();
	print("TX Si4432", 5);

	//		rfm69_init();
	framebuffer_apply();
	while (1) {

		bshal_delay_ms(1000);
		packet.header.size = 9;
		packet.data[0] = 0xDE;
		packet.data[1] = 0xAD;
		packet.data[2] = 0xBE;
		packet.data[3] = 0xEF;
		packet.data[4]++;
		si4x3x_send_request(&packet, &packet);
		//rfm69_send_request(&packet, &packet);
		sprintf(strbuff, "TX %02X", packet.data[4]);
		draw_plain_background();
		print(strbuff, 1);
		framebuffer_apply();
	}
}
void rfm69_recv_test() {
	rfm69_air_packet_t packet = { 0 };
	char strbuff[32];

	rfm69_init();
	print("RX ", 5);
	framebuffer_apply();
	rfm69_set_mode(rfm69_mode_rx);
	rfm69_restart();

	while (1) {

		if (!rfm69_receive_request(&packet)) {
			printf("Packet Received\n");
			if (packet.header.size < 16)
				//				for (int i = 0; i < packet.header.size-sizeof(packet.header); i++)
				//					printf("%02X ", packet.data[i]);

				sprintf(strbuff, "RX %02X", packet.data[4]);
			print(strbuff, 1);
			framebuffer_apply();
			memset(&packet, 0, sizeof(packet));
			draw_plain_background();

		}
	}
}
void rfm69_send_test() {
	rfm69_air_packet_t packet = { 0 };
	char strbuff[32];

	print("TX SX123x", 5);

	rfm69_init();
	framebuffer_apply();
	while (1) {

		bshal_delay_ms(1000);
		packet.header.size = 9;
		packet.data[0] = 0xDE;
		packet.data[1] = 0xAD;
		packet.data[2] = 0xBE;
		packet.data[3] = 0xEF;
		packet.data[4]++;
		rfm69_send_request(&packet, &packet);
		sprintf(strbuff, "TX %02X", packet.data[4]);
		draw_plain_background();
		print(strbuff, 1);
		framebuffer_apply();
	}
}

int main() {
	SystemClock_Config();
	SystemCoreClockUpdate();
	dwt_init();
	HAL_Init();

	bshal_delay_init();
	gp_i2c = i2c_init();
	display_init();

	draw_plain_background();
	print("RFM69 DEMO", 4);
	framebuffer_apply();

	bshal_delay_ms(1000);
	radio_init();

	if (0x87141031 == SERIALNUMBER) {
		//si4x3x_send_test();
		//si4x3x_recv_test();
		rfm69_recv_test();
		//rfm69_send_test();

	} else {
		//rfm69_send_test();
		si4x3x_send_test();
	}
}
