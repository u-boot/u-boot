/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <div64.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

DECLARE_GLOBAL_DATA_PTR;

#define timestamp	(gd->tbl)
#define lastinc		(gd->lastinc)

/* General purpose timers bitfields */
#define GPTCR_SWR       (1<<15)	/* Software reset */
#define GPTCR_FRR       (1<<9)	/* Freerun / restart */
#define GPTCR_CLKSOURCE_32   (0x100<<6)	/* Clock source */
#define GPTCR_CLKSOURCE_IPG (0x001<<6)	/* Clock source */
#define GPTCR_TEN       (1)	/* Timer enable */

#define	TIMER_FREQ_HZ	mxc_get_clock(MXC_IPG_CLK)

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, TIMER_FREQ_HZ);

	return tick;
}

static inline unsigned long long us_to_tick(unsigned long long usec)
{
	usec *= TIMER_FREQ_HZ;
	do_div(usec, 1000000);

	return usec;
}

int timer_init(void)
{
	int i;
	struct gpt_regs *gpt = (struct gpt_regs *)GPT1_BASE_ADDR;

	/* setup GP Timer 1 */
	writel(GPTCR_SWR, &gpt->ctrl);
	for (i = 0; i < 100; i++)
		writel(0, &gpt->ctrl);	/* We have no udelay by now */

	writel(0, &gpt->pre);
	/* Freerun Mode, PERCLK1 input */
	writel(readl(&gpt->ctrl) |
		GPTCR_CLKSOURCE_IPG | GPTCR_TEN,
		&gpt->ctrl);

	return 0;
}

unsigned long long get_ticks(void)
{
	struct gpt_regs *gpt = (struct gpt_regs *)GPT1_BASE_ADDR;
	ulong now = readl(&gpt->counter); /* current tick value */

	if (now >= lastinc) {
		/*
		 * normal mode (non roll)
		 * move stamp forward with absolut diff ticks
		 */
		timestamp += (now - lastinc);
	} else {
		/* we have rollover of incrementer */
		timestamp += (0xFFFFFFFF - lastinc) + now;
	}
	lastinc = now;
	return timestamp;
}

ulong get_timer_masked(void)
{
	/*
	 * get_ticks() returns a long long (64 bit), it wraps in
	 * 2^64 / CONFIG_MX25_CLK32 = 2^64 / 2^15 = 2^49 ~ 5 * 10^14 (s) ~
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
	return TIMER_FREQ_HZ;
}
