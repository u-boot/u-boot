/*
 * (C) Copyright 2010
 *   Renesas Solutions Corp.
 *   Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
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
 */

/*
 * Linux SuperH zImage loading and boot
 */

#include <common.h>
#include <asm/io.h>
#include <asm/zimage.h>

int do_sh_zimageboot (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong (*zboot_entry)(int, char * const []) = NULL;
	char *s0, *s1;
	unsigned char *param = NULL;
	char *cmdline;
	char *bootargs;

	disable_interrupts();

	if (argc >= 3) {
		/* argv[1] holds the address of the zImage */
		s0 = argv[1];
		/* argv[2] holds the address of zero page */
		s1 = argv[2];
	} else {
		goto exit;
	}

	if (s0)
		zboot_entry = (ulong (*)(int, char * const []))simple_strtoul(s0, NULL, 16);

	/* empty_zero_page */
	if (s1)
		param = (unsigned char*)simple_strtoul(s1, NULL, 16);

	/* Linux kernel command line */
	cmdline = (char *)param + COMMAND_LINE;
	bootargs = getenv("bootargs");

	/* Clear zero page */
	memset(param, 0, 0x1000);

	/* Set commandline */
	strcpy(cmdline, bootargs);

	/* Boot */
	zboot_entry(0, NULL);

exit:
	return -1;
}

U_BOOT_CMD(
	zimageboot, 3, 0,	do_sh_zimageboot,
	"Boot zImage for Renesas SH",
	""
);
