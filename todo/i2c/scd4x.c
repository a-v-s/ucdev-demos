/*
 * scd4x.c
 *
 *  Created on: 4 dec. 2022
 *      Author: andre
 */

#include "scd4x.h"

#include <stdbool.h>
#include <stdint.h>

int scd4x_start(scd4x_t *scd4x) {
	int status;
	uint16_t cmd;

	cmd = SCD4X_CMD_START_PERIODIC_MEASUREMENT;
	status = bshal_i2cm_send(scd4x->p_i2c, scd4x->addr, &cmd, sizeof(cmd),
			false);
	return status;
}


int scd4x_selftest(scd4x_t *scd4x) {
	int status;
	uint16_t cmd;
	cmd = SCD4X_CMD_PERFORM_SELF_TEST;
	status = bshal_i2cm_send(scd4x->p_i2c, scd4x->addr, &cmd, sizeof(cmd),
			false);
	bshal_delay_ms(10000);
	uint8_t self_test_result[3] = {1,2,3};
	status = bshal_i2cm_recv(scd4x->p_i2c, scd4x->addr, self_test_result,
			sizeof(self_test_result), false);
	if (status) return status;
	return self_test_result[0] || self_test_result[1];
}

int scd4x_factory_reset(scd4x_t *scd4x) {
	int status;
	uint16_t cmd;

	cmd = SCD4X_CMD_STOP_PERIODIC_MEASUREMENT;
	status = bshal_i2cm_send(scd4x->p_i2c, scd4x->addr, &cmd, sizeof(cmd),
			false);
	bshal_delay_ms(500);

	cmd = SCD4X_CMD_PERFORM_FACTORY_RESET;
	status = bshal_i2cm_send(scd4x->p_i2c, scd4x->addr, &cmd, sizeof(cmd),
			false);
	bshal_delay_ms(1200);

	return status;
}

int scd4x_get_serial(scd4x_t *scd4x, char* serial) {
	uint8_t serial_data[9];
	uint16_t cmd;
	int status;
	cmd = SCD4X_CMD_GET_SERIAL_NUMBER;
	status = bshal_i2cm_send(scd4x->p_i2c, scd4x->addr, &cmd, sizeof(cmd),
			false);
	if (status) return status;
	bshal_delay_ms(1);
	status = bshal_i2cm_recv(scd4x->p_i2c, scd4x->addr, &serial_data,
			sizeof(serial_data), false);
	if (status) return status;
	serial[0] = serial_data[0];
	serial[1] = serial_data[1];

	serial[2] = serial_data[3];
	serial[3] = serial_data[4];

	serial[4] = serial_data[5];
	serial[5] = serial_data[6];

	return 0;
}

int scd4x_get_result_float(scd4x_t *scd4x, uint16_t *co2_ppm, float *temp_C,
		uint16_t *humidity_percent) {
	int status;
	scd4x_result_t result = {0};
	uint16_t cmd;

	uint8_t data_ready[3]= {0};
	cmd = SCD4X_CMD_GET_DATA_READY_STATUS;
	status = bshal_i2cm_send(scd4x->p_i2c, scd4x->addr, &cmd, sizeof(cmd),
			true);
	if (status)
		return status;
	bshal_delay_ms(1); // execution time
	status = bshal_i2cm_recv(scd4x->p_i2c, scd4x->addr, data_ready,
			sizeof(data_ready), false);
	if (status)
		return status;
	if (data_ready[0] || data_ready[1]) {
		cmd = SCD4X_CMD_READ_MEASUREMENT;
		status = bshal_i2cm_send(scd4x->p_i2c, scd4x->addr, &cmd, sizeof(cmd),
				false);
		if (status)
			return status;
		bshal_delay_ms(1); // execution time
		status = bshal_i2cm_recv(scd4x->p_i2c, scd4x->addr, &result,
				sizeof(result), false);
			if (status)
				return status;
	} else {
		// no data;
		return -7;
	}

	*humidity_percent = 100.0f * (float) (be16toh(result.humidity))
			/ (float) (UINT16_MAX - 1);
	*temp_C = -45.0f
			+ 175.0f * (float) (be16toh(result.temperature))
					/ (float) (UINT16_MAX - 1);
	*co2_ppm = be16toh(result.co2);

	return status;
}
