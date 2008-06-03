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
 * hacked for MIP405
 */

#include <common.h>
#include <command.h>
#include "mip405.h"
#include "../common/common_util.h"


extern void print_mip405_info(void);
extern int do_mplcommon(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);


/* ------------------------------------------------------------------------- */

int do_mip405(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
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
	"mip405  - MIP405 specific Cmds\n",
	"flash mem [SrcAddr] - updates U-Boot with image in memory\n"
	"mip405 flash mps - updates U-Boot with image from MPS\n"
	"mip405 info      - displays board information\n"
	"mip405 led <on>  - switches LED on (on=1) or off (on=0)\n"
	"mip405 mem [cnt] - Memory Test <cnt>-times, <cnt> = -1 loop forever\n"
);

/* ------------------------------------------------------------------------- */
