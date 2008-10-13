/*
 * Control GPIO pins on the fly
 *
 * Copyright (c) 2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>

#include <asm/blackfin.h>

int do_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc != 3) {
 show_usage:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/* parse the behavior */
	ulong port_cmd = 0;
	switch (argv[1][0]) {
		case 'i': break;
		case 's': port_cmd = (PORTFIO_SET - PORTFIO); break;
		case 'c': port_cmd = (PORTFIO_CLEAR - PORTFIO); break;
		case 't': port_cmd = (PORTFIO_TOGGLE - PORTFIO); break;
		default:  goto show_usage;
	}

	/* parse the pin with format: [p]<fgh><#> */
	const char *str_pin = argv[2];

	/* grab the [p]<fgh> portion */
	ulong port_base;
	if (*str_pin == 'p') ++str_pin;
	switch (*str_pin) {
		case 'f': port_base = PORTFIO; break;
		case 'g': port_base = PORTGIO; break;
		case 'h': port_base = PORTHIO; break;
		default:  goto show_usage;
	}

	/* grab the <#> portion */
	ulong pin = simple_strtoul(str_pin+1, NULL, 10);
	ulong pin_mask = (1 << pin);
	if (pin > 15)
		goto show_usage;

	/* finally, let's do it: set direction and exec command */
	switch (*str_pin) {
		case 'f': bfin_write_PORTF_FER(bfin_read_PORTF_FER() & ~pin_mask); break;
		case 'g': bfin_write_PORTG_FER(bfin_read_PORTG_FER() & ~pin_mask); break;
		case 'h': bfin_write_PORTH_FER(bfin_read_PORTH_FER() & ~pin_mask); break;
	}

	ulong port_dir = port_base + (PORTFIO_DIR - PORTFIO);
	if (argv[1][0] == 'i')
		bfin_write16(port_dir, bfin_read16(port_dir) & ~pin_mask);
	else {
		bfin_write16(port_dir, bfin_read16(port_dir) | pin_mask);
		bfin_write16(port_base + port_cmd, pin_mask);
	}

	printf("gpio: pin %li on port %c has been %c\n", pin, *str_pin, argv[1][0]);

	return 0;
}

U_BOOT_CMD(gpio, 3, 0, do_gpio,
	"gpio    - set/clear/toggle gpio output pins\n",
	"<s|c|t> <port><pin>\n"
	"    - set/clear/toggle the specified pin\n");
