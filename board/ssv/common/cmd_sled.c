/*
 * (C) Copyright 2004, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
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
#include <command.h>
#include <status_led.h>

#if	defined(CONFIG_STATUS_LED)

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!!       Q u i c k   &   D i r t y   H a c k	 !!!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!!						 !!!!!
 * !!!!! Next type definition was coming from original	 !!!!!
 * !!!!! status LED driver drivers/misc/status_led.c	 !!!!!
 * !!!!! and should be exported for using it here.	 !!!!!
 * !!!!!						 !!!!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

typedef struct {
	led_id_t mask;
	int state;
	int period;
	int cnt;
} led_dev_t;

extern led_dev_t led_dev[];

#if defined(CONFIG_CMD_BSP)
int do_sled (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int led_id = 0;

	if (argc > 1) {
#ifdef	STATUS_LED_BOOT
		if (!strcmp (argv[1], "boot")) {
			led_id = STATUS_LED_BOOT + 1;
		}
#endif
#ifdef	STATUS_LED_RED
		if (!strcmp (argv[1], "red")) {
			led_id = STATUS_LED_RED + 1;
		}
#endif
#ifdef	STATUS_LED_YELLOW
		if (!strcmp (argv[1], "yellow")) {
			led_id = STATUS_LED_YELLOW + 1;
		}
#endif
#ifdef	STATUS_LED_GREEN
		if (!strcmp (argv[1], "green")) {
			led_id = STATUS_LED_GREEN + 1;
		}
#endif
	}

	switch (argc) {
	case 1:
#if	(STATUS_LED_BITS > 3)
		for (; led_id < 4; led_id++)
#elif	(STATUS_LED_BITS > 2)
		for (; led_id < 3; led_id++)
#elif	(STATUS_LED_BITS > 1)
		for (; led_id < 2; led_id++)
#elif	(STATUS_LED_BITS > 0)
		for (; led_id < 1; led_id++)
#else
#error "*** STATUS_LED_BITS not correct defined ***"
#endif
		{
			printf ("Status LED '%s' is %s\n",
				led_id == STATUS_LED_BOOT ? "boot"
				: led_id == STATUS_LED_RED ? "red"
				: led_id == STATUS_LED_YELLOW ? "yellow"
				: led_id ==
				STATUS_LED_GREEN ? "green" : "unknown",
				led_dev[led_id].state ==
				STATUS_LED_ON ? "on" : led_dev[led_id].
				state ==
				STATUS_LED_OFF ? "off" : led_dev[led_id].
				state ==
				STATUS_LED_BLINKING ? "blinking" : "unknown");
		}
		return 0;
	case 2:
		if (led_id) {
			printf ("Status LED '%s' is %s\n", argv[1],
				led_dev[led_id - 1].state ==
				STATUS_LED_ON ? "on" : led_dev[led_id -
							       1].state ==
				STATUS_LED_OFF ? "off" : led_dev[led_id -
								 1].state ==
				STATUS_LED_BLINKING ? "blinking" : "unknown");
			return 0;
		} else
			break;
	case 3:
		if (led_id) {
			if (!strcmp (argv[2], "on")) {
				status_led_set (led_id - 1, STATUS_LED_ON);
				return 0;
			} else if (!strcmp (argv[2], "off")) {
				status_led_set (led_id - 1, STATUS_LED_OFF);
				return 0;
			} else if (!strcmp (argv[2], "blink")) {
				status_led_set (led_id - 1,
						STATUS_LED_BLINKING);
				return 0;
			} else
				break;
		} else
			break;
	default:
		break;
	}
	cmd_usage(cmdtp);
	return 1;
}

#ifdef	STATUS_LED_BOOT
#ifdef	STATUS_LED_RED
#ifdef	STATUS_LED_YELLOW
#ifdef	STATUS_LED_GREEN
#define __NAME_STR		"    - name: boot|red|yellow|green\n"
#else
#define __NAME_STR		"    - name: boot|red|yellow\n"
#endif
#else
#define __NAME_STR		"    - name: boot|red\n"
#endif
#else
#define __NAME_STR		"    - name: boot\n"
#endif
#else
#define __NAME_STR		"    - name: (no such defined)\n"
#endif

U_BOOT_CMD (sled, 3, 0, do_sled,
	    "check and set status led",
	    "sled [name [state]]\n" __NAME_STR "    - state: on|off|blink");
#endif
#endif	/* CONFIG_STATUS_LED */
