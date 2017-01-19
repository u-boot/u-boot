/*
 * Copyright (c) 2013, Google Inc.
 *
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 *  - Added prep subcommand support
 *  - Reorganized source - modeled after powerpc version
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdt_support.h>
#ifdef CONFIG_ARMV7_NONSEC
#include <asm/armv7.h>
#endif
#include <asm/psci.h>
#include <asm/spin_table.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_fixup_fdt(void *blob)
{
	bd_t *bd = gd->bd;
	int bank, ret;
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = bd->bi_dram[bank].start;
		size[bank] = bd->bi_dram[bank].size;
#ifdef CONFIG_ARMV7_NONSEC
		ret = armv7_apply_memory_carveout(&start[bank], &size[bank]);
		if (ret)
			return ret;
#endif
	}

	ret = fdt_fixup_memory_banks(blob, start, size, CONFIG_NR_DRAM_BANKS);
	if (ret)
		return ret;

#ifdef CONFIG_ARMV8_SPIN_TABLE
	ret = spin_table_update_dt(blob);
	if (ret)
		return ret;
#endif

#if defined(CONFIG_ARMV7_NONSEC) || defined(CONFIG_ARMV8_PSCI) || \
	defined(CONFIG_SEC_FIRMWARE_ARMV8_PSCI)
	ret = psci_update_dt(blob);
	if (ret)
		return ret;
#endif

	return 0;
}
