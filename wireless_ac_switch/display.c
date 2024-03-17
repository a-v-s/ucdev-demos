#include "system.h"

#include "u8x8_i2c.h"
#include "u8g2.h"
#include "bshal_i2cm.h"

static u8g2_t m_u8g2;
extern bshal_i2cm_instance_t *gp_i2c;
static char m_key;

void display_init(void) {
	u8g2_SetUserPtr(&m_u8g2, gp_i2c);
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&m_u8g2, U8G2_R0,
			bshal_u8x8_byte_i2c, bshal_u8x8_gpio_and_delay);
	u8g2_InitDisplay(&m_u8g2);
	u8g2_SetPowerSave(&m_u8g2, 0);
	u8g2_ClearBuffer(&m_u8g2);
}

void display_print_upper(char *str) {
	//u8g2_SetFont(&m_u8g2, u8g2_font_unifont_t_symbols); //square, with symbols like ⏴ ⏵ ⏶ ⏷
	u8g2_SetFont(&m_u8g2, u8g2_font_inr16_mr);
	u8g2_DrawUTF8(&m_u8g2, 0, 16, str);
}

void display_print_large(char *str) {
	u8g2_SetFont(&m_u8g2, u8g2_font_inr33_mr);
	u8g2_DrawUTF8(&m_u8g2, 0, 56, str);
}

void display_apply() {
	u8g2_UpdateDisplay(&m_u8g2);
}

void display_clear() {
	u8g2_ClearBuffer(&m_u8g2);
}

void display_process(void) {
	if (m_key) {
		char buff[5] = "    ";
		buff [2] = m_key;
		m_key = 0;
		display_clear();
		display_print_large(buff);
		display_apply();
	}
}

void display_set_key(char key) {
	m_key = key;
}
