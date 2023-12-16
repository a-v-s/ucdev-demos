#include <stdbool.h>
#include <string.h>
#include <stdfix.h>

#include "system.h"

#include "bshal_spim.h"
#include "bshal_delay.h"
#include "bshal_i2cm.h"
#include "bshal_uart.h"
#include "bshal_gpio.h"

#define APN "TM" // APN for Things Mobile, adjust if needed

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

void dtu_command_send(char *cmd) {
	static char buffer[64];
	if (!cmd)
		return;
	if (strlen(cmd) > 60)
		return;
	sprintf(buffer, "%s\r\n", cmd);
	test_uart_send(buffer, strlen(buffer));
}

void dtu_string_send(char *cmd) {
	if (!cmd)
		return;
	test_uart_send(cmd, strlen(cmd));
}

typedef struct {
	char command[64];
	char responses[5][64];
	char status_string[16];
	int status;
	int state;
	void (*cb)(void*); // cannot refer to itself yet
} dtu_command_t;

typedef void (*dtu_command_cb_f)(dtu_command_t*);
typedef void (*dtu_unsolicited_cb_f)(char*);
typedef struct {
	char command[64];
	dtu_unsolicited_cb_f cb;
} dtu_unsolicited_cb_registration;

#define dtu_unsolicited_cb_registration_count (8)
dtu_unsolicited_cb_registration dtu_unsolicited_cb_registrations[dtu_unsolicited_cb_registration_count];

typedef struct {
	dtu_command_cb_f cb;
	char command[64];
	bool rawstring;
} dtu_command_queue_entry_t;

void dtu_cgact01_cb(dtu_command_t *cmd);
void dtu_cgact11_cb(dtu_command_t *cmd);
void dtu_cgdcont_cb(dtu_command_t *cmd);
void dtu_cgactQ_cb(dtu_command_t *cmd);

static dtu_command_t dtu_command;
#define dtu_command_queue_length (8)
dtu_command_queue_entry_t dtu_command_queue[dtu_command_queue_length];
uint32_t dtu_command_queue_read_pos = 0;
uint32_t dtu_command_queue_write_pos = 0;

int dtu_command_enqueue(char *cmd, dtu_command_cb_f cb) {
	if ((dtu_command_queue_write_pos + 1) != dtu_command_queue_read_pos) {
		dtu_command_queue_write_pos++;
		dtu_command_queue_write_pos %= dtu_command_queue_length;
		dtu_command_queue[dtu_command_queue_write_pos].cb = cb;
		strncpy(dtu_command_queue[dtu_command_queue_write_pos].command, cmd,
				64);
		dtu_command_queue[dtu_command_queue_write_pos].rawstring = false;
//		printf("QPos %d, Queuing %s\n", dtu_command_queue_write_pos,
//				dtu_command_queue[dtu_command_queue_write_pos].command);
		return 0;
	} else {
		puts("Queue is full");
		return -1;
	}
}

int dtu_string_enqueue(char *cmd, dtu_command_cb_f cb) {
	if ((dtu_command_queue_write_pos + 1) != dtu_command_queue_read_pos) {
		dtu_command_queue_write_pos++;
		dtu_command_queue_write_pos %= dtu_command_queue_length;
		dtu_command_queue[dtu_command_queue_write_pos].cb = cb;
		strncpy(dtu_command_queue[dtu_command_queue_write_pos].command, cmd,
				64);
		dtu_command_queue[dtu_command_queue_write_pos].rawstring = true;
//		printf("QPos %d, Queuing %s\n", dtu_command_queue_write_pos,
//				dtu_command_queue[dtu_command_queue_write_pos].command);
		return 0;
	} else {
		puts("Queue is full");
		return -1;
	}
}

void dtu_command_queue_process(void) {
	if (dtu_command.state == 0) {
		while (dtu_command_queue_read_pos != dtu_command_queue_write_pos) {
			dtu_command_queue_read_pos++;
			dtu_command_queue_read_pos %= dtu_command_queue_length;
			if (dtu_command_queue[dtu_command_queue_read_pos].cb) {
				memset(&dtu_command, 0, sizeof(dtu_command));
				dtu_command.cb =
						dtu_command_queue[dtu_command_queue_read_pos].cb;
//				printf("QPos %d, Sending %s\n", dtu_command_queue_read_pos,
//						dtu_command_queue[dtu_command_queue_read_pos].command);
				dtu_command.state = 1+dtu_command_queue[dtu_command_queue_read_pos].rawstring;
				if (dtu_command_queue[dtu_command_queue_read_pos].rawstring)
					dtu_string_send(
							dtu_command_queue[dtu_command_queue_read_pos].command);
				else
					dtu_command_send(
							dtu_command_queue[dtu_command_queue_read_pos].command);

				return;
			}
		}
	}
}

