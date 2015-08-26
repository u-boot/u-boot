/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

/* General purpose timers registers */
struct mxc_gpt {
	unsigned int control;
	unsigned int prescaler;
	unsigned int status;
	unsigned int nouse[6];
	unsigned int counter;
};

static struct mxc_gpt *cur_gpt = (struct mxc_gpt *)GPT1_BASE_ADDR;

/* General purpose timers bitfields */
#define GPTCR_SWR		(1 << 15)	/* Software reset */
#define GPTCR_24MEN	    (1 << 10)	/* Enable 24MHz clock input */
#define GPTCR_FRR		(1 << 9)	/* Freerun / restart */
#define GPTCR_CLKSOURCE_32	(4 << 6)	/* Clock source 32khz */
#define GPTCR_CLKSOURCE_OSC	(5 << 6)	/* Clock source OSC */
#define GPTCR_CLKSOURCE_PRE	(1 << 6)	/* Clock source PRECLK */
#define GPTCR_CLKSOURCE_MASK (0x7 << 6)
#define GPTCR_TEN		1		/* Timer enable */

#define GPTPR_PRESCALER24M_SHIFT 12
#define GPTPR_PRESCALER24M_MASK (0xF << GPTPR_PRESCALER24M_SHIFT)

DECLARE_GLOBAL_DATA_PTR;

static inline int gpt_has_clk_source_osc(void)
{
#if defined(CONFIG_MX6)
	if (((is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D)) &&
	    (soc_rev() > CHIP_REV_1_0)) || is_cpu_type(MXC_CPU_MX6DL) ||
	     is_cpu_type(MXC_CPU_MX6SOLO) || is_cpu_type(MXC_CPU_MX6SX) ||
	     is_cpu_type(MXC_CPU_MX6UL))
		return 1;

	return 0;
#else
	return 0;
#endif
}

static inline ulong gpt_get_clk(void)
{
#ifdef CONFIG_MXC_GPT_HCLK
	if (gpt_has_clk_source_osc())
		return MXC_HCLK >> 3;
	else
		return mxc_get_clock(MXC_IPG_PERCLK);
#else
	return MXC_CLK32;
#endif
}

int timer_init(void)
{
	int i;

	/* setup GP Timer 1 */
	__raw_writel(GPTCR_SWR, &cur_gpt->control);

	/* We have no udelay by now */
	for (i = 0; i < 100; i++)
		__raw_writel(0, &cur_gpt->control);

	i = __raw_readl(&cur_gpt->control);
	i &= ~GPTCR_CLKSOURCE_MASK;

#ifdef CONFIG_MXC_GPT_HCLK
	if (gpt_has_clk_source_osc()) {
		i |= GPTCR_CLKSOURCE_OSC | GPTCR_TEN;

		/* For DL/S, SX, UL, set 24Mhz OSC Enable bit and prescaler */
		if (is_cpu_type(MXC_CPU_MX6DL) ||
		    is_cpu_type(MXC_CPU_MX6SOLO) ||
		    is_cpu_type(MXC_CPU_MX6SX) ||
		    is_cpu_type(MXC_CPU_MX6UL)) {
			i |= GPTCR_24MEN;

			/* Produce 3Mhz clock */
			__raw_writel((7 << GPTPR_PRESCALER24M_SHIFT),
				     &cur_gpt->prescaler);
		}
	} else {
		i |= GPTCR_CLKSOURCE_PRE | GPTCR_TEN;
	}
#else
	__raw_writel(0, &cur_gpt->prescaler); /* 32Khz */
	i |= GPTCR_CLKSOURCE_32 | GPTCR_TEN;
#endif
	__raw_writel(i, &cur_gpt->control);

	gd->arch.tbl = __raw_readl(&cur_gpt->counter);
	gd->arch.tbu = 0;

	return 0;
}

unsigned long timer_read_counter(void)
{
	return __raw_readl(&cur_gpt->counter); /* current tick value */
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gpt_get_clk();
}
