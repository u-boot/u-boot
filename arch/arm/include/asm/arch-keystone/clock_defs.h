/*
 * keystone2: common pll clock definitions
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _CLOCK_DEFS_H_
#define _CLOCK_DEFS_H_

#include <asm/arch/hardware.h>

#define BIT(x)			(1 << (x))

/* PLL Control Registers */
struct pllctl_regs {
	u32	ctl;		/* 00 */
	u32	ocsel;		/* 04 */
	u32	secctl;		/* 08 */
	u32	resv0;
	u32	mult;		/* 10 */
	u32	prediv;		/* 14 */
	u32	div1;		/* 18 */
	u32	div2;		/* 1c */
	u32	div3;		/* 20 */
	u32	oscdiv1;	/* 24 */
	u32	resv1;		/* 28 */
	u32	bpdiv;		/* 2c */
	u32	wakeup;		/* 30 */
	u32	resv2;
	u32	cmd;		/* 38 */
	u32	stat;		/* 3c */
	u32	alnctl;		/* 40 */
	u32	dchange;	/* 44 */
	u32	cken;		/* 48 */
	u32	ckstat;		/* 4c */
	u32	systat;		/* 50 */
	u32	ckctl;		/* 54 */
	u32	resv3[2];
	u32	div4;		/* 60 */
	u32	div5;		/* 64 */
	u32	div6;		/* 68 */
	u32	div7;		/* 6c */
	u32	div8;		/* 70 */
	u32	div9;		/* 74 */
	u32	div10;		/* 78 */
	u32	div11;		/* 7c */
	u32	div12;		/* 80 */
};

static struct pllctl_regs *pllctl_regs[] = {
	(struct pllctl_regs *)(CLOCK_BASE + 0x100)
};

#define pllctl_reg(pll, reg)            (&(pllctl_regs[pll]->reg))
#define pllctl_reg_read(pll, reg)       __raw_readl(pllctl_reg(pll, reg))
#define pllctl_reg_write(pll, reg, val) __raw_writel(val, pllctl_reg(pll, reg))

#define pllctl_reg_rmw(pll, reg, mask, val) \
	pllctl_reg_write(pll, reg, \
		(pllctl_reg_read(pll, reg) & ~(mask)) | val)

#define pllctl_reg_setbits(pll, reg, mask) \
	pllctl_reg_rmw(pll, reg, 0, mask)

#define pllctl_reg_clrbits(pll, reg, mask) \
	pllctl_reg_rmw(pll, reg, mask, 0)

#define pll0div_read(N) ((pllctl_reg_read(CORE_PLL, div##N) & 0xff) + 1)

/* PLLCTL Bits */
#define PLLCTL_BYPASS           BIT(23)
#define PLL_PLLRST              BIT(14)
#define PLLCTL_PAPLL            BIT(13)
#define PLLCTL_CLKMODE          BIT(8)
#define PLLCTL_PLLSELB          BIT(7)
#define PLLCTL_ENSAT            BIT(6)
#define PLLCTL_PLLENSRC         BIT(5)
#define PLLCTL_PLLDIS           BIT(4)
#define PLLCTL_PLLRST           BIT(3)
#define PLLCTL_PLLPWRDN         BIT(1)
#define PLLCTL_PLLEN            BIT(0)
#define PLLSTAT_GO              BIT(0)

#define MAIN_ENSAT_OFFSET       6

#define PLLDIV_ENABLE           BIT(15)

#define PLL_DIV_MASK            0x3f
#define PLL_MULT_MASK           0x1fff
#define PLL_MULT_SHIFT          6
#define PLLM_MULT_HI_MASK       0x7f
#define PLLM_MULT_HI_SHIFT      12
#define PLLM_MULT_HI_SMASK      (PLLM_MULT_HI_MASK << PLLM_MULT_HI_SHIFT)
#define PLLM_MULT_LO_MASK       0x3f
#define PLL_CLKOD_MASK          0xf
#define PLL_CLKOD_SHIFT         19
#define PLL_CLKOD_SMASK         (PLL_CLKOD_MASK << PLL_CLKOD_SHIFT)
#define PLL_BWADJ_LO_MASK       0xff
#define PLL_BWADJ_LO_SHIFT      24
#define PLL_BWADJ_LO_SMASK      (PLL_BWADJ_LO_MASK << PLL_BWADJ_LO_SHIFT)
#define PLL_BWADJ_HI_MASK       0xf

#define PLLM_RATIO_DIV1         (PLLDIV_ENABLE | 0)
#define PLLM_RATIO_DIV2         (PLLDIV_ENABLE | 0)
#define PLLM_RATIO_DIV3         (PLLDIV_ENABLE | 1)
#define PLLM_RATIO_DIV4         (PLLDIV_ENABLE | 4)
#define PLLM_RATIO_DIV5         (PLLDIV_ENABLE | 17)

#endif  /* _CLOCK_DEFS_H_ */
