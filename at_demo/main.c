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

void at_command_send(char *cmd) {
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
	char status_string[16];
	int status;
	int state;
	void (*cb)(void*); // cannot refer to itself yet
} at_command_t;

typedef void (*at_command_cb_f)(at_command_t*);
typedef void (*at_unsolicited_cb_f)(char*);
typedef struct {
	char command[64];
	at_unsolicited_cb_f cb;
} at_unsolicited_cb_registration;

#define at_unsolicited_cb_registration_count (8)
at_unsolicited_cb_registration at_unsolicited_cb_registrations[at_unsolicited_cb_registration_count];

typedef struct {
	at_command_cb_f cb;
	char command[64];
} at_command_queue_entry_t;

void at_cgact10_cb(at_command_t *cmd);
void at_cgact11_cb(at_command_t *cmd);
void at_cgdcont_cb(at_command_t *cmd);
void at_cgactQ_cb(at_command_t *cmd);

static at_command_t at_command;
#define at_command_queue_length (8)
at_command_queue_entry_t at_command_queue[at_command_queue_length];
uint32_t at_command_queue_read_pos = 0;
uint32_t at_command_queue_write_pos = 0;

int at_command_enqueue(char *cmd, at_command_cb_f cb) {
	if ((at_command_queue_write_pos + 1) != at_command_queue_read_pos) {
		at_command_queue_write_pos++;
		at_command_queue_write_pos %= at_command_queue_length;
		at_command_queue[at_command_queue_write_pos].cb = cb;
		strncpy(at_command_queue[at_command_queue_write_pos].command, cmd, 64);
		printf("QPos %d, Queuing %s\n", at_command_queue_write_pos,
				at_command_queue[at_command_queue_write_pos].command);
		return 0;
	} else {
		puts("Queue is full");
		return -1;
	}
}

void at_command_queue_process(void) {
	if (at_command.state == 0) {
		while (at_command_queue_read_pos != at_command_queue_write_pos) {
			at_command_queue_read_pos++;
			at_command_queue_read_pos %= at_command_queue_length;
			if (at_command_queue[at_command_queue_read_pos].cb) {
				memset(&at_command, 0, sizeof(at_command));
				at_command.cb = at_command_queue[at_command_queue_read_pos].cb;
				at_command.state = 1;
				printf("QPos %d, Sending %s\n", at_command_queue_read_pos,
						at_command_queue[at_command_queue_read_pos].command);
				at_command_send(
						at_command_queue[at_command_queue_read_pos].command);
				return;
			}
		}
	}
}

void uart_at_cb(char *data, size_t size) {
//	printf("%s:%s\n", __FUNCTION__, data);
	if (!size)
		return;
	//
	//	if (!strlen(data))
	//		__BKPT(0);

	switch (at_command.state) {
	case 0:
		// Unsolicited callback?
		for (int i = 0; i < at_unsolicited_cb_registration_count; i++) {
			if (at_unsolicited_cb_registrations[i].cb
					&& !strncmp(data,
							at_unsolicited_cb_registrations[i].command,
							strlen(
									at_unsolicited_cb_registrations[i].command))) {
				at_unsolicited_cb_registrations[i].cb(data);
			}
		}

		break;
	case 1:
		if (size < (sizeof(at_command.command) - 1)) {
			strncpy(at_command.command, data, sizeof(at_command.command));
			at_command.state++;
		}
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		// In order to handle CMEE=1, we look if "ERROR" is included in the string
		//if (!strcmp(data, "OK") || !strcmp(data, "ERROR")) {
		if (!strcmp(data, "OK") || strstr(data, "ERROR")) {
			at_command.state = 7;
			//			printf("\t\t\tEarly Status is %s\n", data);
		[[fallthrough]];
	} else if (size
			&& size
					< (sizeof(at_command.responses[at_command.state - 2]) - 1)) {
		strncpy(at_command.responses[at_command.state - 2], data,
				sizeof(at_command.responses[at_command.state - 2]));

		//		printf("\t\t\tResponse is %s\n",
		//				at_command.responses[at_command.state]);
		at_command.state++;
		break;
	}
	if (!size)
		return;
case 7:
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
	at_command.state = 0;

	break;
	}
}

