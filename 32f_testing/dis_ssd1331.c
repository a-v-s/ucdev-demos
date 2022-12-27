/*
 * dis_st7735.c
 *
 *  Created on: 26 mrt. 2022
 *      Author: andre
 */

#include "system.h"

#include "ucg.h"
static ucg_t m_ucg;

#include "ucg_spi.h"
#include "bshal_spim.h"

void display_init(void) {


	bshal_spim_instance_t screen_spi_config;
	screen_spi_config.frequency = 1000000;
	screen_spi_config.bit_order = 0; //MSB
	screen_spi_config.mode = 0;

	screen_spi_config.hw_nr = 1;
	screen_spi_config.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);
	screen_spi_config.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	screen_spi_config.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);
	screen_spi_config.cs_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_4);
	screen_spi_config.rs_pin = -1;//bshal_gpio_encode_pin(GPIOB, GPIO_PIN_1);

	bshal_spim_init(&screen_spi_config);

	static bshal_ucg_t m_ucg_spi;
	(void) m_ucg_spi.transport;

	m_ucg_spi.spim.instance = screen_spi_config;
	m_ucg_spi.spim.ncd_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_10);
	bshal_gpio_port_enable_clock(m_ucg_spi.spim.ncd_pin);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	ucg_SetUserPtr(&m_ucg, &m_ucg_spi);
	ucg_Init(&m_ucg, ucg_dev_ssd1331_18x96x64_univision, ucg_ext_ssd1331_18, ucg_com_bshal);

	ucg_SetFontMode(&m_ucg, UCG_FONT_MODE_TRANSPARENT);
	ucg_ClearScreen(&m_ucg);

}

void print(char *str, int line) {

	ucg_SetColor(&m_ucg, 0, 0xFF, 0xFF, 0xFF);
	//ucg_SetFont(&m_ucg, ucg_font_5x8_tf);
	ucg_SetFontMode(&m_ucg, UCG_FONT_MODE_TRANSPARENT);

	ucg_SetFont(&m_ucg, ucg_font_profont11_mf);
	ucg_SetFontPosTop(&m_ucg);
	ucg_DrawString(&m_ucg, 0, line * 12, 0, str);
}

void framebuffer_apply() {
}
void draw_background() {
	ucg_SetColor(&m_ucg, 0, 0xFF, 0x00, 0x00);
	ucg_DrawBox(&m_ucg, 25, 25, 25, 25);

}
void draw_plain_background() {
	ucg_ClearScreen(&m_ucg);
}

