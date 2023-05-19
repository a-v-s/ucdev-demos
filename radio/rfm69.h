/*
 * rfm69.h
 *
 *  Created on: 24 mrt. 2023
 *      Author: andre
 */

#ifndef COMMON_RFM69_H_
#define COMMON_RFM69_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

#define RFM69_READ 0x00
#define RFM69_WRITE 0x80

#define RFM69_REG_FIFO 0x00
#define RFM69_REG_OPMODE 0x01
#define RFM69_REG_DATAMODUL 0x02
#define RFM69_REG_BITRATEMSB 0x03
#define RFM69_REG_BITRATELSB 0x04
#define RFM69_REG_FDEVMSB 0x05
#define RFM69_REG_FDEVLSB 0x06
#define RFM69_REG_FRFMSB 0x07
#define RFM69_REG_FRFMID 0x08
#define RFM69_REG_FRFLSB 0x09
#define RFM69_REG_OSC1 0x0A
#define RFM69_REG_AFCCTRL 0x0B
#define RFM69_REG_LOWBAT 0x0C
#define RFM69_REG_LISTEN1 0x0D
#define RFM69_REG_LISTEN2 0x0E
#define RFM69_REG_LISTEN3 0x0F
#define RFM69_REG_VERSION 0x10
#define RFM69_REG_PALEVEL 0x11
#define RFM69_REG_PARAMP 0x12
#define RFM69_REG_OCP 0x13
#define RFM69_REG_LNA 0x18
#define RFM69_REG_RXBW 0x19
#define RFM69_REG_AFCBW 0x1A
#define RFM69_REG_OOKPEAK 0x1B
#define RFM69_REG_OOKAVG 0x1C
#define RFM69_REG_OOKFIX 0x1D
#define RFM69_REG_AFCFEI 0x1E
#define RFM69_REG_AFCMSB 0x1F
#define RFM69_REG_AFCLSB 0x20
#define RFM69_REG_FEIMSB 0x21
#define RFM69_REG_FEILSB 0x22
#define RFM69_REG_RSSICONFIG 0x23
#define RFM69_REG_RSSIVALUE 0x24
#define RFM69_REG_DIOMAPPING1 0x25
#define RFM69_REG_DIOMAPPING2 0x26
#define RFM69_REG_IRQFLAGS1 0x27
#define RFM69_REG_IRQFLAGS2 0x28
#define RFM69_REG_RSSITHRESH 0x29
#define RFM69_REG_RXTIMEOUT1 0x2A
#define RFM69_REG_RXTIMEOUT2 0x2B
#define RFM69_REG_PREAMBLEMSB 0x2C
#define RFM69_REG_PREAMBLELSB 0x2D
#define RFM69_REG_SYNCCONFIG 0x2E
#define RFM69_REG_SYNCVALUE1 0x2F
#define RFM69_REG_SYNCVALUE2 0x30
#define RFM69_REG_SYNCVALUE3 0x31
#define RFM69_REG_SYNCVALUE4 0x32
#define RFM69_REG_SYNCVALUE5 0x33
#define RFM69_REG_SYNCVALUE6 0x34
#define RFM69_REG_SYNCVALUE7 0x35
#define RFM69_REG_SYNCVALUE8 0x36
#define RFM69_REG_PACKETCONFIG1 0x37
#define RFM69_REG_PAYLOADLENGTH 0x38
#define RFM69_REG_NODEADRS 0x39
#define RFM69_REG_BROADCASTADRS 0x3A
#define RFM69_REG_AUTOMODES 0x3B
#define RFM69_REG_FIFOTHRESH 0x3C
#define RFM69_REG_PACKETCONFIG2 0x3D
#define RFM69_REG_AESKEY1 0x3E
#define RFM69_REG_AESKEY2 0x3F
#define RFM69_REG_AESKEY3 0x40
#define RFM69_REG_AESKEY4 0x41
#define RFM69_REG_AESKEY5 0x42
#define RFM69_REG_AESKEY6 0x43
#define RFM69_REG_AESKEY7 0x44
#define RFM69_REG_AESKEY8 0x45
#define RFM69_REG_AESKEY9 0x46
#define RFM69_REG_AESKEY10 0x47
#define RFM69_REG_AESKEY11 0x48
#define RFM69_REG_AESKEY12 0x49
#define RFM69_REG_AESKEY13 0x4A
#define RFM69_REG_AESKEY14 0x4B
#define RFM69_REG_AESKEY15 0x4C
#define RFM69_REG_AESKEY16 0x4D
#define RFM69_REG_TEMP1 0x4E
#define RFM69_REG_TEMP2 0x4F
#define RFM69_REG_TESTLNA 0x58
#define RFM69_REG_TESTPA1 0x5A
#define RFM69_REG_TESTPA2 0x5C
#define RFM69_REG_TESTDAGC 0x6F
#define RFM69_REG_TESTAFC 0x71

