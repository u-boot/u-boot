/*
 * flash.c - helper commands for working with GPIO-assisted flash
 *
 * Copyright (c) 2005-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <asm/blackfin.h>
#include "gpio_cfi_flash.h"

int do_ph(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong faddr = CONFIG_SYS_FLASH_BASE;
	ushort data;
	ulong dflg;

	if (argc > 1) {
		dflg = simple_strtoul(argv[1], NULL, 16);
		faddr |= (dflg << 21);
		gpio_cfi_flash_swizzle((void *)faddr);
	} else {
		data = bfin_read_PORTHIO();
		printf("Port H data %04x (PH0:%i)\n", data, !!(data & PH0));
	}

	return 0;
}

U_BOOT_CMD(ph, 3, 0, do_ph,
	"set/clear PH0 GPIO flash bank switch\n",
	"<ph0> - set PH0 GPIO pin state\n");
