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
#define GPTCR_FRR		(1 << 9)	/* Freerun / restart */
#define GPTCR_CLKSOURCE_32	(4 << 6)	/* Clock source */
#define GPTCR_TEN		1		/* Timer enable */

DECLARE_GLOBAL_DATA_PTR;

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, MXC_CLK32);

	return tick;
}

static inline unsigned long long us_to_tick(unsigned long long usec)
{
	usec = usec * MXC_CLK32 + 999999;
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

	__raw_writel(0, &cur_gpt->prescaler); /* 32Khz */

	/* Freerun Mode, PERCLK1 input */
	i = __raw_readl(&cur_gpt->control);
	__raw_writel(i | GPTCR_CLKSOURCE_32 | GPTCR_TEN, &cur_gpt->control);

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
	 * 2^64 / MXC_CLK32 = 2^64 / 2^15 = 2^49 ~ 5 * 10^14 (s) ~
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
	return MXC_CLK32;
}
