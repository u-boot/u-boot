// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch-rockchip/grf_rk3568.h>
#include <asm/arch-rockchip/hardware.h>
#include <dt-bindings/clock/rk3568-cru.h>

#define PMUGRF_BASE		0xfdc20000
#define GRF_BASE		0xfdc60000

/* PMU_GRF_GPIO0D_IOMUX_L */
enum {
	GPIO0D1_SHIFT		= 4,
	GPIO0D1_MASK		= GENMASK(6, 4),
	GPIO0D1_GPIO		= 0,
	GPIO0D1_UART2_TXM0,

	GPIO0D0_SHIFT		= 0,
	GPIO0D0_MASK		= GENMASK(2, 0),
	GPIO0D0_GPIO		= 0,
	GPIO0D0_UART2_RXM0,
};

/* GRF_IOFUNC_SEL3 */
enum {
	UART2_IO_SEL_SHIFT	= 10,
	UART2_IO_SEL_MASK	= GENMASK(11, 10),
	UART2_IO_SEL_M0		= 0,
};

static struct mm_region rk3568_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xf0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x300000000,
		.phys = 0x300000000,
		.size = 0x0c0c00000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3568_mem_map;

void board_debug_uart_init(void)
{
	static struct rk3568_pmugrf * const pmugrf = (void *)PMUGRF_BASE;
	static struct rk3568_grf * const grf = (void *)GRF_BASE;

	/* UART2 M0 */
	rk_clrsetreg(&grf->iofunc_sel3, UART2_IO_SEL_MASK,
		     UART2_IO_SEL_M0 << UART2_IO_SEL_SHIFT);

	/* Switch iomux */
	rk_clrsetreg(&pmugrf->pmu_gpio0d_iomux_l,
		     GPIO0D1_MASK | GPIO0D0_MASK,
		     GPIO0D1_UART2_TXM0 << GPIO0D1_SHIFT |
		     GPIO0D0_UART2_RXM0 << GPIO0D0_SHIFT);
}

int arch_cpu_init(void)
{
	return 0;
}
