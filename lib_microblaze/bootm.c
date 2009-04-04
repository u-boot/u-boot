/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>

DECLARE_GLOBAL_DATA_PTR;

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	/* First parameter is mapped to $r5 for kernel boot args */
	void	(*theKernel) (char *);
	char	*commandline = getenv ("bootargs");

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	theKernel = (void (*)(char *))images->ep;

	show_boot_progress (15);

#ifdef DEBUG
	printf ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong) theKernel);
#endif

	theKernel (commandline);
	/* does not return */

	return 1;
}
