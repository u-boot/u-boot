// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <asm/io.h>
#include <linux/types.h>
#include "clkinit.h"
#include "dmcinit.h"

#ifdef CONFIG_CGU0_SCLK0_DIV
	#define VAL_CGU0_SCLK0_DIV CONFIG_CGU0_SCLK0_DIV
#else
	#define VAL_CGU0_SCLK0_DIV 1
#endif
#ifdef CONFIG_CGU0_SCLK1_DIV
	#define VAL_CGU0_SCLK1_DIV CONFIG_CGU0_SCLK1_DIV
#else
	#define VAL_CGU0_SCLK1_DIV 1
#endif
#ifdef CONFIG_CGU0_DIV_S0SELEX
	#define VAL_CGU0_DIV_S0SELEX CONFIG_CGU0_DIV_S0SELEX
#else
	#define VAL_CGU0_DIV_S0SELEX -1
#endif
#ifdef CONFIG_CGU0_DIV_S1SELEX
	#define VAL_CGU0_DIV_S1SELEX CONFIG_CGU0_DIV_S1SELEX
#else
	#define VAL_CGU0_DIV_S1SELEX -1
#endif
#ifdef CONFIG_CGU0_CLKOUTSEL
	#define VAL_CGU0_CLKOUTSEL CONFIG_CGU0_CLKOUTSEL
#else
	#define VAL_CGU0_CLKOUTSEL -1
#endif
#ifdef CONFIG_CGU1_SCLK0_DIV
	#define VAL_CGU1_SCLK0_DIV CONFIG_CGU1_SCLK0_DIV
#else
	#define VAL_CGU1_SCLK0_DIV 1
#endif
#ifdef CONFIG_CGU1_SCLK1_DIV
	#define VAL_CGU1_SCLK1_DIV CONFIG_CGU1_SCLK1_DIV
#else
	#define VAL_CGU1_SCLK1_DIV 1
#endif
#ifdef CONFIG_CGU1_DIV_S0SELEX
	#define VAL_CGU1_DIV_S0SELEX CONFIG_CGU1_DIV_S0SELEX
#else
	#define VAL_CGU1_DIV_S0SELEX -1
#endif
#ifdef CONFIG_CGU1_DIV_S1SELEX
	#define VAL_CGU1_DIV_S1SELEX CONFIG_CGU1_DIV_S1SELEX
#else
	#define VAL_CGU1_DIV_S1SELEX -1
#endif
#ifdef CONFIG_CGU1_CLKOUTSEL
	#define VAL_CGU1_CLKOUTSEL CONFIG_CGU1_CLKOUTSEL
#else
	#define VAL_CGU1_CLKOUTSEL -1
#endif

#define REG_MISC_REG10_tst_addr     0x310A902C

#define CGU0_REGBASE	0x3108D000
#define CGU1_REGBASE	0x3108E000

#define CGU_CTL		0x00 // CGU0 Control Register
#define CGU_PLLCTL	0x04 // CGU0 PLL Control Register
#define CGU_STAT	0x08 // CGU0 Status Register
#define CGU_DIV		0x0C // CGU0 Clocks Divisor Register
#define CGU_CLKOUTSEL	0x10 // CGU0 CLKOUT Select Register
#define CGU_DIVEX	0x40 // CGU0 DIV Register Extension

#define BITP_CGU_DIV_OSEL                   22    // OUTCLK Divisor
#define BITP_CGU_DIV_DSEL                   16    // DCLK Divisor
#define BITP_CGU_DIV_S1SEL                  13    // SCLK 1 Divisor
#define BITP_CGU_DIV_SYSSEL                  8    // SYSCLK Divisor
#define BITP_CGU_DIV_S0SEL                   5    // SCLK 0 Divisor
#define BITP_CGU_DIV_CSEL                    0    // CCLK Divisor

#define BITP_CGU_CTL_MSEL                    8    // Multiplier Select
#define BITP_CGU_CTL_DF                      0    // Divide Frequency

#define BITM_CGU_STAT_CLKSALGN      0x00000008
#define BITM_CGU_STAT_PLOCK         0x00000004
#define BITM_CGU_STAT_PLLBP         0x00000002
#define BITM_CGU_STAT_PLLEN         0x00000001

