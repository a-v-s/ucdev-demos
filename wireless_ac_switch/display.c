#include "system.h"

#include "u8x8_i2c.h"
#include "u8g2.h"
#include "bshal_i2cm.h"

#include <time.h>

#include "bsradio.h"
extern bsradio_instance_t *gp_radio;

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
	u8g2_SetFont(&m_u8g2, u8g2_font_inr16_mf);
	u8g2_DrawUTF8(&m_u8g2, 8, 16, str);
}

void display_print_middle(char *str) {
	//u8g2_SetFont(&m_u8g2, u8g2_font_inr33_mr);
	u8g2_SetFont(&m_u8g2, u8g2_font_inr16_mf);
	u8g2_DrawUTF8(&m_u8g2, 8, 40, str);
}

void display_print_lower(char *str) {
	//u8g2_SetFont(&m_u8g2, u8g2_font_inr33_mr);
	u8g2_SetFont(&m_u8g2, u8g2_font_inr16_mf);
	u8g2_DrawUTF8(&m_u8g2, 8, 64, str);
}

void display_print_large(char *str) {
	u8g2_SetFont(&m_u8g2, u8g2_font_inr33_mr);
//	u8g2_SetFont(&m_u8g2, u8g2_font_inr16_mr);
	u8g2_DrawUTF8(&m_u8g2, 0, 56, str);
}

void display_apply() {
	u8g2_UpdateDisplay(&m_u8g2);
}

void display_clear() {
	u8g2_ClearBuffer(&m_u8g2);
}

void display_process(void) {
	char buff[16];
	static time_t prev_time = 0;
	if (m_key) {
		memset(buff, 0x20,8);
		buff [2] = m_key;
		m_key = 0;

		if (m_key == '*') {
			// Toggle Light
		}

		if (m_key == '#') {
			// Display Sensors
		}
		display_print_large(buff);
	} else
	if (prev_time == time(NULL)) return;
	prev_time = time(NULL);
	display_clear();
	struct tm * timeinfo = localtime (&prev_time);

	sprintf(buff, "%02d:%02d:%02d",
			timeinfo->tm_hour,
			timeinfo->tm_min,
			timeinfo->tm_sec);
	display_print_upper(buff);

 {
		extern uint16_t bh1750_illuminance_lux;
		extern int16_t lm75b_temperature_centi_celcius;

		switch (timeinfo->tm_sec/10) {
		case 0:
		sprintf(buff,"addr %3d", gp_radio->rfconfig.node_id);
		display_print_middle(buff);
		display_print_lower(buff);
		break;
		case 1:
		case 3:
		case 5:
			sprintf(buff,"%5d lx", bh1750_illuminance_lux);
			display_print_middle(buff);
			sprintf(buff,"%3d.02°C", lm75b_temperature_centi_celcius/100, lm75b_temperature_centi_celcius%100);
			display_print_lower(buff);
			break;
		default:
			sprintf(buff, "%02d-%02d-%02d",
					timeinfo->tm_mday,
					timeinfo->tm_mon+1,
					timeinfo->tm_year%100);
			display_print_middle(buff);
			break;

		}
	}




	display_apply();
}

void display_set_key(char key) {
	m_key = key;
}
