/*
 * si4x6x.c
 *
 *  Created on: 16 mei 2023
 *      Author: andre
 */

#include "si4x6x.h"

#include "bshal_spim.h"
#include "bshal_delay.h"

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
	}
	if (status != 0xFF)
		return -1;
	if (response_size)
		result = bshal_spim_receive(&radio_spi_config, response, response_size,
				false);
	return result;
}

int si4x6x_test(void) {
	si4x6x_part_info_t part_info;
	si4x6x_func_info_t func_info;

	// Active High Reset
	bshal_gpio_write_pin(radio_spi_config.rs_pin, 1);
	bshal_delay_ms(5);
	bshal_gpio_write_pin(radio_spi_config.rs_pin, 0);
	bshal_delay_ms(50);

	si4x6x_command(SI4X6X_CMD_FUNC_INFO, NULL, 0, &func_info,
			sizeof(func_info));
	if (func_info.func == 0x00) {
		// In bootloader mode
		si4x6x_power_up_t power_up = { };
		power_up.boot_options.func = 1;
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

	return 0;

}