/*  PLL Multiplier and Divisor Selections (Required Value, Bit Position) */
/* PLL Multiplier Select */
#define MSEL(X)		(((X) << BITP_CGU_CTL_MSEL) & \
				 BITM_CGU_CTL_MSEL)
/* Divide frequency[true or false] */
#define DF(X)		(((X) << BITP_CGU_CTL_DF) & \
				 BITM_CGU_CTL_DF)
/* Core Clock Divisor Select */
#define CSEL(X)		(((X) << BITP_CGU_DIV_CSEL) & \
				 BITM_CGU_DIV_CSEL)
/* System Clock Divisor Select */
#define SYSSEL(X)	(((X) << BITP_CGU_DIV_SYSSEL) & \
				 BITM_CGU_DIV_SYSSEL)
/* SCLK0 Divisor Select  */
#define S0SEL(X)	(((X) << BITP_CGU_DIV_S0SEL) & \
				 BITM_CGU_DIV_S0SEL)
/* SCLK1 Divisor Select  */
#define S1SEL(X)	(((X) << BITP_CGU_DIV_S1SEL) & \
				 BITM_CGU_DIV_S1SEL)
/* DDR Clock Divisor Select */
#define DSEL(X)		(((X) << BITP_CGU_DIV_DSEL) & \
				 BITM_CGU_DIV_DSEL)
/* OUTCLK Divisor Select */
#define OSEL(X)		(((X) << BITP_CGU_DIV_OSEL) & \
				 BITM_CGU_DIV_OSEL)
/* CLKOUT select	*/
#define CLKOUTSEL(X)	(((X) << BITP_CGU_CLKOUTSEL_CLKOUTSEL) & \
				 BITM_CGU_CLKOUTSEL_CLKOUTSEL)
#define S0SELEX(X)	(((X) << BITP_CGU_DIVEX_S0SELEX) & \
				 BITM_CGU_DIVEX_S0SELEX)
#define S1SELEX(X)	(((X) << BITP_CGU_DIVEX_S1SELEX) & \
				 BITM_CGU_DIVEX_S1SELEX)

struct CGU_Settings {
	phys_addr_t rbase;
	u32 ctl_MSEL:7;
	u32 ctl_DF:1;
	u32 div_CSEL:5;
	u32 div_SYSSEL:5;
	u32 div_S0SEL:3;
	u32 div_S1SEL:3;
	u32 div_DSEL:5;
	u32 div_OSEL:7;
	s16 divex_S0SELEX;
	s16 divex_S1SELEX;
	s8  clkoutsel;
};

/* CGU Registers */
#define BITM_CGU_CTL_LOCK	0x80000000 /* Lock */

#define BITM_CGU_CTL_MSEL	0x00007F00 /* Multiplier Select */
#define BITM_CGU_CTL_DF		0x00000001 /* Divide Frequency */
#define BITM_CGU_CTL_S1SELEXEN	0x00020000 /* SCLK1 Extension Divider Enable */
#define BITM_CGU_CTL_S0SELEXEN	0x00010000 /* SCLK0 Extension Divider Enable */

#define BITM_CGU_DIV_LOCK	0x80000000 /* Lock */
#define BITM_CGU_DIV_UPDT	0x40000000 /* Update Clock Divisors */
#define BITM_CGU_DIV_ALGN	0x20000000 /* Align */
#define BITM_CGU_DIV_OSEL	0x1FC00000 /* OUTCLK Divisor */
#define BITM_CGU_DIV_DSEL	0x001F0000 /* DCLK Divisor */
#define BITM_CGU_DIV_S1SEL	0x0000E000 /* SCLK 1 Divisor */
#define BITM_CGU_DIV_SYSSEL	0x00001F00 /* SYSCLK Divisor */
#define BITM_CGU_DIV_S0SEL	0x000000E0 /* SCLK 0 Divisor */
#define BITM_CGU_DIV_CSEL	0x0000001F /* CCLK Divisor */

#define BITP_CGU_DIVEX_S0SELEX	0
#define BITM_CGU_DIVEX_S0SELEX	0x000000FF /*  SCLK 0 Extension Divisor */

#define BITP_CGU_DIVEX_S1SELEX	16
#define BITM_CGU_DIVEX_S1SELEX	0x00FF0000 /*  SCLK 1 Extension Divisor */

