/*
 * U-boot - cmd_bf537led.c
 *
 * Copyright (C) 2006 Aaron Gage, Ocean Optics Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <config.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm-blackfin/string.h>
#ifdef CONFIG_BF537_STAMP_LEDCMD

/* Define the command usage in a reusable way */
#define USAGE_LONG \
	"led <number> <action>\n" \
	"    <number>  - Index (0-5) of LED to change, or \"all\"\n" \
	"    <action>  - Must be one of:\n" \
	"		on off toggle"

/* Number of LEDs supported by the board */
#define NUMBER_LEDS     6
/* The BF537 stamp has 6 LEDs.  This mask indicates that all should be lit. */
#define LED_ALL_MASK    0x003F

void show_cmd_usage(void);
void set_led_state(int index, int state);
void configure_GPIO_to_output(int index);

/* Map of LEDs according to their GPIO ports.  This can be rearranged or
 * otherwise changed to account for different GPIO configurations.
 */
int led_ports[] = { PF6, PF7, PF8, PF9, PF10, PF11 };

#define ACTION_TOGGLE   -1
#define ACTION_OFF      0
#define ACTION_ON       1

#define LED_STATE_OFF   0
#define LED_STATE_ON    1

/* This is a trivial atoi implementation since we don't have one available */
int atoi(char *string)
{
	int length;
	int retval = 0;
	int i;
	int sign = 1;

	length = strlen(string);
	for (i = 0; i < length; i++) {
		if (0 == i && string[0] == '-') {
			sign = -1;
			continue;
		}
		if (string[i] > '9' || string[i] < '0') {
			break;
		}
		retval *= 10;
		retval += string[i] - '0';
	}
	retval *= sign;
	return retval;
}

int do_bf537led(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int led_mask = 0;
	int led_current_state = 0;
	int action = ACTION_OFF;
	int temp;

	if (3 != argc) {
		/* Not enough arguments, so just show usage information */
		show_cmd_usage();
		return 1;
	}

	if (strcmp(argv[1], "all") == 0) {
		led_mask = LED_ALL_MASK;
	} else {
		temp = atoi(argv[1]);
		if (temp < 0 || temp >= NUMBER_LEDS) {
			printf("Invalid LED number [%s]\n", argv[1]);
			show_cmd_usage();
			return 2;
		}
		led_mask |= (1 << temp);
	}

	if (strcmp(argv[2], "off") == 0) {
		action = ACTION_OFF;
	} else if (strcmp(argv[2], "on") == 0) {
		action = ACTION_ON;
	} else if (strcmp(argv[2], "toggle") == 0) {
		action = ACTION_TOGGLE;
	} else {
		printf("Invalid action [%s]\n", argv[2]);
		show_cmd_usage();
		return 3;
	}

	for (temp = 0; temp < NUMBER_LEDS; temp++) {
		if ((led_mask & (1 << temp)) > 0) {
			/*
			 * It is possible that the user has wired one of PF6-PF11 to
			 * something other than an LED, so this will only change a pin
			 * to output if the user has indicated a state change.  This may
			 * happen a lot, but this way is safer than just setting all pins
			 * to output.
			 */
			configure_GPIO_to_output(temp);

			led_current_state =
			    ((*pPORTFIO & led_ports[temp]) >
			     0) ? LED_STATE_ON : LED_STATE_OFF;
	/*
		printf("LED state for index %d (%x) is %d\n", temp, led_ports[temp],
			led_current_state);
		printf("*pPORTFIO is %x\n", *pPORTFIO);
	*/
			if (ACTION_ON == action
			    || (ACTION_TOGGLE == action
				&& 0 == led_current_state)) {
				printf("Turning LED %d on\n", temp);
				set_led_state(temp, LED_STATE_ON);
			} else {
				printf("Turning LED %d off\n", temp);
				set_led_state(temp, LED_STATE_OFF);
			}
		}
	}

	return 0;
}

/*
 * The GPIO pins that go to the LEDs on the BF537 stamp must be configured
 * as output.  This function simply configures them that way.  This could
 * be done to all of the GPIO lines at once, but if a user is using a
 * custom board, this will try to be nice and only change the GPIO lines
 * that the user specifically names.
 */
void configure_GPIO_to_output(int index)
{
	int port;

	port = led_ports[index];

	/* Clear the Port F Function Enable Register */
	*pPORTF_FER &= ~port;
	/* Set the Port F I/O direction register */
	*pPORTFIO_DIR |= port;
	/* Clear the Port F I/O Input Enable Register */
	*pPORTFIO_INEN &= ~port;
}

/* Enforce the given state on the GPIO line for the indicated LED */
void set_led_state(int index, int state)
{
	int port;

	port = led_ports[index];

	if (LED_STATE_OFF == state) {
		/* Clear the bit to turn off the LED */
		*pPORTFIO &= ~port;
	} else {
		/* Set the bit to turn on the LED */
		*pPORTFIO |= port;
	}
}

/* Display usage information */
void show_cmd_usage()
{
	printf("Usage:\n%s\n", USAGE_LONG);
}

/* Register information for u-boot to find this command */
U_BOOT_CMD(led, 3, 1, do_bf537led,
	   "Control BF537 stamp LEDs", USAGE_LONG);

#endif
