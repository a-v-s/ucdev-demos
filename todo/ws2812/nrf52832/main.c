/*

 File: 		main.c
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2017, 2018, 2019, 2020 André van Schoubroeck  <andre@blaatschaap.be>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 */

/*
 * WS2812 DMA library demo for nRF52832 (nrfx library)
 */

#include <stdint.h>
#include "nrfx_systick.h"

void delay_ms(uint32_t ms) {
	nrfx_systick_delay_ms(ms);
}

void ws2812_demo(void);


#include "nrfx.h"
#include "nrfx_saadc.h"



#define ADC_CHANNEL_COUNT (1)
nrf_saadc_value_t g_adc_values[ADC_CHANNEL_COUNT];

void adc_init(void) {
    nrfx_saadc_init(6);
    nrfx_saadc_channel_t channel_configs[ADC_CHANNEL_COUNT];
    channel_configs[0] = (nrfx_saadc_channel_t)NRFX_SAADC_DEFAULT_CHANNEL_SE(NRF_SAADC_INPUT_AIN0, 0);
    channel_configs[0].channel_config.gain = NRF_SAADC_GAIN1_4;

    nrfx_saadc_channels_config(channel_configs, ADC_CHANNEL_COUNT);
    nrfx_saadc_simple_mode_set(0b1, NRF_SAADC_RESOLUTION_12BIT, NRF_SAADC_OVERSAMPLE_4X, NULL);
}

int adc_measure() {
    int result = -1;   
    nrfx_saadc_buffer_set(g_adc_values, ADC_CHANNEL_COUNT);
    if (NRFX_SUCCESS == nrfx_saadc_mode_trigger()) {
        result = 0;
    }

	float mv = (float)(*g_adc_values) / 4096.0f * 4800.0f;
    return mv;
}

int main() {
	//nrfx_systick_init();
//	adc_init();
//
//	int bat = 0;
//	while(1) {
//		bat = adc_measure();
//	}
	ws2812_demo();

	while (1)
		;;
}
