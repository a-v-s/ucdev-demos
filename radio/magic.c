/*
 * magic.c
 *
 *  Created on: 24 mei 2023
 *      Author: andre
 */

#include "12500_10.h"

#include <bshal_spim.h>
#include <stdbool.h>

extern bshal_spim_instance_t radio_spi_config;

void si4x6x_load_magic_values(void) {

	uint8_t magic_values[] = //RADIO_CONFIGURATION_DATA_ARRAY;

	 {
	   //     0x07, RF_POWER_UP,
	   //     0x08, RF_GPIO_PIN_CFG,
	   //     0x06, RF_GLOBAL_XO_TUNE_2,
	   //     0x05, RF_GLOBAL_CONFIG_1,
	   //     0x05, RF_INT_CTL_ENABLE_1,
	   //     0x08, RF_FRR_CTL_A_MODE_4,

	   //     0x0D, RF_PREAMBLE_TX_LENGTH_9,
	   //     0x0E, RF_SYNC_CONFIG_10,


	   //     0x10, RF_PKT_CRC_CONFIG_12,
	   //     0x10, RF_PKT_RX_THRESHOLD_12,
	   //     0x10, RF_PKT_FIELD_3_CRC_CONFIG_12,
	   //     0x10, RF_PKT_RX_FIELD_1_CRC_CONFIG_12,
	   //     0x09, RF_PKT_RX_FIELD_4_CRC_CONFIG_5,
	   //     0x08, RF_PKT_CRC_SEED_31_24_4,

	   //     0x10, RF_MODEM_MOD_TYPE_12,
	   //     0x05, RF_MODEM_FREQ_DEV_0_1,


	        0x10, RF_MODEM_TX_RAMP_DELAY_12,
	        //0x10, RF_MODEM_BCR_NCO_OFFSET_2_12,
//	        0x07, RF_MODEM_AFC_LIMITER_1_3,
//	        0x05, RF_MODEM_AGC_CONTROL_1,
//	        0x10, RF_MODEM_AGC_WINDOW_SIZE_12,
	        0x0E, RF_MODEM_RAW_CONTROL_10,
//	        0x06, RF_MODEM_RAW_SEARCH2_2,
//	        0x06, RF_MODEM_SPIKE_DET_2,
//	        0x05, RF_MODEM_RSSI_MUTE_1,
//	        0x09, RF_MODEM_DSA_CTRL1_5,
//	        0x10, RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12,
//	        0x10, RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12,
//	        0x10, RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12,


//	        0x08, RF_PA_MODE_4,
//	        0x0B, RF_SYNTH_PFDCP_CPFF_7,
//	        0x10, RF_MATCH_VALUE_1_12,

	 //       0x0C, RF_FREQ_CONTROL_INTE_8,
	        0x00
	 };


	uint8_t * magic_pointer = magic_values;
	while ( *magic_pointer) {
		bshal_spim_transmit(&radio_spi_config, magic_pointer+1, *magic_pointer, false);
		magic_pointer += 1+(*magic_pointer);
		bshal_delay_ms(10);
	}

}


