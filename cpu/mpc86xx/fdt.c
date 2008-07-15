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

void ft_cpu_setup(void *blob, bd_t *bd)
{
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
	fdt_fixup_ethernet(blob, bd);
#endif

#ifdef CFG_NS16550
	do_fixup_by_compat_u32(blob, "ns16550",
			       "clock-frequency", CFG_NS16550_CLK, 1);
#endif
}
