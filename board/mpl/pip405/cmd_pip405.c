/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * hacked for PIP405
 */

#include <common.h>
#include <command.h>
#include "pip405.h"
#include "../common/common_util.h"


extern void print_pip405_info(void);
extern int do_mplcommon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);


/* ------------------------------------------------------------------------- */

int do_pip405(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	ulong led_on,led_nr;

	if (strcmp(argv[1], "info") == 0)
	{
		print_pip405_info();
		return 0;
	}
	if (strcmp(argv[1], "led") == 0)
	{
		led_nr = (ulong)simple_strtoul(argv[2], NULL, 10);
		led_on = (ulong)simple_strtoul(argv[3], NULL, 10);
		if(!led_nr)
			user_led0(led_on);
		else
			user_led1(led_on);
		return 0;
	}

	return (do_mplcommon(cmdtp, flag, argc, argv));
}
U_BOOT_CMD(
	pip405,	6,	1,	do_pip405,
	"PIP405 specific Cmds",
	"flash mem [SrcAddr] - updates U-Boot with image in memory\n"
	"pip405 flash floppy [SrcAddr] - updates U-Boot with image from floppy\n"
	"pip405 flash mps - updates U-Boot with image from MPS"
);

/* ------------------------------------------------------------------------- */
