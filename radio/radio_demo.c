#include "system.h"
#include "bshal_spim.h"
#include "bshal_gpio.h"
#include "stm32/bshal_gpio_stm32.h"
#include <stdio.h>
#include "sxv1.h"
#include "si4x3x.h"

#include "bsradio.h"

#include "spi_flash.h"

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

//bshal_spim_instance_t spi_radio_config;
bshal_spim_instance_t spi_flash_config;

#define SERIALNUMBER  *((uint32_t*) (0x1FFFF7F0))

void test() {
	uint8_t partno = 0;
	sxv1_read_reg(0x10, &partno);
	printf("Verification partno %02X\n", partno);
}

void spi_radio_init(bsradio_instance_t *bsradio) {

	bsradio->spim.frequency = 10000000;
	bsradio->spim.bit_order = 0; //MSB
	bsradio->spim.mode = 0;

	bsradio->spim.hw_nr = 1;
	bsradio->spim.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);
	bsradio->spim.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	bsradio->spim.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);
	bsradio->spim.cs_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_4);
	bsradio->spim.rs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_10);

	bshal_spim_init(&bsradio->spim);
}

void spi_flash_init(void) {
	spi_flash_config.frequency = 10000000;
	spi_flash_config.bit_order = 0; //MSB
	spi_flash_config.mode = 0;

	spi_flash_config.hw_nr = 1;
	spi_flash_config.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);
	spi_flash_config.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	spi_flash_config.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);
	spi_flash_config.cs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_1);
	spi_flash_config.rs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_10);

	bshal_spim_init(&spi_flash_config);
}

void SystemClock_Config(void) {
	ClockSetup_HSE8_SYS48();
}

void *gp_i2c = NULL;

int demo_sxv1_init(bsradio_instance_t *bsradio) {

	// reset is active high!!!!
	bshal_gpio_write_pin(bsradio->spim.rs_pin, 1);
	bshal_delay_ms(5);
	bshal_gpio_write_pin(bsradio->spim.rs_pin, 0);
	bshal_delay_ms(50);

	uint8_t partno = 0;
	sxv1_read_reg(bsradio, 0x10, &partno);

	printf("Verification partno %02X\n", partno);

	switch (bsradio->hwconfig.frequency_band) {
	case 434:
		sxv1_set_frequency(bsradio, 434000);
		sxv1_set_tx_power(bsradio, 0);
		break;
	case 868:

		//sxv1_set_frequency(bsradio, 869850);
		sxv1_set_frequency(bsradio, 870000);

//		sxv1_set_frequency(bsradio, 868000);
//		sxv1_set_tx_power(bsradio, 7);
		sxv1_set_tx_power(bsradio, 0);
		break;
	case 915:
		sxv1_set_frequency(bsradio, 915000);
		sxv1_set_tx_power(bsradio, -3);
		break;
	}

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


	sxv1_init(bsradio);

	sxv1_set_sync_word32(bsradio, 0xdeadbeef);

	sxv1_set_bitrate(bsradio, 12500);
	sxv1_set_fdev(bsradio, 12500);
	sxv1_set_bandwidth(bsradio, 25000);



	return 0;
}

int si4x3x_init(bsradio_instance_t *bsradio) {

	// reset is active high!!!!
	bshal_gpio_write_pin(bsradio->spim.rs_pin, 1);
	bshal_delay_ms(5);
	bshal_gpio_write_pin(bsradio->spim.rs_pin, 0);
	bshal_delay_ms(50);

	uint8_t dt = 0;
	si4x3x_read_reg8(bsradio, 0x00, &dt);
	if (0b00001000 != dt) {
		print("BAD DEVICE", 5);
		framebuffer_apply();
		while (1)
			;
	}
	print("Si4x3x", 6);
	framebuffer_apply();

	// The frequency that was tuned to was 11 kHz too low for
	// low band and 22 kHz too low for high band.
	// Seems register 9 is needed to set the correct load capacitance.
	// As on these AliExpress modules, there are no specs for the crystal
	// So I have to trial and error to find the correct value,
	// Tested various values, found 0x69 to give the correct frequency
	// on the 868 MHz module. Need to repeat the test on 434 MHz modules
	si4x3x_write_reg8(bsradio, 0x09, bsradio->hwconfig.tune);

	switch (bsradio->hwconfig.frequency_band) {
	case 434:
		si4x3x_set_frequency(bsradio, 434000);
		si4x3x_set_tx_power(bsradio, 0);
		break;
	case 868:
		//si4x3x_set_frequency(bsradio, 869850);
		si4x3x_set_frequency(bsradio, 870000);


//		si4x3x_set_frequency(bsradio, 868000);
		si4x3x_set_tx_power(bsradio, 18);
//		si4x3x_set_tx_power(bsradio, 0);
		break;
	case 915:
		si4x3x_set_frequency(bsradio, 915000);
		si4x3x_set_tx_power(bsradio, -3);
		break;
	}

	si4x3x_configure_packet(bsradio);

	si4x3x_set_bitrate(bsradio, 12500);
	si4x3x_set_fdev(bsradio, 12500);
	si4x3x_set_bandwidth(bsradio, 25000);
	si4x3x_update_clock_recovery(bsradio);

	si4x3x_set_sync_word32(bsradio, 0xdeadbeef);

	return 0;
}

