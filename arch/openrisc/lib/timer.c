/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/system.h>
#include <asm/openrisc_exc.h>

static ulong timestamp;

/* how many counter cycles in a jiffy */
#define TIMER_COUNTER_CYCLES  (CONFIG_SYS_CLK_FREQ/CONFIG_SYS_OPENRISC_TMR_HZ)
/* how many ms elapses between each timer interrupt */
#define TIMER_TIMESTAMP_INC   (1000/CONFIG_SYS_OPENRISC_TMR_HZ)
/* how many cycles per ms */
#define TIMER_CYCLES_MS       (CONFIG_SYS_CLK_FREQ/1000)
/* how many cycles per us */
#define TIMER_CYCLES_US       (CONFIG_SYS_CLK_FREQ/1000000uL)

void timer_isr(void)
{
	timestamp += TIMER_TIMESTAMP_INC;
	mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT |
		(TIMER_COUNTER_CYCLES & SPR_TTMR_TP));
}

int timer_init(void)
{
	/* Install timer exception handler */
	exception_install_handler(EXC_TIMER, timer_isr);

	/* Set up the timer for the first expiration. */
	timestamp = 0;

	mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT |
		(TIMER_COUNTER_CYCLES & SPR_TTMR_TP));

	/* Enable tick timer exception in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_TEE);

	return 0;
}

void reset_timer(void)
{
	timestamp = 0;

	mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT |
		(TIMER_COUNTER_CYCLES & SPR_TTMR_TP));
}

/*
 * The timer value in ms is calculated by taking the
 * value accumulated by full timer revolutions plus the value
 * accumulated in this period
 */
ulong get_timer(ulong base)
{
	return timestamp + mfspr(SPR_TTCR)/TIMER_CYCLES_MS - base;
}

void set_timer(ulong t)
{
	reset_timer();
	timestamp = t;
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

void __udelay(ulong usec)
{
	ulong elapsed = 0;
	ulong tick;
	ulong last_tick;

	last_tick = mfspr(SPR_TTCR);
	while ((elapsed / TIMER_CYCLES_US) < usec) {
		tick = mfspr(SPR_TTCR);
		if (tick >= last_tick)
			elapsed += (tick - last_tick);
		else
			elapsed += TIMER_COUNTER_CYCLES - (last_tick - tick);
		last_tick = tick;
	}
}
