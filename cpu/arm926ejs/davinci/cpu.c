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

#ifdef CONFIG_SOC_DM6447
#define ARM_PLLDIV	PLLC_PLLDIV2
#define DSP_PLLDIV	PLLC_PLLDIV1
#define DDR_PLLDIV	PLLC_PLLDIV1
#endif


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
	unsigned	base = CONFIG_SYS_HZ_CLOCK / 1000;

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
