/*
 * si4x6x.c
 *
 *  Created on: 16 mei 2023
 *      Author: andre
 */

#include "si4x6x.h"

#include "bshal_spim.h"
#include "bshal_delay.h"

#include "rfm69.h"

#include <endian.h>

extern bshal_spim_instance_t radio_spi_config;

#define CMD_TIMEOUT_MS 20

int si4x6x_command(uint8_t cmd, void *request, uint8_t request_size,
		void *response, uint8_t response_size) {
	int result = bshal_spim_transmit(&radio_spi_config, &cmd, 1, request_size);
	if (result)
		return result;
	if (request_size)
		result = bshal_spim_transmit(&radio_spi_config, request, request_size,
				false);
	uint8_t status = 0;
	for (int i = 0; i < CMD_TIMEOUT_MS; i++) {
		uint8_t get_cts = SI4X6X_CMD_READ_CMD_BUFF;
		result = bshal_spim_transmit(&radio_spi_config, &get_cts, 1, true);
		result = bshal_spim_receive(&radio_spi_config, &status, 1,
				response_size);
		if (result)
			return result;
		if (status == 0xFF)
			break;
		bshal_gpio_write_pin(radio_spi_config.cs_pin, !radio_spi_config.cs_pol);
		bshal_delay_ms(1);
	}
	if (status != 0xFF)
		return -1;
	if (response_size)
		result = bshal_spim_receive(&radio_spi_config, response, response_size,
				false);
	return result;
}

int si4x6x_set_properties(uint8_t group, uint8_t first_property, void *data,
		uint8_t count) {

	// TODO: max count is 0xC, adjust to this fact

	// uint8_t request[count + 3] = { group, count, first_property };
	// error: variable-sized object may not be initialized except with an empty initializer
	// This should be valid in the upcoming C23 standard, right?
	// Oh well... let's do it the old fashioned way then.
	uint8_t request[count + 3] = { };
	request[0] = group;
	request[1] = count;
	request[2] = first_property;

	memcpy(request + 3, data, count);
	return si4x6x_command(SI4X6X_CMD_SET_PROPERTY, request, sizeof(request),
			NULL, 0);
}

int si4x6x_set_property(uint8_t group, uint8_t property, uint8_t value) {
	uint8_t request[] = { group, 1, property, value };
	return si4x6x_command(SI4X6X_CMD_SET_PROPERTY, request, sizeof(request),
			NULL, 0);
}

int si4x6x_get_properties(uint8_t group, uint8_t first_property, void *data,
		uint8_t count) {

	// TODO: max count is 0xC, adjust to this fact

	uint8_t request[] = { group, count, first_property };
	return si4x6x_command(SI4X6X_CMD_GET_PROPERTY, request, sizeof(request),
			data, count);
}

int si4x6x_get_property(uint8_t group, uint8_t property, uint8_t *value) {
	uint8_t request[] = { group, 1, property, value };
	return si4x6x_command(SI4X6X_CMD_SET_PROPERTY, request, sizeof(request),
			value, 1);
}

int si4x6x_write_fifo(void *data, uint8_t size) {
	//return si4x6x_command(SI4X6X_CMD_WRITE_TX_FIFO, data, size, NULL, 0);

	// Seems variable length packet.can't automatically set size
	// on the Si4x6x  (It did on the Si4x3x)
	// So we'll have to write the length value to the fifo like we
	// do on the RFM69
	uint8_t clear_fifo = 0x01;
	si4x6x_command(SI4X6X_CMD_FIFO_INFO, &clear_fifo, 1, NULL, 0);

	uint8_t cmd[] = { SI4X6X_CMD_WRITE_TX_FIFO, size };
	int result = bshal_spim_transmit(&radio_spi_config, &cmd, 2, true);
	bshal_spim_transmit(&radio_spi_config, data, size, false);

	return 0;

}

