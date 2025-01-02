/*
 * i2c.h
 *
 *  Created on: 9 aug. 2021
 *      Author: andre
 */

#ifndef I2C_H_
#define I2C_H_

#include "stm32/bshal_i2cm_stm32.h"
bshal_i2cm_instance_t *i2c_init(void);

#endif /* I2C_H_ */
