#include "rfm69.h"

#include <stdbool.h>

#include <bshal_spim.h>
#include <bshal_gpio.h>

bool g_rfm69_interrupt_flag;
bshal_spim_instance_t radio_spi_config;

// Looks like a pettern, we can optimise this
// so we don't need to keep a table
const static rfm69_rxbw_entry_t m_rxbw_entries[] = {
		{ 2600, { 7, 0b10, 0b010 } }, { 3100, { 7, 0b01, 0b010 } }, { 3900, { 7,
				0b00, 0b010 } },

		{ 5200, { 6, 0b10, 0b010 } }, { 6300, { 6, 0b01, 0b010 } }, { 7800, { 6,
				0b00, 0b010 } },

		{ 10400, { 5, 0b10, 0b010 } }, { 12500, { 5, 0b01, 0b010 } }, { 15600, {
				5, 0b00, 0b010 } },

		{ 20800, { 4, 0b10, 0b010 } }, { 25000, { 4, 0b01, 0b010 } }, { 31300, {
				4, 0b00, 0b010 } },

		{ 41700, { 3, 0b10, 0b010 } }, { 50000, { 3, 0b01, 0b010 } }, { 62500, {
				3, 0b00, 0b010 } },

		{ 83300, { 2, 0b10, 0b010 } }, { 100000, { 2, 0b01, 0b010 } }, { 125000,
				{ 2, 0b00, 0b010 } },

		{ 116700, { 1, 0b10, 0b010 } }, { 200000, { 1, 0b01, 0b010 } }, { 250000,
				{ 1, 0b00, 0b010 } },

		{ 333300, { 0, 0b10, 0b010 } }, { 400000, { 0, 0b01, 0b010 } }, {
				500000, { 0, 0b00, 0b010 } },

		// Termination
		{ 0, { 0, 0, 0 } },

};

int rfm69_write_reg(uint8_t reg, uint8_t val) {
	uint8_t buff[2] = { reg | RFM69_WRITE, val };
	return bshal_spim_transmit(&radio_spi_config, buff, sizeof(buff), false);
}

int rfm69_read_reg(uint8_t reg, uint8_t *val) {
	uint8_t buff[2] = { reg | RFM69_READ, 0xFF };
	int result = bshal_spim_transceive(&radio_spi_config, buff, sizeof(buff),
			false);
	*val = buff[1];
	return result;
}

int rfm69_write_fifo(void *data, uint8_t size) {
	int result;
	uint8_t buff[1] = { RFM69_REG_FIFO | RFM69_WRITE };
	result = bshal_spim_transmit(&radio_spi_config, buff, sizeof(buff), true);
	if (result)
		return result;
	result = bshal_spim_transmit(&radio_spi_config, &size, sizeof(size), true);
	if (result)
		return result;

	return bshal_spim_transmit(&radio_spi_config, data, size, false);

}

int rfm69_read_fifo(void *data, uint8_t *size) {
	int result;
	uint8_t buff[1] = { RFM69_REG_FIFO | RFM69_READ };
	result = bshal_spim_transmit(&radio_spi_config, buff, sizeof(buff), true);
	if (result)
		return result;
	uint8_t recv_size = 0;

	// temporary disabled for testing
	// using fixed size packets for debugging purposes
	if (0) {
		result = bshal_spim_receive(&radio_spi_config, &recv_size, 1, true);
		if (result)
			return result;
		if (recv_size > *size)
			return -1;
		*size = recv_size;
	} else {
		recv_size = *size;
	}
	return bshal_spim_receive(&radio_spi_config, data, recv_size, false);
}

int rfm69_set_frequency(int kHz) {

	//int regval = (float) (1000*kHz) / (float) RFM69_FSTEP_HZ;

	// Without the need of float
	int regval = ((uint64_t)(1000 * kHz) << 19) / RFM69_XTAL_FREQ;

	int status;

	/*
	 > The Frf setting is split across three bytes. A change in the center frequency is only
	 > taken into account when the Least Significant Byte FrfLsb in RegFrfLsb is written.

	 This suggest LSB should be written last
	 */

	status = rfm69_write_reg(RFM69_REG_FRFMSB, (regval & 0xFF0000) >> 16);
	if (status)
		return status;
	status = rfm69_write_reg(RFM69_REG_FRFMID, (regval & 0xFF00) >> 8);
	if (status)
		return status;
	status = rfm69_write_reg(RFM69_REG_FRFLSB, regval & 0xFF);
	if (status)
		return status;
	return 0;
}

