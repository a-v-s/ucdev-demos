/*
 * radio_test.c
 *
 *  Created on: 4 okt. 2022
 *      Author: andre
 */

#include "bshal_spim.h"
#include "bshal_i2cm.h"
#include "bshal_delay.h"

#include "system.h"
#include <endian.h>

#include "S2LP_Driver/bit_helpers.h"
#include "S2LP_Driver/s2lp_constants.h"
#include "S2LP_Driver/s2lp_gpio.h"
#include "S2LP_Driver/s2lp.h"
#include "S2LP_Driver/s2lp_mcu_interface.h"
#include "S2LP_Driver/s2lp_packet.h"
#include "S2LP_Driver/s2lp_power.h"
#include "S2LP_Driver/s2lp_rf.h"
#include "S2LP_Driver/s2lp_rx.h"
#include "S2LP_Driver/s2lp_tx.h"
#include "S2LP_Driver/s2lp_utils.h"


S2LP_Handle handle;


void test_tx(void) {




	uint32_t irq ;
	while (1) {


		S2LP_SetInterruptMasks(&handle,5); // rx, tx packet
		irq=S2LP_GetInterrupts(&handle); // clear
		irq=S2LP_GetInterrupts(&handle); // clear
		S2LP_SendCommand(&handle, S2LP_CMD_FLUSHTXFIFO);

		S2LP_PCKT_SetPacketLength(&handle, 4);
		S2LP_WriteFIFO(&handle, 4, "abcd");

		S2LP_SendCommand(&handle, S2LP_CMD_TX);
		S2LP_Status  status = {0};
		uint32_t irq = S2LP_GetInterrupts(&handle);
		while ((irq&4)!=4){
			irq = S2LP_GetInterrupts(&handle);
			status = S2LP_GetStatus(&handle);
		}
		//bshal_delay_ms(25);
	}


//		S2LP_Status status = S2LP_ReadStatus(&handle);
//		while (status.state==S2LP_STATE_TX)
//			status = S2LP_ReadStatus(&handle);
//		}

}

int main(void) {	
	S2LP_Status status = {0};

	S2LP_Initialize(&handle, S2LP_CLOCK_FREQ_50MHZ);
	//S2LP_Initialize(&handle, S2LP_CLOCK_FREQ_26MHZ);

	S2LP_SendCommand(&handle, S2LP_CMD_SRES); // reset
	S2LP_SendCommand(&handle, S2LP_CMD_STANDBY); // standby
	S2LP_Initialize(&handle, S2LP_CLOCK_FREQ_50MHZ);
	S2LP_RF_SetChargePumpCurrent(&handle, S2LP_CHARGE_PUMP_140UA);
	//S2LP_RF_SetChargePumpCurrent(&handle, S2LP_CHARGE_PUMP_120UA);
	//S2LP_RF_SetChargePumpCurrent(&handle, S2LP_CHARGE_PUMP_200UA);
	//S2LP_RF_SetChargePumpCurrent(&handle, S2LP_CHARGE_PUMP_240UA);


	bool calib = S2LP_CallibrateRCO(&handle);
	if (!calib) {
		__BKPT(0);
	}
	S2LP_SendCommand(&handle, S2LP_CMD_READY);



	uint8_t partnumber = S2LP_GetDevicePartNumber(&handle);
	uint8_t versionnumber = S2LP_GetDeviceVersionNumber(&handle);


	S2LP_RF_SetSynthBand(&handle,S2LP_SYNTH_BAND_MID);

	S2LP_RF_SetBaseFrequency(&handle,433500000);

//	//S2LP_RF_SetModulationType(&handle, S2LP_MODULATION_NONE);
//	S2LP_WriteRegister(&handle, S2LP_REG_MOD2, 0x77);
//
//	S2LP_SendCommand(&handle, S2LP_CMD_TX);

//	status = S2LP_GetStatus(&handle);
//	while(1) {
//
//
//		status = S2LP_ReadStatus(&handle);
//	}





	S2LP_PCKT_SetPacketFormat(&handle,S2LP_PACKET_BASIC);
	S2LP_RF_SetDataRate(&handle, 40000); // Si4332 default rate is 40K, so we put our s2lp on the same rate
	S2LP_RF_SetFrequencyDeviation(&handle, 80000); // Deviation to 2x datarate
	// We should also set the channel filtert, but it appears not fully implemented yet
	S2LP_RF_SetModulationType(&handle, S2LP_MODULATION_2GFSK);
	S2LP_PCKT_SetLengthFieldSize(&handle, 1);
	S2LP_TX_SetDataSource(&handle, S2LP_TX_SOURCE_NORMAL);


	S2LP_TX_SetDataSource(&handle, S2LP_TX_SOURCE_PN9);
	S2LP_SendCommand(&handle, S2LP_CMD_TX);


	status = S2LP_GetStatus(&handle);
//	while(1) {
//
//
//		status = S2LP_ReadStatus(&handle);
//		if (status.state == 0x74) {
//			// Undefined state error
//			S2LP_SendCommand(&handle, S2LP_CMD_SABORT);
//			status = S2LP_GetStatus(&handle);
//			S2LP_SendCommand(&handle, S2LP_CMD_TX);
//			status = S2LP_GetStatus(&handle);
//		}
//	}



	test_tx();
	while (1);


}
