/*
 * avr.c
 *
 * AVR functions
 *
 * Copyright (C) 2006 Mihai Georgian <u-boot@linuxnotincluded.org.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <ns16550.h>
#include <console.h>

/* Button codes from the AVR */
#define PWRR			0x20		/* Power button release	*/
#define PWRP			0x21		/* Power button push	*/
#define RESR			0x22		/* Reset button release	*/
#define RESP			0x23		/* Reset button push	*/
#define AVRINIT			0x33		/* Init complete	*/
#define AVRRESET		0x31		/* Reset request	*/

/* LED commands */
#define PWRBLINKSTRT		'['		/* Blink power LED	*/
#define PWRBLINKSTOP		'Z'		/* Solid power LED	*/
#define HDDLEDON		'W'		/* HDD LED on		*/
#define HDDLEDOFF		'V'		/* HDD LED off		*/
#define HDDBLINKSTRT		'Y'		/* HDD LED start blink	*/
#define HDDBLINKSTOP		'X'		/* HDD LED stop blink	*/

/* Timings for LEDs blinking to show choice */
#define PULSETIME		250		/* msecs		*/
#define LONGPAUSE		(5 * PULSETIME)

/* Button press times */
#define PUSHHOLD		1000		/* msecs		*/
#define NOBUTTON		(6 * (LONGPAUSE+PULSETIME))

/* Boot and console choices */
#define MAX_BOOT_CHOICE		3

static char *consoles[] = {
	"serial",
#if defined(CONFIG_NETCONSOLE)
	"nc",
#endif
};
#define MAX_CONS_CHOICE		(sizeof(consoles)/sizeof(char *))

#if !defined(CONFIG_NETCONSOLE)
#define DEF_CONS_CHOICE		0
#else
#define DEF_CONS_CHOICE		1
#endif

#define perror(fmt, args...) printf("%s: " fmt, __FUNCTION__ , ##args)

extern void miconCntl_SendCmd(unsigned char dat);
extern void miconCntl_DisWDT(void);

static int boot_stop;

static int boot_choice = 1;
static int cons_choice = DEF_CONS_CHOICE;

static char envbuffer[16];

void init_AVR_DUART (void)
{
	NS16550_t AVR_port = (NS16550_t) CFG_NS16550_COM2;
	int clock_divisor = CFG_NS16550_CLK / 16 / 9600;

	/*
	 * AVR port init sequence taken from
	 * the original Linkstation init code
	 * Normal U-Boot serial reinit doesn't
	 * work because the AVR uses even parity
	 */
	AVR_port->lcr = 0x00;
	AVR_port->ier = 0x00;
	AVR_port->lcr = LCR_BKSE;
	AVR_port->dll = clock_divisor & 0xff;
	AVR_port->dlm = (clock_divisor >> 8) & 0xff;
	AVR_port->lcr = LCR_WLS_8 | LCR_PEN | LCR_EPS;
	AVR_port->mcr = 0x00;
	AVR_port->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;

	miconCntl_DisWDT();

	boot_stop = 0;
	miconCntl_SendCmd(PWRBLINKSTRT);
}

static inline int avr_tstc(void)
{
	return (NS16550_tstc((NS16550_t)CFG_NS16550_COM2));
}

static inline char avr_getc(void)
{
	return (NS16550_getc((NS16550_t)CFG_NS16550_COM2));
}

static int push_timeout(char button_code)
{
	ulong push_start = get_timer(0);
	while (get_timer(push_start) <= PUSHHOLD)
		if (avr_tstc() && avr_getc() == button_code)
			return 0;
	return 1;
}

