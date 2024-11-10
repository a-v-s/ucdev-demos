/*

 File: 		demo.c
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2017, 2018, 2019, 2020 André van Schoubroeck

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
 * WS2812 DMA library demo
 */

#include <stdint.h>
#include "ws2812.h"

#include "bshal_delay.h"

#pragma pack 1
typedef struct {
	uint8_t g;
	uint8_t r;
	uint8_t b;
} rgb_t;

#define RED (rgb_t){0,32,0}
#define ORANGE (rgb_t){16,32,0}
#define YELLOW (rgb_t){32,32,0}
#define GREEN (rgb_t){32,0,0}
#define CYAN (rgb_t){32,0,32}
#define BLUE (rgb_t){0,0,32}
#define PURPLE (rgb_t){0,32,32}
#define WHITE (rgb_t){16,16,16}

#define LED_COUNT (19)


void ws2812_demo() {
//	rgb_t colours[LED_COUNT]  = {
//		RED,ORANGE,YELLOW,   // 3
//		GREEN,CYAN,BLUE,     // 6
//		PURPLE,RED, ORANGE,  // 9
//		YELLOW, GREEN,CYAN, // 12
//		BLUE, PURPLE, RED,  // 15
//		YELLOW, GREEN, BLUE, // 18
//		PURPLE, // 19
//		};

	rgb_t colours[LED_COUNT]  = {
			{ 0x3F, 0x00, 0x00},
			{ 0x3F, 0x00, 0x14},
			{ 0x3F, 0x00, 0x28},
			{ 0x3F, 0x00, 0x3C},
			{ 0x2E, 0x00, 0x3F},
			{ 0x1A, 0x00, 0x3F},
			{ 0x06, 0x00, 0x3F},
			{ 0x00, 0x0D, 0x3F},
			{ 0x00, 0x21, 0x3F},
			{ 0x00, 0x35, 0x3F},
			{ 0x00, 0x3F, 0x35},
			{ 0x00, 0x3F, 0x21},
			{ 0x00, 0x3F, 0x0D},
			{ 0x06, 0x3F, 0x00},
			{ 0x1A, 0x3F, 0x00},
			{ 0x2E, 0x3F, 0x00},
			{ 0x3F, 0x3C, 0x00},
			{ 0x3F, 0x28, 0x00},
			{ 0x3F, 0x14, 0x00},

	};

	rgb_t temp;

	bshal_delay_init();
//	ws2812_init();

	while (1) {
		while (ws2812_is_busy());
		ws2812_fill_buffer_decompress(0, sizeof(colours), (uint8_t *)&colours);
		ws2812_apply(sizeof(colours));
		bshal_delay_ms(100);
		temp=colours[0];
		for (int i = 0; i < (LED_COUNT-1); i++) {
			colours[i]=colours[i+1];
		}
		colours[(LED_COUNT-1)]=temp;
	}
}



