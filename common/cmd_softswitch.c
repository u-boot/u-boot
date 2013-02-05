/*
 * cmd_softswitch.c - set the softswitch for bf60x
 *
 * Copyright (c) 2012 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm/soft_switch.h>

int do_softswitch(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int switchaddr, value, pin, port;

	if (argc != 5)
		return CMD_RET_USAGE;

	if (strcmp(argv[2], "GPA") == 0)
		port = IO_PORT_A;
	else if (strcmp(argv[2], "GPB") == 0)
		port = IO_PORT_B;
	else
		return CMD_RET_USAGE;

	switchaddr = simple_strtoul(argv[1], NULL, 16);
	pin = simple_strtoul(argv[3], NULL, 16);
	value = simple_strtoul(argv[4], NULL, 16);

	config_switch_bit(switchaddr, port, (1 << pin), IO_PORT_OUTPUT, value);

	return 0;
}

U_BOOT_CMD(
	softswitch_output, 5, 1, do_softswitch,
	"switchaddr GPA/GPB pin_offset value",
	""
);