void si4x3x_recv_test(bsradio_instance_t *bsradio) {
	sxv1_air_packet_t packet = { 0 };
	char strbuff[32];

	si4x3x_init(bsradio);
	print("RX Si4x3x", 5);
	framebuffer_apply();
	si4x3x_configure_packet(bsradio);
	si4x3x_clear_rx_fifo(bsradio);
	si4x3x_set_mode(bsradio, si4x3x_mode_reveive);

	while (1) {

		if (!si4x3x_receive_request(bsradio, &packet)) {
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
void si4x3x_send_test(bsradio_instance_t *bsradio) {
	sxv1_air_packet_t packet = { 0 };
	char strbuff[32];

	//		// Si4x3x test
	si4x3x_init(bsradio);
	print("TX Si4x3x", 5);

	//		sxv1_init();
	framebuffer_apply();
	while (1) {

		bshal_delay_ms(1000);
		packet.header.size = 9;
		packet.data[0] = 0xDE;
		packet.data[1] = 0xAD;
		packet.data[2] = 0xBE;
		packet.data[3] = 0xEF;
		packet.data[4]++;
		si4x3x_send_request(bsradio, &packet, &packet);
		//sxv1_send_request(&packet, &packet);
		sprintf(strbuff, "TX %02X", packet.data[4]);
		draw_plain_background();
		print(strbuff, 1);
		framebuffer_apply();
	}
}
void sxv1_recv_test(bsradio_instance_t *bsradio) {
	sxv1_air_packet_t packet = { 0 };
	char strbuff[32];

	sxv1_init(bsradio);
	print("RX SX123x", 5);
	framebuffer_apply();
	sxv1_set_mode(bsradio, sxv1_mode_rx);
	sxv1_rx_restart(bsradio);

	int cnt = 0;
	while (1) {

		if (!sxv1_receive_request(bsradio, &packet)) {
			printf("Packet Received\n");
			// if (packet.header.size < 16)
			//				for (int i = 0; i < packet.header.size-sizeof(packet.header); i++)
			//					printf("%02X ", packet.data[i]);

			sprintf(strbuff, "RX %02X", packet.data[4]);
			print(strbuff, 1);
			sprintf(strbuff, "CNT %02X", cnt++);
			print(strbuff, 2);
			framebuffer_apply();
			memset(&packet, 0, sizeof(packet));
			draw_plain_background();

		}
	}
}

void unsupported(void) {
	draw_plain_background();
	print("UNSUPPORTED", 4);
	framebuffer_apply();
}

void sxv1_send_test(bsradio_instance_t *bsradio) {
	sxv1_air_packet_t packet = { 0 };
	char strbuff[32];

	print("TX SX123x", 5);

	sxv1_init(bsradio);
	framebuffer_apply();
	while (1) {

		bshal_delay_ms(1000);
		packet.header.size = 9;
		packet.data[0] = 0xDE;
		packet.data[1] = 0xAD;
		packet.data[2] = 0xBE;
		packet.data[3] = 0xEF;
		packet.data[4]++;
		sxv1_send_request(bsradio, &packet, &packet);
		sprintf(strbuff, "TX %02X", packet.data[4]);
		draw_plain_background();
		print(strbuff, 1);
		framebuffer_apply();
	}
}

void sxv2_test(bsradio_instance_t *bsradio) {
	// reset is active low!!!!
	bshal_gpio_write_pin(bsradio->spim.rs_pin, 0);
	bshal_delay_ms(5);
	bshal_gpio_write_pin(bsradio->spim.rs_pin, 1);
	bshal_delay_ms(50);

	uint8_t partno = 0;
	sxv1_read_reg(bsradio, 0x42, &partno);


}

int main() {
	SystemClock_Config();
	SystemCoreClockUpdate();
	SEGGER_RTT_Init();
	dwt_init();
	bshal_delay_init();
	HAL_Init();

	bshal_delay_init();
	gp_i2c = i2c_init();
	display_init();

	draw_plain_background();
	print("RADIO DEMO", 4);
	framebuffer_apply();

	bshal_delay_ms(1000);

	bsradio_instance_t bsradio = { };

	spi_radio_init(&bsradio);
	//sxv2_test(&bsradio);

	spi_flash_init();

	uint8_t buffert[256] = { 0 };

	bool write_to_flash = false;

	protocol_header_t *header = buffert;
	bsradio_hwconfig_t *config = buffert + sizeof(protocol_header_t);
	if (write_to_flash) {
		spi_flash_read(&spi_flash_config, 0x000000, buffert, sizeof(buffert));
		spi_flash_erase_page_256(&spi_flash_config, 0x000000);
		config->tune = 108;

		//		memset(buffert, 0xFF, sizeof(buffert));
//		header->size = sizeof(protocol_header_t) + sizeof(bsradio_hwconfig_t);
//		header->cmd = 0x02;
//		header->sub = 0x20;
//		header->res = 'R';
//
//		config->chip_brand = chip_brand_semtech;
//		config->chip_type = 1;
//		config->chip_variant = -1;
//		config->module_brand = module_brand_hoperf;
//		config->module_variant = -1;
//		config->frequency_band = 868;
//		config->tune = -10;
//		config->pa_config = 1;
//		config->antenna_type = -1;
//		config->xtal_freq = 32000000;


		spi_flash_program(&spi_flash_config, 0x000000, buffert, header->size);
	} else {
		spi_flash_read(&spi_flash_config, 0x000000, buffert, sizeof(buffert));
	}

	if ((header->size == 0xFF) || (header->size == 0x00)) {
		////		// gNiceRF Si4463 module
//		bsradio.hwconfig.chip_brand = chip_brand_silabs;
//		bsradio.hwconfig.chip_type = 2;
//		bsradio.hwconfig.chip_variant = 2;
//		bsradio.hwconfig.frequency_band = 868;
//		bsradio.hwconfig.xtal_tune = 0x59;
//		bsradio.hwconfig.xtal_freq = 30000000;

//		// NoName Si4432 module
		bsradio.hwconfig.chip_brand = chip_brand_silabs;
		bsradio.hwconfig.chip_type = 1;
		bsradio.hwconfig.chip_variant = 2;
		bsradio.hwconfig.frequency_band = 868;
		bsradio.hwconfig.tune = 0x69;
		bsradio.hwconfig.xtal_freq = 30000000;


//
////		// gNiceRF Si4432 module
//		bsradio.hwconfig.chip_brand = chip_brand_silabs;
//		bsradio.hwconfig.chip_type = 1;
//		bsradio.hwconfig.chip_variant = 2;
//		bsradio.hwconfig.frequency_band = 868;
//		bsradio.hwconfig.xtal_tune = 0x79;
//		bsradio.hwconfig.xtal_freq = 30000000;
	} else {
		bsradio.hwconfig = *config;
	}
	//si4x6x_test();
//	while (1);

	if (0x87141031 != SERIALNUMBER) {
//	if (false) {
		switch (bsradio.hwconfig.chip_brand) {
		case chip_brand_semtech:
			switch (bsradio.hwconfig.chip_type) {
			case 1:
				sxv1_recv_test(&bsradio);
				break;
			case 2:
				// TODO RFM9x
				unsupported();
				break;
			default:
				unsupported();
				break;
			}
			break;
		case chip_brand_silabs:
			switch (bsradio.hwconfig.chip_type) {
			case 1:
				si4x3x_recv_test(&bsradio);
				break;
			case 2:
				si4x6x_recv_test(&bsradio);
				break;
			default:
				unsupported();
				break;
			}
			break;

		default:
			unsupported();
			break;
		}

	} else {
		switch (bsradio.hwconfig.chip_brand) {
		case chip_brand_semtech:
			switch (bsradio.hwconfig.chip_type) {
			case 1:
				sxv1_send_test(&bsradio);
				break;
			case 2:
				// TODO RFM9x
				unsupported();
				break;
			default:
				unsupported();
				break;
			}
			break;
		case chip_brand_silabs:
			switch (bsradio.hwconfig.chip_type) {
			case 1:
				si4x3x_send_test(&bsradio);
				break;
			case 2:
				si4x6x_send_test(&bsradio);
				break;
			default:
				unsupported();
				break;
			}
			break;

		default:
			unsupported();
			break;
		}

	}
}