int rfm69_set_mode(rfm69_mode_t mode) {
	rfm69_val_opmode_t val;
	int status = status = rfm69_read_reg(RFM69_REG_OPMODE, &val.as_uint8);
	if (status)
		return status;
	if (val.mode == mode)
		return 0;
	val.mode = mode;
	status = rfm69_write_reg(RFM69_REG_OPMODE, val.as_uint8);
	if (status)
		return status;
	rfm69_irq_flags_1_t irq_flags_1 = { 0 };
	int timeout_us = get_time_us() + RFM69_MODESWITCH_TIMEOUT_US;
	while (!irq_flags_1.mode_ready) {
		status = rfm69_read_reg(RFM69_REG_IRQFLAGS1, &irq_flags_1.as_uint8);
		if (timeout_us < get_time_us()) {
			// timeout
			return -1;
		}
	}

	return 0;
}

int rfm69_calibarte_rc(void) {
	int result;
	result = rfm69_set_mode(rfm69_mode_standby);
	if (result)
		return result;
	result = rfm69_write_reg(RFM69_REG_OSC1, 0x80);
	if (result)
		return result;
	uint8_t regval = 0x00;
	while (!regval) {
		// TODO Add Timeout
		result = rfm69_read_reg(RFM69_REG_OSC1, &regval);
		if (result)
			return result;
	}
	return 0;
}

int rfm69_set_sync_word(uint32_t sync_word) {
	//debug_println("Setting SYNC word to %08X", sync_word);
	rfm69_sync_config_t config;
	config.fifo_fill_condition = 0;
	config.sync_on = 1;
	config.sync_size = 3; // size = sync_size + 1, thus 4
	config.sync_tol = 0;
	rfm69_write_reg(RFM69_REG_SYNCCONFIG, config.as_uint8);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE1, sync_word >> 24);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE2, sync_word >> 16);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE3, sync_word >> 8);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE4, sync_word );

	rfm69_write_reg(RFM69_REG_SYNCVALUE1, sync_word & 0xFF);
	rfm69_write_reg(RFM69_REG_SYNCVALUE2, (sync_word & 0xFF00) >> 8);
	rfm69_write_reg(RFM69_REG_SYNCVALUE3, (sync_word & 0xFF0000) >> 16);
	rfm69_write_reg(RFM69_REG_SYNCVALUE4, (sync_word & 0xFF000000) >> 24);
	return 0;
}

int rfm69_set_sync_word_24bit(uint32_t sync_word) {
	//debug_println("Setting SYNC word to %08X", sync_word);
	rfm69_sync_config_t config;
	config.fifo_fill_condition = 0;
	config.sync_on = 1;
	config.sync_size = 2; // size = sync_size + 1, thus 3
	config.sync_tol = 0;
	rfm69_write_reg(RFM69_REG_SYNCCONFIG, config.as_uint8);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE1, sync_word >> 24);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE2, sync_word >> 16);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE3, sync_word >> 8);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE4, sync_word );

	rfm69_write_reg(RFM69_REG_SYNCVALUE1, sync_word & 0xFF);
	rfm69_write_reg(RFM69_REG_SYNCVALUE2, (sync_word & 0xFF00) >> 8);
	rfm69_write_reg(RFM69_REG_SYNCVALUE3, (sync_word & 0xFF0000) >> 16);
//	rfm69_write_reg(RFM69_REG_SYNCVALUE4, (sync_word & 0xFF000000) >> 24);
	return 0;
}

int rfm_set_sync_word_16bit(uint16_t sync_word) {
	rfm69_sync_config_t config;
	config.fifo_fill_condition = 0;
	config.sync_on = 1;
	config.sync_size = 1; // size = sync_size + 1, thus 2
	config.sync_tol = 0;
	rfm69_write_reg(RFM69_REG_SYNCCONFIG, config.as_uint8);
	rfm69_write_reg(RFM69_REG_SYNCVALUE1, sync_word & 0xFF);
	rfm69_write_reg(RFM69_REG_SYNCVALUE2, (sync_word & 0xFF00) >> 8);
	return 0;
}

int rfm69_restart(void) {
	// Restarting RX before changing to TX mode is something done
	// in the LowPowerLab library, with a comment it prevents a deadlock

	rfm69_packet_config2_t config2;
	rfm69_read_reg(RFM69_REG_PACKETCONFIG2, &config2.as_uint8);
	config2.restart_rx = 1;
	rfm69_write_reg(RFM69_REG_PACKETCONFIG2, config2.as_uint8);
	return 0;
}

