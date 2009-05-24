/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 *
 * hacked for PIP405
 */

#include <common.h>
#include <command.h>
#include "pip405.h"
#include "../common/common_util.h"


extern void print_pip405_info(void);
extern int do_mplcommon(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);


/* ------------------------------------------------------------------------- */

int do_pip405(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
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
