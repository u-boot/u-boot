/*
 * Copyright (C) 2004 Texas Instruments.
 * Copyright (C) 2009 David Brownell
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

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

int set_cpu_clk_info(void)
{
	gd->bd->bi_arm_freq = clk_get(DAVINCI_ARM_CLKID) / 1000000;
	/* DDR PHY uses an x2 input clock */
	gd->bd->bi_ddr_freq = cpu_is_da830() ? 0 :
				(clk_get(DAVINCI_DDR_CLKID) / 1000000);
	gd->bd->bi_dsp_freq = 0;
	return 0;
}

#else /* CONFIG_SOC_DA8XX */

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
#elif defined(CONFIG_SOC_DM365)
	return pll_div(pllbase, PLLC_PREDIV);
#endif
	return 1;
}

static inline unsigned pll_postdiv(volatile void *pllbase)
{
#if defined(CONFIG_SOC_DM355) || defined(CONFIG_SOC_DM365)
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
	unsigned	base = CONFIG_REFCLK_FREQ / 1000;
#else
	unsigned	base = CONFIG_SYS_HZ_CLOCK / 1000;
#endif

	/* the PLL might be bypassed */
	if (readl(pllbase + PLLC_PLLCTL) & BIT(0)) {
		base /= pll_prediv(pllbase);
#if defined(CONFIG_SOC_DM365)
		base *=  2 * (readl(pllbase + PLLC_PLLM) & 0x0ff);
#else
		base *= 1 + (REG(pllbase + PLLC_PLLM) & 0x0ff);
#endif
		base /= pll_postdiv(pllbase);
	}
	return DIV_ROUND_UP(base, 1000 * pll_div(pllbase, div));
}

#ifdef DAVINCI_DM6467EVM
unsigned int davinci_arm_clk_get()
{
	return pll_sysclk_mhz(DAVINCI_PLL_CNTRL0_BASE, ARM_PLLDIV) * 1000000;
}
#endif

#if defined(CONFIG_SOC_DM365)
unsigned int davinci_clk_get(unsigned int div)
{
	return pll_sysclk_mhz(DAVINCI_PLL_CNTRL0_BASE, div) * 1000000;
}
#endif

int set_cpu_clk_info(void)
{
	unsigned int pllbase = DAVINCI_PLL_CNTRL0_BASE;
#if defined(CONFIG_SOC_DM365)
	pllbase = DAVINCI_PLL_CNTRL1_BASE;
#endif
	gd->bd->bi_arm_freq = pll_sysclk_mhz(pllbase, ARM_PLLDIV);

#ifdef DSP_PLLDIV
	gd->bd->bi_dsp_freq =
		pll_sysclk_mhz(DAVINCI_PLL_CNTRL0_BASE, DSP_PLLDIV);
#else
	gd->bd->bi_dsp_freq = 0;
#endif

	pllbase = DAVINCI_PLL_CNTRL1_BASE;
#if defined(CONFIG_SOC_DM365)
	pllbase = DAVINCI_PLL_CNTRL0_BASE;
#endif
	gd->bd->bi_ddr_freq = pll_sysclk_mhz(pllbase, DDR_PLLDIV) / 2;

	return 0;
}

#endif /* !CONFIG_SOC_DA8XX */

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