#define BITM_CGU_PLLCTL_PLLEN		0x00000008	/* PLL Enable */
#define BITM_CGU_PLLCTL_PLLBPCL		0x00000002	/* PLL Bypass Clear */
#define BITM_CGU_PLLCTL_PLLBPST		0x00000001	/* PLL Bypass Set */

#define BITP_CGU_CLKOUTSEL_CLKOUTSEL	0		/* CLKOUT Select */
#define BITM_CGU_CLKOUTSEL_CLKOUTSEL	0x0000001F	/* CLKOUT Select */

#define CGU_STAT_MASK (BITM_CGU_STAT_PLLEN | BITM_CGU_STAT_PLOCK | \
	    BITM_CGU_STAT_CLKSALGN)
#define CGU_STAT_ALGN_LOCK (BITM_CGU_STAT_PLLEN | BITM_CGU_STAT_PLOCK)

/* Clock Distribution Unit Registers */
#define REG_CDU0_CFG0			0x3108F000
#define REG_CDU0_CFG1			0x3108F004
#define REG_CDU0_CFG2			0x3108F008
#define REG_CDU0_CFG3			0x3108F00C
#define REG_CDU0_CFG4			0x3108F010
#define REG_CDU0_CFG5			0x3108F014
#define REG_CDU0_CFG6			0x3108F018
#define REG_CDU0_CFG7			0x3108F01C
#define REG_CDU0_CFG8			0x3108F020
#define REG_CDU0_CFG9			0x3108F024
#define REG_CDU0_CFG10			0x3108F028
#define REG_CDU0_CFG11			0x3108F02C
#define REG_CDU0_CFG12			0x3108F030
#define REG_CDU0_CFG13			0x3108F034
#define REG_CDU0_CFG14			0x3108F038
#define REG_CDU0_STAT			0x3108F040
#define REG_CDU0_CLKINSEL		0x3108F044
#define REG_CDU0_REVID			0x3108F048

#define BITM_REG10_MSEL3		0x000007F0
#define BITP_REG10_MSEL3		4

#define BITM_REG10_DSEL3		0x0001F000
#define BITP_REG10_DSEL3		12

/* Selected clock macros */
#define CGUn_MULT(cgu)		((CONFIG_CGU##cgu##_VCO_MULT == 0) ? \
				 128 : CONFIG_CGU##cgu##_VCO_MULT)
#define CGUn_DIV(clkname, cgu)	((CONFIG_CGU##cgu##_##clkname##_DIV == 0) ? \
				 32 : CONFIG_CGU##cgu##_##clkname##_DIV)
#define CCLK1_n_RATIO(cgu)	(((CGUn_MULT(cgu)) / \
				  (1 + CONFIG_CGU##cgu##_DF_DIV)) / \
				   CGUn_DIV(CCLK, cgu))
#define CCLK2_n_RATIO(cgu)	(((CGUn_MULT(cgu) * 2) / 3) / \
				  (1 + CONFIG_CGU##cgu##_DF_DIV))
#define DCLK_n_RATIO(cgu)	(((CGUn_MULT(cgu)) / \
				 (1 + CONFIG_CGU##cgu##_DF_DIV)) / \
				  CGUn_DIV(DCLK, cgu))
#define SYSCLK_n_RATIO(cgu)	(((CGUn_MULT(cgu)) / \
				 (1 + CONFIG_CGU##cgu##_DF_DIV)) / \
				  CGUn_DIV(SCLK, cgu))
#define PLL3_RATIO		((CONFIG_CGU1_PLL3_VCO_MSEL) / \
				 (CONFIG_CGU1_PLL3_DCLK_DIV))

#if (1 == CONFIG_CDU0_CLKO2)
	#define ARMCLK_IN	0
	#define ARMCLK_RATIO	CCLK1_n_RATIO(0)
#elif (3 == CONFIG_CDU0_CLKO2) && \
	(defined(CONFIG_SC57X) || defined(CONFIG_SC58X))
	#define ARMCLK_IN	0
	#define ARMCLK_RATIO	SYSCLK_n_RATIO(0)
#elif (5 == CONFIG_CDU0_CLKO2) && defined(CONFIG_SC59X_64)
	#define ARMCLK_IN	0
	#define ARMCLK_RATIO	CCLK2_n_RATIO(0)
