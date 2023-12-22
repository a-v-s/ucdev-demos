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

#include "timer.h"
#include "sensor_protocol.h"
#include "bsradio.h"

extern bsradio_instance_t *gp_radio;

#include <stdint.h>

lm75b_t lm75b = { };
sht3x_t sht3x = { };
bh1750_t bh1750 = { };
bmp280_t bmp280 = { };
scd4x_t scd4x = { };
ccs811_t ccs811 = { };

int16_t lm75b_temperature_centi_celcius;

int16_t sht3x_temperature_centi_celcius;
uint16_t sht3x_humidify_relative_promille;

uint16_t bh1750_illuminance_lux;

int16_t bmp280_temperature_centi_celcius;
uint16_t bmp280_air_pressure_deci_pascal;

int16_t scd4x_temperature_centi_celcius;
uint16_t scd4x_humidify_relative_promille;
uint16_t scd4x_co2_ppm;

uint16_t ccs811_eco2_ppm;
uint16_t ccs811_etvoc_ppb;

bool lm75b_ready = false;
bool sht3x_ready = false;
bool bh1750_ready = false;
bool bmp280_ready = false;
bool scd4x_ready = false;
bool ccs811_ready = false;

extern bshal_i2cm_instance_t *gp_i2c;

void sensors_send(void) {
	bsradio_packet_t request = { }, response = { };
	request.from = 0x01;
	request.to = 0x00;
#pragma pack (push,1)
	struct sensor_data_packet {
		protocol_header_t head;
		bsprot_sensor_enviromental_data_t data;
	} sensor_data_packet;
	itph_protocol_packet_t *packet = &sensor_data_packet;
	sensor_data_packet.head.size = sizeof(sensor_data_packet);
	sensor_data_packet.head.cmd = BSCP_CMD_SENSOR_ENVIOREMENTAL_VALUE;
	sensor_data_packet.head.sub = ITPH_SUB_SDAT;
#pragma pack (pop)
	if (lm75b_ready) {
		lm75b_ready = false;

		sensor_data_packet.data.id = 0;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_temperature;
		sensor_data_packet.data.value.temperature_centi_celcius =
				lm75b_temperature_centi_celcius;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}
	}

	if (sht3x_ready) {
		sht3x_ready = false;

		sensor_data_packet.data.id = 1;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_temperature;
		sensor_data_packet.data.value.temperature_centi_celcius =
				sht3x_temperature_centi_celcius;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}

		sensor_data_packet.data.id = 1;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_humidity;
		sensor_data_packet.data.value.humidify_relative_promille =
				sht3x_humidify_relative_promille;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}
	}

	if (bh1750_ready) {
		bh1750_ready = false;

		sensor_data_packet.data.id = 2;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_illuminance;
		sensor_data_packet.data.value.illuminance_lux = bh1750_illuminance_lux;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}
	}

	if (bmp280_ready) {
		bmp280_ready = false;

		sensor_data_packet.data.id = 3;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_temperature;
		sensor_data_packet.data.value.temperature_centi_celcius =
				bmp280_temperature_centi_celcius;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}

		sensor_data_packet.data.id = 3;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_airpressure;
		sensor_data_packet.data.value.air_pressure_deci_pascal =
				bmp280_air_pressure_deci_pascal;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}
	}

	if (scd4x_ready) {
		scd4x_ready = false;

		sensor_data_packet.data.id = 4;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_temperature;
		sensor_data_packet.data.value.temperature_centi_celcius =
				scd4x_temperature_centi_celcius;
		;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}

		sensor_data_packet.data.id = 4;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_humidity;
		sensor_data_packet.data.value.humidify_relative_promille =
				scd4x_humidify_relative_promille;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}

		sensor_data_packet.data.id = 4;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_co2;
		sensor_data_packet.data.value.co2_ppm = scd4x_co2_ppm;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}
	}

	if (ccs811_ready) {
		ccs811_ready = false;

		// uint16_t ;
		//uint16_t ;
		sensor_data_packet.data.id = 5;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_eco2;
		sensor_data_packet.data.value.eco2_ppm = ccs811_eco2_ppm;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}

		sensor_data_packet.data.id = 5;
		sensor_data_packet.data.type = bsprot_sensor_enviromental_etvoc;
		sensor_data_packet.data.value.etvoc_ppb = ccs811_etvoc_ppb;
		if (protocol_packet_merge(request.payload, sizeof(request.payload),
				packet)) {
			// packet is full, send it.
			request.length = 4 + protocol_merged_packet_size(request.payload,
					sizeof(request.payload));
			bsradio_send_request(gp_radio, &request, &response);
			request.payload[0] = 0;
			protocol_packet_merge(request.payload, sizeof(request.payload),
					packet);
		}
	}

	// That's all, send remaining
	request.length = 4 + protocol_merged_packet_size(request.payload,
			sizeof(request.payload));
	bsradio_send_request(gp_radio, &request, &response);
	request.payload[0] = 0;

}

