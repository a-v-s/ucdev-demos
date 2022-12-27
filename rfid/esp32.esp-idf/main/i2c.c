/*
 * i2c.c
 *
 *  Created on: 8 aug. 2021
 *      Author: andre
 */

#include "i2c.h"

static bshal_i2cm_instance_t m_i2c;
bshal_i2cm_instance_t *gp_i2c = &m_i2c;

bshal_i2cm_instance_t * i2c_init(int sda, int scl) {
	m_i2c.sda_pin = sda;
	m_i2c.scl_pin = scl;
	m_i2c.hw_nr = 0;
	m_i2c.speed_hz = 100000;
	bshal_i2cm_init(&m_i2c);
	return &m_i2c;
}
