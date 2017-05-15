/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 * Copyright (c) 2016 Andreas Färber
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3368.h>
#include <asm/arch/grf_rk3368.h>
#include <syscon.h>

#define IMEM_BASE                  0xFF8C0000

/* Max MCU's SRAM value is 8K, begin at (IMEM_BASE + 4K) */
#define MCU_SRAM_BASE			(IMEM_BASE + 1024 * 4)
#define MCU_SRAM_BASE_BIT31_BIT28	((MCU_SRAM_BASE & GENMASK(31, 28)) >> 28)
#define MCU_SRAM_BASE_BIT27_BIT12	((MCU_SRAM_BASE & GENMASK(27, 12)) >> 12)
/* exsram may using by mcu to accessing dram(0x0-0x20000000) */
#define MCU_EXSRAM_BASE    (0)
#define MCU_EXSRAM_BASE_BIT31_BIT28       ((MCU_EXSRAM_BASE & GENMASK(31, 28)) >> 28)
#define MCU_EXSRAM_BASE_BIT27_BIT12       ((MCU_EXSRAM_BASE & GENMASK(27, 12)) >> 12)
/* experi no used, reserved value = 0 */
#define MCU_EXPERI_BASE    (0)
#define MCU_EXPERI_BASE_BIT31_BIT28       ((MCU_EXPERI_BASE & GENMASK(31, 28)) >> 28)
#define MCU_EXPERI_BASE_BIT27_BIT12       ((MCU_EXPERI_BASE & GENMASK(27, 12)) >> 12)

static struct mm_region rk3368_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
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
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3368_mem_map;

#ifdef CONFIG_ARCH_EARLY_INIT_R
static int mcu_init(void)
{
	struct rk3368_grf *grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	struct rk3368_cru *cru = rockchip_get_cru();

	rk_clrsetreg(&grf->soc_con14, MCU_SRAM_BASE_BIT31_BIT28_MASK,
		     MCU_SRAM_BASE_BIT31_BIT28 << MCU_SRAM_BASE_BIT31_BIT28_SHIFT);
	rk_clrsetreg(&grf->soc_con11, MCU_SRAM_BASE_BIT27_BIT12_MASK,
		     MCU_SRAM_BASE_BIT27_BIT12 << MCU_SRAM_BASE_BIT27_BIT12_SHIFT);
	rk_clrsetreg(&grf->soc_con14, MCU_EXSRAM_BASE_BIT31_BIT28_MASK,
		     MCU_EXSRAM_BASE_BIT31_BIT28 << MCU_EXSRAM_BASE_BIT31_BIT28_SHIFT);
	rk_clrsetreg(&grf->soc_con12, MCU_EXSRAM_BASE_BIT27_BIT12_MASK,
		     MCU_EXSRAM_BASE_BIT27_BIT12 << MCU_EXSRAM_BASE_BIT27_BIT12_SHIFT);
	rk_clrsetreg(&grf->soc_con14, MCU_EXPERI_BASE_BIT31_BIT28_MASK,
		     MCU_EXPERI_BASE_BIT31_BIT28 << MCU_EXPERI_BASE_BIT31_BIT28_SHIFT);
	rk_clrsetreg(&grf->soc_con13, MCU_EXPERI_BASE_BIT27_BIT12_MASK,
		     MCU_EXPERI_BASE_BIT27_BIT12 << MCU_EXPERI_BASE_BIT27_BIT12_SHIFT);

	rk_clrsetreg(&cru->clksel_con[12], MCU_PLL_SEL_MASK | MCU_CLK_DIV_MASK,
		     (MCU_PLL_SEL_GPLL << MCU_PLL_SEL_SHIFT) |
		     (5 << MCU_CLK_DIV_SHIFT));

	 /* mcu dereset, for start running */
	rk_clrreg(&cru->softrst_con[1], MCU_PO_SRST_MASK | MCU_SYS_SRST_MASK);

	return 0;
}

int arch_early_init_r(void)
{
	return mcu_init();
}
#endif
