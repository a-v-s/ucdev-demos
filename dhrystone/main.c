#include <stdint.h>
#include <system.h>

void dwt_init(void) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t dwt_get(void) {
	return DWT->CYCCNT;
}

uint32_t get_time_ms(void) {
	uint32_t ticks = dwt_get();
	return ticks / (SystemCoreClock/1000);
}

int main(void) {
	SystemCoreClockUpdate();
	SEGGER_RTT_Init();
	puts("----------");
	dwt_init();
	uint32_t loops = 50000;
	uint32_t benchtime = Proc0(loops);

	printf("Dhrystone time for %ld passes = %ld\n",
		(long) loops, benchtime);

		float DSEC = (1000.0f * ((float) loops / (float)benchtime));
		float DMIPS = DSEC /1757; 
		float DMIPS_MHZ = 1000000.0f * DMIPS / (float)SystemCoreClock;

		printf("This machine benchmarks at %ld dhrystones/second\n", (long) DSEC);
		printf("This machine benchmarks at %ld.%03ld DMIPS\n",  (long)DMIPS, (long) ( 1000.0f* DMIPS) % 1000);
		printf("This machine benchmarks at %ld.%03ld DMIPS/MHz\n", (long)DMIPS_MHZ, (long) ( 1000.0f* DMIPS_MHZ) % 1000);    

	while (1);

}
