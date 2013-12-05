/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <div64.h>
#include <asm/io.h>

#if CONFIG_SYS_HZ != 1000
#warning "CONFIG_SYS_HZ must be 1000 and should not be defined by platforms"
#endif

#ifndef CONFIG_WD_PERIOD
# define CONFIG_WD_PERIOD	(10 * 1000 * 1000)	/* 10 seconds default*/
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_TIMER_RATE
ulong notrace get_tbclk(void)
{
	return CONFIG_SYS_TIMER_RATE;
}
#endif

#ifdef CONFIG_SYS_TIMER_COUNTER
unsigned long notrace timer_read_counter(void)
{
#ifdef CONFIG_SYS_TIMER_COUNTS_DOWN
	return ~readl(CONFIG_SYS_TIMER_COUNTER);
#else
	return readl(CONFIG_SYS_TIMER_COUNTER);
#endif
}
#else
extern unsigned long __weak timer_read_counter(void);
#endif

unsigned long long __weak notrace get_ticks(void)
{
	unsigned long now = timer_read_counter();

	/* increment tbu if tbl has rolled over */
	if (now < gd->timebase_l)
		gd->timebase_h++;
	gd->timebase_l = now;
	return ((unsigned long long)gd->timebase_h << 32) | gd->timebase_l;
}

static unsigned long long notrace tick_to_time(uint64_t tick)
{
	unsigned int div = get_tbclk();

	tick *= CONFIG_SYS_HZ;
	do_div(tick, div);
	return tick;
}

ulong __weak get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

unsigned long __weak notrace timer_get_us(void)
{
	return tick_to_time(get_ticks() * 1000);
}
static unsigned long long usec_to_tick(unsigned long usec)
{
	uint64_t tick = usec;
	tick *= get_tbclk();
	do_div(tick, 1000000);
	return tick;
}

void __weak __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;

	tmo = usec_to_tick(usec);
	tmp = get_ticks() + tmo;	/* get current timestamp */

	while (get_ticks() < tmp)	/* loop till event */
		 /*NOP*/;
}

/* ------------------------------------------------------------------------- */

void udelay(unsigned long usec)
{
	ulong kv;

	do {
		WATCHDOG_RESET();
		kv = usec > CONFIG_WD_PERIOD ? CONFIG_WD_PERIOD : usec;
		__udelay (kv);
		usec -= kv;
	} while(usec);
}

void mdelay(unsigned long msec)
{
	while (msec--)
		udelay(1000);
}
