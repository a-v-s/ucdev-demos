#include "system.h"
#include "ucg.h"
#include "ucg_spi.h"
#include "bshal_spim.h"
#include "bshal_gpio.h"
#include "stm32/bshal_gpio_stm32.h"

static ucg_t m_ucg;


#if defined __ARM_EABI__
void SysTick_Handler(void) {
	HAL_IncTick();
}
#endif


void display_init(void) {
	bshal_spim_instance_t screen_spi_config = {};
	screen_spi_config.frequency = 1000000;
	screen_spi_config.bit_order = 0; //MSB
	screen_spi_config.mode = 0;

	screen_spi_config.hw_nr = 1;
	screen_spi_config.sck_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_5);
	screen_spi_config.miso_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_6);
	screen_spi_config.mosi_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_7);
	screen_spi_config.cs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_1);
	screen_spi_config.rs_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_10);

	bshal_spim_init(&screen_spi_config);

	static bshal_ucg_t m_ucg_spi;
	(void) m_ucg_spi.transport;

	m_ucg_spi.spim.instance = screen_spi_config;
	m_ucg_spi.spim.ncd_pin = bshal_gpio_encode_pin(GPIOB, GPIO_PIN_11);
	bshal_gpio_port_enable_clock(m_ucg_spi.spim.ncd_pin);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);



	ucg_SetUserPtr(&m_ucg, &m_ucg_spi);

//	ucg_Init(&m_ucg, ucg_dev_st7735_18x128x160, ucg_ext_st7735_18, ucg_com_bshal);
	ucg_Init(&m_ucg, ucg_dev_ili9341_18x240x320, ucg_ext_ili9341_18, ucg_com_bshal);

	//ucg_Init(&m_ucg, ucg_dev_ssd1331_18x96x64_univision, ucg_ext_ssd1331_18, ucg_com_bshal);
	// ucg_InitBuffer(&m_ucg, ucg_dev_ssd1331_18x96x64_univision, ucg_ext_ssd1331_18, ucg_com_bshal);

	if (0) { // ssd1331
		// "Set master current attenuation factor"
		// Default value is 0x0F
		// Setting this to a lower value such as 0x04 solves the flicker
		uint8_t cmd[] = { 0x87, 0x02 };

		ucg_com_SetCDLineStatus(&m_ucg, 0);
		ucg_com_SetCSLineStatus(&m_ucg, 0);
		ucg_com_SendString(&m_ucg, sizeof(cmd), cmd);
		ucg_com_SetCSLineStatus(&m_ucg, 1);

	}

	//ucg_SetRotate270(&m_ucg);
	//ucg_SetRotate90(&m_ucg);
	ucg_SetFontMode(&m_ucg, UCG_FONT_MODE_TRANSPARENT);
	ucg_SetRotate90(&m_ucg);
	ucg_ClearScreen(&m_ucg);

}

void SystemClock_Config(void) {
	ClockSetup_HSE8_SYS72();
}

void print(char *str, int line) {

	ucg_SetColor(&m_ucg, 0, 0xFF, 0xFF, 0xFF);
	//ucg_SetFont(&m_ucg, ucg_font_5x8_tf);
	ucg_SetFontMode(&m_ucg, UCG_FONT_MODE_TRANSPARENT);
	ucg_SetFontPosTop(&m_ucg);

	//ucg_SetFont(&m_ucg, ucg_font_profont11_mf); // 6x10
	//ucg_DrawString(&m_ucg, 0, line * 12, 0, str);

	ucg_SetFont(&m_ucg, ucg_font_amstrad_cpc_8f); // 8x8
	ucg_DrawString(&m_ucg, 0, line * (m_ucg.font_info.max_char_height+1), 0, str);

	//ucg_SetFont(&m_ucg, ucg_font_profont15_8f); // 8x16
	//ucg_DrawString(&m_ucg, 0, 2+line * m_ucg.font_info.max_char_height, 0, str);

}

void next_draw() {
	static int colour = 0;
	colour += 4;
	int r0, g0, b0;
	int r1, g1, b1;
	int r2, g2, b2;
	int r3, g3, b3;
	int c = (colour >> 8) % 3;
	int c2 = colour & 0xff;
	switch (c) {
	case 0:
		b0 = c2;
		r0 = 0xFF - c2;
		g0 = 0;

		r1 = c2;
		g1 = 0xFF - c2;
		b1 = 0;

		g2 = c2;
		b2 = 0xFF - c2;
		r2 = 0;

		b3 = c2;
		r3 = 0xFF - c2;
		g3 = 0;

		break;

	case 1:
		g0 = c2;
		b0 = 0xFF - c2;
		r0 = 0;

		b1 = c2;
		r1 = 0xFF - c2;
		g1 = 0;

		r2 = c2;
		g2 = 0xFF - c2;
		b2 = 0;

		g3 = c2;
		b3 = 0xFF - c2;
		r3 = 0;
		break;

	case 2:
		r0 = c2;
		g0 = 0xFF - c2;
		b0 = 0;

		g1 = c2;
		b1 = 0xFF - c2;
		r1 = 0;

		b2 = c2;
		r2 = 0xFF - c2;
		g2 = 0;

		r3 = c2;
		g3 = 0xFF - c2;
		b3 = 0;
		break;
	}

	ucg_SetColor(&m_ucg, 0, r0, g0, b0);
	ucg_SetColor(&m_ucg, 1, r1, g1, b1);
	ucg_SetColor(&m_ucg, 2, r2, g2, b2);
	ucg_SetColor(&m_ucg, 3, r3, g3, b3);

	ucg_DrawGradientBox(&m_ucg, 0, 0, m_ucg.dimension.w, m_ucg.dimension.h);
	//ucg_DrawBox(&m_ucg, 0, 0, m_ucg.dimension.w, m_ucg.dimension.h);
}

int main() {
	SystemClock_Config();
	SystemCoreClockUpdate();

	HAL_Init();

	bshal_delay_init();
	bshal_delay_us(10);
	display_init();

	print("Stroomtijd:",0);
	print("L1 230.1 V  11 A   " , 1);
	print("Verbruik:  12.345 KW" , 2);
	print("Levering:  12.345 KW" , 3);

	print("L1 230.1 V  11 A   " , 5);
	print("Verbruik:  12.345 KW" , 6);
	print("Levering:  12.345 KW" , 7);

	print("L1 230.1 V  11 A   " , 9);
	print("Verbruik:  12.345 KW" , 10);
	print("Levering:  12.345 KW" , 11);

	print("Dagstroom",13);
	print("Verbruik:  12.345 KWh" , 14);
	print("Levering:  12.345 KWh" , 15);

	print("Nachtstroom",17);
	print("Verbruik:  12.345 KWh" , 18);
	print("Levering:  12.345 KWh" , 19);

	print("Gastijd",21);
	print("Gas: 1.290 m3",22);




	while(1){}

}
