/*
 * identify.c
 *
 *  Created on: 3 jun. 2022
 *      Author: andre
 */

#include <stdint.h>
#include <stdbool.h>
#include <system.h>

#if defined __riscv

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#pragma pack(push,1)
typedef enum {
	RV32 = 1,
	RV64 = 2,
	RV128 = 3,
} misa_mxl_t;

typedef struct {
	union {
		uint32_t as_uint32;
		union {
			unsigned int Extensions : 25;
			struct {
				unsigned int A : 1;
				unsigned int B : 1;
				unsigned int C : 1;
				unsigned int D : 1;
				unsigned int E : 1;
				unsigned int F : 1;
				unsigned int G : 1;
				unsigned int H : 1;
				unsigned int I : 1;
				unsigned int J : 1;
				unsigned int K : 1;
				unsigned int L : 1;
				unsigned int M : 1;
				unsigned int N : 1;
				unsigned int O : 1;
				unsigned int P : 1;
				unsigned int Q : 1;
				unsigned int R : 1;
				unsigned int S : 1;
				unsigned int T : 1;
				unsigned int U : 1;
				unsigned int V : 1;
				unsigned int W : 1;
				unsigned int X : 1;
				unsigned int Y : 1;
				unsigned int Z : 1;
			} Extension;
		};
		unsigned int			: 4;
		misa_mxl_t MXL		: 2;
	};

} misa_t;
#pragma pack(pop)

char * cpuid(){
	static char arch[32];
	misa_t misa = {.as_uint32 = read_csr(misa)};
	sprintf(arch, "RV32");
	int i = strlen(arch);

	// I should be selected over E if both are available.
	if (misa.Extension.I) {
		arch[i]='I';
		i++;
	} else if (misa.Extension.E) {
		arch[i]='E';
		i++;
	} else {
		sprintf(arch, "Unkown");
		return;
	}


	if (misa.Extension.M) {
		arch[i]='M';
		i++;
	}
	if (misa.Extension.A) {
		arch[i]='A';
		i++;
	}
	if (misa.Extension.F) {
		arch[i]='F';
		i++;
	}
	if (misa.Extension.D) {
		arch[i]='D';
		i++;
	}
	if (misa.Extension.G) {
		arch[i]='G';
		i++;
	}
	if (misa.Extension.Q) {
		arch[i]='Q';
		i++;
	}
	if (misa.Extension.L) {
		arch[i]='L';
		i++;
	}
	if (misa.Extension.C) {
		arch[i]='C';
		i++;
	}
	if (misa.Extension.B) {
		arch[i]='B';
		i++;
	}
	if (misa.Extension.J) {
		arch[i]='J';
		i++;
	}
	if (misa.Extension.T) {
		arch[i]='T';
		i++;
	}
	if (misa.Extension.P) {
		arch[i]='P';
		i++;
	}
	if (misa.Extension.V) {
		arch[i]='V';
		i++;
	}
	if (misa.Extension.N) {
		arch[i]='N';
		i++;
	}
	return arch;
}


bool is_fpu_present(){
	misa_t misa = {.as_uint32 = read_csr(misa)};
	return (misa.Extension.F);
}

#define  GD32_MARCHID     0x80000022		// Identifiers for GD32VF103
#define  GD32_MVENDORID   0x0000031E		// Identifiers for GD32VF103

#define  CH32_MARCHID     0x00000000		// Identifiers for CH32V103
#define  CH32_MVENDORID   0x01020304		// Identifiers for CH32V103

char * mcuid(){
	if (read_csr(marchid) == GD32_MARCHID && read_csr(mvendorid) == GD32_MVENDORID)
		return "GD32VF103";
	if (read_csr(marchid) == CH32_MARCHID && read_csr(mvendorid) == CH32_MVENDORID)
			return "CH32F103";
	return "Unkown";
}
#elif defined __ARM_EABI__
#include "arm_cpuid.h"

char* parse_32f103_bootrom() {
	// System ROM starts at 0x1FFFF000
	// This contains some uart bootloader
	// at 0x1FFFF7E0 sits the flash size
	// at 0x1FFFF7E8 sits the UUID
	// Will calculate the CRC up to 0x1FFFF400
	// It seems some clones have copied ST's bootloader
	// That is nasty

	char *prob = "Unknown";
	__HAL_RCC_CRC_CLK_ENABLE();
	CRC_HandleTypeDef crc_handle;
	crc_handle.Instance = CRC;
	HAL_CRC_Init(&crc_handle);

	// HAL_CRC_Accumulate length is in 32-bit words rather then 8-bit bytes
	uint32_t crc32 = HAL_CRC_Calculate(&crc_handle, 0x1FFFF000, 0x100);
	switch (crc32) {
	case 0xda6104d0:
		prob = "STM32F103x6";
		break;
	case 0x27377129:
		prob = "STM32F103xB";
		// STM32, CS32, APM32
		if (0x0CF300FF == (*(int32_t*) (0x1FFFF7d0))) {
			// Seeing 0xFF 0xFF 0xFF 0xFF on STM
			// Seeing 0xFF 0x00 0xF3 0x0C on APM
			prob = "APM32 (ApexMic)";
		}
		break;
	case 0x1527d032:
		// GD32F101C6
		// GD32F103CB
		prob = "GD32F10x";
		break;
	case 0x52d42adb:
		prob = "HK32";
		break;
	case 0xdcbd2235:
		prob = "CH32";
		break;
	}
	return prob;

}

