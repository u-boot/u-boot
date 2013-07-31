/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * hacked for MIP405
 */

#include <common.h>
#include <command.h>
#include "mip405.h"
#include "../common/common_util.h"


extern void print_mip405_info(void);
extern int do_mplcommon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);


/* ------------------------------------------------------------------------- */

int do_mip405(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	ulong led_on;

	if (strcmp(argv[1], "info") == 0)
	{
		print_mip405_info();
		return 0;
	}
	if (strcmp(argv[1], "led") == 0)
	{
		led_on = (ulong)simple_strtoul(argv[2], NULL, 10);
		user_led0(led_on);
		return 0;
	}
	return (do_mplcommon(cmdtp, flag, argc, argv));
}
U_BOOT_CMD(
	mip405,	8,	1,	do_mip405,
	"MIP405 specific Cmds",
	"flash mem [SrcAddr] - updates U-Boot with image in memory\n"
	"mip405 flash mps - updates U-Boot with image from MPS\n"
	"mip405 info      - displays board information\n"
	"mip405 led <on>  - switches LED on (on=1) or off (on=0)"
);

/* ------------------------------------------------------------------------- */
