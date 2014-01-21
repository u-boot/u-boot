/*
 * Marvell PXA2xx/3xx timer driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/pxa-regs.h>
#include <asm/io.h>
#include <common.h>
#include <div64.h>

DECLARE_GLOBAL_DATA_PTR;

#define	TIMER_LOAD_VAL	0xffffffff

#define	timestamp	(gd->arch.tbl)
#define	lastinc		(gd->arch.lastinc)

#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define	TIMER_FREQ_HZ	3250000
#elif defined(CONFIG_CPU_PXA25X)
#define	TIMER_FREQ_HZ	3686400
#else
#error "Timer frequency unknown - please config PXA CPU type"
#endif

static unsigned long long tick_to_time(unsigned long long tick)
{
	return lldiv(tick * CONFIG_SYS_HZ, TIMER_FREQ_HZ);
}

static unsigned long long us_to_tick(unsigned long long us)
{
	return lldiv(us * TIMER_FREQ_HZ, 1000000);
}

int timer_init(void)
{
	writel(0, OSCR);
	return 0;
}

unsigned long long get_ticks(void)
{
	/* Current tick value */
	uint32_t now = readl(OSCR);

	if (now >= lastinc) {
		/*
		 * Normal mode (non roll)
		 * Move stamp forward with absolute diff ticks
		 */
		timestamp += (now - lastinc);
	} else {
		/* We have rollover of incrementer */
		timestamp += (TIMER_LOAD_VAL - lastinc) + now;
	}

	lastinc = now;
	return timestamp;
}

ulong get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = us_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

ulong get_tbclk(void)
{
	return TIMER_FREQ_HZ;
}