#elif (7 == CONFIG_CDU0_CLKO2) && defined(CONFIG_SC59X_64)
	#define ARMCLK_IN	CDU0_CGU1_CLKIN
	#define ARMCLK_RATIO	CCLK2_n_RATIO(1)
#endif

#ifdef CONFIG_CGU1_PLL3_DDRCLK
	#define DDRCLK_IN	CDU0_CGU1_CLKIN
	#define DDRCLK_RATIO	PLL3_RATIO
#elif (1 == CONFIG_CDU0_CLKO3)
	#define DDRCLK_IN	0
	#define DDRCLK_RATIO	DCLK_n_RATIO(0)
#elif (3 == CONFIG_CDU0_CLKO3)
	#define DDRCLK_IN	CDU0_CGU1_CLKIN
	#define DDRCLK_RATIO	DCLK_n_RATIO(1)
#endif

#ifndef ARMCLK_RATIO
	#error Invalid/unknown ARMCLK selection!
#endif
#ifndef DDRCLK_RATIO
	#error Invalid/unknown DDRCLK selection!
#endif

#define ARMDDR_CLK_RATIO_FPERCISION 1000

#if ARMCLK_IN != DDRCLK_IN
	#ifndef CUSTOM_ARMDDR_CLK_RATIO
		/**
		 * SYS_CLKINx are defined within the device tree, not configs.
		 * Thus, we can only determine cross-CGU clock ratios if they
		 * use the same SYS_CLKINx.
		 */
		#error Define CUSTOM_ARMDDR_CLK_RATIO for different SYS_CLKINs
	#else
		#define ARMDDR_CLK_RATIO CUSTOM_ARMDDR_CLK_RATIO
	#endif
#else
	#define ARMDDR_CLK_RATIO (ARMDDR_CLK_RATIO_FPERCISION *\
				   ARMCLK_RATIO / DDRCLK_RATIO)
#endif

void dmcdelay(uint32_t delay)
{
	/* There is no zero-overhead loop on ARM, so assume each iteration
	 * takes 4 processor cycles (based on examination of -O3 and -Ofast
	 * output).
	 */
	u32 i, remainder;

	/* Convert DDR cycles to core clock cycles */
	u32 f = delay * ARMDDR_CLK_RATIO;

	delay = f + 500;
	delay /= ARMDDR_CLK_RATIO_FPERCISION;

	/* Round up to multiple of 4 */
	remainder = delay % 4;
	if (remainder != 0u)
		delay += (4u - remainder);

	for (i = 0; i < delay; i += 4)
		asm("nop");
}

static void program_cgu(const struct CGU_Settings *cgu)
{
	const uintptr_t b = cgu->rbase;
	const bool use_extension0 = cgu->divex_S0SELEX >= 0;
	const bool use_extension1 = cgu->divex_S1SELEX >= 0;
	u32 temp;

	temp =  OSEL(cgu->div_OSEL);
	temp |= SYSSEL(cgu->div_SYSSEL);
	temp |= CSEL(cgu->div_CSEL);
	temp |= DSEL(cgu->div_DSEL);
	temp |= (S0SEL(cgu->div_S0SEL));
	temp |= (S1SEL(cgu->div_S1SEL));
	temp &= ~BITM_CGU_DIV_LOCK;

	//Put PLL in to Bypass Mode
	writel(BITM_CGU_PLLCTL_PLLEN | BITM_CGU_PLLCTL_PLLBPST,
	       b + CGU_PLLCTL);
	while (!(readl(b + CGU_STAT) & BITM_CGU_STAT_PLLBP))
		;

	while (!((readl(b + CGU_STAT) & CGU_STAT_MASK) == CGU_STAT_ALGN_LOCK))
		;

	dmcdelay(1000);

	writel(temp & (~BITM_CGU_DIV_ALGN) & (~BITM_CGU_DIV_UPDT),
	       b + CGU_DIV);

	dmcdelay(1000);

	temp = MSEL(cgu->ctl_MSEL) | DF(cgu->ctl_DF);
	if (use_extension0)
		temp |= BITM_CGU_CTL_S0SELEXEN;
	if (use_extension1)
		temp |= BITM_CGU_CTL_S1SELEXEN;

	writel(temp & (~BITM_CGU_CTL_LOCK), b + CGU_CTL);

	if (use_extension0 || use_extension1) {
		u32 mask = BITM_CGU_CTL_S1SELEXEN | BITM_CGU_CTL_S0SELEXEN;

		while (!(readl(b + CGU_CTL) & mask))
			;

		temp = readl(b + CGU_DIVEX);

		if (use_extension0) {
			temp &= ~BITM_CGU_DIVEX_S0SELEX;
			temp |= S0SELEX(cgu->divex_S0SELEX);
		}

		if (use_extension1) {
			temp &= ~BITM_CGU_DIVEX_S1SELEX;
			temp |= S1SELEX(cgu->divex_S1SELEX);
		}

		writel(temp, b + CGU_DIVEX);
	}

	dmcdelay(1000);

	//Take PLL out of Bypass Mode
	writel(BITM_CGU_PLLCTL_PLLEN | BITM_CGU_PLLCTL_PLLBPCL,
	       b + CGU_PLLCTL);
	while ((readl(b + CGU_STAT) &
	       (BITM_CGU_STAT_PLLBP | BITM_CGU_STAT_CLKSALGN)))
		;

	dmcdelay(1000);

	if (cgu->clkoutsel >= 0) {
		temp = readl(b + CGU_CLKOUTSEL);
		temp &= ~BITM_CGU_CLKOUTSEL_CLKOUTSEL;
		temp |= CLKOUTSEL(cgu->clkoutsel);
		writel(temp, b + CGU_CLKOUTSEL);
	}
}