int rfm69_set_tx_power(int tx_power) {
	// For RFM69HW Module
	// As the module states up to 20 dBm
	// it should output at PA_BOOST

	rfm69_val_palevel_t pa_level;
	if (tx_power <= -3) {

		// out of range
		return -1;
	} else if ((tx_power + 18) <= 0b11111) {
		// PA1 only

		pa_level.pa0_on = 0;
		pa_level.pa1_on = 1;
		pa_level.pa2_on = 0;
		pa_level.output_power = tx_power + 18;
	} else if ((tx_power + 14) <= 0b11111) {
		// PA1 + PA2

		pa_level.pa0_on = 0;
		pa_level.pa1_on = 1;
		pa_level.pa2_on = 1;
		pa_level.output_power = tx_power + 14;
	} else if ((tx_power + 11) <= 0b11111) {
		// PA1 + PA2 + High Power

		pa_level.pa0_on = 0;
		pa_level.pa1_on = 1;
		pa_level.pa2_on = 1;
		pa_level.output_power = tx_power + 11;
	} else {
		// out of range
		return -1;
	}
	rfm69_write_reg(RFM69_REG_PALEVEL, pa_level.as_uint8);
	return 0;
}

int rfm69_send_response(rfm69_air_packet_t *p_packet) {
	// Should we care about the data format by the library?
	int status;

	rfm69_restart();

	rfm69_set_mode(rfm69_mode_standby);
	rfm69_write_fifo(p_packet, p_packet->header.size);
	rfm69_set_mode(rfm69_mode_tx);
	rfm69_irq_flags_1_t irq_flags_1 = { 0 };
	rfm69_irq_flags_2_t irq_flags_2 = { 0 };

	int begin = get_time_us();
	while (!irq_flags_2.packet_send) {
		// TODO: ADD TIMEOUT
		status = rfm69_read_reg(RFM69_REG_IRQFLAGS2, &irq_flags_2.as_uint8);
		status = rfm69_read_reg(RFM69_REG_IRQFLAGS1, &irq_flags_1.as_uint8);
	}
	// Determining what the timeout should be
	// This will depend on packet size...
	debug_printf("Sending response took %d us\n", get_time_us() - begin);

	// Switch to RX mode, wait for answer
	rfm69_set_mode(rfm69_mode_rx);
	rfm69_restart();

	return 0;
}
int rfm69_send_request(rfm69_air_packet_t *p_request,
		rfm69_air_packet_t *p_response) {
	// Should we care about the data format by the library?
	int status;

	rfm69_restart();

	rfm69_set_mode(rfm69_mode_standby);
	rfm69_write_fifo(p_request, p_request->header.size);
	rfm69_set_mode(rfm69_mode_tx);
	rfm69_irq_flags_1_t irq_flags_1 = { 0 };
	rfm69_irq_flags_2_t irq_flags_2 = { 0 };

	int begin = get_time_us();
	while (!irq_flags_2.packet_send) {
		// TODO: ADD TIMEOUT
		status = rfm69_read_reg(RFM69_REG_IRQFLAGS2, &irq_flags_2.as_uint8);
		status = rfm69_read_reg(RFM69_REG_IRQFLAGS1, &irq_flags_1.as_uint8);
	}
	rfm69_set_mode(rfm69_mode_standby);
	// Determining what the timeout should be
	// This will depend on packet size...

//	begin = get_time_us();
//
//	// Switch to RX mode, wait for answer
//	rfm69_set_mode(rfm69_mode_rx);
//	rfm69_restart();
//	irq_flags_2.as_uint8 = 0;
//	uint32_t response_timeout_ms = get_time_ms() + RFM69_TXRX_TIMEOUT_MS;
//	while (!irq_flags_2.payload_ready) {
//		if (get_time_ms() > response_timeout_ms) {
//			// timeout;
//
//			rfm69_restart();
//			return -1;
//		}
//		status = rfm69_read_reg(RFM69_REG_IRQFLAGS2, &irq_flags_2.as_uint8);
//		status = rfm69_read_reg(RFM69_REG_IRQFLAGS1, &irq_flags_1.as_uint8);
//	}
//
//	rfm69_read_fifo(p_response, sizeof(rfm69_air_packet_t));
//
//	uint8_t rssi_raw;
//	rfm69_read_reg(RFM69_REG_RSSIVALUE, rssi_raw);
//	int8_t rssi = (-rssi_raw) / 2;
//	printf("RSSI val %d\n", rssi);

//	rfm69_restart();
	return 0;
}

