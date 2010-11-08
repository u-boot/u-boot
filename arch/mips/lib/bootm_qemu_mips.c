/*
 * (C) Copyright 2008
 * Jean-Christophe PLAGNIOL-VILLARD <jcplagniol@jcrosoft.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/addrspace.h>

DECLARE_GLOBAL_DATA_PTR;

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	void	(*theKernel) (int, char **, char **, int *);
	char	*bootargs = getenv ("bootargs");
	char	*start;
	uint	len;

	/* find kernel entry point */
	theKernel = (void (*)(int, char **, char **, int *))images->ep;

	show_boot_progress (15);

	debug ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong) theKernel);

	gd->bd->bi_boot_params = gd->bd->bi_memstart + (16 << 20) - 256;
	debug ("%-12s= 0x%08lX\n", "boot_params", (ulong)gd->bd->bi_boot_params);

	/* set Magic */
	*(int32_t *)(gd->bd->bi_boot_params - 4) = 0x12345678;
	/* set ram_size */
	*(int32_t *)(gd->bd->bi_boot_params - 8) = gd->ram_size;

	start = (char*)gd->bd->bi_boot_params;

	len = strlen(bootargs);

	strncpy(start, bootargs, len + 1);

	start += len;

	len = images->rd_end - images->rd_start;
	if (len > 0) {
		start += sprintf(start," rd_start=0x%08X rd_size=0x%0X",
		(uint) UNCACHED_SDRAM (images->rd_start),
		(uint) len);
	}

	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");

	theKernel (0, NULL, NULL, 0);
	/* does not return */
	return 1;
}
