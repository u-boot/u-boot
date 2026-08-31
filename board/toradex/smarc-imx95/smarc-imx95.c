// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (C) 2025 Toradex */

#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <errno.h>
#include <fdt_support.h>
#include <init.h>

#include "../common/tdx-cfg-block.h"

/*
 * The address wrap-around is not linear: address bits above the module
 * capacity alias back to the base with some low address bits XORed, as
 * measured on 4GB and 8GB modules (bit 32 -> XOR 0xc000, bit 33 -> XOR
 * 0x2000).
 */
static const struct ram_alias_check ram_alias_checks[] = {
	{ (void *)((uintptr_t)PHYS_SDRAM + SZ_4G), (void *)((uintptr_t)PHYS_SDRAM + 0xc000), SZ_8G },
	{ (void *)((uintptr_t)PHYS_SDRAM + SZ_2G), (void *)(PHYS_SDRAM), SZ_4G },
	{ NULL }
};

int board_phys_sdram_size(phys_size_t *size)
{
	phys_size_t sz;

	sz = probe_ram_size_by_alias(ram_alias_checks);
	if (!sz) {
		puts("## WARNING: Less than 4GB RAM detected\n");
		return -EINVAL;
	}

	*size = sz - PHYS_SDRAM_FW_RSVD;

	return 0;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT) && IS_ENABLED(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif
