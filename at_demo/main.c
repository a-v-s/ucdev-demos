#include <stdbool.h>
#include <string.h>
#include <stdfix.h>

#include "system.h"

#include "bshal_spim.h"
#include "bshal_delay.h"
#include "bshal_i2cm.h"
#include "bshal_uart.h"
#include "bshal_gpio.h"

//#include "i2c.h"

void SysTick_Handler(void) {
	HAL_IncTick();
}

void SystemClock_Config(void) {

#ifdef STM32F1
	ClockSetup_HSE8_SYS72();
#endif

#ifdef STM32F4
	SystemClock_HSE25_SYS84();
#endif
}

void test_at_sned(char *cmd) {
	static char buffer[64];
	if (!cmd)
		return;
	if (strlen(cmd) > 60)
		return;
	sprintf(buffer, "%s\r\n", cmd);
	test_uart_send(buffer, strlen(buffer));
}

typedef struct {
	char command[64];
	char responses[5][64];
	char status_string[8];
	int status;
	int state;
	void (*cb)(void*);
} at_command_t;

static at_command_t at_command;

void uart_at_cb(char *data, size_t size) {
	if (!size)
		return;
	if (data[0] == '/n')
		__BKPT(0);

	switch (at_command.state) {
	case 0:
		if (size < (sizeof(at_command.command) - 1)) {
			strncpy(at_command.command, data, sizeof(at_command.command));
			at_command.state++;
		}
//		printf("\t\t\tCommand is %s\n", at_command.command);
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		if (!strcmp(data, "OK") || !strcmp(data, "ERROR")) {
			at_command.state = 6;
//			printf("\t\t\tEarly Status is %s\n", data);
		[[fallthrough]];
	} else if (size
			&& size
					< (sizeof(at_command.responses[at_command.state - 1]) - 1)) {
		strncpy(at_command.responses[at_command.state - 1], data,
				sizeof(at_command.responses[at_command.state - 1]));

//		printf("\t\t\tResponse is %s\n",
//				at_command.responses[at_command.state]);
		at_command.state++;
		break;
	}
	if (!size)
		break;
case 6:
	if (size < (sizeof(at_command.status_string) - 1)) {
		if (data[0] == '\n') {
			strncpy(at_command.status_string, data + 1,
					sizeof(at_command.status_string));
		} else {
			strncpy(at_command.status_string, data,
					sizeof(at_command.status_string));
		}
	}
	// TODO:  There might be additional data
	// We need to handle that situation
	at_command.status = strcmp("OK", at_command.status_string);
	at_command.cb(&at_command);

	at_command.state++;
	break;
	}
}

static bshal_uart_async_t bshal_uart_async;
static bshal_uart_instance_t bshal_uart_instance;
void uart_init(void) {

	//  Configuration for the Async handler, to configure the synchronosation
	bshal_uart_async.callback = uart_at_cb; // Callback

	static uint8_t receive_buffer[128];
	bshal_uart_async.receive_buffer = receive_buffer;
	bshal_uart_async.receive_buffer_len = sizeof(receive_buffer);

	static uint8_t postprocess_buffer[256];
	bshal_uart_async.postprocess_buffer = postprocess_buffer;
	bshal_uart_async.postprocess_buffer_len = sizeof(postprocess_buffer);

	static uint8_t preprocess_buffer[256];
	bshal_uart_async.preprocess_buffer = preprocess_buffer;
	bshal_uart_async.preprocess_buffer_len = sizeof(preprocess_buffer);

	bshal_uart_async.null_terminated_string = true; // Terminate the resulting string

	bshal_uart_async.sync_begin_data = "\n";
	bshal_uart_async.sync_begin_len = 1;    //
	bshal_uart_async.sync_begin_include = false; // Include the $ in the result

	bshal_uart_async.sync_end = "\r";     // NMEA sentences end with \r\n
	bshal_uart_async.sync_end_len = 1;
	bshal_uart_async.sync_end_include = false; // Do include the \r\n in the result

	bshal_uart_async.max_data_len = 64; // The maximum length of a line

	bshal_uart_instance.async = &bshal_uart_async; // Asign the async handler to the uart instance

	// 115200,8,N,1
	bshal_uart_instance.bps = 115200;
	//	bshal_uart_instance.bps = 38400; // sinowell
	bshal_uart_instance.data_bits = 8;
	bshal_uart_instance.parity = bshal_uart_parity_none;
	bshal_uart_instance.stop_bits = 1;

	bshal_uart_instance.fc = bshal_uart_flow_control_none;

	bshal_uart_instance.hw_nr = 2;  // UASRT 2
	bshal_uart_instance.cts_pin = -1; // No flow control
	bshal_uart_instance.rts_pin = -1; // No flow control
	bshal_uart_instance.rxd_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_3); // PA3
	bshal_uart_instance.txd_pin = bshal_gpio_encode_pin(GPIOA, GPIO_PIN_2); // PA2

	bshal_stm32_uart_init(&bshal_uart_instance);

}
typedef struct {
	char Manufacturer[32];
	char Model[32];
	char Revision[32];
	char Serial[32];
	char Info[5][32];
} at_modem_info_t;

at_modem_info_t at_modem_info;

void ati_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("ATI failed");
	} else {
		puts("ATI succeeded");
		for (int i = 0; i < 5; i++) {
			strncpy(at_modem_info.Info[i], cmd->responses[i],
					sizeof(at_modem_info.Info[i]));
		}
	}
}