void sensors_process(void) {
	static uint32_t process_time = 0;
	if (get_time_ms() > process_time) {
		process_time = get_time_ms() + 5000;

		if (lm75b.addr) {
			float temperature_f;
			lm75b_get_temperature_C_float(&lm75b, &temperature_f);
			lm75b_temperature_centi_celcius = 100 * temperature_f;
			lm75b_ready = true;
		}

		if (sht3x.addr) {
			float temperature_f;
			float huminity_f;
			sht3x_get_humidity_float(&sht3x, &huminity_f);
			sht3x_get_temperature_C_float(&sht3x, &temperature_f);
			sht3x_temperature_centi_celcius = 100 * temperature_f;
			sht3x_humidify_relative_promille = 10 * huminity_f;
			sht3x_ready = true;
		}

		if (bh1750.addr) {
			static int lux = 0;
			bh1750_measure_ambient_light(&bh1750, &lux);
			bh1750_illuminance_lux = lux;
			bh1750_ready = true;
		}

		if (bmp280.addr) {
			float temperature_f;
			float pressure_f;
			bmp280_measure_f(&bmp280, &temperature_f, &pressure_f);
			bmp280_temperature_centi_celcius = 100 * temperature_f;
			bmp280_air_pressure_deci_pascal = pressure_f / 10;
			bmp280_ready = true;
		}

		if (scd4x.addr) {
			static float temp_C = 0, humidity_percent = 0;
			static uint16_t co2_ppm = 0;
			if (!scd4x_get_result_float(&scd4x, &co2_ppm, &temp_C,
					&humidity_percent)) {
				scd4x_temperature_centi_celcius = 100 * temp_C;
				scd4x_humidify_relative_promille = humidity_percent * 10;
				scd4x_co2_ppm = co2_ppm;
				scd4x_ready = true;
			}
		}

		if (ccs811.addr) {
			uint16_t eTVOC = 0;
			uint16_t eCO2 = 0;
			css811_measure(&ccs811, &eCO2, &eTVOC);
			ccs811_eco2_ppm = eCO2;
			ccs811_etvoc_ppb = eTVOC;
			ccs811_ready = true;
		}

		sensors_send();

	}
}

void sensors_init(void) {
	if (0 == bshal_i2cm_isok(gp_i2c, LM75B_I2C_ADDR)) {
		lm75b.addr = LM75B_I2C_ADDR;
		lm75b.p_i2c = gp_i2c;
	}
	if (0 == bshal_i2cm_isok(gp_i2c, SHT3X_I2C_ADDR)) {
		sht3x.addr = SHT3X_I2C_ADDR;
		sht3x.p_i2c = gp_i2c;
	}
	if (0 == bshal_i2cm_isok(gp_i2c, BH1750_I2C_ADDR)) {
		bh1750.addr = BH1750_I2C_ADDR;
		bh1750.p_i2c = gp_i2c;
	}
	if (0 == bshal_i2cm_isok(gp_i2c, BMP280_I2C_ADDR)) {
		bmp280.addr = BMP280_I2C_ADDR;
		bmp280.p_i2c = gp_i2c;
		bmp280_init(&bmp280);
	}
	if (0 == bshal_i2cm_isok(gp_i2c, SCD4X_I2C_ADDR)) {
		scd4x.addr = SCD4X_I2C_ADDR;
		scd4x.p_i2c = gp_i2c;
		scd4x_start(&scd4x);
	}
	if (0 == bshal_i2cm_isok(gp_i2c, CCS811_I2C_ADDR1)) {
		ccs811.addr = CCS811_I2C_ADDR1;
		ccs811.p_i2c = gp_i2c;
		ccs811_init(&ccs811);
	}
}

