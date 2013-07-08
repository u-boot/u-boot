/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <div64.h>
#include <watchdog.h>
#include <asm/io.h>

#define TIMER_BASE 0x53f90000 /* General purpose timer 1 */

/* General purpose timers registers */
#define GPTCR	__REG(TIMER_BASE)		/* Control register	*/
#define GPTPR	__REG(TIMER_BASE + 0x4)		/* Prescaler register	*/
#define GPTSR	__REG(TIMER_BASE + 0x8)		/* Status register	*/
#define GPTCNT	__REG(TIMER_BASE + 0x24)	/* Counter register	*/

/* General purpose timers bitfields */
#define GPTCR_SWR		(1 << 15)	/* Software reset	*/
#define GPTCR_FRR		(1 << 9)	/* Freerun / restart	*/
#define GPTCR_CLKSOURCE_32	(4 << 6)	/* Clock source		*/
#define GPTCR_TEN		1		/* Timer enable		*/

DECLARE_GLOBAL_DATA_PTR;

/*
 * "time" is measured in 1 / CONFIG_SYS_HZ seconds,
 * "tick" is internal timer period
 */

#ifdef CONFIG_MX31_TIMER_HIGH_PRECISION
/* ~0.4% error - measured with stop-watch on 100s boot-delay */
static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, MXC_CLK32);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	time *= MXC_CLK32;
	do_div(time, CONFIG_SYS_HZ);
	return time;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us = us * MXC_CLK32 + 999999;
	do_div(us, 1000000);
	return us;
}
#else
/* ~2% error */
#define TICK_PER_TIME	((MXC_CLK32 + CONFIG_SYS_HZ / 2) / CONFIG_SYS_HZ)
#define US_PER_TICK	(1000000 / MXC_CLK32)

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	do_div(tick, TICK_PER_TIME);
	return tick;
}

static inline unsigned long long time_to_tick(unsigned long long time)
{
	return time * TICK_PER_TIME;
}

static inline unsigned long long us_to_tick(unsigned long long us)
{
	us += US_PER_TICK - 1;
	do_div(us, US_PER_TICK);
	return us;
}
#endif

/* The 32768Hz 32-bit timer overruns in 131072 seconds */
int timer_init(void)
{
	int i;

	/* setup GP Timer 1 */
	GPTCR = GPTCR_SWR;
	for (i = 0; i < 100; i++)
		GPTCR = 0; /* We have no udelay by now */
	GPTPR = 0; /* 32Khz */
	/* Freerun Mode, PERCLK1 input */
	GPTCR |= GPTCR_CLKSOURCE_32 | GPTCR_TEN;

	return 0;
}

unsigned long long get_ticks(void)
{
	ulong now = GPTCNT; /* current tick value */

	if (now >= gd->arch.lastinc)	/* normal mode (non roll) */
		/* move stamp forward with absolut diff ticks */
		gd->arch.tbl += (now - gd->arch.lastinc);
	else			/* we have rollover of incrementer */
		gd->arch.tbl += (0xFFFFFFFF - gd->arch.lastinc) + now;
	gd->arch.lastinc = now;
	return gd->arch.tbl;
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

/* delay x useconds AND preserve advance timestamp value */
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
