/*
 * sht3x.h
 *
 *  Created on: 9 aug. 2021
 *      Author: andre
 */


#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdfix.h>

#include "bshal_i2cm.h"

typedef struct {
	bshal_i2cm_instance_t * p_i2c;
	uint8_t addr;
} sht2x_t;


#pragma pack (push,1)
typedef struct {
	int value : 14;
	int status : 2;
} sht2x_value_t;
#pragma pack (pop)


int sht2x_get_temperature_C_float(sht2x_t* sht2x, float * result);
int sht2x_get_humidity_float(sht2x_t* sht2x, float * result);