int si4x6x_read_fifo(void *data, uint8_t *size) {
	int result;
	si4x6x_cmd_fifo_count_response_t fifo_count;
	si4x6x_command(SI4X6X_CMD_FIFO_INFO, NULL, 0, &fifo_count,
			sizeof(fifo_count));

	uint8_t recv_size = fifo_count.rx_fifo_count;
	if (*size < recv_size) {
		result = -1;
	} else {
		*size = recv_size;
	}

	uint8_t cmd = SI4X6X_CMD_READ_RX_FIFO;
	result = bshal_spim_transmit(&radio_spi_config, &cmd, 1, true);
	if (result) {
		bshal_gpio_write_pin(radio_spi_config.cs_pin, !radio_spi_config.cs_pol);
		return result;
	}

	bshal_spim_receive(&radio_spi_config, data, *size, false);

	return result;
}

int si4x6x_set_sync_word(uint32_t sync_word) {
	uint8_t debug[5];
	si4x6x_get_properties(0x11, 0x00, debug, 5);

	bool pol = (sync_word & 0x80000000);
	// is the polarity of the sync the problem?
	si4x6x_set_property(0x10, 0x04, 1 | (pol << 5));

	si4x6x_set_property(0x11, 0x00, 0x03);

	// The bits in the sync word are transmitted in the opposite
	// order of the values put in the register.
	// Its awkward to write in C, when its just 2 assembly instructions
	// TODO: is there a RISC-V equivalent of the rbit instruction?
#if defined(__arm__)
//	asm("rbit %0,%0" : "=r"(sync_word) );
//	asm("rev %0,%0" : "=r"(sync_word)); // enianness?
		asm("rbit %0,%1" : "=r"(sync_word) : "r"(sync_word));
		asm("rev %0,%1" : "=r"(sync_word) : "r"(sync_word));
#else
#error "Not implemented on other architectures yet"
#endif

	si4x6x_set_properties(0x11, 0x01, &sync_word, 4);
	si4x6x_get_properties(0x11, 0x00, debug, 5);
	return 0;
}

int si4x6x_init(void) {
	si4x6x_part_info_t part_info = { };
	si4x6x_func_info_t func_info = { };

	// Active High Reset
	bshal_gpio_write_pin(radio_spi_config.rs_pin, 1);
	bshal_delay_ms(5);
	bshal_gpio_write_pin(radio_spi_config.rs_pin, 0);
	bshal_delay_ms(50);

	// First command will fail to give the correct results
	// Anyhow, giving a NOP first, we can read the PART_INFO from
	// Bootloader mode
	si4x6x_command(SI4X6X_CMD_NOP, NULL, 0, NULL, 0);

	si4x6x_command(SI4X6X_CMD_PART_INFO, NULL, 0, &part_info,
			sizeof(part_info));

	si4x6x_command(SI4X6X_CMD_FUNC_INFO, NULL, 0, &func_info,
			sizeof(func_info));

	if (func_info.func == 0x00) {
		// In bootloader mode
		si4x6x_cmd_power_up_t power_up = { };
		power_up.boot_options.func = 1;  // Normal mode
		//power_up.boot_options.func = 2; // 802.15.4 mode, no worky
		power_up.xo_freq_be = htobe16(30000000);
		si4x6x_command(SI4X6X_CMD_POWER_UP, &power_up, sizeof(power_up), NULL,
				0);
	}

	si4x6x_command(SI4X6X_CMD_FUNC_INFO, NULL, 0, &func_info,
			sizeof(func_info));
	if (func_info.func == 0x00) {
		// Still in bootloader mode?!?!
	}

	si4x6x_command(SI4X6X_CMD_PART_INFO, NULL, 0, &part_info,
			sizeof(part_info));
	(void) part_info;

// 	This seems to work
	uint8_t buffer[4];
//

	si4x6x_set_frequency(867975);	// works now

//	si4x6x_set_frequency(870000);	// works now
//
	//si4x6x_set_frequency(434000); 	// works now

	// What did it generate???
//	si4x6x_set_property(0x12, 0x08, 0x2A); // Configuration bits for reception of a variable length packet.

	// Configuration bits for reception of a variable length packet.
	// This property is applicable only in RX mode,
	si4x6x_set_property(0x12, 0x08, 0x02);

	si4x6x_set_property(0x12, 0x09, 0x01); // Field number containing the received packet length byte(s).

	si4x6x_set_sync_word(0xdeadbeef);

	char prop[] = { 0x00, //   PKT_FIELD_1_LENGTH_12_8 - Unsigned 13-bit Field 1 length value.
			0x01, //   PKT_FIELD_1_LENGTH_7_0 - Unsigned 13-bit Field 1 length value.
			0x04, //   PKT_FIELD_1_CONFIG - General data processing and packet configuration bits for Field 1.
			0x00, //   PKT_FIELD_1_CRC_CONFIG - Configuration of CRC control bits across Field 1.
			0x00, //   PKT_FIELD_2_LENGTH_12_8 - Unsigned 13-bit Field 2 length value.
			0x3F, //   PKT_FIELD_2_LENGTH_7_0 - Unsigned 13-bit Field 2 length value.
			0x00, //   PKT_FIELD_2_CONFIG - General data processing and packet configuration bits for Field 2.
			0x00, //   PKT_FIELD_2_CRC_CONFIG - Configuration of CRC control bits across Field 2.
			};
	si4x6x_set_properties(0x12, 0x0d, prop, sizeof(prop));




		si4x6x_set_bitrate(12500);
		si4x6x_set_fdev(12500);
		//si4x6x_set_bandwidth(25000);


	return 0;
}