static bshal_uart_async_at_t bshal_uart_at_async;
static bshal_uart_instance_t bshal_uart_instance;
void uart_init(void) {

	//  Configuration for the Async handler, to configure the synchronosation
	bshal_uart_at_async.command_callback = uart_at_cb; // Callback

	static uint8_t receive_buffer[128];
	bshal_uart_at_async.receive_buffer = receive_buffer;
	bshal_uart_at_async.receive_buffer_len = sizeof(receive_buffer);

	static uint8_t postprocess_buffer[256];
	bshal_uart_at_async.postprocess_buffer = postprocess_buffer;
	bshal_uart_at_async.postprocess_buffer_len = sizeof(postprocess_buffer);

	static uint8_t preprocess_buffer[256];
	bshal_uart_at_async.preprocess_buffer = preprocess_buffer;
	bshal_uart_at_async.preprocess_buffer_len = sizeof(preprocess_buffer);

	bshal_uart_instance.async = &bshal_uart_at_async; // Asign the async handler to the uart instance


	bshal_uart_instance.bps = 115200;	// most modems default at 115200,8,N,1 or auto detect
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

char* at_modem_stat2str( stat) {
	switch (stat) {
	case 0:
		return "not registered, MT is not currently searching an operator to register to";
	case 1:
		return "registered, home network";
	case 2:
		return "not registered, but MT is currently trying to attach or searching an operator to register to";
	case 3:
		return "registration denied";
	case 4:
		return "unknown (e.g. out of coverage)";
	case 5:
		return "registered, roaming";
	case 6:
		return "registered for \"SMS only\", home network";
	case 7:
		return "registered for \"SMS only\", roaming";
	case 8:
		return "registered for emergency services only";
	case 9:
		return "registered for \"CSFB not preferred\", home network";
	case 10:
		return "registered for \"CSFB not preferred\", roaming";
	case 11:
		return "attached for access to RLOS";
	default:
		return "Unknown";
	}
}

char* at_modem_act2str( act) {
	switch (act) {
	case 0:
		return "GSM";
	case 1:
		return "GSM Compact";
	case 2:
		return "UTRAN";
	case 3:
		return "GSM w/EGPRS";
	case 4:
		return "UTRAN w/HSDPA";
	case 5:
		return "UTRAN w/HSUPA";
	case 6:
		return "UTRAN w/HSDPA";
	case 7:
		return "E-UTRAN";
	case 8:
		return "EC-GSM-IoT (A/Gb mode)";
	case 9:
		return "E-UTRAN (NB-S1 mode)";
	case 10:
		return "E-UTRA connected to a 5GCN";
	case 11:
		return "NR connected to a 5GCN";
	case 12:
		return "NG-RAN";
	case 13:
		return "E-UTRA-NR dual connectivity";
	default:
		return "Unknown";
	}
}

typedef struct {
	int stat;
	union {
		int lac; // 2G, 3G
		int tac; // 4G, 5G
	};
	int ci;
	int act;
} at_modem_registration_status_t;

typedef struct {
	char Manufacturer[32];
	char Model[32];
	char Revision[32];
	char Serial[32];
	char Info[5][32];

	at_modem_registration_status_t creg;
	at_modem_registration_status_t cgreg;
	at_modem_registration_status_t cereg;
	at_modem_registration_status_t c5greg;

} at_modem_info_t;

at_modem_info_t at_modem_info;

void any_unsol_cb_test(char *cmd) {
	puts("any_unsol");
	puts(cmd);
}

void at_generic_cb(at_command_t *cmd) {
	puts(cmd->command);
	puts(cmd->responses[0]);
	puts(cmd->status_string);
}

void at_xiic_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	at_command_enqueue("AT+XIIC?", at_generic_cb);
}

