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
	     (is_soc_rev(CHIP_REV_1_0) > 0)) || is_cpu_type(MXC_CPU_MX6DL) ||
	      is_cpu_type(MXC_CPU_MX6SOLO) || is_cpu_type(MXC_CPU_MX6SX))
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
static inline unsigned long long tick_to_time(unsigned long long tick)
{
	ulong gpt_clk = gpt_get_clk();

	tick *= CONFIG_SYS_HZ;
	do_div(tick, gpt_clk);

	return tick;
}

static inline unsigned long long us_to_tick(unsigned long long usec)
{
	ulong gpt_clk = gpt_get_clk();

	usec = usec * gpt_clk + 999999;
	do_div(usec, 1000000);

	return usec;
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

		/* For DL/S, SX, set 24Mhz OSC Enable bit and prescaler */
		if (is_cpu_type(MXC_CPU_MX6DL) ||
		    is_cpu_type(MXC_CPU_MX6SOLO) ||
		    is_cpu_type(MXC_CPU_MX6SX)) {
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

unsigned long long get_ticks(void)
{
	ulong now = __raw_readl(&cur_gpt->counter); /* current tick value */

	/* increment tbu if tbl has rolled over */
	if (now < gd->arch.tbl)
		gd->arch.tbu++;
	gd->arch.tbl = now;
	return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}

ulong get_timer_masked(void)
{
	/*
	 * get_ticks() returns a long long (64 bit), it wraps in
	 * 2^64 / GPT_CLK = 2^64 / 2^15 = 2^49 ~ 5 * 10^14 (s) ~
	 * 5 * 10^9 days... and get_ticks() * CONFIG_SYS_HZ wraps in
	 * 5 * 10^6 days - long enough.
	 */
	return tick_to_time(get_ticks());
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/* delay x useconds AND preserve advance timstamp value */
void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gpt_get_clk();
}

/*
 * This function is intended for SHORT delays only.
 * It will overflow at around 10 seconds @ 400MHz,
 * or 20 seconds @ 200MHz.
 */
unsigned long usec2ticks(unsigned long usec)
{
	ulong ticks;

	if (usec < 1000)
		ticks = ((usec * (get_tbclk()/1000)) + 500) / 1000;
	else
		ticks = ((usec / 10) * (get_tbclk() / 100000));

	return ticks;
}
