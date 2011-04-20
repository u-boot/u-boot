/*
 * (C) Copyright 2010
 * Jason Kridner <jkridner@beagleboard.org>
 *
 * Based on cmd_led.c patch from:
 * http://www.mail-archive.com/u-boot@lists.denx.de/msg06873.html
 * (C) Copyright 2008
 * Ulf Samuelsson <ulf.samuelsson@atmel.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include <status_led.h>

struct led_tbl_s {
	char		*string;	/* String for use in the command */
	led_id_t	mask;		/* Mask used for calling __led_set() */
	void		(*off)(void);	/* Optional function for turning LED off */
	void		(*on)(void);	/* Optional function for turning LED on */
};

typedef struct led_tbl_s led_tbl_t;

static const led_tbl_t led_commands[] = {
#ifdef CONFIG_BOARD_SPECIFIC_LED
#ifdef STATUS_LED_BIT
	{ "0", STATUS_LED_BIT, NULL, NULL },
#endif
#ifdef STATUS_LED_BIT1
	{ "1", STATUS_LED_BIT1, NULL, NULL },
#endif
#ifdef STATUS_LED_BIT2
	{ "2", STATUS_LED_BIT2, NULL, NULL },
#endif
#ifdef STATUS_LED_BIT3
	{ "3", STATUS_LED_BIT3, NULL, NULL },
#endif
#endif
#ifdef STATUS_LED_GREEN
	{ "green", STATUS_LED_GREEN, green_LED_off, green_LED_on },
#endif
#ifdef STATUS_LED_YELLOW
	{ "yellow", STATUS_LED_YELLOW, yellow_LED_off, yellow_LED_on },
#endif
#ifdef STATUS_LED_RED
	{ "red", STATUS_LED_RED, red_LED_off, red_LED_on },
#endif
#ifdef STATUS_LED_BLUE
	{ "blue", STATUS_LED_BLUE, blue_LED_off, blue_LED_on },
#endif
	{ NULL, 0, NULL, NULL }
};

int str_onoff (char *var)
{
	if (strcmp(var, "off") == 0) {
		return 0;
	}
	if (strcmp(var, "on") == 0) {
		return 1;
	}
	return -1;
}

int do_led (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int state, i, match = 0;

	/* Validate arguments */
	if ((argc != 3)) {
		return cmd_usage(cmdtp);
	}

	state = str_onoff(argv[2]);
	if (state < 0) {
		return cmd_usage(cmdtp);
	}

	for (i = 0; led_commands[i].string; i++) {
		if ((strcmp("all", argv[1]) == 0) ||
		    (strcmp(led_commands[i].string, argv[1]) == 0)) {
		    	match = 1;
			if (led_commands[i].on) {
				if (state) {
					led_commands[i].on();
				} else {
					led_commands[i].off();
				}
			} else {
				__led_set(led_commands[i].mask, state);
			}
			break;
		}
	}

	/* If we ran out of matches, print Usage */
	if (!match) {
		return cmd_usage(cmdtp);
	}

	return 0;
}

U_BOOT_CMD(
	led, 3, 1, do_led,
	"led\t- ["
#ifdef CONFIG_BOARD_SPECIFIC_LED
#ifdef STATUS_LED_BIT
	"0|"
#endif
#ifdef STATUS_LED_BIT1
	"1|"
#endif
#ifdef STATUS_LED_BIT2
	"2|"
#endif
#ifdef STATUS_LED_BIT3
	"3|"
#endif
#endif
#ifdef STATUS_LED_GREEN
	"green|"
#endif
#ifdef STATUS_LED_YELLOW
	"yellow|"
#endif
#ifdef STATUS_LED_RED
	"red|"
#endif
#ifdef STATUS_LED_BLUE
	"blue|"
#endif
	"all] [on|off]\n",
	"led [led_name] [on|off] sets or clears led(s)\n"
);