void adi_config_third_pll(void)
{
#if defined(CONFIG_CGU1_PLL3_VCO_MSEL) && defined(CONFIG_CGU1_PLL3_DCLK_DIV)
	u32 temp;

	u32 msel = CONFIG_CGU1_PLL3_VCO_MSEL - 1;
	u32 dsel = CONFIG_CGU1_PLL3_DCLK_DIV - 1;

	temp = readl(REG_MISC_REG10_tst_addr);
	temp &= 0xFFFE0000;
	writel(temp, REG_MISC_REG10_tst_addr);

	dmcdelay(4000u);

	//update MSEL [10:4]
	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= ((msel << BITP_REG10_MSEL3) & BITM_REG10_MSEL3);
	writel(temp, REG_MISC_REG10_tst_addr);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= 0x2;
	writel(temp, REG_MISC_REG10_tst_addr);

	dmcdelay(100000u);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= 0x1;
	writel(temp, REG_MISC_REG10_tst_addr);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= 0x800;
	writel(temp, REG_MISC_REG10_tst_addr);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp &= 0xFFFFF7F8;
	writel(temp, REG_MISC_REG10_tst_addr);

	dmcdelay(4000u);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= ((dsel << BITP_REG10_DSEL3) & BITM_REG10_DSEL3);
	writel(temp, REG_MISC_REG10_tst_addr);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= 0x4;
	writel(temp, REG_MISC_REG10_tst_addr);

	dmcdelay(100000u);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= 0x1;
	writel(temp, REG_MISC_REG10_tst_addr);

	temp = readl(REG_MISC_REG10_tst_addr);
	temp |= 0x800;
	writel(temp, REG_MISC_REG10_tst_addr);
#endif
}

static void Active_To_Fullon(const struct CGU_Settings *pCGU)
{
	u32 tmp;

	while (1) {
		tmp = readl(pCGU->rbase + CGU_STAT);
		if ((tmp & BITM_CGU_STAT_PLLEN) &&
		    (tmp & BITM_CGU_STAT_PLLBP))
			break;
	}

	writel(BITM_CGU_PLLCTL_PLLBPCL, pCGU->rbase + CGU_PLLCTL);

	while (1) {
		tmp = readl(pCGU->rbase + CGU_STAT);
		if ((tmp & BITM_CGU_STAT_PLLEN) &&
		    ~(tmp & BITM_CGU_STAT_PLLBP) &&
		    ~(tmp & BITM_CGU_STAT_CLKSALGN))
			break;
	}
}

