/*
 * (C) Copyright 2011 Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 *
 * Based on microblaze implementation by:
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

int do_bootm_linux(int flag, int argc, char * const argv[],
			bootm_headers_t *images)
{
	void	(*kernel) (unsigned int);
	ulong	rd_data_start, rd_data_end;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	int	ret;

	char	*of_flat_tree = NULL;
#if defined(CONFIG_OF_LIBFDT)
	/* did generic code already find a device tree? */
	if (images->ft_len)
		of_flat_tree = images->ft_addr;
#endif

	kernel = (void (*)(unsigned int))images->ep;

	/* find ramdisk */
	ret = boot_get_ramdisk(argc, argv, images, IH_ARCH_OPENRISC,
			&rd_data_start, &rd_data_end);
	if (ret)
		return 1;

	show_boot_progress(15);

	if (!of_flat_tree && argc > 3)
		of_flat_tree = (char *)simple_strtoul(argv[3], NULL, 16);
#ifdef DEBUG
	printf("## Transferring control to Linux (at address 0x%08lx) " \
				"ramdisk 0x%08lx, FDT 0x%08lx...\n",
		(ulong) kernel, rd_data_start, (ulong) of_flat_tree);
#endif
	if (dcache_status() || icache_status())
		flush_cache((ulong)kernel, max(checkdcache(), checkicache()));

	/*
	 * Linux Kernel Parameters (passing device tree):
	 * r3: pointer to the fdt, followed by the board info data
	 */
	kernel((unsigned int) of_flat_tree);
	/* does not return */

	return 1;
}
