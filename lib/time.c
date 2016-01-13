/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <watchdog.h>
#include <div64.h>
#include <asm/io.h>

#ifndef CONFIG_WD_PERIOD
# define CONFIG_WD_PERIOD	(10 * 1000 * 1000)	/* 10 seconds default */
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_TIMER_RATE
/* Returns tick rate in ticks per second */
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

#ifdef CONFIG_TIMER
static int notrace dm_timer_init(void)
{
	struct udevice *dev;
	int ret;

	if (!gd->timer) {
		ret = uclass_first_device(UCLASS_TIMER, &dev);
		if (ret)
			return ret;
		if (!dev)
			return -ENODEV;
		gd->timer = dev;
	}

	return 0;
}

ulong notrace get_tbclk(void)
{
	int ret;

	ret = dm_timer_init();
	if (ret)
		return ret;

	return timer_get_rate(gd->timer);
}

uint64_t notrace get_ticks(void)
{
	u64 count;
	int ret;

	ret = dm_timer_init();
	if (ret)
		return ret;

	ret = timer_get_count(gd->timer, &count);
	if (ret)
		return ret;

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

unsigned long __weak notrace timer_get_us(void)
{
	return tick_to_time(get_ticks() * 1000);
}

static uint64_t usec_to_tick(unsigned long usec)
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
