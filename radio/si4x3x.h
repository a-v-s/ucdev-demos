/*
 Si4x3x driver

 This driver is developed on an 	Si4431

 It should work with:
 Silicon Labs
 Si4430
 Si4431
 Si4432
 Si4313	 (RX Only)
 Si4030	 (TX Only)
 Si4031	 (TX Only)
 Si4032	 (TX Only)
 HopeRF
 RFM22B  (probably rebranded Si4431)
 RFM23B	(probably rebranded Si4432)

 The Si4x32 appears to be an Si4x31 with more transmission power (+7dB)
 This is always offsetted at the configured value.

 The Si403x appears to be an Transmitter only version. As such, the
 transmission part of this implementation should work.

 Note there appears to way to identify which variant it is.
 However, the device revision can be read. This implementation should work
 with revision "B1" and does not incoperate fixes for earlier revisions.


 */

#include <stdint.h>
#pragma pack(push,1)

typedef struct {
	uint8_t reg;
	uint16_t value;
} si4x3x_spi_16bit_t;

typedef union {
	struct {
		unsigned int filset :4;
		unsigned int ndec_exp :3;
		unsigned int dwn3_bypass :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_1c_t;

typedef union {
	struct {
		unsigned int prealen8 :1;
		unsigned int synclen :2;
		unsigned int fixpklen :1;
		unsigned int hdlen :3;
		unsigned int skipsyn :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_33_t;


typedef union {
	struct {
		unsigned int enwhite : 1;
		unsigned int enmanch : 1;
		unsigned int enmaninv : 1;
		unsigned int manppol : 1;
		unsigned int enphpwdn : 1;
		unsigned int txdtrtscale : 1;
		unsigned int : 2;
	};
	uint8_t as_uint8;
} si4x3x_reg_70_t;

typedef union {
	struct {
		unsigned int modtyp : 2;
		unsigned int fd8 : 1;
		unsigned int eninv : 1;
		unsigned int dtmod : 2;
		unsigned int trclk : 2;
	};
	uint8_t as_uint8;
} si4x3x_reg_71_t;

typedef union {
	struct {
		unsigned int fb :5;
		unsigned int hbsel :1;
		unsigned int sbsel :1;
		unsigned int :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_75_t;

typedef union {
	struct {
		unsigned int txpow :3;
		unsigned int lna_sw :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_6d_t;



typedef struct {
	uint16_t bandwidth;
	si4x3x_reg_1c_t rxbw;
} si4x3x_rxbw_entry_t;


#pragma pack(pop)

int si4x3x_set_frequency(int frequency);
int si4x3x_set_sync_word(uint32_t sync_word);
int si4x3x_set_bitrate(int bps);
int si4x3x_set_fdev(int hz);
int si4x3x_set_bandwidth(int hz);
int si4x3x_set_tx_power(int tx_power);
void si4x3x_configure_packet(void);

