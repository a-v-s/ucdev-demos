/*
 * radio.h
 *
 *  Created on: 9 jun. 2023
 *      Author: andre
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <protocol.h>
#include <bshal_spim.h>

#pragma pack (push, 1)

typedef enum {
	chip_brand_st = 0x01,
	chip_brand_semtech = 0x02,
	chip_brand_silabs = 0x03,
	chip_brand_ti = 0x04,
	chip_brand_nordic = 0x05,
	chip_brand_amiccom = 0x06,
} bsradio_chip_brand_t;

typedef enum {
	module_brand_generic = 0x01,
	module_brand_radiocontrolli = 0x02,
	module_brand_hoperf = 0x03,
	module_brand_gnicerf = 0x04,
	module_brand_dreamlnk = 0x05,
	module_brand_aithinker = 0x06,
} bsradio_module_brand_t;

typedef enum {
	antenna_type_trace = 0x01,
	antenna_type_chip = 0x02,
	antenna_type_spring = 0x03,
} bsradio_antenna_type_t;



typedef struct {
	bsradio_chip_brand_t chip_brand : 8;
	unsigned int chip_type : 8;
	unsigned int  chip_variant : 16;
	bsradio_module_brand_t module_brand: 8;
	unsigned int  module_variant : 8;
	unsigned int frequency_band : 16;
	unsigned int xtal_tune : 8;
	unsigned int pa_config : 8;
	unsigned int antenna_type : 8;
	unsigned int : 8;
	unsigned int xtal_freq : 32;
} bsradio_config_t;

typedef struct {
	bsradio_config_t config;
	bshal_spim_instance_t spim;
} bsradio_instance_t;

#pragma pack(pop)

#endif /* RADIO_H_ */
