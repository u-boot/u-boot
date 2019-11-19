// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2012 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <common.h>
#include <dm/device.h>
#include <fdt_support.h>

unsigned long rpi_bcm283x_base = 0x3f000000;

int arch_cpu_init(void)
{
	icache_enable();

	return 0;
}

int mach_cpu_init(void)
{
	int ret, soc_offset;
	u64 io_base, size;

	/* Get IO base from device tree */
	soc_offset = fdt_path_offset(gd->fdt_blob, "/soc");
	if (soc_offset < 0)
		return soc_offset;

	ret = fdt_read_range((void *)gd->fdt_blob, soc_offset, 0, NULL,
				&io_base, &size);
	if (ret)
		return ret;

	rpi_bcm283x_base = io_base;

	return 0;
}

#ifdef CONFIG_ARMV7_LPAE
void enable_caches(void)
{
	dcache_enable();
}
#endif
