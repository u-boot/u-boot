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
 * SPDX-License-Identifier:	GPL-2.0+
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

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

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

	if (!of_flat_tree && argc > 1)
		of_flat_tree = (char *)simple_strtoul(argv[1], NULL, 16);
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
