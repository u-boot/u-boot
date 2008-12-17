/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;

void ft_cpu_setup(void *blob, bd_t *bd)
{
#if (CONFIG_NUM_CPUS > 1)
	int off;
	u32 bootpg;
#endif

	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "timebase-frequency", bd->bi_busfreq / 4, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "soc", 4,
			     "bus-frequency", bd->bi_busfreq, 1);

	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);

#if defined(CONFIG_HAS_ETH0) || defined(CONFIG_HAS_ETH1) \
    || defined(CONFIG_HAS_ETH2) || defined(CONFIG_HAS_ETH3)
	fdt_fixup_ethernet(blob);
#endif

#ifdef CONFIG_SYS_NS16550
	do_fixup_by_compat_u32(blob, "ns16550",
			       "clock-frequency", CONFIG_SYS_NS16550_CLK, 1);
#endif

#if (CONFIG_NUM_CPUS > 1)
	/* if we have 4G or more of memory, put the boot page at 4Gb-1M */
	if (gd->ram_size > 0xfffff000)
		bootpg = 0xfff00000;
	else
		bootpg = gd->ram_size - (1024 * 1024);

	/* Reserve the boot page so OSes dont use it */
	off = fdt_add_mem_rsv(blob, bootpg, (u64)4096);
	if (off < 0)
		printf("%s: %s\n", __FUNCTION__, fdt_strerror(off));
#endif
}
