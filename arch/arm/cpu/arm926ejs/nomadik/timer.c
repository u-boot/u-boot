/*
 * (C) Copyright 2009 Alessandro Rubini
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mtu.h>

/*
 * The timer is a decrementer, we'll left it free running at 2.4MHz.
 * We have 2.4 ticks per microsecond and an overflow in almost 30min
 */
#define TIMER_CLOCK		(24 * 100 * 1000)
#define COUNT_TO_USEC(x)	((x) * 5 / 12)	/* overflows at 6min */
#define USEC_TO_COUNT(x)	((x) * 12 / 5)	/* overflows at 6min */
#define TICKS_PER_HZ		(TIMER_CLOCK / CONFIG_SYS_HZ)
#define TICKS_TO_HZ(x)		((x) / TICKS_PER_HZ)

/* macro to read the decrementing 32 bit timer as an increasing count */
#define READ_TIMER() (0 - readl(CONFIG_SYS_TIMERBASE + MTU_VAL(0)))

/* Configure a free-running, auto-wrap counter with no prescaler */
int timer_init(void)
{
	ulong val;

	writel(MTU_CRn_ENA | MTU_CRn_PRESCALE_1 | MTU_CRn_32BITS,
	       CONFIG_SYS_TIMERBASE + MTU_CR(0));

	/* Reset the timer */
	writel(0, CONFIG_SYS_TIMERBASE + MTU_LR(0));
	/*
	 * The load-register isn't really immediate: it changes on clock
	 * edges, so we must wait for our newly-written value to appear.
	 * Since we might miss reading 0, wait for any change in value.
	 */
	val = READ_TIMER();
	while (READ_TIMER() == val)
		;

	return 0;
}

/* Return how many HZ passed since "base" */
ulong get_timer(ulong base)
{
	return  TICKS_TO_HZ(READ_TIMER()) - base;
}

/* Delay x useconds */
void __udelay(unsigned long usec)
{
	ulong ini, end;

	ini = READ_TIMER();
	end = ini + USEC_TO_COUNT(usec);
	while ((signed)(end - READ_TIMER()) > 0)
		;
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
