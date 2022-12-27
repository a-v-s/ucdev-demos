/*

 File: 		rfid_demo.c
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2017 - 2023 André van Schoubroeck <andre@blaatschaap.be>

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

#include <string.h>
#include <stdio.h>

#include "picc.h"
#include "pdc.h"

void framebuffer_apply();
void draw_plain_background();
void display_init(void);
void print(char* str, int line);

int find_card(bs_pdc_t *pdc, picc_t *picc) {
	rc52x_result_t status = 0;
	status = PICC_RequestA(pdc, picc);
	if (status)
		return status;
	status = PICC_Select(pdc, picc, 0);
	if (status)
		return status;
	return status;
}

void demo_loop(bs_pdc_t *pdcs, size_t size) {
	char str[32];
	while (1) {
		picc_t picc;

		for (int i = 0; i < size; i++) {

			if (!find_card(&pdcs[i], &picc)) {
				sprintf(str, "UID  ");
				for (int i = 0; i < picc.uid_size; i++)
					sprintf(str + strlen(str), "%02X", picc.uid[i]);
				print(str, i);
			} else {
				print("No Card found", i);
			}
		}

		framebuffer_apply();
		draw_plain_background();
		//HAL_Delay(1000);

	}
}
