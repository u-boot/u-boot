/*
 * Copyright (C) 2004 Texas Instruments.
 * Copyright (C) 2009 David Brownell
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

/* offsets from PLL controller base */
#define PLLC_PLLCTL	0x100
#define PLLC_PLLM	0x110
#define PLLC_PREDIV	0x114
#define PLLC_PLLDIV1	0x118
#define PLLC_PLLDIV2	0x11c
#define PLLC_PLLDIV3	0x120
#define PLLC_POSTDIV	0x128
#define PLLC_BPDIV	0x12c
#define PLLC_PLLDIV4	0x160
#define PLLC_PLLDIV5	0x164
#define PLLC_PLLDIV6	0x168
#define PLLC_PLLDIV7	0x16c
#define PLLC_PLLDIV8	0x170
#define PLLC_PLLDIV9	0x174

#define BIT(x)		(1 << (x))

/* SOC-specific pll info */
#ifdef CONFIG_SOC_DM355
#define ARM_PLLDIV	PLLC_PLLDIV1
#define DDR_PLLDIV	PLLC_PLLDIV1
#endif

#ifdef CONFIG_SOC_DM644X
#define ARM_PLLDIV	PLLC_PLLDIV2
#define DSP_PLLDIV	PLLC_PLLDIV1
#define DDR_PLLDIV	PLLC_PLLDIV2
#endif

#ifdef CONFIG_SOC_DM646X
#define DSP_PLLDIV	PLLC_PLLDIV1
#define ARM_PLLDIV	PLLC_PLLDIV2
#define DDR_PLLDIV	PLLC_PLLDIV1
#endif

#ifdef CONFIG_SOC_DA8XX
unsigned int sysdiv[9] = {
	PLLC_PLLDIV1, PLLC_PLLDIV2, PLLC_PLLDIV3, PLLC_PLLDIV4, PLLC_PLLDIV5,
	PLLC_PLLDIV6, PLLC_PLLDIV7, PLLC_PLLDIV8, PLLC_PLLDIV9
};

int clk_get(enum davinci_clk_ids id)
{
	int pre_div;
	int pllm;
	int post_div;
	int pll_out;
	unsigned int pll_base;

	pll_out = CONFIG_SYS_OSCIN_FREQ;

	if (id == DAVINCI_AUXCLK_CLKID)
		goto out;

	if ((id >> 16) == 1)
		pll_base = (unsigned int)davinci_pllc1_regs;
	else
		pll_base = (unsigned int)davinci_pllc0_regs;

	id &= 0xFFFF;

	/*
	 * Lets keep this simple. Combining operations can result in
	 * unexpected approximations
	 */
	pre_div = (readl(pll_base + PLLC_PREDIV) &
		DAVINCI_PLLC_DIV_MASK) + 1;
	pllm = readl(pll_base + PLLC_PLLM) + 1;

	pll_out /= pre_div;
	pll_out *= pllm;

	if (id == DAVINCI_PLLM_CLKID)
		goto out;

	post_div = (readl(pll_base + PLLC_POSTDIV) &
		DAVINCI_PLLC_DIV_MASK) + 1;

	pll_out /= post_div;

	if (id == DAVINCI_PLLC_CLKID)
		goto out;

	pll_out /= (readl(pll_base + sysdiv[id - 1]) &
		DAVINCI_PLLC_DIV_MASK) + 1;

out:
	return pll_out;
}
#endif /* CONFIG_SOC_DA8XX */

#ifdef CONFIG_DISPLAY_CPUINFO

static unsigned pll_div(volatile void *pllbase, unsigned offset)
{
	u32	div;

	div = REG(pllbase + offset);
	return (div & BIT(15)) ? (1 + (div & 0x1f)) : 1;
}

static inline unsigned pll_prediv(volatile void *pllbase)
{
#ifdef CONFIG_SOC_DM355
	/* this register read seems to fail on pll0 */
	if (pllbase == (volatile void *)DAVINCI_PLL_CNTRL0_BASE)
		return 8;
	else
		return pll_div(pllbase, PLLC_PREDIV);
#endif
	return 1;
}

static inline unsigned pll_postdiv(volatile void *pllbase)
{
#ifdef CONFIG_SOC_DM355
	return pll_div(pllbase, PLLC_POSTDIV);
#elif defined(CONFIG_SOC_DM6446)
	if (pllbase == (volatile void *)DAVINCI_PLL_CNTRL0_BASE)
		return pll_div(pllbase, PLLC_POSTDIV);
#endif
	return 1;
}

static unsigned pll_sysclk_mhz(unsigned pll_addr, unsigned div)
{
	volatile void	*pllbase = (volatile void *) pll_addr;
#ifdef CONFIG_SOC_DM646X
	unsigned	base = CFG_REFCLK_FREQ / 1000;
#else
	unsigned	base = CONFIG_SYS_HZ_CLOCK / 1000;
#endif

	/* the PLL might be bypassed */
	if (REG(pllbase + PLLC_PLLCTL) & BIT(0)) {
		base /= pll_prediv(pllbase);
		base *= 1 + (REG(pllbase + PLLC_PLLM) & 0x0ff);
		base /= pll_postdiv(pllbase);
	}
	return DIV_ROUND_UP(base, 1000 * pll_div(pllbase, div));
}

int print_cpuinfo(void)
{
	/* REVISIT fetch and display CPU ID and revision information
	 * too ... that will matter as more revisions appear.
	 */
	printf("Cores: ARM %d MHz",
			pll_sysclk_mhz(DAVINCI_PLL_CNTRL0_BASE, ARM_PLLDIV));

#ifdef DSP_PLLDIV
	printf(", DSP %d MHz",
			pll_sysclk_mhz(DAVINCI_PLL_CNTRL0_BASE, DSP_PLLDIV));
#endif

	printf("\nDDR:   %d MHz\n",
			/* DDR PHY uses an x2 input clock */
			pll_sysclk_mhz(DAVINCI_PLL_CNTRL1_BASE, DDR_PLLDIV)
				/ 2);
	return 0;
}

#ifdef DAVINCI_DM6467EVM
unsigned int davinci_arm_clk_get()
{
	return pll_sysclk_mhz(DAVINCI_PLL_CNTRL0_BASE, ARM_PLLDIV) * 1000000;
}
#endif
#endif

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_DRIVER_TI_EMAC)
	davinci_emac_initialize();
#endif
	return 0;
}