#define BOOTROM_STM32F103x6 0xda6104d0
#define BOOTROM_STM32F103xB 0x27377129
#define BOOTROM_GD32F10x 	0x1527d032
#define BOOTROM_HK32 		0x52d42adb
#define BOOTROM_CH32 		0xdcbd2235
#define BOOTROM_APM32 		0x7be2315b
#define BOOTROM_FCM32		0xd000a3e2

// RX32 and MM32 give "random" results
// on the CRC calculation as they appear
// to have inconsequent behaviour reading
// from BOOTROM area? There is something else
// at these addresses.
//#define BOOTROM_RX32		0x6e7ee3de
//#define BOOTROM_MM32		0x610aa470

const char* mcuid() {

	intptr_t ROMTABLE = (intptr_t)(0xE00FF000);
	romtable_id_t *rid = (romtable_id_t*) (ROMTABLE | 0xFD0);
	romtable_pid_t romtable_pid = extract_romtable_pid(rid);

	// System ROM starts at 0x1FFFF000
	// This contains some uart bootloader
	// at 0x1FFFF7E0 sits the flash size
	// at 0x1FFFF7E8 sits the UUID
	// Will calculate the CRC up to 0x1FFFF400
	// It seems some clones have copied ST's bootloader
	// HAL_CRC_Accumulate length is in 32-bit words rather then 8-bit bytes
	// Thus 0x100 is given to calculate up to 0x1FFFF400

	__DSB();
	__HAL_RCC_CRC_CLK_ENABLE();

	CRC_HandleTypeDef crc_handle;
	crc_handle.Instance = CRC;
	HAL_CRC_Init(&crc_handle);
	uint32_t bootrom_crc32 = HAL_CRC_Calculate(&crc_handle, 0x1FFFF000, 0x100);

	cpuid_t *cpuid = (cpuid_t*) (&SCB->CPUID);
	if ((cpuid->PartNo & (0b11 << 10)) == (0b11 << 10)) {
		// Cortex family
		if ((cpuid->PartNo & (0b10 << 4)) == (0b10 << 4)) {
			// Cortex M
			int m = cpuid->PartNo & 0xF;
			int r = cpuid->Variant;
			int p = cpuid->Revision;

			if (romtable_pid.jep106_used) {
				if (romtable_pid.identity_code == 32
						&& romtable_pid.continuation_code == 0) {
					if (m == 3 && r == 1 && p == 1) {
						switch (bootrom_crc32) {
						case BOOTROM_STM32F103xB:
							return "STM32F103xB (DIE410)";
						case BOOTROM_STM32F103x6:
							return "STM32F103x6 (DIE412)";
						default:
							return "Unknown";
						}
					} else if (m == 4&& r == 0 && p == 1 &&
					bootrom_crc32 == BOOTROM_FCM32) {
						return "FCM32";
					} else if (m == 3 && r == 2 && p == 1) {
						// Reading into RX32 BOOTROM appears to
						// read ramdom content. Looking at bootrom CRC
						// or read from memory location 0x1FFFF7d0
						// cannot be used to identify. Only going by
						// a Cortex m3r1p1 with STM in the romtable is
						// little to go by
						return "Maybe RX32";
					}
				}
				if (romtable_pid.identity_code == 81
						&& romtable_pid.continuation_code == 7) {
					return "GD32F10x";
				}
				if (romtable_pid.identity_code == 59
						&& romtable_pid.continuation_code == 4) {

					switch (bootrom_crc32) {

					case BOOTROM_APM32:
						return "APM32F103 (Geehy)";
					case BOOTROM_CH32:
						return "CH32";
					case BOOTROM_STM32F103xB:
						// THey've copied STM's bootrom!!!!
						// On location 0x1FFFF7d0
						// Seeing 0xFF 0xFF 0xFF 0xFF on STM
						// Seeing 0xFF 0x00 0xF3 0x0C on APM

						switch ((*(int32_t*) (0x1FFFF7d0))) {
						case 0xFFFFFFFF:
							return "CS32";
						case 0x0CF300FF:
							return "APM32 (ApexMic)";
						default:
							return "Unknown";
						}

					default:
						return "Unknown";

					}
				}
			} else {
				// JEP106 not used. Legacy ASCII values are used. This should not be used
				// on new products. And this note was written in the ADI v5 specs.
				// The Only value I've been able to find is 0x41 for ARM.

				// The identity/continuation code are not filled according the JEP106
				// According to specs, this is the legacy identification where
				// the Identity Code contains an ASCII value. On the HK32 we read
				// JEP106 = false / Identity = 0x55 / Continuation = 5
				// 0x55 corresponds with 'U'. This looks like this could be an ASCII Identifier.
				// However, if ASCII IDs are used, the expected Continuation would be 0, as
				// this field is "reserved, read as zero" when legacy ASCII IDs are used.

				// Even though these values are violating the specs, we can use
				// JEP106 = false, ID = 0x55, Cont = 5 to detect HK32.

				if (romtable_pid.identity_code == 0x55
						&& romtable_pid.continuation_code == 5)
					return "HK32F103";

				if (romtable_pid.identity_code == 0x00
						&& romtable_pid.continuation_code == 0)
					return "MH32F103/Air32F103";
			}
		} else {
			// Not Cortex-M
		}
	} else {
		// Not Cortex
	}
	return "Unknown";

}

bool is_fpu_present() {
	uint32_t initial_val = SCB->CPACR;
	uint32_t enable_fpu = (0b1111 << 20);
	SCB->CPACR |= enable_fpu;
	bool fpu_present = (SCB->CPACR & enable_fpu);
	SCB->CPACR = initial_val;
}

#else
#error "unsupported architecture"
#endif
