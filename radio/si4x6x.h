/*
 * si4x6x.h
 *
 *  Created on: 16 mei 2023
 *      Author: andre
 */

#ifndef SI4X6X_H_
#define SI4X6X_H_

#include <stdint.h>
int si4x6x_command(uint8_t cmd, void *request, uint8_t request_size,
		void *response, uint8_t response_size);

#define SI4X6X_CMD_NOP			0x00
#define SI4X6X_CMD_PART_INFO	0x01
#define SI4X6X_CMD_POWER_UP 0x02
#define SI4X6X_CMD_FUNC_INFO	0x10
#define SI4X6X_CMD_READ_CMD_BUFF	0x44

#pragma pack (push,1)
typedef struct {
	unsigned int chiprev :8;
	unsigned int part_be :16;
	unsigned int pbuild :8;
	unsigned int id_be :16;
	unsigned int customer :8;
	unsigned int romid :8;
} si4x6x_part_info_t;

typedef struct {
	union {
		struct {
			unsigned int func :6;
			unsigned int :1;
			unsigned int patch :1;
		};
		uint8_t as_uint8;
	} boot_options;
	union {
		struct {
			unsigned int txco :6;
		};
		uint8_t as_uint8;
	} xtal_options;
	unsigned int xo_freq_be :32;
} si4x6x_power_up_t;

typedef struct {
	unsigned int revext :8;
	unsigned int revbranch :8;
	unsigned int revint :8;
	unsigned int patch_be :16;
	unsigned int func :8;
} si4x6x_func_info_t;

#pragma pack(pop)

#endif /* SI4X6X_H_ */
