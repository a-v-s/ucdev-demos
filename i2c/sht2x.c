/*
 * sht3x.c
 *
 *  Created on: 9 aug. 2021
 *      Author: andre
 */

#include "sht2x.h"
#include "endian.h"


//#define CLOCK_STRETCH

int sht2x_get_temperature_C_float(sht2x_t* sht2x, float * result){
#ifdef CLOCK_STRETCH
	uint8_t cmd[] = {0b11100011};	// temp with clock stretch
#else
	uint8_t cmd[] =   {0b11110011}; // temp without clock stretch
#endif

	sht2x_value_t value;

	int status;
	status = bshal_i2cm_send(sht2x->p_i2c, sht2x->addr, &cmd, sizeof(cmd), false);
	if (status) return status;
#ifndef CLOCK_STRETCH
	bshal_delay_ms(85); // 14 bit max time is 85 ms.
#endif
	status = bshal_i2cm_recv(sht2x->p_i2c, sht2x->addr, &value, sizeof(value), false);
	if (status) return status;

	*result = -48.85f + 175.72f * (float)(be16toh(value.value)) / (float)(UINT16_MAX - 1);
	return status;
}


int sht2x_get_humidity_float(sht2x_t* sht2x, float * result){
#ifdef CLOCK_STRETCH
	uint8_t cmd[] = {0b11100101};	// temp with clock stretch
#else
	uint8_t cmd[] =   {0b11110101}; // temp without clock stretch
#endif
	sht2x_value_t value;

	int status;
	status = bshal_i2cm_send(sht2x->p_i2c, sht2x->addr, &cmd, sizeof(cmd), false);
	if (status) return status;
#ifndef CLOCK_STRETCH
	bshal_delay_ms(29); // 12 bit max time is 29 ms.
#endif
	status = bshal_i2cm_recv(sht2x->p_i2c, sht2x->addr, &value, sizeof(value), false);
	if (status) return status;

	*result = -6.0f + 125.0f * (float)(be16toh(value.value)) / (float)(UINT16_MAX - 1);
	return status;
}

int sht2x_get_serial(sht2x_t* sht2x, uint64_t * result){
	uint8_t cmd1[] = {0xFA, 0x0F};
	uint8_t cmd2[] = {0xFC, 0xC9};
	*result = 0;
	uint8_t *data = result;
	int status;

	status = bshal_i2cm_send(sht2x->p_i2c, sht2x->addr, &cmd1, sizeof(cmd1), false);
	if (status) return status;
	status = bshal_i2cm_recv(sht2x->p_i2c, sht2x->addr, data, 4, false);
	if (status) return status;

	status = bshal_i2cm_send(sht2x->p_i2c, sht2x->addr, &cmd2, sizeof(cmd2), false);
	if (status) return status;
	status = bshal_i2cm_recv(sht2x->p_i2c, sht2x->addr, data+4, 4, false);
	if (status) return status;

	return status;
}