#define RFM69_XTAL_FREQ 32000000
// 32 Mhz / 2^19
//#define RFM69_FSTEP				(61)
#define RFM69_FSTEP_HZ (61.03515625f)
#define RFM69_FSTEP_KHZ (0.06103515625f)
#define RFM69_TXRX_TIMEOUT_MS (100)

// Switching from TX to RX may take up to 2200 µS
// With some margin, wait 3000 µS
#define RFM69_MODESWITCH_TIMEOUT_US (3000)

typedef union {
    struct {
        unsigned int : 2;
        unsigned int mode : 3;
        unsigned int listen_abort : 1;
        unsigned int listen_on : 1;
        unsigned int sequencer_off : 1;
    };
    uint8_t as_uint8;
} rfm69_val_opmode_t;

typedef union {
    struct {
        unsigned int output_power : 5;
        unsigned int pa2_on : 1;
        unsigned int pa1_on : 1;
        unsigned int pa0_on : 1;
    };
    uint8_t as_uint8;
} rfm69_val_palevel_t;

typedef enum {
    rfm69_mode_sleep = 0b000,
    rfm69_mode_standby = 0b001,
    rfm69_mode_fs = 0b010,
    rfm69_mode_tx = 0b011,
    rfm69_mode_rx = 0b100,
} rfm69_mode_t;

typedef union {
    struct {
        unsigned int sync_tol : 3;
        unsigned int sync_size : 3;
        unsigned int fifo_fill_condition : 1;
        unsigned int sync_on : 1;
    };
    uint8_t as_uint8;
} rfm69_sync_config_t;

typedef union {
    struct {
        unsigned int : 1;
        unsigned int address_filtering : 2;
        unsigned int crc_auto_clear_off : 1;
        unsigned int crc_on : 1;
        unsigned int dc_free : 2;
        unsigned int packet_format : 1;
    };
    uint8_t as_uint8;
} rfm69_packet_config1_t;

typedef union {
    struct {
        unsigned int aes_on : 1;
        unsigned int auto_rx_restart_on : 1;
        unsigned int restart_rx : 1;
        unsigned int : 1;
        unsigned int inter_packet_rx_delay : 4;
    };
    uint8_t as_uint8;
} rfm69_packet_config2_t;

typedef union {
    struct {
        unsigned int sync_address_match : 1;
        unsigned int auto_mode : 1;
        unsigned int timeout : 1;
        unsigned int rssi : 1;
        unsigned int pll_lock : 1;
        unsigned int tx_ready : 1;
        unsigned int rx_ready : 1;
        unsigned int mode_ready : 1;
    };
    uint8_t as_uint8;
} rfm69_irq_flags_1_t;

typedef union {
    struct {
        unsigned int : 1;
        unsigned int crc_ok : 1;
        unsigned int payload_ready : 1;
        unsigned int packet_send : 1;
        unsigned int fifo_overrun : 1;
        unsigned int fifo_level : 1;
        unsigned int fifo_not_empty : 1;
        unsigned int fifo_full : 1;
    };
    uint8_t as_uint8;
} rfm69_irq_flags_2_t;

typedef union {
	struct {
		unsigned int modulation_shaping : 2;
		unsigned int : 1;
		unsigned int modulation_type : 2;
		unsigned int data_mode : 2;
		unsigned int : 1;
	};
	uint8_t as_uint8;
} rfm69_data_modul_t;;


typedef union {
	struct {
		unsigned int lna_gain_select : 3;
		unsigned int lna_current_gain : 3;
		unsigned int : 1;
		unsigned int lna_zin : 1;
	};
	uint8_t as_uint8;
} rfm69_lna_t;


typedef union{
	 struct{
		unsigned int rx_bw_exp : 3; // default 101
		unsigned int rx_bw_mant : 2; // default 10
		unsigned int dcc_freq : 3;  // default 010
	};
	uint8_t as_uint8;
} rfm69_rxbw_t;

typedef struct {
	uint32_t bandwidth;
	rfm69_rxbw_t rxbw;
} rfm69_rxbw_entry_t;


////-------------------
// Attempt to be on-air compatible with LowPowerLab's RFM69 library
typedef struct {
    struct {
        uint8_t size;
        uint8_t target;
        uint8_t sender;
        uint8_t control;
    } header;
    uint8_t data[64];
} rfm69_air_packet_t;





#define RFM69_ACK_REQUEST 0x40
#define RFM69_ACK_RESPONSE 0x80

#pragma pack(pop)
#endif /* COMMON_RFM69_H_ */
