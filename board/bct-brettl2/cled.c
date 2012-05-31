/*
 * cled.c - control color led
 *
 * Copyright (c) 2010 BCT Electronic GmbH
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm/io.h>

int do_cled(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr = 0x20000000 + 0x200000; /* AMS2 */
	uchar data;

	if (argc < 2)
		return cmd_usage(cmdtp);

	data = simple_strtoul(argv[1], NULL, 10);
	outb(data, addr);

	printf("cled, write %02x\n", data);

	return 0;
}

U_BOOT_CMD(cled, 2, 0, do_cled,
	"set/clear color LED",
	"");
