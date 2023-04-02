#include <stdint.h>
#include <system.h>
#include <string.h>
#include <stdio.h>

#if defined __ARM_EABI__
void SysTick_Handler(void) {
	HAL_IncTick();
}


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
void dwt_init(void) {
	timer_init();
}
#endif

float bmresults[3][5];

void bm(void) {
	int index;
	switch (SystemCoreClock) {
	case 8000000:
		index = 0;
		break;
	case 48000000:
		index = 1;
		break;
	case 72000000:
		index = 2;
		break;
	default:
		return;
	}

	puts("---------------------------------------");
	printf("SystemCoreClock is %lu\n", SystemCoreClock);
	printf("Starting benchmark, please wait...\n");
	//uint32_t loops = SystemCoreClock >> 5;
	//uint32_t loops = SystemCoreClock >> 6;
	uint32_t loops = SystemCoreClock >> 7; // Last without losing accuracy
	//uint32_t loops = SystemCoreClock >> 8; // Losing accuracy here
	float DSEC, DMIPS, DMIPS_MHZ;
	uint32_t benchtime;
	puts("---------------------------------------");
	puts(" O0 ");
	dwt_init();
	__DSB();
	__ISB();

	benchtime = d0(loops);

	printf("Dhrystone time for %lu passes is %lu ms\n", loops, benchtime);

	DSEC = (1000.0f * ((float) loops / (float) benchtime));
	DMIPS = DSEC / 1757;
	DMIPS_MHZ = 1000000.0f * DMIPS / (float) SystemCoreClock;

	printf("This machine benchmarks at %lu dhrystones/second\n",
			(uint32_t) DSEC);
	printf("This machine benchmarks at %lu.%03lu DMIPS\n", (uint32_t) DMIPS,
			(uint32_t)(1000.0f * DMIPS) % 1000);
	printf("This machine benchmarks at %lu.%03lu DMIPS/MHz\n",
			(uint32_t) DMIPS_MHZ, (uint32_t)(1000.0f * DMIPS_MHZ) % 1000);

	bmresults[index][0] = DMIPS_MHZ;
	puts("---------------------------------------");
	puts(" O1 ");
	dwt_init();
	__DSB();
	__ISB();

	benchtime = d1(loops);

	printf("Dhrystone time for %lu passes is %lu ms\n", loops, benchtime);

	DSEC = (1000.0f * ((float) loops / (float) benchtime));
	DMIPS = DSEC / 1757;
	DMIPS_MHZ = 1000000.0f * DMIPS / (float) SystemCoreClock;

	printf("This machine benchmarks at %lu dhrystones/second\n",
			(uint32_t) DSEC);
	printf("This machine benchmarks at %lu.%03lu DMIPS\n", (uint32_t) DMIPS,
			(uint32_t)(1000.0f * DMIPS) % 1000);
	printf("This machine benchmarks at %lu.%03lu DMIPS/MHz\n",
			(uint32_t) DMIPS_MHZ, (uint32_t)(1000.0f * DMIPS_MHZ) % 1000);

	bmresults[index][1] = DMIPS_MHZ;
	puts("---------------------------------------");
	puts(" O2 ");
	dwt_init();
	__DSB();
	__ISB();

	benchtime = d2(loops);

	printf("Dhrystone time for %lu passes is %lu ms\n", loops, benchtime);

	DSEC = (1000.0f * ((float) loops / (float) benchtime));
	DMIPS = DSEC / 1757;
	DMIPS_MHZ = 1000000.0f * DMIPS / (float) SystemCoreClock;

	printf("This machine benchmarks at %lu dhrystones/second\n",
			(uint32_t) DSEC);
	printf("This machine benchmarks at %lu.%03lu DMIPS\n", (uint32_t) DMIPS,
			(uint32_t)(1000.0f * DMIPS) % 1000);
	printf("This machine benchmarks at %lu.%03lu DMIPS/MHz\n",
			(uint32_t) DMIPS_MHZ, (uint32_t)(1000.0f * DMIPS_MHZ) % 1000);

	bmresults[index][2] = DMIPS_MHZ;
	puts("---------------------------------------");
	puts(" O3 ");
	dwt_init();
	__DSB();
	__ISB();

	benchtime = d3(loops);

	printf("Dhrystone time for %lu passes is %lu ms\n", loops, benchtime);

	DSEC = (1000.0f * ((float) loops / (float) benchtime));
	DMIPS = DSEC / 1757;
	DMIPS_MHZ = 1000000.0f * DMIPS / (float) SystemCoreClock;

	printf("This machine benchmarks at %lu dhrystones/second\n",
			(uint32_t) DSEC);
	printf("This machine benchmarks at %lu.%03lu DMIPS\n", (uint32_t) DMIPS,
			(uint32_t)(1000.0f * DMIPS) % 1000);
	printf("This machine benchmarks at %lu.%03lu DMIPS/MHz\n",
			(uint32_t) DMIPS_MHZ, (uint32_t)(1000.0f * DMIPS_MHZ) % 1000);

	bmresults[index][3] = DMIPS_MHZ;
	puts("---------------------------------------");
	puts(" Os ");
	dwt_init();
	__DSB();
	__ISB();

	benchtime = ds(loops);

	printf("Dhrystone time for %lu passes is %lu ms\n", loops, benchtime);

	DSEC = (1000.0f * ((float) loops / (float) benchtime));
	DMIPS = DSEC / 1757;
	DMIPS_MHZ = 1000000.0f * DMIPS / (float) SystemCoreClock;

	printf("This machine benchmarks at %lu dhrystones/second\n",
			(uint32_t) DSEC);
	printf("This machine benchmarks at %lu.%03lu DMIPS\n", (uint32_t) DMIPS,
			(uint32_t)(1000.0f * DMIPS) % 1000);
	printf("This machine benchmarks at %lu.%03lu DMIPS/MHz\n",
			(uint32_t) DMIPS_MHZ, (uint32_t)(1000.0f * DMIPS_MHZ) % 1000);

	bmresults[index][4] = DMIPS_MHZ;

}

