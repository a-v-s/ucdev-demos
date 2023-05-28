/*

 File: 		spi_flash.h
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

#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include <stdint.h>

#pragma pack (push,1)
typedef struct {
	uint8_t signature[4];
	struct {
		uint8_t minor;
		uint8_t major;
	} revision;
	uint8_t nph;
	uint8_t ap;
} sfdp_header_t;

typedef struct {
	uint8_t lsb;
	struct {
		uint8_t minor;
		uint8_t major;
	} revision;
	uint8_t length; // in double words, their word size is 16, thus 32 bit
	union {
		unsigned int ptp_be :24; // parameter table pointer, 24 bit big endian
		uint8_t ptp_as_uint8[3];
	};
	uint8_t msb;
} sfdp_parameter_header_t;

typedef union {
	uint32_t words[23];
	struct {
		union {
			struct {
				unsigned int block_sector_erase_sizes :2;
				unsigned int write_granularity :1;
				unsigned int volatile_status_register_block_protest_bits :1;
			};
			uint32_t word1;
		};
		union {
			uint32_t flash_memory_density_le;
		};
	};

} basic_flash_parameters_t;

typedef struct {
	uint8_t manufacturer_id;
	uint8_t memory_type;
	uint8_t memory_density;
} spi_flash_rdid_t;

typedef struct {
	uint8_t electronic_id
} spi_flash_res_t;

typedef struct {
	uint8_t manufacturer_id;
	uint8_t device_id;
} spi_flash_rems_t;


#define SPI_FLASH_CMD_RDID	0x9F
#define SPI_FLASH_CMD_REMS	0x90
#define SPI_FLASH_CMD_RES	0xAB

#define SPI_FLASH_CMD_RDSFDP 0x5A

#pragma pack (pop)

#endif // __SPI_FLASH_H