int rfm69_receive_request(rfm69_air_packet_t *p_packet) {
	int status;
	rfm69_irq_flags_1_t irq_flags_1 = { 0 };
	rfm69_irq_flags_2_t irq_flags_2 = { 0 };
	status = rfm69_read_reg(RFM69_REG_IRQFLAGS2, &irq_flags_2.as_uint8);
	status = rfm69_read_reg(RFM69_REG_IRQFLAGS1, &irq_flags_1.as_uint8);
	p_packet->header.size = 0;

	if (status)
		return status;
	if (irq_flags_2.payload_ready) {
		// there is data, but how much to read
		// Is there a FIFO LEVEL register???
		uint8_t size = sizeof(rfm69_air_packet_t);
		//rfm69_read_fifo(p_packet, &size);

		uint8_t buffer[64] = {};
		size = 64;
		rfm69_read_fifo(buffer, &size);
		memcpy(p_packet, buffer, 64);


//		uint8_t buffer[64];
//		rfm69_read_fifo(buffer, sizeof(buffer));

		uint8_t rssi_raw;
		rfm69_read_reg(RFM69_REG_RSSIVALUE, &rssi_raw);
		int8_t rssi = (-rssi_raw) / 2;
		printf("RSSI val %d\n", rssi);

		char buff[32] = { 0 };
		sprintf(buff, "RSSI %d\n", rssi);
		print(buff, 0);

		// only restart after receiving packet
		rfm69_restart();

		status = 0;
	} else {
		status = -1;
	}

//	if (irq_flags_1.timeout) {
//		// What does timeout mean?
//		rfm69_restart();
//	}

	bshal_delay_ms(1);
	return status;
}

void rfm69_irq_handler(void) {
	g_rfm69_interrupt_flag = true;
}

void rfm69_configure_packet(void) {
	rfm69_packet_config1_t config1;
	config1.address_filtering = 0b00;
	config1.crc_auto_clear_off = 0;

	config1.dc_free = 0b00;


//	config1.crc_on = 1;
//	config1.packet_format = 1; // Variable Length


	config1.crc_on = 0; // for  inter-module testing
	config1.packet_format = 0; // Fixed Length for intermodule testing



	rfm69_write_reg(RFM69_REG_PACKETCONFIG1, config1.as_uint8);

	rfm69_packet_config2_t config2;
	config2.aes_on = 0;
	config2.auto_rx_restart_on = 1;
	// config2.inter_packet_rx_delay=0; //?? Verify this value
	config2.inter_packet_rx_delay = 1; //?? Verify this value
	config2.restart_rx = 0;
	rfm69_write_reg(RFM69_REG_PACKETCONFIG2, config2.as_uint8);

	rfm69_data_modul_t data_modul;
	data_modul.data_mode = 0b00; // Packet mode
	data_modul.modulation_type = 0b00; // FSK

	//data_modul.modulation_shaping = 0b00; // Gaussian filter off
	//data_modul.modulation_shaping = 0b01; // Gaussian filter 1.0
	data_modul.modulation_shaping = 0b10; // Gaussian filter 0.5
	//data_modul.modulation_shaping = 0b11; // Gaussian filter 0.3

	rfm69_write_reg(RFM69_REG_DATAMODUL, data_modul.as_uint8);

	rfm69_write_reg(RFM69_REG_PAYLOADLENGTH, 0x40); // Max size when receiving
	rfm69_write_reg(RFM69_REG_FIFOTHRESH, 0x80); // Start sending when 1 byte is in fifo

	// Two RFM69 can communicate with RSSI timeout set to 0x10
	// But receiving an Si4432 requires a higher value.
	// I put it to 0x40 but it can probably be smaller
	//rfm69_write_reg(RFM69_REG_RXTIMEOUT2, 0x10); // RSSI Timeout
	rfm69_write_reg(RFM69_REG_RXTIMEOUT2, 0x40); // RSSI Timeout

}

int rfm69_set_bitrate(int bps) {
	int bitratereg = RFM69_XTAL_FREQ / bps;
	rfm69_write_reg(RFM69_REG_BITRATEMSB, (bitratereg & 0xFF00) >> 8);
	rfm69_write_reg(RFM69_REG_BITRATELSB, bitratereg & 0xFF);
	return 0;
}

int rfm69_set_fdev(int hz) {
	int fdevregreg = hz / RFM69_FSTEP_HZ;
	rfm69_write_reg(RFM69_REG_FDEVMSB, (fdevregreg & 0xFF00) >> 8);
	rfm69_write_reg(RFM69_REG_FDEVLSB, fdevregreg & 0xFF);
	return 0;
}

int rfm69_set_bandwidth(int hz) {
	rfm69_rxbw_t rxbw;
	int i;
	for (i = 0; m_rxbw_entries[i].bandwidth; i++)
		if (m_rxbw_entries[i].bandwidth > hz)
			break;
	if (m_rxbw_entries[i].bandwidth) {
		rxbw = m_rxbw_entries[i].rxbw;
		rfm69_write_reg(RFM69_REG_RXBW, rxbw.as_uint8);
		return 0;
	}
	return -1;
}