void uart_dtu_cb(char *data, size_t size) {
	//printf("%s:%s\n", __FUNCTION__, data);
	if (!size)
		return;
	//
	//	if (!strlen(data))
	//		__BKPT(0);

	switch (dtu_command.state) {
	case 0:
		// Unsolicited callback?
		for (int i = 0; i < dtu_unsolicited_cb_registration_count; i++) {
			if (dtu_unsolicited_cb_registrations[i].cb
					&& !strncmp(data,
							dtu_unsolicited_cb_registrations[i].command,
							strlen(
									dtu_unsolicited_cb_registrations[i].command))) {
				dtu_unsolicited_cb_registrations[i].cb(data);
			}
		}

		break;
	case 1:
		if (size < (sizeof(dtu_command.command) - 1)) {
			strncpy(dtu_command.command, data, sizeof(dtu_command.command));
			dtu_command.state++;
		}
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		// "Err:" is a non-compliant response used by FS800E
		if (!strcmp(data, "OK") || strstr(data, "ERROR")
				|| strstr(data, "Err:")) {
			dtu_command.state = 7;
			//			printf("\t\t\tEarly Status is %s\n", data);
		[[fallthrough]];
	} else if (size
			&& size
					< (sizeof(dtu_command.responses[dtu_command.state - 2]) - 1)) {
		strncpy(dtu_command.responses[dtu_command.state - 2], data,
				sizeof(dtu_command.responses[dtu_command.state - 2]));

		//		printf("\t\t\tResponse is %s\n",
		//				dtu_command.responses[dtu_command.state]);
		dtu_command.state++;
		break;
	}
	if (!size)
		return;
case 7:
	if (size < (sizeof(dtu_command.status_string) - 1)) {
		if (data[0] == '\n') {
			strncpy(dtu_command.status_string, data + 1,
					sizeof(dtu_command.status_string));
		} else {
			strncpy(dtu_command.status_string, data,
					sizeof(dtu_command.status_string));
		}
	}
	// TODO:  There might be additional data
	// We need to handle that situation
	dtu_command.status = strcmp("OK", dtu_command.status_string);
	dtu_command.cb(&dtu_command);
	dtu_command.state = 0;

	break;
	}
}

static bshal_uart_async_at_t bshal_uart_dtu_async;
static bshal_uart_instance_t bshal_uart_instance;
void uart_init(void) {

	//  Configuration for the Async handler, to configure the synchronosation
	bshal_uart_dtu_async.command_callback = uart_dtu_cb; // Callback

	static uint8_t receive_buffer[128];
	bshal_uart_dtu_async.receive_buffer = receive_buffer;
	bshal_uart_dtu_async.receive_buffer_len = sizeof(receive_buffer);

	static uint8_t postprocess_buffer[256];
	bshal_uart_dtu_async.postprocess_buffer = postprocess_buffer;
	bshal_uart_dtu_async.postprocess_buffer_len = sizeof(postprocess_buffer);

	static uint8_t preprocess_buffer[256];
	bshal_uart_dtu_async.preprocess_buffer = preprocess_buffer;
	bshal_uart_dtu_async.preprocess_buffer_len = sizeof(preprocess_buffer);

	bshal_uart_instance.async = &bshal_uart_dtu_async; // Asign the async handler to the uart instance

	bshal_uart_instance.bps = 115200;// most modems default at 115200,8,N,1 or auto detect
//	bshal_uart_instance.bps = 38400; // Sinowell G590E defaults at 38400,N,1
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
	int stat;
	union {
		int lac; // 2G, 3G
		int tac; // 4G, 5G
	};
	int ci;
	int act;
} dtu_modem_registration_status_t;

void any_unsol_cb_test(char *cmd) {
	puts("any_unsol");
	puts(cmd);
}

void dtu_generic_cb(dtu_command_t *cmd) {
	puts(cmd->command);
	puts(cmd->responses[0]);
	puts(cmd->status_string);
}

void dtu_s_cb(dtu_command_t *cmd) {
	dtu_generic_cb(cmd);
	puts("Settings saves, the modem should reboot now");

}

void dtu_apnS_cb(dtu_command_t *cmd) {
	dtu_generic_cb(cmd);
	puts("APN set, saving");
	dtu_command_enqueue("AT+S", dtu_s_cb);
}

void dtu_apnQ_cb(dtu_command_t *cmd) {
	dtu_generic_cb(cmd);
	// "+APN:CMNET,,,0"
	char *current_ap = strtok(cmd->responses[0] + 5, ",");
	if (strcmp(APN, current_ap)) {
		puts("Current APN incorrect");
		dtu_command_enqueue("AT+APN="APN",,,0", dtu_apnS_cb);
	} else {
		puts("Current APN correct");
		dtu_command_enqueue("AT+CIP?", dtu_generic_cb);
	}
}

void setup_cb(dtu_command_t *cmd) {
	dtu_command_enqueue("AT+APN?", dtu_apnQ_cb);
//	
}

void info2_cb(dtu_command_t *cmd) {
	dtu_generic_cb(cmd);
	dtu_command_enqueue("AT+CSQ?", dtu_generic_cb);
	dtu_command_enqueue("AT+CREG?", dtu_generic_cb);
	dtu_command_enqueue("AT+LBS?", dtu_generic_cb);
	dtu_command_enqueue("AT+CCLK?", dtu_generic_cb);
	dtu_command_enqueue("AT+RUNST?", dtu_generic_cb);
	dtu_command_enqueue("AT+CIP?", setup_cb);
}

void command_mode_cb(dtu_command_t *cmd) {
	dtu_command_enqueue("AT+VER?", dtu_generic_cb);
	dtu_command_enqueue("AT+BUILD?", dtu_generic_cb);
	dtu_command_enqueue("AT+SN?", dtu_generic_cb);
	dtu_command_enqueue("AT+IMEI?", dtu_generic_cb);
	dtu_command_enqueue("AT+ICCID?", info2_cb);
}

int main() {

	SystemClock_Config();
	SystemCoreClockUpdate();

	bshal_delay_init();
	bshal_delay_us(10);

	puts("DTU test");
	uart_init();
	bshal_delay_ms(1000);

	int cnt = 0;

	dtu_unsolicited_cb_registrations[0].cb = any_unsol_cb_test;
	dtu_unsolicited_cb_registrations[0].command[0] = '+';

	dtu_string_enqueue("+++", command_mode_cb);

	while (1) {
		bshal_uart_at_recv_process(&bshal_uart_dtu_async);
		dtu_command_queue_process();
	}

}
