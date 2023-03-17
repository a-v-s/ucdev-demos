
#include "bshal_gpio.h"
#include "bshal_delay.h"
#include <system.h>

#include <stdint.h>
#define BLINKY_PIN 45 // PC13

void SystemClock_Config(void) {
#ifdef STM32F0
	//ClockSetup_HSI8_SYS48();
	//ClockSetup_HSE8_SYS32();
	//ClockSetup_HSE8_SYS24(); 
	ClockSetup_HSE8_SYS48(); 
//	ClockSetup_HSI48_SYS48(); // Seems to run at 48 but SystemCoreClockUpdate fails
#endif

#ifdef STM32F1
	ClockSetup_HSE8_SYS72();
#endif

#ifdef STM32F3
	ClockSetup_HSE8_SYS72();
#endif

#ifdef STM32F4
	SystemClock_HSE25_SYS84();
#endif

#ifdef STM32L0
//	SystemClock_HSE8_SYS32();
	SystemClock_HSI16_SYS32();
#endif

#ifdef STM32L1
	SystemClock_HSE8_SYS48();
#endif
}

void SysTick_Handler(void){}

extern uint32_t SystemCoreClock;



 __attribute__( ( section(".ramfunc") ) )
void test_cycles_ram(uint32_t time_cycles) {
	asm("ramloop:" );
	asm("subs  r0, 1"  ); 
	asm("bhi ramloop");
}

 
void test_cycles_rom(uint32_t time_cycles) {
	asm("romloop:" );
	asm("subs  r0, 1"  ); 
	asm("bhi romloop");
}

void dwt_init(void) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t dwt_get(void) {
	return DWT->CYCCNT;
}

char* getserialstring(void) {
	static char serialstring[25];
	char *serial = (char*) (0x1FFFF7E8);
	sprintf(serialstring, "%02X%02X%02X%02X"
			"%02X%02X%02X%02X"
			"%02X%02X%02X%02X", serial[0], serial[1], serial[2], serial[3],
			serial[4], serial[5], serial[6], serial[7], serial[8], serial[9],
			serial[10], serial[11]);
	return serialstring;
}

void run_test(void){
	uint32_t begin, diff;
	begin = dwt_get();
	test_cycles_rom(1000);
	diff = dwt_get() - begin;
	printf("1000 ROM loops took %d cyles\n", diff);
	begin = dwt_get();
	test_cycles_ram(1000);
	diff = dwt_get() - begin;
	printf("1000 RAM loops took %d cyles\n", diff);
}


void peri_test(void) {
	// This will test the available peripherals
	// by turning their clocks on and see which
	// bits will stick.
	RCC->AHBENR = -1;
	RCC->APB1ENR = -1;
	RCC->APB2ENR = -1;

	uint32_t ahb = RCC->AHBENR;
	uint32_t apb1 = RCC->APB1ENR;
	uint32_t apb2 = RCC->APB2ENR;

	printf("AHB   %08X\n", ahb);
	printf("APB1  %08X\n", apb1);
	printf("APB2  %08X\n", apb2);

}


extern int error_val;
typedef void * gen_ptr;
extern int try_read_access(gen_ptr p);

void scan_bus() {
	// Cortex-M specifies 0x40000000 to 0x60000000
	// as peripheral address space. STM32F103 appears
	// to have 0x40000000 to 0x40030000 defined.

	// Scanning the phole peripheral address space succeeds
	// on some ranges and fails on others.

	// Note: 0x42000000 to 0x44000000  is bit banding region
	// Results are in this range


	uint32_t results = 0;
	puts("Scanning peripheral bus: 0x40000000 to 0x40030000");
	puts("Offset 0x400");
	for (int addr = 0x40000000; addr < 0x40030000; addr += 0x400) {
	//for (int addr = 0x40000000; addr < 0x60000000; addr += 0x400) {
		int pos = (addr >> 10) & 0b11111;
		try_read_access(addr);
		results |= ((!error_val) << (31-pos));
		if (pos == 31) {
			if (results) {
				printf("%08X %08X\n", addr - 0x7c00, results);
			}
			results = 0;
			bshal_delay_us(50);
		}
	}
	puts("\n"
	"----------------------------------------------------------------------------"
		);
}

void scan_ram() {
	for (int addr = 0x20005000; addr < 0x2000F000; addr += 0x400) {
			try_read_access(addr);
			printf("%3dk %d\n", 1+(addr-0x20000000)/0x400, !error_val);;
	}
}

int main(void){
	__disable_irq(); // just to make sure interrupt don't mess with our timing



	puts(
"----------------------------------------------------------------------------"
	);
	puts("32F103 Instruction latency tester");
	printf(" Core         : %s\n", cpuid());
	printf(" Guessed chip : %s\n", mcuid());
	printf(" Bootrom type : %s\n",parse_32f103_bootrom());
	printf(" Serial number: %s\n", getserialstring());
	puts(
"----------------------------------------------------------------------------"
	);

	scan_bus();
	peri_test();
	scan_ram();
	puts(
"----------------------------------------------------------------------------"
	);

	dwt_init();

	SystemCoreClockUpdate();
	puts("Default clocking (8 MHz):");
	run_test();
//
//	ClockSetup_HSE8_SYS48();
//	SystemCoreClockUpdate();
//	puts("ClockSetup_HSE8_SYS48:");
//	run_test();
//
//	ClockSetup_HSE8_SYS72();
//	SystemCoreClockUpdate();
//	puts("ClockSetup_HSE8_SYS72:");
//	run_test();
	


	while (1);
}

//void _init(void) {}
