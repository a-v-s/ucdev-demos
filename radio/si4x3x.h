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
		unsigned int dt :5;
	};
	uint8_t as_uint8;
} si4x3x_reg_00_t;

typedef union {
	struct {
		unsigned int vc :5;
	};
	uint8_t as_uint8;
} si4x3x_reg_01_t;

typedef union {
	struct {
		unsigned int cps :2;
		unsigned int :2;
		unsigned int freqerr :1;
		unsigned int headerr :1;
		unsigned int rxffem :1;
		unsigned int ffunfl :1;
		unsigned int ffovfl :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_02_t;

typedef union {
	struct {
		unsigned int icrcerror :1;
		unsigned int ipkvalid :1;
		unsigned int ipksent :1;
		unsigned int iext :1;
		unsigned int irxffafull :1;
		unsigned int itxffaem :1;
		unsigned int itxffafull :1;
		unsigned int ifferr :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_03_t;

typedef union {
	struct {
		unsigned int ipor :1;
		unsigned int ichiprdy :1;
		unsigned int ilbd :1;
		unsigned int iwut :1;
		unsigned int irssi :1;
		unsigned int ipreainval :1;
		unsigned int ipreaval :1;
		unsigned int iswdet :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_04_t;

typedef union {
	struct {
		unsigned int encrcerror :1;
		unsigned int enpkvalid :1;
		unsigned int enpksent :1;
		unsigned int enext :1;
		unsigned int enrxffafull :1;
		unsigned int entxffaem :1;
		unsigned int entxffafull :1;
		unsigned int enfferr :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_05_t;

typedef union {
	struct {
		unsigned int enpor :1;
		unsigned int enchiprdy :1;
		unsigned int enlbd :1;
		unsigned int enwut :1;
		unsigned int enrssi :1;
		unsigned int enpreainval :1;
		unsigned int enpreaval :1;
		unsigned int enswdet :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_06_t;

typedef union {
	struct {
		unsigned int xton :1;
		unsigned int pllon :1;
		unsigned int rxon :1;
		unsigned int txon :1;
		unsigned int x32ksel :1;
		unsigned int enwt :1;
		unsigned int enlbd :1;
		unsigned int swres :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_07_t;

typedef union {
	struct {
		unsigned int ffclrtx :1;
		unsigned int ffclrrx :1;
		unsigned int enldm :1;
		unsigned int autotx :1;
		unsigned int rxmpk :1;
		unsigned int antdiv :3;
	};
	uint8_t as_uint8;
} si4x3x_reg_08_t;

typedef union {
	struct {
		unsigned int xlc :7;
		unsigned int xtalshft :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_09_t;

typedef union {
	struct {
		unsigned int mclk :3;
		unsigned int enlfc :1;
		unsigned int clkt :2;
	};
	uint8_t as_uint8;
} si4x3x_reg_0A_t;

typedef union {
	struct {
		unsigned int gpio0 :5;
		unsigned int pup0 :1;
		unsigned int gpiodrv0 :2;
	};
	uint8_t as_uint8;
} si4x3x_reg_0B_t;

typedef union {
	struct {
		unsigned int gpio1 :5;
		unsigned int pup1 :1;
		unsigned int gpiodrv1 :2;
	};
	uint8_t as_uint8;
} si4x3x_reg_0C_t;

typedef union {
	struct {
		unsigned int gpio2 :5;
		unsigned int pup2 :1;
		unsigned int gpiodrv2 :2;
	};
	uint8_t as_uint8;
} si4x3x_reg_0D_t;

typedef union {
	struct {
		unsigned int dio0 :1;
		unsigned int dio1 :1;
		unsigned int dio2 :1;
		unsigned int itsdo :1;
		unsigned int extitst0 :1;
		unsigned int extitst1 :1;
		unsigned int extitst2 :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_0E_t;

typedef union {
	struct {
		unsigned int adcgain :2;
		unsigned int adcref :2;
		unsigned int adcsel :3;
		unsigned int adcstart_adcdone :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_0F_t;

//---

typedef union {
	struct {
		unsigned int adcoffs :4;
	};
	uint8_t as_uint8;
} si4x3x_reg_10_t;

typedef union {
	struct {
		unsigned int adc :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_11_t;

typedef union {
	struct {
		unsigned int tstrim :4;
		unsigned int entstrim :1;
		unsigned int entsoffs :1;
		unsigned int tsrange :2;
	};
	uint8_t as_uint8;
} si4x3x_reg_12_t;

typedef union {
	struct {
		unsigned int tvoffs :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_13_t;

typedef union {
	struct {
		unsigned int wtr :5;
	};
	uint8_t as_uint8;
} si4x3x_reg_14_t;

// 15 and 16 contain 16 bit value wtm

// 17 and 18 contain 16 bit value wtv

typedef union {
	struct {
		unsigned int ldc :5;
	};
	uint8_t as_uint8;
} si4x3x_reg_19_t;

typedef union {
	struct {
		unsigned int lbdt :5;
	};
	uint8_t as_uint8;
} si4x3x_reg_1A_t;

typedef union {
	struct {
		unsigned int vbat :5;
	};
	uint8_t as_uint8;
} si4x3x_reg_1B_t;

typedef union {
	struct {
		unsigned int filset :4;
		unsigned int ndec_exp :3;
		unsigned int dwn3_bypass :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_1C_t;

typedef union {
	struct {
		unsigned int ph0size :1;
		unsigned int matap :1;
		unsigned int _1p5bypass :1;
		unsigned int afcgearh :3;
		unsigned int enafc :1;
		unsigned int afcbd :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_1D_t;

typedef union {
	struct {
		unsigned int anwait :1;
		unsigned int shwait :1;
		unsigned int swant_timer :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_1E_t;

typedef union {
	struct {
		unsigned int crslow :3;
		unsigned int crfast :3;
	};
	uint8_t as_uint8;
} si4x3x_reg_1F_t;

//---

// reg 20, 21 stretch 11 bit value txosr
// reg 21, 22, 23 stretch a 20 bit value ncoff
// reg 24,25, stretch 11 bit value crgain

typedef union {
	struct {
		unsigned int rssi :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_26_t;

typedef union {
	struct {
		unsigned int rssith :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_27_t;

typedef union {
	struct {
		unsigned int adrssi :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_28_t;

typedef union {
	struct {
		unsigned int adrssi2 :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_29_t;

typedef union {
	struct {
		unsigned int adclim :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_2A_t;

// 2b, 2c, 10 bit val afc_corr
// 2c, 2d, 11 bit val ookcnt

typedef union {
	struct {
		unsigned int decay :4;
		unsigned int attacl :3;
	};
	uint8_t as_uint8;
} si4x3x_reg_2E_t;

// 2f undefined

//---

typedef union {
	struct {
		unsigned int crc :2;
		unsigned int encrc :1;
		unsigned int enpactx :1;
		unsigned int skip2ph :1;
		unsigned int crcdonly :1;
		unsigned int lsbfrst :1;
		unsigned int enpacrx :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_30_t;

typedef union {
	struct {
		unsigned int pksent :1;
		unsigned int pktx :1;
		unsigned int crcerror :1;
		unsigned int pkvalid :1;
		unsigned int pkrx :1;
		unsigned int pksrch :1;
		unsigned int rxcrc1 :1;
	};
	uint8_t as_uint8;
} si4x3x_reg_31_t;

typedef union {
	struct {
		unsigned int hdch :4;
		unsigned int bcen :4;
	};
	uint8_t as_uint8;
} si4x3x_reg_32_t;

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
		unsigned int prealen :8;
	};
	uint8_t as_uint8;
} si4x3x_reg_34_t;

typedef union {
	struct {
		unsigned int rssi_offset :3;
		unsigned int preath :5;
	};
	uint8_t as_uint8;
} si4x3x_reg_35_t;

typedef union {
	struct {
		unsigned int enwhite :1;
		unsigned int enmanch :1;
		unsigned int enmaninv :1;
		unsigned int manppol :1;
		unsigned int enphpwdn :1;
		unsigned int txdtrtscale :1;
		unsigned int :2;
	};
	uint8_t as_uint8;
} si4x3x_reg_70_t;

typedef union {
	struct {
		unsigned int modtyp :2;
		unsigned int fd8 :1;
		unsigned int eninv :1;
		unsigned int dtmod :2;
		unsigned int trclk :2;
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
	si4x3x_reg_1C_t rxbw;
} si4x3x_rxbw_entry_t;

#pragma pack(pop)

int si4x3x_set_frequency(int frequency);
int si4x3x_set_sync_word(uint32_t sync_word);
int si4x3x_set_bitrate(int bps);
int si4x3x_set_fdev(int hz);
int si4x3x_set_bandwidth(int hz);
int si4x3x_set_tx_power(int tx_power);
void si4x3x_configure_packet(void);