int main(void) {
	__disable_irq();

	SEGGER_RTT_Init();
	puts("----------");
	puts("Dhrystone Benchmark for 32F103");
	puts("----------");

	SystemCoreClockUpdate();

	puts("----------");
	dwt_init();

	bm();
	ClockSetup_HSE8_SYS48();
	bm();
	ClockSetup_HSE8_SYS72();
	bm();

	puts("----------");
	printf("Benchmark results (DMIPS/MHz) for %s\n", mcuid());

	for (int j = 0; j < 3; j++) {
		switch (j) {
		case 0:
			printf("\n 8 MHz        ");
			break;
		case 1:
			printf("\n48 MHz        ");
			break;
		case 2:
			printf("\n72 MHz        ");
			break;
		}
		for (int i = 0; i < 5; i++) {
			putchar('O');
			putchar(i == 4 ? 's' : i + '0');
			putchar(':');
			printf(" %lu.%03lu       ", (uint32_t) bmresults[j][i],
					(uint32_t)(1000.0f * bmresults[j][i]) % 1000);
		}
	}
	putchar('\n');

	printf("<table>");
	for (int j = 0; j < 3; j++) {
		switch (j) {
		case 0:
			printf("\n<tr><td> 8 MHz</td>");
			break;
		case 1:
			printf("</tr>\n<tr><td>48 MHz</td>");
			break;
		case 2:
			printf("</tr>\n<tr><td>47 MHz</td>");
			break;
		}
		for (int i = 0; i < 5; i++) {
			printf("<td>O");
			putchar(i == 4 ? 's' : i + '0');
			putchar(':');
			printf(" %lu.%03lu</td>", (uint32_t) bmresults[j][i],
					(uint32_t)(1000.0f * bmresults[j][i]) % 1000);
		}
	}

	puts("</tr>\n</table>");
	while (1)
		;

}

