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
} at_command_t;

static at_command_t at_command;

void uart_at_cb(char *data, size_t size) {
	if(!size) return;
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
			&& size < (sizeof(at_command.responses[at_command.state-1]) - 1)) {
		strncpy(at_command.responses[at_command.state-1], data,
				sizeof(at_command.responses[at_command.state-1]));

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
//	printf("\t\t\tStatus is %s\n", at_command.status_string);
	at_command.status = strcmp("OK", at_command.status_string);

//	if (!at_command.status) {
//		puts("AT command succeeded");
//		puts(at_command.command);
//		for (int i = 0; i < 5; i++) {
//			if (strlen(at_command.responses[i]))
//				puts(at_command.responses[i]);
//		}
//		puts(at_command.status_string);
//	} else {
//		puts("AT command failed");
//		puts(at_command.command);
//		puts(at_command.status_string);
//	}
	at_command.state++;
	break;
	}
}

void uart_init(void) {
	//static uint8_t receive_buffer[512];
	static uint8_t receive_buffer[1024];
	static bshal_uart_async_t bshal_uart_async;
	static bshal_uart_instance_t bshal_uart_instance;

	//  Configuration for the Async handler, to configure the synchronosation
	bshal_uart_async.callback = uart_at_cb; // Callback

	bshal_uart_async.receive_buffer = receive_buffer;
	bshal_uart_async.receive_buffer_len = sizeof(receive_buffer) - 128;
	bshal_uart_async.process_buffer = receive_buffer + sizeof(receive_buffer)
			- 128;
	bshal_uart_async.process_buffer_len = 128;

	bshal_uart_async.null_terminated_string = true; // Terminate the resulting string

	bshal_uart_async.sync_begin_data = "\n";
	bshal_uart_async.sync_begin_len = 1;    //
	bshal_uart_async.sync_begin_include = false; // Include the $ in the result

	bshal_uart_async.sync_end = "\r";     // NMEA sentences end with \r\n
	bshal_uart_async.sync_end_len = 1;
	bshal_uart_async.sync_end_include = false; // Do include the \r\n in the result

	bshal_uart_async.max_data_len = 83; // The maximum length of a line

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

char *at_commands[] = { //
		"ATZ", //
				"ATI", //
				"AT+CGMI", //
				"AT+CGMM", //
				"AT+CGMR", //
				"AT+CGSN", //
				"AT+CPIN?", //
				"AT+CIMI", //
				"AT+CNUM", //
				"AT+COPS?", //
				"AT+CREG=2", //
				"AT+CREG?", //
				"AT+CSQ", //
				"AT+BLAAT",
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
			putchar('.');
			fflush(0);
			bshal_delay_ms(100);
		}
		if (!at_command.status) {
			puts("AT command succeeded");
		} else {
			puts("AT command failed");
		}

		printf("Command:    %s\n", at_command.command);
		for (int i = 0; i < 5; i++) {
			if (strlen(at_command.responses[i]))
				printf("Response %d  %s\n", i+1,at_command.responses[i]);
		}
		printf("Status :    %s\n", at_command.status_string);



//		bshal_delay_ms(1000);

	}
}