void si4x6x_send_test(void) {
	si4x6x_init();
	int cnt = 0;
	while (true) {

		bshal_delay_ms(10000);
		rfm69_air_packet_t packet = { };
		packet.header.size = 4 + 9;
		packet.data[0] = 1;
		packet.data[1] = 2;
		packet.data[2] = 4;
		packet.data[3] = 8;

		packet.data[4] = cnt++;

		packet.data[5] = 0xAA;
		packet.data[6] = 0xAA;
		packet.data[7] = 0x55;
		packet.data[8] = 0x55;
		si4x6x_send_request(&packet, &packet);

	}
}

void si4x6x_recv_test(void) {
	si4x6x_init();
	rfm69_air_packet_t packet;
	char strbuff[32];
	int cnt = 0;
	while (true) {
		memset(&packet, 0, sizeof(packet));
		si4x6x_receive_request(&packet);
		if (packet.header.size) {
			puts("Packet Received");

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
int si4x6x_set_frequency(int kHz) {
	// Assuming 30 MHz crystal
	// 15000 = 15 kHz = 15 MHz = 2 * fxo / 4
	// This is leaving MODEM_CLKGEN_BAND at its default value.
	// WDS would change this value.
	// Is there a minimum INTE value? Because I could go
	// all the way down to 15 MHz this way. Now my step size
	// is 51 Hz. This could be smaller if I adjust the
	// MODEM_CLKGEN_BAND but is that the reason?

	// The MSB of FCFRAC must be set... but are there
	// any other restrictions?
	// From testing... it seems into should be (test > 0x35 && test < 0x4A)
	// Officially supported bands:
	// 172..175
	// 284..340
	// 340..420
	// 240..525
	// 850...1050
	//
	// Looking at the 850...1050 band, inte values are 0x37 to 0x46
	// Also... the 868 band seems to be FVCO_DIV_4, and the
	// 434 band seems to be FVCO_DIV_8. The FVCO_DIV_6 band seems
	// to give an unsupported frequency range. Trying to use it
	// gives to transmissions. (It's an illegal band anyways but still
	// why is it there?)

	int band = -1;
	uint64_t intefrac;
	int test;
	test = kHz / 15000;
	if (test > 0x35 && test < 0x4A) {
		band = 0;
		intefrac = ((uint64_t) kHz << 19) / 15000;
	}
	test = kHz / 10000;
	if (test > 0x35 && test < 0x4A) {
		band = 1;
		intefrac = ((uint64_t) kHz << 19) / 10000;
	}
	test = kHz / 7500;
	if (test > 0x35 && test < 0x4A) {
		band = 2;
		intefrac = ((uint64_t) kHz << 19) / 7500;
	}
	test = kHz / 5000;
	if (test > 0x35 && test < 0x4A) {
		band = 3;
		intefrac = ((uint64_t) kHz << 19) / 5000;
	}
	test = kHz / 3750;
	if (test > 0x35 && test < 0x4A) {
		intefrac = ((uint64_t) kHz << 19) / 3750;
		band = 4;
	}
	test = kHz / 2500;
	if (test > 0x35 && test < 0x4A) {
		band = 5;
		intefrac = ((uint64_t) kHz << 19) / 2500;
	}

	if (band == -1)
		return -1;

	si4x6x_prop_modem_clkgen_band_t bandval = { .band = band, .sy_sel = 1, };

	si4x6x_set_property(0x20, 0x51, bandval.as_uint8);

	// THis worked for 868 MHz (815 to 1132 even)
	// But not for 434. I think it would need setting
	// the band anyways, as yet untested code above does )
	// intefrac = ((uint64_t) kHz << 19) / 15000;

	uint32_t frac = intefrac & 0xFFFFF;
	uint8_t inte = (intefrac >> 19);

	// The entire fcfrac word is 20-bits in length,
	// but the MSB should always be set to 1.
	// If the bit is not set, we shift one value
	// from the integer part to the fraction part.
	if (frac < (1 << 19)) {
		frac |= (1 << 19);
//		inte--;
	}

	// It seems it must always be subtracted,
	// Not only when we set the MSB on the fracture part
	// manually. It seems to work now for all values.
	// But more testing is required
	inte--;

	si4x6x_prop_freq_control_intefraq_t val = { };
	// Big Endian... also gap between bit 15 and 16
	// Can't handle this as one number this way...
	val.inte = inte;
	val.frac_7_0 = frac;
	val.frac_15_8 = frac >> 8;
	val.frac_19_16 = frac >> 16;

	return si4x6x_set_properties(0x40, 0x00, &val, sizeof(val));
}

int si4x6x_receive_request(rfm69_air_packet_t *p_request) {
	si4x6x_cmd_start_rx_t start_rx = { .rx_len_7_0 = 64, .rxtimeout_state = 0,
			.rxvalid_state = 3, .rxinvalid_state = 3, };

	si4x6x_command(SI4X6X_CMD_START_RX, &start_rx, sizeof(start_rx), NULL, 0);

	// Quick and dirty get PACKET_RX
	uint8_t test[9] = { };

	si4x6x_command(SI4X6X_CMD_REQUEST_DEVICE_STATE, NULL, 0, test, 2);
//	if (test[0] != 8) {
//		si4x6x_command(SI4X6X_CMD_START_RX, &start_rx, sizeof(start_rx),
//						NULL, 0);
//	}

	while (!(test[3] & 0b10000)) {

		si4x6x_command(0x20, NULL, 0, test, 9);
	}

	//si4x6x_read_fifo(p_packet, sizeof (rfm69_air_packet_t) );
	uint8_t buffer[64] = { 0 };
	uint8_t size = sizeof(buffer);
	si4x6x_read_fifo(buffer, &size);
	// quick an ddirty for testing
	memcpy(p_request, buffer, 64);
	return 0;
}

int si4x6x_send_request(rfm69_air_packet_t *p_request,
		rfm69_air_packet_t *p_response) {
	int result = si4x6x_write_fifo(p_request, p_request->header.size);
	if (result)
		return result;

	// Seems variable length packet.can't automatically set size
	// on the Si4x6x  (It did on the Si4x3x)
	// So we'll have to write the length value to the fifo like we
	// do on the RFM69. TODO: confirm this is really the case, am I not
	// overlooking something?? One would think setting the size to 0
	// to let the packet handler handle the length *should* fix this.
	// Also the options to let the CRC include or exclude the length
	// suggest it should be able to handle it, so I think I am missing
	// something.
	si4x6x_cmd_start_tx_t start_tx = { .tx_len_7_0 = p_request->header.size + 1,
			.tx_complete_state = 3 };

	//si4x6x_cmd_start_tx_t start_tx = { 	.tx_complete_state = 3 };

	return si4x6x_command(SI4X6X_CMD_START_TX, &start_tx, sizeof(start_tx),
			NULL, 0);
}

int si4x6x_set_bitrate(int bps) {
	// MODEM_DATA_RATE contains a 24 bit unsiged value
	// In default configuration, 10 x the data rate
	// This is because the default value of MODEM_TX_NCO_MODE
	// is to divide the crystal frequency by 30 MHz, and then have 10x
	// oversampling. For bitrates above 200 kbps, the documentation
	// recommends setting the divisor to 3 MHz instead.
	// For now, we go with the defaults, as we are not using such
	// high data rates at this point

	int factor = 10;

//	if (200000 < bps) {
//		factor = 10;
//	} else {
//		factor = 1;
//	}
//
//  TODO: to support > 200 kpbs data rates we must set
//	MODEM_TX_NCO_MODE appropriately.

	uint32_t data_rate = htobe32(bps * factor) >> 8;
	return si4x6x_set_properties(0x20, 0x03, &data_rate, 3);

}
int si4x6x_set_fdev(int hz) {
	si4x6x_prop_modem_clkgen_band_t bandval = { };
	si4x6x_get_property(0x20, 0x51, &bandval);

	uint64_t outdiv;
	switch (bandval.band) {
	case 0:
		outdiv = 4;
		break;
	case 1:
		outdiv = 6;
		break;
	case 2:
		outdiv = 8;
		break;
	case 3:
		outdiv = 12;
		break;
	case 4:
		outdiv = 16;
		break;
	case 5:
	case 6:
	case 7:
		outdiv = 24;
		break;
	}

	uint64_t presc = bandval.sy_sel ? 2 : 4;

	uint64_t fxo = 30000000; // 30 MHz crystal
	uint64_t fdevval = ((uint64_t)hz * (outdiv << 19)) /
			(presc* fxo);

	// Endiannes and alignment... less efficient but easier this way
	si4x6x_set_property( 0x20, 0x0a, fdevval >> 16);
	si4x6x_set_property( 0x20, 0x0b, fdevval >> 8);
	si4x6x_set_property( 0x20, 0x0c, fdevval >> 0);
	return 0;
}

int si4x6x_set_bandwidth(int hz) {
	return -1;

	/*
	 * There is no table in the datasheet mentioning
	 * the values we should use here.
	 * In the API documentation it says
	 *
	 * 		Silicon Labs has pre-calculated 15 different sets
	 * 		of filter tap coefficients. The WDS Calculator will
	 * 		recommend one of these filter sets, based upon the
	 * 		RX filter bandwidth required to receive the desired
	 * 		signal. The filter bandwidth is a function of both
	 * 		the selected filter set, as well as the filter clock
	 * 		decimation ratio (see the MODEM_DECIMATION_CFG1/0 properties).
	 *
	 * I guess this means feeding the WDS program various inputs and
	 * try to find the pattern in the output.
	 *
	 * According to
	 * https://community.silabs.com/s/question/0D58Y00008sknv9SAA/license-for-example-projects-exported-with-wds?language=en_US
	 * the generated code is under Zlib license, so this generated code
	 * can legally be used in this project.
	 *
	 * Initial tests show output like
	 *
	 // # WB filter 1 (BW =   9.54 kHz);  NB-filter 1 (BW = 9.54 kHz)
	 // # WB filter 2 (BW =  25.77 kHz);  NB-filter 2 (BW = 25.77 kHz)
	 // # WB filter 2 (BW = 137.42 kHz);  NB-filter 2 (BW = 137.42 kHz)
	  *
	  * suggesting there are filters and multiplication factors for them
	  * similar to the filset and ndec_exp parameters seen on the Si4x3x.
	  * However, 137.42 / 25.77 does not give a power of 2. So it won't be
	  * as simple as find a set of filter values, and doubling them.
	  *
	  * I will have to take a closer look at the actual values to see what
	  * is actually going on here.
	  *
	  * One other thing. I have noticed the auto-selected bandwidth is rather wide
	  * eg. selecting a 103.06 kHz bandwidth for a frequency deviation of 12.5 kHz.
	  * Where I would expect something like 25 kHz. It seems this is due the
	  * selected crystal accuracy, set to 20 ppm. When selecting a more accurate
	  * signal the selected bandwidth goes down. I did notice some frequency
	  * offset of the transmission frequency. I initially attributed this to an
	  * incorrect load capacitance, but it might as well be the accuracy itself.
	 */
}