void at_cgdcontQ_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	if (cmd->status) {
		// error
	} else {

		// SimCom SIM800L returns
		// +CGDCONT: 1,"IP","TM","10.6.64.218",0,0

		// SimCom A7670C returns
		// +CGDCONT: 1,"IP","tm.mnc050.mcc234.gprs","10.5.170.50",0,0,,,,

		char *str_Pcgdcont = strtok(cmd->responses[0], ": ");
		char *str_cid = strtok(NULL, ", ");
		char *str_pdp_type = strtok(NULL, ", ");
		char *str_apn = strtok(NULL, ", ");
		char *str_pdp_addr = strtok(NULL, ", ");
		char *str_d_comp = strtok(NULL, ", ");
		char *str_h_comp = strtok(NULL, ", ");

		if (str_apn) {
			printf("Current APN is %s\n", str_apn);
			if (((strlen(str_apn) == (strlen(APN) + 2)
					|| (strlen(str_apn) > (strlen(APN) + 2)
							&& ('.' == *(str_apn + strlen(APN) + 1))))
					&& (!strncasecmp(APN, str_apn + 1, strlen(APN))))
			) {
				puts("Current APN is correct");
			} else {

				puts("Current APN is incorrect, detaching");
				at_command_enqueue("AT+CGACT=1,0", at_cgact10_cb);
			}

		} else {
			puts("Could not determine current PDP context, detaching");
			at_command_enqueue("AT+CGACT=1,0", at_cgact10_cb);
		}
	}

}

void at_cgact10_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	// +CGPADDR
	at_command_enqueue("AT+CGACT?", at_cgactQ_cb);
}

void at_cgact11_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	if (cmd->status) {
		/// error
	} else {
		puts("PDP context activated");
		at_command_enqueue("AT+CGDCONT?", at_generic_cb);
	}
}

void at_cgdcont_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	if (cmd->status) {
	} else {
		at_command_enqueue("AT+CGACT=1,1", at_cgact11_cb);
	}
}

void at_cgactQ_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	if (cmd->status) {
		// error
	} else {
		char *str_Pcgact = strtok(cmd->responses[0], ":");
		char *str_cid = strtok(NULL, ", ");
		char *str_state = strtok(NULL, ", ");
		if (str_state) {
			if (*str_state == '1') {
				puts("PDP Context already active");
				at_command_enqueue("AT+CGDCONT?", at_cgdcontQ_cb);
			} else {
				puts("PDP Context not active");
				at_command_enqueue("AT+CGDCONT=1,\"IP\",\""APN"\"",
						at_cgdcont_cb);
			}
		} else {
			// There is no PDP context????
			puts("Cannot determine current PDP context, attempting to set");
			at_command_enqueue("AT+CGDCONT=1,\"IP\",\""APN"\"", at_cgdcont_cb);
		}
	}
}

void at_cgatt1_cb(at_command_t *cmd) {
	if(cmd->status) {
		// error
	} else {
		at_command_enqueue("AT+CGACT?", at_cgactQ_cb);
	}
}

void print_registration_status() {
	puts("2G/3G Circuit Mode Registration:");
	printf("Status             : %s \n",
			at_modem_stat2str(at_modem_info.creg.stat));
	printf("Location Area Code : %04X \n", at_modem_info.creg.lac);
	printf("Cell ID            : %08X \n", at_modem_info.creg.ci);
	printf("Access Technology  : %s \n",
			at_modem_act2str(at_modem_info.creg.act));

	puts("2G/3G Packet  Mode Registration:");
	printf("Status             : %s \n",
			at_modem_stat2str(at_modem_info.cgreg.stat));
	printf("Location Area Code : %04X \n", at_modem_info.cgreg.lac);
	printf("Cell ID            : %08X \n", at_modem_info.cgreg.ci);
	printf("Access Technology  : %s \n",
			at_modem_act2str(at_modem_info.cgreg.act));

	puts("4G Registration:");
	printf("Status             : %s \n",
			at_modem_stat2str(at_modem_info.cereg.stat));
	printf("Tracking Area Code : %08X \n", at_modem_info.cereg.tac);
	printf("Cell ID            : %08X \n", at_modem_info.cereg.ci);
	printf("Access Technology  : %s \n",
			at_modem_act2str(at_modem_info.cereg.act));

	puts("5G Registration:");
	printf("Status             : %s \n",
			at_modem_stat2str(at_modem_info.c5greg.stat));
	printf("Tracking Area Code : %08X \n", at_modem_info.c5greg.tac);
	printf("Cell ID            : %08X \n", at_modem_info.c5greg.ci);
	printf("Access Technology  : %s \n",
			at_modem_act2str(at_modem_info.c5greg.act));

}

void at_c5gregQ_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	at_modem_info.c5greg = (at_modem_registration_status_t ) { -1 };
	if (cmd->status) {
		puts("AT+CEREG failed");
	} else {
		char *str_n = strtok(cmd->responses[0], ",");
		char *str_stat = strtok(NULL, ", ");
		char *str_tac = strtok(NULL, ", ");
		char *str_ci = strtok(NULL, ", ");
		char *str_act = strtok(NULL, ", ");
		at_modem_info.c5greg.stat = strtol(str_stat, NULL, 10);
		at_modem_info.c5greg.tac = strtol(str_tac + 1, NULL, 16);
		at_modem_info.c5greg.ci = strtol(str_ci + 1, NULL, 16);
		at_modem_info.c5greg.act = strtol(str_act, NULL, 10);
	}

	//print_registration_status();

}
void at_c5greg2_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+C5GREG=2 failed");
		at_modem_info.c5greg = (at_modem_registration_status_t ) { -2 };
		//print_registration_status();

	} else {
		at_command_enqueue("AT+C5GREG?", at_c5gregQ_cb);
	}
}
void at_ceregQ_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	at_modem_info.cereg = (at_modem_registration_status_t ) { -1 };
	if (cmd->status) {
		puts("AT+CEREG failed");
	} else {
		char *str_n = strtok(cmd->responses[0], ",");
		char *str_stat = strtok(NULL, ", ");
		char *str_tac = strtok(NULL, ", ");
		char *str_ci = strtok(NULL, ", ");
		char *str_act = strtok(NULL, ", ");
		at_modem_info.cereg.stat = strtol(str_stat, NULL, 10);
		at_modem_info.cereg.tac = strtol(str_tac + 1, NULL, 16);
		at_modem_info.cereg.ci = strtol(str_ci + 1, NULL, 16);
		at_modem_info.cereg.act = strtol(str_act, NULL, 10);
	}

}
void at_cereg2_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CEREG=2 failed");
		at_modem_info.cereg = (at_modem_registration_status_t ) { -2 };

	} else {
		at_command_enqueue("AT+CEREG?", at_ceregQ_cb);
	}
}
void at_cgregQ_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	at_modem_info.cgreg = (at_modem_registration_status_t ) { -1 };
	if (cmd->status) {
		puts("AT+CGREG failed");
	} else {
		char *str_n = strtok(cmd->responses[0], ",");
		char *str_stat = strtok(NULL, ", ");
		char *str_lac = strtok(NULL, ", ");
		char *str_ci = strtok(NULL, ", ");
		char *str_act = strtok(NULL, ", ");
		at_modem_info.cgreg.stat = strtol(str_stat, NULL, 10);
		at_modem_info.cgreg.lac = strtol(str_lac + 1, NULL, 16);
		at_modem_info.cgreg.ci = strtol(str_ci + 1, NULL, 16);
		at_modem_info.cgreg.act = strtol(str_act, NULL, 10);

		bshal_delay_ms(100);
		puts("2G/3G Packet  Mode Registration:");
		printf("Status             : %s \n",
				at_modem_stat2str(at_modem_info.cgreg.stat));
		printf("Location Area Code : %04X \n", at_modem_info.cgreg.lac);
		printf("Cell ID            : %08X \n", at_modem_info.cgreg.ci);
		printf("Access Technology  : %s \n",
				at_modem_act2str(at_modem_info.cgreg.act));
		bshal_delay_ms(100);

		if (at_modem_info.cgreg.stat == 1 || at_modem_info.cgreg.stat == 5) {
			puts("Registered to a packet service");
			at_command_enqueue("AT+CGACT?", at_cgactQ_cb);
		} else if ((at_modem_info.creg.stat == 1 || at_modem_info.creg.stat == 5)) {
			puts("Circuit registration but no Packer registration? Not attached?");
			at_command_enqueue("AT+CGATT=1", at_cgatt1_cb);
		}

	}
}

