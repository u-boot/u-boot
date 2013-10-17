/*
 * Copyright 2008 (C) Bryan O'Donoghue
 *
 * Code copied & edited from Freescale mpc85xx stuff.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

void ft_cpu_setup(void *blob, bd_t *bd)
{
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
		"timebase-frequency", get_tbclk(), 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
		"bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
		"clock-frequency", bd->bi_intfreq, 1);
	do_fixup_by_compat_u32(blob, "fsl,cpm-brg", "clock-frequency",
		gd->arch.brg_clk, 1);

	/* Fixup ethernet MAC addresses */
	fdt_fixup_ethernet(blob);

	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}
