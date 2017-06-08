/*
 * Copyright (c) 2017 Andy Yan
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <fdtdec.h>
#include <asm/arch/clock.h>
#include <asm/arch/grf_rk3368.h>

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	struct rk3368_pmu_grf *pmugrf;
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1, "rockchip,rk3368-pmugrf");
	pmugrf = (struct rk3368_pmu_grf *)fdtdec_get_addr(gd->fdt_blob, node, "reg");

	rk_clrsetreg(&pmugrf->gpio0d_iomux,
		     GPIO0D0_MASK | GPIO0D1_MASK |
		     GPIO0D2_MASK | GPIO0D3_MASK,
		     GPIO0D0_GPIO << GPIO0D0_SHIFT |
		     GPIO0D1_GPIO << GPIO0D1_SHIFT |
		     GPIO0D2_UART4_SOUT << GPIO0D2_SHIFT |
		     GPIO0D3_UART4_SIN << GPIO0D3_SHIFT);
	return 0;
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = 0x40000000;

	return 0;
}

int dram_init_banksize(void)
{
	 /* Reserve 0x200000 for ATF bl31 */
	gd->bd->bi_dram[0].start = 0x200000;
	gd->bd->bi_dram[0].size = 0x3fe00000;

	return 0;
}