void at_cgreg2_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGREG=2 failed");
		at_modem_info.cgreg = (at_modem_registration_status_t ) { -2 };

	} else {
		at_command_enqueue("AT+CGREG?", at_cgregQ_cb);
	}
}

void at_cregQ_cb(at_command_t *cmd) {
	at_generic_cb(cmd);
	at_modem_info.creg = (at_modem_registration_status_t ) { -1 };
	if (cmd->status) {
		puts("AT+CREG? failed");
	} else {
		char *str_n = strtok(cmd->responses[0], ",");
		char *str_stat = strtok(NULL, ", ");
		char *str_lac = strtok(NULL, ", ");
		char *str_ci = strtok(NULL, ", ");
		char *str_act = strtok(NULL, ", ");
		at_modem_info.creg.stat = strtol(str_stat, NULL, 10);
		at_modem_info.creg.lac = strtol(str_lac + 1, NULL, 16);
		at_modem_info.creg.ci = strtol(str_ci + 1, NULL, 16);
		at_modem_info.creg.act = strtol(str_act, NULL, 10);

		bshal_delay_ms(100);
		puts("2G/3G Circuit Mode Registration:");
		printf("Status             : %s \n",
				at_modem_stat2str(at_modem_info.creg.stat));
		printf("Location Area Code : %04X \n", at_modem_info.creg.lac);
		printf("Cell ID            : %08X \n", at_modem_info.creg.ci);
		printf("Access Technology  : %s \n",
				at_modem_act2str(at_modem_info.creg.act));
		bshal_delay_ms(100);
	}

}

void at_creg2_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CREG=2 failed");
		at_modem_info.creg = (at_modem_registration_status_t ) { -2 };
	} else {
		at_command_enqueue("AT+CREG?", at_cregQ_cb);
	}
}

void query_registration_status(void) {
	at_command_enqueue("AT+CREG=2", at_creg2_cb);
	at_command_enqueue("AT+CGREG=2", at_cgreg2_cb);
	at_command_enqueue("AT+CEREG=2", at_cereg2_cb);
	at_command_enqueue("AT+C5GREG=2", at_c5greg2_cb);
}

void at_gcap_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+GCAP failed");
		puts(cmd->status_string);
		// We might be talking to a non-complaiont device
		// Attempt to query cellular registration status despite
		// it does not report to be a cellular modem.
		query_registration_status();

	} else {
		if (strstr(cmd->responses[0], "+CGSM")) {
			puts("Device appears to be a cellular modem");
			puts("Querying Network Registration Status");
			query_registration_status();
		} else {
			puts("Device does not appear to be a cellular modem");
		}
	}
}

