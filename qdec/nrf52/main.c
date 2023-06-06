#include <nrfx_qdec.h>
#include <nrfx.h>

#include "nrf_gpio.h"

void qdec_event_handler(nrfx_qdec_event_t event) {
	static int pos = 0;
	switch (event.type) {
	case NRF_QDEC_EVENT_SAMPLERDY: {
		//printf("QDEC SAMPLE %6d\n", event.data.sample);
	}
		break;
	case NRF_QDEC_EVENT_REPORTRDY: {
		//debug_println("QDEC REPORT %6d", event.data.report.acc);
		pos += event.data.report.acc;
		printf("QDEC Current Pos: %d\n", pos);

		break;
	}
	case NRF_QDEC_EVENT_ACCOF: {
		puts("QDEC ACCOF");
		break;

	}
	}
}
static void qdec_init(void) {
	nrfx_qdec_config_t qdec_config;
	qdec_config.reportper = NRF_QDEC_REPORTPER_280;
	qdec_config.sampleper = NRF_QDEC_SAMPLEPER_128us;
	qdec_config.psela = 3;
	qdec_config.pselled = NRF_QDEC_LED_NOT_CONNECTED; // we have no led
	qdec_config.pselb = 4;
	qdec_config.ledpre = 2;// we have no led?
	qdec_config.ledpol = NRF_QDEC_LEPOL_ACTIVE_HIGH; // no led, direct connection
	qdec_config.dbfen = 0; //1; // enable debounce
	qdec_config.sample_inten = 1; // enable interrupt
	qdec_config.interrupt_priority = 6;

	nrfx_qdec_init(&qdec_config, qdec_event_handler);

    nrf_gpio_cfg_input(3, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(4, NRF_GPIO_PIN_PULLUP);
	nrfx_qdec_enable();

}

int main(void) {

	SEGGER_RTT_Init();
	puts("QDEC");
	qdec_init();

	int i = 0;
	while (1) {
		i++;

	}

}
