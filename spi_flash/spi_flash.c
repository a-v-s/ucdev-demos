/*

 File: 		spi_flash.c
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2023 André van Schoubroeck <andre@blaatschaap.be>

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
#include <stdfix.h>

#include "system.h"

#include "bshal_spim.h"
#include "bshal_delay.h"
#include "bshal_i2cm.h"

#include "spi_flash.h"

#include "endian.h"

bshal_spim_instance_t* spi_init() {
	static bshal_spim_instance_t flash_spi_config;
	//flash_spi_config.frequency = 1000000;
	flash_spi_config.frequency = 8000000;
	flash_spi_config.bit_order = 0; //MSB
	flash_spi_config.mode = 0;

	flash_spi_config.hw_nr = 1; // SPI2
	flash_spi_config.cs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_1);
	flash_spi_config.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);
	flash_spi_config.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	flash_spi_config.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);

	bshal_spim_init(&flash_spi_config);
	return &flash_spi_config;
}

int spi_flash_cmd(bshal_spim_instance_t *spim, uint8_t cmd, void *request,
		uint16_t request_size, uint8_t dummy_count, void *response,
		uint16_t reponse_size) {
	// We must transmit 0xFF while receiving
	memset(response, 0xFF, reponse_size);
	int result;
	result = bshal_spim_transmit(spim, &cmd, 1, true);
	if (result)
		goto error;
	if (request && request_size) {
		result = bshal_spim_transmit(spim, request, request_size, true);
		if (result)
			goto error;
	}
	if (dummy_count) {
		uint8_t dummies[dummy_count] = { };
		result = bshal_spim_transmit(spim, dummies, dummy_count, true);
		if (result)
			goto error;
	}

	result = bshal_spim_transceive(spim, response, reponse_size, false);
	if (result) {
		goto error;
	}

	return 0;
	error:

	// TODO deselct
	return result;
}
bool spi_flash_busy(bshal_spim_instance_t *spim) {
	spi_flash_sr1_t sr1;
	spi_flash_cmd(spim, SPI_FLASH_CMD_RDSR1, NULL, 0, SPI_FLASH_DMY_RDSR1, &sr1, sizeof(sr1));
	return sr1.busy;
}

bool spi_flash_wel(bshal_spim_instance_t *spim) {
	spi_flash_sr1_t sr1;
	spi_flash_cmd(spim, SPI_FLASH_CMD_RDSR1, NULL, 0, SPI_FLASH_DMY_RDSR1, &sr1, sizeof(sr1));
	return sr1.wel;
}


int spi_flash_write_enable(bshal_spim_instance_t *spim){
	puts("Sending Write Enable Command");
	spi_flash_cmd(spim, SPI_FLASH_CMD_WREN, NULL, 0, SPI_FLASH_DMY_WREN, NULL, 0);
	if (!spi_flash_wel(spim)) {
		puts("Failed to enable write. Is the Write Protect enabled?");
		return -1;
	}
	return 0;
}

int spi_flash_erase_page_256(bshal_spim_instance_t *spim, uint32_t address){
	// Note: Not all flash chips implement page erase
	// So far I've only seen this on PUYA flash
	size_t page_size = 256;
	if (spi_flash_write_enable(spim)) return -1;
	uint8_t addr[3] = {};
	addr[2] = address;
	addr[1] = address>> 8;
	addr[0] = address>> 16;
	spi_flash_cmd(spim, SPI_FLASH_CMD_PE, addr, sizeof(addr), SPI_FLASH_DMY_PE, NULL, 0);
	while (spi_flash_busy(spim));
	return 0;
}

int spi_flash_erase_sector_4k(bshal_spim_instance_t *spim, uint32_t address){
	size_t sector_size = 4096;
	if (spi_flash_write_enable(spim)) return -1;
	uint8_t addr[3] = {};
	addr[2] = address;
	addr[1] = address>> 8;
	addr[0] = address>> 16;
	spi_flash_cmd(spim, SPI_FLASH_CMD_SE, addr, sizeof(addr), SPI_FLASH_DMY_SE, NULL, 0);
	while (spi_flash_busy(spim));
	return 0;
}


int spi_flash_erase_block_32k(bshal_spim_instance_t *spim, uint32_t address){
	size_t block_size = 32768;
	if (spi_flash_write_enable(spim)) return -1;
	uint8_t addr[3] = {};
	addr[2] = address;
	addr[1] = address>> 8;
	addr[0] = address>> 16;
	spi_flash_cmd(spim, SPI_FLASH_CMD_BE32, addr, sizeof(addr), SPI_FLASH_DMY_BE32, NULL, 0);
	while (spi_flash_busy(spim));
	return 0;
}

int spi_flash_erase_block_64k(bshal_spim_instance_t *spim, uint32_t address){
	size_t block_size = 65536;
	if (spi_flash_write_enable(spim)) return -1;
	uint8_t addr[3] = {};
	addr[2] = address;
	addr[1] = address>> 8;
	addr[0] = address>> 16;
	spi_flash_cmd(spim, SPI_FLASH_CMD_BE64, addr, sizeof(addr), SPI_FLASH_DMY_BE64, NULL, 0);
	while (spi_flash_busy(spim));
	return 0;
}

int spi_flash_erase_chip(bshal_spim_instance_t *spim){
	size_t block_size = 65536;
	if (spi_flash_write_enable(spim)) return -1;
	spi_flash_cmd(spim, SPI_FLASH_CMD_CE, NULL, 0, SPI_FLASH_DMY_CE, NULL, 0);
	while (spi_flash_busy(spim));
	return 0;
}


void spi_flash_program(bshal_spim_instance_t *spim, uint32_t address, void* data, size_t size) {
	size_t page_size = 256;
	uint8_t addr[3] = {};
	size_t write_size = page_size - (address % page_size);
	if (write_size > size) write_size = size;
	int data_offset = 0;

	while (size) {
		const uint8_t cmd[] = {SPI_FLASH_CMD_PP};

		// Write Enable iIs reset after every write
		spi_flash_write_enable(spim);

		addr[2] = (address+data_offset);
		addr[1] = (address+data_offset)>> 8;
		addr[0] = (address+data_offset)>> 16;

		printf("Writing %3d bytes to address %02X%02X%02X\n",
				write_size, addr[0], addr[1], addr[2]);


		bshal_spim_transmit(spim, cmd, sizeof(cmd), true);
		bshal_spim_transmit(spim, addr, sizeof(addr), true);
		bshal_spim_transmit(spim,  data + data_offset, write_size, false);

		puts("Waiting while flash is busy");
		while (spi_flash_busy(spim));
		puts("Write finished");

		data_offset += write_size;
		size -= write_size;
		write_size = (size < page_size) ? size : page_size;
	}

	// Write Disable
	puts("Sending Write Disable Command");
	spi_flash_cmd(spim, SPI_FLASH_CMD_WRDI, NULL, 0, SPI_FLASH_DMY_WRDI, NULL, 0);
}

int spi_flash_read(bshal_spim_instance_t *spim, uint32_t address, void* data, size_t size) {
	uint8_t addr[3] = {};
	addr[2] = address;
	addr[1] = address>> 8;
	addr[0] = address>> 16;
	return spi_flash_cmd(spim, SPI_FLASH_CMD_READ, addr, sizeof(addr), SPI_FLASH_DMY_READ, data, size);
}

char* printable(char* raw) {
	static char printable_data[16];
	for (int i = 0 ; i < 16 ; i++)
		printable_data[i] = (raw[i] >= 32 && raw[i] <=127) ? raw[i] : '.';
	return printable_data;
}

int main() {
	SEGGER_RTT_Init();
	bshal_spim_instance_t *spim = spi_init();


	spi_flash_cmd(spim, SPI_FLASH_CMD_NOP, NULL, 0, 0, NULL, 0);

	spi_flash_rdid_t rdid;
	spi_flash_rems_t rems;
	spi_flash_res_t res;

	spi_flash_cmd(spim, SPI_FLASH_CMD_RDID, NULL, 0, SPI_FLASH_DMY_RDID, &rdid, sizeof(rdid));
	spi_flash_cmd(spim, SPI_FLASH_CMD_REMS, NULL, 0, SPI_FLASH_DMY_REMS, &rems, sizeof(rems));
	spi_flash_cmd(spim, SPI_FLASH_CMD_RES, NULL, 0, SPI_FLASH_DMY_RES, &res, sizeof(res));

	printf("RDID %02X %02X %02X\n", rdid.manufacturer_id, rdid.memory_type,
			rdid.memory_density);
	printf("REMS %02X          \n", rems.manufacturer_id, rems.device_id);
	printf("RES  %02X          \n", res.electronic_id);

	// When we don't support SFDP we can derive our size from the RFID response
	// While these calculations appear to work for the flash chips I've got
	// this might not work for all chips:
	// https://community.infineon.com/t5/Nor-Flash/Device-ID-definition-of-S25FL064L-and-S25FL064P/td-p/229905

	puts("Flash size:");
//	printf("Detected size is %8d bit \n", (8 << rdid.memory_density));
//	printf("                 %8d kib \n", (8 << rdid.memory_density) >> 10);
//	printf("                 %8d KiB \n", (8 << rdid.memory_density) >> 13);
//	printf("                 %8d MiB \n", (8 << rdid.memory_density) >> 23);

	printf("Detected size is %8d Byte\n", (1 << rdid.memory_density));
	printf("                 %8d KiB \n", (1 << rdid.memory_density) >> 10);
	printf("                 %8d MiB \n", (1 << rdid.memory_density) >> 20);


	sfdp_header_t sfdp_header = { };
	uint8_t address[3] = { 0x00, 0x00, 0x00 };
	spi_flash_cmd(spim, SPI_FLASH_CMD_RDSFDP, address, sizeof(address), SPI_FLASH_DMY_RDSFDP,
			&sfdp_header, sizeof(sfdp_header));
	static const uint8_t magic[] = { 'S', 'F', 'D', 'P' };
	if (!memcmp(magic, sfdp_header.signature, 4)) {
		puts("SFDP supported");
		printf("Revision %d.%d\n", sfdp_header.revision.major,
				sfdp_header.revision.minor);
		printf("Number of parameter headers: %d\n", sfdp_header.nph + 1);
		putchar('\n');

		sfdp_parameter_header_t parameter_headers[sfdp_header.nph + 1];
		address[2] = sizeof(sfdp_header); // Big Endian!!
		spi_flash_cmd(spim, SPI_FLASH_CMD_RDSFDP, address, sizeof(address), SPI_FLASH_DMY_RDSFDP,
				parameter_headers, sizeof(parameter_headers));
		for (int i = 0; i <= sfdp_header.nph; i++) {
			printf("Parameter Header %d\n", i);
			printf("Revision %d.%d\n", parameter_headers[i].revision.major,
					parameter_headers[i].revision.minor);
			printf("Parameter %02X %02X\n", parameter_headers[i].msb,
					parameter_headers[i].lsb);

			if (parameter_headers[i].msb == 0xFF, parameter_headers[i].lsb
					== 0x00) {
				puts("JEDEC Basic Flash Parameter found");
				printf("Contains %d words\n", parameter_headers[i].length);
				basic_flash_parameters_t basic_flash_parameters;

				// The field we read is little endian but I need to supply it big endian?!?!
				address[0] = parameter_headers[i].ptp_as_uint8[2];
				address[1] = parameter_headers[i].ptp_as_uint8[1];
				address[2] = parameter_headers[i].ptp_as_uint8[0];

				// So, the values we read are little endian.
				spi_flash_cmd(spim, SPI_FLASH_CMD_RDSFDP, address,
						sizeof(address), SPI_FLASH_DMY_RDSFDP, &basic_flash_parameters,
						sizeof(basic_flash_parameters));
//				for (int j = 0; j < parameter_headers[i].length; j++) {
//					printf("%d: %08X\n", j, basic_flash_parameters.words[j]);
//					bshal_delay_ms(1);
//				}

				if (basic_flash_parameters.flash_memory_density_le
						& 0x8000000) {
					//uint64_t bit_size =  1 << (basic_flash_parameters.flash_memory_density_le & ~0x8000000);
					puts("Large flash detected");

					// The parameter is 2^N bits. This representation can easily overflow our variable
					// But how likely is it to encounter such a big flash chip???

					uint64_t bit_size = 1
							<< (basic_flash_parameters.flash_memory_density_le
									& ~0x8000000);
					uint64_t kib_size = 1
							<< ((basic_flash_parameters.flash_memory_density_le
									& ~0x8000000) - 10);
					uint64_t kiB_size = 1
							<< ((basic_flash_parameters.flash_memory_density_le
									& ~0x8000000) - 13);
					uint64_t MiB_size = 1
							<< ((basic_flash_parameters.flash_memory_density_le
									& ~0x8000000) - 23);
					uint64_t GiB_size = 1
							<< ((basic_flash_parameters.flash_memory_density_le
									& ~0x8000000) - 33);
					uint64_t TiB_size = 1
							<< ((basic_flash_parameters.flash_memory_density_le
									& ~0x8000000) - 43);
				} else {
					puts("Flash size:");
					printf("Detected size is %8d bit      \n",
							1 + basic_flash_parameters.flash_memory_density_le);
					printf("                 %8d kib  \n",
							(1 + basic_flash_parameters.flash_memory_density_le)
									>> 10);
					printf("                 %8d KiB \n",
							(1 + basic_flash_parameters.flash_memory_density_le)
									>> 13);
					printf("                 %8d MiB \n",
							(1 + basic_flash_parameters.flash_memory_density_le)
									>> 23);
				}
			}
		}


			;
	} else {
		puts("SFDP not supported");

		printf(" Expected: %02X %02X %02X %02X \n", magic[0], magic[1],
				magic[2], magic[3]);
		printf(" Received: %02X %02X %02X %02X \n", sfdp_header.signature[0],
				sfdp_header.signature[1], sfdp_header.signature[2],
				sfdp_header.signature[3]);

	}

	puts("Begin chip erase");
	spi_flash_erase_chip(spim);
	puts("Chip erased");

	uint8_t test [32];
	for (int i = 0 ; i < sizeof(test) ; i++) {
		test[i] = (i % 26) + 'a';
	}

	spi_flash_program(spim, 15, test, sizeof(test));

	spi_flash_program(spim, 5, "Blaat",5);

	spi_flash_program(spim, 115, "Schaap",6);

	spi_flash_program(spim, 254, "Miauw",5);

	spi_flash_program(spim, 0, "", 1);

	uint8_t buffert[512] = {};

	spi_flash_read(spim, 0, buffert, sizeof(buffert));


	for (int i = 0 ; i < sizeof(buffert); i++ ) {
		if (0 == (i%4)) putchar(0x20);
		if (0 == (i%16)) {
			if (i >= 16) { printf("   %s",printable(buffert+i-16)); }
			bshal_delay_ms(10);
			putchar(0xA);
		}

		printf("%02X ", buffert[i]);
	}
	printf("    %s",printable(buffert+sizeof(buffert)-(16-(sizeof(buffert)%16))));
	putchar(0x0A);

	while (1)
		;
}