static void CGU_Init(const struct CGU_Settings *pCGU)
{
	const uintptr_t b = pCGU->rbase;

#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	if (readl(b + CGU_STAT) & BITM_CGU_STAT_PLLEN)
		writel(BITM_CGU_PLLCTL_PLLEN, b + CGU_PLLCTL);

	dmcdelay(1000);
#endif

	/* Check if processor is in Active mode */
	if (readl(b + CGU_STAT) & BITM_CGU_STAT_PLLBP)
		Active_To_Fullon(pCGU);

#if defined(CONFIG_SC59X) || defined(CONFIG_SC59X_64)
	dmcdelay(1000);
#endif

	program_cgu(pCGU);
}

void cgu_init(void)
{
	const struct CGU_Settings dividers0 = {
		.rbase =		CGU0_REGBASE,
		.ctl_MSEL =		CONFIG_CGU0_VCO_MULT,
		.ctl_DF =		CONFIG_CGU0_DF_DIV,
		.div_CSEL =		CONFIG_CGU0_CCLK_DIV,
		.div_SYSSEL =		CONFIG_CGU0_SCLK_DIV,
		.div_S0SEL =		VAL_CGU0_SCLK0_DIV,
		.div_S1SEL =		VAL_CGU0_SCLK1_DIV,
		.div_DSEL =		CONFIG_CGU0_DCLK_DIV,
		.div_OSEL =		CONFIG_CGU0_OCLK_DIV,
		.divex_S0SELEX =	VAL_CGU0_DIV_S0SELEX,
		.divex_S1SELEX =	VAL_CGU0_DIV_S1SELEX,
		.clkoutsel =		VAL_CGU0_CLKOUTSEL,
	};
	const struct CGU_Settings dividers1 = {
		.rbase =		CGU1_REGBASE,
		.ctl_MSEL =		CONFIG_CGU1_VCO_MULT,
		.ctl_DF =		CONFIG_CGU1_DF_DIV,
		.div_CSEL =		CONFIG_CGU1_CCLK_DIV,
		.div_SYSSEL =		CONFIG_CGU1_SCLK_DIV,
		.div_S0SEL =		VAL_CGU1_SCLK0_DIV,
		.div_S1SEL =		VAL_CGU1_SCLK1_DIV,
		.div_DSEL =		CONFIG_CGU1_DCLK_DIV,
		.div_OSEL =		CONFIG_CGU1_OCLK_DIV,
		.divex_S0SELEX =	VAL_CGU1_DIV_S0SELEX,
		.divex_S1SELEX =	VAL_CGU1_DIV_S1SELEX,
		.clkoutsel =		VAL_CGU1_CLKOUTSEL,
	};

	CGU_Init(&dividers0);
	CGU_Init(&dividers1);
}

#define CONFIGURE_CDU0(a, b, c) \
	writel(a, b); \
	while (readl(REG_CDU0_STAT) & (1 << (c)))

void cdu_init(void)
{
	while (readl(REG_CDU0_STAT) & 0xffff)
		;
	writel((CONFIG_CDU0_CGU1_CLKIN & 0x1), REG_CDU0_CLKINSEL);

	CONFIGURE_CDU0(CONFIG_CDU0_CLKO0, REG_CDU0_CFG0, 0);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO1, REG_CDU0_CFG1, 1);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO2, REG_CDU0_CFG2, 2);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO3, REG_CDU0_CFG3, 3);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO4, REG_CDU0_CFG4, 4);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO5, REG_CDU0_CFG5, 5);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO6, REG_CDU0_CFG6, 6);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO7, REG_CDU0_CFG7, 7);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO8, REG_CDU0_CFG8, 8);
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO9, REG_CDU0_CFG9, 9);
#ifdef CONFIG_CDU0_CLKO10
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO10, REG_CDU0_CFG10, 10);
#endif
#ifdef CONFIG_CDU0_CLKO12
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO12, REG_CDU0_CFG12, 12);
#endif
#ifdef CONFIG_CDU0_CLKO13
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO13, REG_CDU0_CFG13, 13);
#endif
#ifdef CONFIG_CDU0_CLKO14
	CONFIGURE_CDU0(CONFIG_CDU0_CLKO14, REG_CDU0_CFG14, 14);
#endif
}

void clks_init(void)
{
	adi_dmc_reset_lanes(true);

	cdu_init();
	cgu_init();

	adi_config_third_pll();

	adi_dmc_reset_lanes(false);
}
