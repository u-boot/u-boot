// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <clock_legacy.h>
#include <bootstage.h>
#include <dm.h>
#include <errno.h>
#include <init.h>
#include <spl.h>
#include <time.h>
#include <timer.h>
#include <watchdog.h>
#include <div64.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/delay.h>

#ifndef CFG_WD_PERIOD
# define CFG_WD_PERIOD	(10 * 1000 * 1000)	/* 10 seconds default */
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CFG_SYS_TIMER_RATE
/* Returns tick rate in ticks per second */
ulong notrace get_tbclk(void)
{
	return CFG_SYS_TIMER_RATE;
}
#endif

#ifdef CFG_SYS_TIMER_COUNTER
unsigned long notrace timer_read_counter(void)
{
#ifdef CONFIG_SYS_TIMER_COUNTS_DOWN
	return ~readl(CFG_SYS_TIMER_COUNTER);
#else
	return readl(CFG_SYS_TIMER_COUNTER);
#endif
}

ulong timer_get_boot_us(void)
{
	ulong count = timer_read_counter();

#ifdef CFG_SYS_TIMER_RATE
	const ulong timer_rate = CFG_SYS_TIMER_RATE;

	if (timer_rate == 1000000)
		return count;
	else if (timer_rate > 1000000)
		return lldiv(count, timer_rate / 1000000);
	else
		return (unsigned long long)count * 1000000 / timer_rate;
#else
	/* Assume the counter is in microseconds */
	return count;
#endif
}

#else
extern unsigned long timer_read_counter(void);
#endif

#if CONFIG_IS_ENABLED(TIMER)
ulong notrace get_tbclk(void)
{
	if (!gd->timer) {
		int ret;

		if (IS_ENABLED(CONFIG_TIMER_EARLY))
			return timer_early_get_rate();

		ret = dm_timer_init();
		if (ret)
			return ret;
	}

	return timer_get_rate(gd->timer);
}

uint64_t notrace get_ticks(void)
{
	u64 count;
	int ret;

	if (!gd->timer) {
		int ret;

		if (IS_ENABLED(CONFIG_TIMER_EARLY))
			return timer_early_get_count();

		ret = dm_timer_init();
		if (ret)
			panic("Could not initialize timer (err %d)\n", ret);
	}

	ret = timer_get_count(gd->timer, &count);
	if (ret) {
		if (xpl_phase() > PHASE_TPL)
			panic("Could not read count from timer (err %d)\n",
			      ret);
		else
			panic("no timer (err %d)\n", ret);
	}

	return count;
}

#else /* !CONFIG_TIMER */

uint64_t __weak notrace get_ticks(void)
{
	unsigned long now = timer_read_counter();

	/* increment tbu if tbl has rolled over */
	if (now < gd->timebase_l)
		gd->timebase_h++;
	gd->timebase_l = now;
	return ((uint64_t)gd->timebase_h << 32) | gd->timebase_l;
}

#endif /* CONFIG_TIMER */

/* Returns time in milliseconds */
static uint64_t notrace tick_to_time(uint64_t tick)
{
	ulong div = get_tbclk();

	tick *= CONFIG_SYS_HZ;
	do_div(tick, div);
	return tick;
}

int __weak timer_init(void)
{
	return 0;
}

/* Returns time in milliseconds */
ulong __weak get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

static uint64_t notrace tick_to_time_us(uint64_t tick)
{
	ulong div = get_tbclk() / 1000;

	tick *= CONFIG_SYS_HZ;
	do_div(tick, div);
	return tick;
}

uint64_t __weak get_timer_us(uint64_t base)
{
	return tick_to_time_us(get_ticks()) - base;
}

unsigned long __weak get_timer_us_long(unsigned long base)
{
	return timer_get_us() - base;
}

unsigned long __weak notrace timer_get_us(void)
{
	return tick_to_time(get_ticks() * 1000);
}

uint64_t usec_to_tick(unsigned long usec)
{
	uint64_t tick = usec;
	tick *= get_tbclk();
	do_div(tick, 1000000);
	return tick;
}

void __weak __udelay(unsigned long usec)
{
	uint64_t tmp;

	tmp = get_ticks() + usec_to_tick(usec);	/* get current timestamp */

	while (get_ticks() < tmp+1)	/* loop till event */
		 /*NOP*/;
}

/* ------------------------------------------------------------------------- */

void udelay(unsigned long usec)
{
	ulong kv;

	do {
		schedule();
		kv = usec > CFG_WD_PERIOD ? CFG_WD_PERIOD : usec;
		__udelay(kv);
		usec -= kv;
	} while(usec);
}
