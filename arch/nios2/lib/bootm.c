/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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

#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/cache.h>

#define NIOS_MAGIC 0x534f494e /* enable command line and initrd passing */

int do_bootm_linux(int flag, int argc, char * const argv[], bootm_headers_t *images)
{
	void (*kernel)(int, int, int, char *) = (void *)images->ep;
	char *commandline = getenv("bootargs");
	ulong initrd_start = images->rd_start;
	ulong initrd_end = images->rd_end;
	char *of_flat_tree = NULL;
#if defined(CONFIG_OF_LIBFDT)
	/* did generic code already find a device tree? */
	if (images->ft_len)
		of_flat_tree = images->ft_addr;
#endif
	if (!of_flat_tree && argc > 3)
		of_flat_tree = (char *)simple_strtoul(argv[3], NULL, 16);
	if (of_flat_tree)
		initrd_end = (ulong)of_flat_tree;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* flushes data and instruction caches before calling the kernel */
	disable_interrupts();
	flush_dcache((ulong)kernel, CONFIG_SYS_DCACHE_SIZE);
	flush_icache((ulong)kernel, CONFIG_SYS_ICACHE_SIZE);

	debug("bootargs=%s @ 0x%lx\n", commandline, (ulong)&commandline);
	debug("initrd=0x%lx-0x%lx\n", (ulong)initrd_start, (ulong)initrd_end);
	/* kernel parameters passing
	 * r4 : NIOS magic
	 * r5 : initrd start
	 * r6 : initrd end or fdt
	 * r7 : kernel command line
	 * fdt is passed to kernel via r6, the same as initrd_end. fdt will be
	 * verified with fdt magic. when both initrd and fdt are used at the
	 * same time, fdt must follow immediately after initrd.
	 */
	kernel(NIOS_MAGIC, initrd_start, initrd_end, commandline);
	/* does not return */

	return 1;
}