static void next_boot_choice(void)
{
	ulong return_start;
	ulong pulse_start;
	int on_times;
	int button_on;
	int led_state;
	char c;

	button_on = 0;
	return_start = get_timer(0);

	on_times = boot_choice;
	led_state = 0;
	miconCntl_SendCmd(HDDLEDOFF);
	pulse_start = get_timer(0);

	while (get_timer(return_start) <= NOBUTTON || button_on) {
		if (avr_tstc()) {
			c = avr_getc();
			if (c == PWRP)
				button_on = 1;
			else if (c == PWRR) {
				button_on = 0;
				return_start = get_timer(0);
				if (++boot_choice > MAX_BOOT_CHOICE)
					boot_choice = 1;
				sprintf(envbuffer, "bootcmd%d", boot_choice);
				if (getenv(envbuffer)) {
					sprintf(envbuffer, "run bootcmd%d", boot_choice);
					setenv("bootcmd", envbuffer);
				}
				on_times = boot_choice;
				led_state = 1;
				miconCntl_SendCmd(HDDLEDON);
				pulse_start = get_timer(0);
			} else {
				perror("Unexpected code: 0x%02X\n", c);
			}
		}
		if (on_times && get_timer(pulse_start) > PULSETIME) {
			if (led_state == 1) {
				--on_times;
				led_state = 0;
				miconCntl_SendCmd(HDDLEDOFF);
			} else {
				led_state = 1;
				miconCntl_SendCmd(HDDLEDON);
			}
			pulse_start = get_timer(0);
		}
		if (!on_times && get_timer(pulse_start) > LONGPAUSE) {
			on_times = boot_choice;
			led_state = 1;
			miconCntl_SendCmd(HDDLEDON);
			pulse_start = get_timer(0);
		}
	}
	if (led_state)
		miconCntl_SendCmd(HDDLEDOFF);
}

void next_cons_choice(int console)
{
	ulong return_start;
	ulong pulse_start;
	int on_times;
	int button_on;
	int led_state;
	char c;

	button_on = 0;
	cons_choice = console;
	return_start = get_timer(0);

	on_times = cons_choice+1;
	led_state = 1;
	miconCntl_SendCmd(HDDLEDON);
	pulse_start = get_timer(0);

	while (get_timer(return_start) <= NOBUTTON || button_on) {
		if (avr_tstc()) {
			c = avr_getc();
			if (c == RESP)
				button_on = 1;
			else if (c == RESR) {
				button_on = 0;
				return_start = get_timer(0);
				cons_choice = (cons_choice + 1) % MAX_CONS_CHOICE;
				console_assign(stdin, consoles[cons_choice]);
				console_assign(stdout, consoles[cons_choice]);
				console_assign(stderr, consoles[cons_choice]);
				on_times = cons_choice+1;
				led_state = 0;
				miconCntl_SendCmd(HDDLEDOFF);
				pulse_start = get_timer(0);
			} else {
				perror("Unexpected code: 0x%02X\n", c);
			}
		}
		if (on_times && get_timer(pulse_start) > PULSETIME) {
			if (led_state == 0) {
				--on_times;
				led_state = 1;
				miconCntl_SendCmd(HDDLEDON);
			} else {
				led_state = 0;
				miconCntl_SendCmd(HDDLEDOFF);
			}
			pulse_start = get_timer(0);
		}
		if (!on_times && get_timer(pulse_start) > LONGPAUSE) {
			on_times = cons_choice+1;
			led_state = 0;
			miconCntl_SendCmd(HDDLEDOFF);
			pulse_start = get_timer(0);
		}
	}
	if (led_state);
	miconCntl_SendCmd(HDDLEDOFF);
}

int avr_input(void)
{
	char avr_button;

	if (!avr_tstc())
		return 0;

	avr_button = avr_getc();
	switch (avr_button) {
	case PWRP:
		if (push_timeout(PWRR)) {
			/* Timeout before power button release */
			boot_stop = ~boot_stop;
			if (boot_stop)
				miconCntl_SendCmd(PWRBLINKSTOP);
			else
				miconCntl_SendCmd(PWRBLINKSTRT);
			/* Wait for power button release */
			while (avr_getc() != PWRR)
				;
		} else
			/* Power button released */
			next_boot_choice();
		break;
	case RESP:
		/* Wait for Reset button release */
		while (avr_getc() != RESR)
			;
		next_cons_choice(cons_choice);
		break;
	case AVRINIT:
		return 0;
	default:
		perror("Unexpected code: 0x%02X\n", avr_button);
		return 0;
	}
	if (boot_stop)
		return (-3);
	else
		return (-2);
}

void avr_StopBoot(void)
{
	boot_stop = ~0;
	miconCntl_SendCmd(PWRBLINKSTOP);
}
