/*
 * sensors.c
 *
 *  Created on: 16 dec. 2023
 *      Author: andre
 */

#include "bshal_delay.h"
#include "bshal_i2cm.h"

#include "lm75b.h"
#include "sht3x.h"
#include "bh1750.h"
#include "hcd1080.h"
#include "si70xx.h"
#include "ccs811.h"
#include "bmp280.h"
#include "scd4x.h"

bh1750_t bh1750 = { };
ccs811_t ccs811 = { };
bmp280_t bmp280 = { };
sht3x_t sht3x = { };
lm75b_t lm75b = { };
scd4x_t scd4x = { };

