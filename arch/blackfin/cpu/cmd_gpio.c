/*
 * Control GPIO pins on the fly
 *
 * Copyright (c) 2008-2010 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>

#include <asm/blackfin.h>
#include <asm/gpio.h>

enum {
	GPIO_INPUT,
	GPIO_SET,
	GPIO_CLEAR,
	GPIO_TOGGLE,
};

int do_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc == 2 && !strcmp(argv[1], "status")) {
		bfin_gpio_labels();
		return 0;
	}

	if (argc != 3)
 show_usage:
		return cmd_usage(cmdtp);

	/* parse the behavior */
	ulong sub_cmd;
	switch (argv[1][0]) {
		case 'i': sub_cmd = GPIO_INPUT;  break;
		case 's': sub_cmd = GPIO_SET;    break;
		case 'c': sub_cmd = GPIO_CLEAR;  break;
		case 't': sub_cmd = GPIO_TOGGLE; break;
		default:  goto show_usage;
	}

	/* parse the pin with format: [p][port]<#> */
	const char *str_pin = argv[2];

	/* grab the [p]<port> portion */
	ulong port_base;
	if (*str_pin == 'p') ++str_pin;
	switch (*str_pin) {
#ifdef GPIO_PA0
		case 'a': port_base = GPIO_PA0; break;
#endif
#ifdef GPIO_PB0
		case 'b': port_base = GPIO_PB0; break;
#endif
#ifdef GPIO_PC0
		case 'c': port_base = GPIO_PC0; break;
#endif
#ifdef GPIO_PD0
		case 'd': port_base = GPIO_PD0; break;
#endif
#ifdef GPIO_PE0
		case 'e': port_base = GPIO_PE0; break;
#endif
#ifdef GPIO_PF0
		case 'f': port_base = GPIO_PF0; break;
#endif
#ifdef GPIO_PG0
		case 'g': port_base = GPIO_PG0; break;
#endif
#ifdef GPIO_PH0
		case 'h': port_base = GPIO_PH0; break;
#endif
#ifdef GPIO_PI0
		case 'i': port_base = GPIO_PI0; break;
#endif
#ifdef GPIO_PJ
		case 'j': port_base = GPIO_PJ0; break;
#endif
		default:  goto show_usage;
	}

	/* grab the <#> portion */
	ulong pin = simple_strtoul(str_pin + 1, NULL, 10);
	if (pin > 15)
		goto show_usage;

	/* grab the pin before we tweak it */
	ulong gpio = port_base + pin;
	gpio_request(gpio, "cmd_gpio");

	/* finally, let's do it: set direction and exec command */
	if (sub_cmd == GPIO_INPUT) {
		gpio_direction_input(gpio);
		printf("gpio: pin %lu on port %c set to input\n", pin, *str_pin);
		return 0;
	}

	ulong value;
	switch (sub_cmd) {
		case GPIO_SET:    value = 1; break;
		case GPIO_CLEAR:  value = 0; break;
		case GPIO_TOGGLE: value = !gpio_get_value(gpio); break;
		default:          goto show_usage;
	}
	gpio_direction_output(gpio, value);
	printf("gpio: pin %lu on port %c (gpio %lu) value is %lu\n",
		pin, *str_pin, gpio, value);

	gpio_free(gpio);

	return 0;
}

U_BOOT_CMD(gpio, 3, 0, do_gpio,
	"set/clear/toggle gpio output pins",
	"<set|clear|toggle> <port><pin>\n"
	"    - set/clear/toggle the specified pin (e.g. PF10)");