void ati_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("ATI failed");
	} else {
		puts("ATI succeeded");
		for (int i = 0; i < 5; i++) {
			strncpy(at_modem_info.Info[i], cmd->responses[i],
					sizeof(at_modem_info.Info[i]));
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
				&& !strcmp(" Undefined", at_modem_info.Revision)
				&& !strcmp("M590", at_modem_info.Model)
				&& !strcmp(at_modem_info.Info[1], at_modem_info.Model)) {
			strcpy(at_modem_info.Manufacturer, at_modem_info.Info[0]);
			strcpy(at_modem_info.Revision, at_modem_info.Info[2]);

			puts("M590 patched data:");
			printf("Manufacturer: %s\n", at_modem_info.Manufacturer);
			printf("Model       : %s\n", at_modem_info.Model);
			printf("Revision    : %s\n", at_modem_info.Revision);
			printf("Serial      : %s\n", at_modem_info.Serial);
		}

		memset(&at_command, 0, sizeof(at_command));

		// Query AT+GCAP
		// Check for +CGSM

		//		at_command.cb = at_gcap_cb;
		//		at_command_send("AT+GCAP");
		fflush(0);

		at_command_enqueue("AT+GCAP", at_gcap_cb);

		// AT+CREG?
		// circuit mode services in GERAN/UTRAN

		// AT+CGREG?
		// GPRS services

		// For GSM and UMTS, we have a separate circuit and packet switched
		// mode, while there is only one registration for LTE

		// AT+CEREG?
		// EPS services in E-UTRAN

		// As circuit and packet are merged in LTE, it makes sense to have
		// a separate registration query for it, but that has changed for
		// NG to deserve a separate command?

		// AT+C5REG?
		// 5G services in NG-RAN

	}
}

void at_cgsn_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGSN failed");
	} else {
		puts("AT+CGSN succeeded");
		strncpy(at_modem_info.Serial, cmd->responses[0],
				sizeof(at_modem_info.Serial));

		//		memset(&at_command, 0, sizeof(at_command));
		//		at_command.cb = ati_cb;
		//		at_command_send("ATI");

	}
}

void at_cgmr_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGMR failed");
	} else {
		puts("AT+CGMR succeeded");
		strncpy(at_modem_info.Revision, cmd->responses[0],
				sizeof(at_modem_info.Revision));

		//		memset(&at_command, 0, sizeof(at_command));
		//		at_command.cb = at_cgsn_cb;
		//		at_command_send("AT+CGSN");

	}
}

void at_cgmm_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGMM failed");
	} else {
		puts("AT+CGMM succeeded");
		strncpy(at_modem_info.Model, cmd->responses[0],
				sizeof(at_modem_info.Model));

		//		memset(&at_command, 0, sizeof(at_command));
		//		at_command.cb = at_cgmr_cb;
		//		at_command_send("AT+CGMR");

	}
}

void at_cgmi_cb(at_command_t *cmd) {
	if (cmd->status) {
		puts("AT+CGMI failed");
	} else {
		puts("AT+CGMI succeeded");
		strncpy(at_modem_info.Manufacturer, cmd->responses[0],
				sizeof(at_modem_info.Manufacturer));

		//		memset(&at_command, 0, sizeof(at_command));
		//		at_command.cb = at_cgmm_cb;
		//		at_command_send("AT+CGMM");

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

	//	memset(&at_command, 0, sizeof(at_command));
	//	at_command.cb = at_cgmi_cb;
	//	at_command_send("AT+CGMI");

	at_unsolicited_cb_registrations[0].cb = any_unsol_cb_test;
	at_unsolicited_cb_registrations[0].command[0] = '+';

	at_command_enqueue("AT+CPIN?", at_generic_cb);

	at_command_enqueue("AT+CGMI", at_cgmi_cb);
	at_command_enqueue("AT+CGSN", at_cgsn_cb);
	at_command_enqueue("AT+CGMR", at_cgmr_cb);
	at_command_enqueue("AT+CGMM", at_cgmm_cb);
	at_command_enqueue("ATI", ati_cb);

	while (1) {
		bshal_uart_at_recv_process(&bshal_uart_at_async);
		at_command_queue_process();
	}


}