void at_cgsn_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGSN failed");
	} else {
		puts("AT+CGSN succeeded");
		strncpy(at_modem_info.Serial, cmd->responses[0],
				sizeof(at_modem_info.Serial));
	}
}

void at_cgmr_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGMR failed");
	} else {
		puts("AT+CGMR succeeded");
		strncpy(at_modem_info.Revision, cmd->responses[0],
				sizeof(at_modem_info.Revision));
	}
}

void at_cgmm_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGMM failed");
	} else {
		puts("AT+CGMM succeeded");
		strncpy(at_modem_info.Model, cmd->responses[0],
				sizeof(at_modem_info.Model));
	}
}

void at_cgmi_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGMI failed");
	} else {
		puts("AT+CGMI succeeded");
		strncpy(at_modem_info.Manufacturer, cmd->responses[0],
				sizeof(at_modem_info.Manufacturer));
	}
}

char *at_commands[] = { //
		"ATZ", //
				"ATI", //

				"AT+GMI", // Request Manufacturer Identification
				"AT+GMM", // Request Model Identification
				"AT+GMR", // Request Revision Identification
				"AT+GSN", // Request Product Serial Number Identification

				"AT+CGMI", // Request Manufacturer Identification
				"AT+CGMM", // Request Model Identification
				"AT+CGMR", // Request Revision Identification
				"AT+CGSN", // Request Product Serial Number Identification

				"AT+GCAP", // Request Complete Capabilities List
				"AT+CPIN?", //
				"AT+CIMI", //
				"AT+CNUM", //
				"AT+COPS?", //
				"AT+CREG=2", //
				"AT+CREG?", //
				"AT+CSQ", // Request Signal Quality
				"AT+BLAAT",
				//"AT+CLAC",
				NULL, //
		};

int main() {

	SystemClock_Config();
	SystemCoreClockUpdate();

	//	HAL_Init();

	bshal_delay_init();
	bshal_delay_us(10);

	//	#gp_i2c = i2c_init();
	puts("AT test");
	uart_init();
	bshal_delay_ms(1000);

	int cnt = 0;

	memset(&at_command, 0, sizeof(at_command));
	at_command.cb = at_cgmi_cb;
	test_at_sned("AT+CGMI");


	while (at_command.state < 7) {
		bshal_uart_recv_process(&bshal_uart_async);
	}


	memset(&at_command, 0, sizeof(at_command));
	at_command.cb = at_cgmm_cb;
	test_at_sned("AT+CGMM");
	while (at_command.state < 7) {
		bshal_uart_recv_process(&bshal_uart_async);
	}


	memset(&at_command, 0, sizeof(at_command));
	at_command.cb = at_cgmr_cb;
	test_at_sned("AT+CGMR");
	while (at_command.state < 7) {
		bshal_uart_recv_process(&bshal_uart_async);
	}


	memset(&at_command, 0, sizeof(at_command));
	at_command.cb = at_cgsn_cb;
	test_at_sned("AT+CGSN");
	while (at_command.state < 7) {
		bshal_uart_recv_process(&bshal_uart_async);
	}


	memset(&at_command, 0, sizeof(at_command));
	at_command.cb = ati_cb;
	test_at_sned("ATI");
	while (at_command.state < 7) {
		bshal_uart_recv_process(&bshal_uart_async);
	}


	printf("Manufacturer: %s\n", at_modem_info.Manufacturer);
	printf("Model       : %s\n", at_modem_info.Model);
	printf("Revision    : %s\n", at_modem_info.Revision);
	printf("Serial      : %s\n", at_modem_info.Serial);
	printf("Additional Info:\n");
	for (int i = 0; i < 5; i++) {
		if (!(strlen(at_modem_info.Info[i])))
			break;
		puts(at_modem_info.Info[i]);
	}

	// Neoway M590 patch
	if (!strcmp(" Undefined", at_modem_info.Manufacturer)
	&& 		!strcmp(" Undefined", at_modem_info.Revision)
	&& 		!strcmp("M590", at_modem_info.Model)
	&& 		!strcmp(at_modem_info.Info[1], at_modem_info.Model)
	) {
		strcpy(at_modem_info.Manufacturer, at_modem_info.Info[0]);
		strcpy(at_modem_info.Revision, at_modem_info.Info[2]);

		puts("M590 patched data:");
		printf("Manufacturer: %s\n", at_modem_info.Manufacturer);
		printf("Model       : %s\n", at_modem_info.Model);
		printf("Revision    : %s\n", at_modem_info.Revision);
		printf("Serial      : %s\n", at_modem_info.Serial);
	}

	if (!strcmp(at_modem_info.Manufacturer,"NEOWAY")) {

	}



	while (1)
		;

	while (1) {
		char *cmd = at_commands[cnt++];
		if (!cmd) {
			cnt = 0;
			continue;
		}
		printf("Sending %s\n", cmd);
		memset(&at_command, 0, sizeof(at_command));
		test_at_sned(cmd);
		while (at_command.state < 7) {
			bshal_uart_recv_process(&bshal_uart_async);
		}
		if (!at_command.status) {
			puts("AT command succeeded");
		} else {
			puts("AT command failed");
		}

		printf("Command:    %s\n", at_command.command);
		for (int i = 0; i < 5; i++) {
			if (strlen(at_command.responses[i]))
				printf("Response %d  %s\n", i + 1, at_command.responses[i]);
		}
		printf("Status :    %s\n", at_command.status_string);

//		bshal_delay_ms(1000);

	}
}
