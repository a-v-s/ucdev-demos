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
	module_variant_xl4432_smt = 0x01,
	module_variant_rf4432pro = 0x02,

	module_variant_rf4463pro = 0x01,

} bsradio_module_variant_t;

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
} bsradio_hwconfig_t;

typedef enum {
	// TODO
	modulation_ook,
	modulation_ask,
	modulation_fsk,

} bsradio_modulation_t;

typedef struct {
	uint32_t frequency_hz;
	uint32_t freq_dev_hz;
	uint32_t bandwidth_hz;
	bsradio_modulation_t modulation : 8;
	uint8_t modulation_shaping; // gaussian filter

} bsradio_rfconfig_t;

typedef struct {
	bshal_spim_instance_t spim;
	bsradio_hwconfig_t hwconfig;
	bsradio_rfconfig_t rfconfig;
} bsradio_instance_t;

#pragma pack(pop)

#endif /* RADIO_H_ */

