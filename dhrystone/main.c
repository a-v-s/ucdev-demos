#include <stdint.h>
#include <system.h>


#ifdef __ARM__
void dwt_init(void) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	__DSB();
	DWT->CTRL &= !DWT_CTRL_CYCCNTENA_Msk;
	__DSB();
	DWT->CYCCNT = 0;
	__DSB();
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	__DSB();
}

uint32_t dwt_get(void) {
	return DWT->CYCCNT;
}

uint32_t get_time_ms(void) {
	uint32_t ticks = dwt_get();
	return ticks / (SystemCoreClock/1000);
}

#else
void dwt_init(void){
	timer_init();
}
#endif

void bm(void) {
	printf("SystemCoreClock is %lu\n", SystemCoreClock);
	printf("Starting benchmark, please wait...\n");
	uint32_t loops = SystemCoreClock>>5;

	dwt_init();
	__DSB();
	__ISB();
	uint32_t benchtime = Proc0(loops);


	printf("Dhrystone time for %lu passes = %lu\n", loops, benchtime);

	float DSEC = (1000.0f * ((float) loops / (float)benchtime));
	float DMIPS = DSEC /1757; 
	float DMIPS_MHZ = 1000000.0f * DMIPS / (float)SystemCoreClock;

	printf("This machine benchmarks at %lu dhrystones/second\n", (uint32_t) DSEC);
	printf("This machine benchmarks at %lu.%03lu DMIPS\n",  (uint32_t)DMIPS, (uint32_t) ( 1000.0f* DMIPS) % 1000);
	printf("This machine benchmarks at %lu.%03lu DMIPS/MHz\n", (uint32_t)DMIPS_MHZ, (uint32_t) ( 1000.0f* DMIPS_MHZ) % 1000);    

}


int main(void) {
	__disable_irq();

//	__HAL_FLASH_SET_LATENCY(1);
//	__HAL_FLASH_PREFETCH_BUFFER_ENABLE();

	SystemCoreClockUpdate();
	SEGGER_RTT_Init();
	puts("----------");
	dwt_init();

	bm();
	ClockSetup_HSE8_SYS48();
	bm();
	ClockSetup_HSE8_SYS72();
	bm();

	while (1);

}
