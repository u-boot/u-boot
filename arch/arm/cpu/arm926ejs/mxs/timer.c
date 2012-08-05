/*
 * Freescale i.MX28 timer driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * (C) Copyright 2009-2010 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

/* Maximum fixed count */
#define TIMER_LOAD_VAL	0xffffffff

DECLARE_GLOBAL_DATA_PTR;

#define timestamp (gd->tbl)
#define lastdec (gd->lastinc)

/*
 * This driver uses 1kHz clock source.
 */
#define	MX28_INCREMENTER_HZ		1000

static inline unsigned long tick_to_time(unsigned long tick)
{
	return tick / (MX28_INCREMENTER_HZ / CONFIG_SYS_HZ);
}

static inline unsigned long time_to_tick(unsigned long time)
{
	return time * (MX28_INCREMENTER_HZ / CONFIG_SYS_HZ);
}

/* Calculate how many ticks happen in "us" microseconds */
static inline unsigned long us_to_tick(unsigned long us)
{
	return (us * MX28_INCREMENTER_HZ) / 1000000;
}

int timer_init(void)
{
	struct mx28_timrot_regs *timrot_regs =
		(struct mx28_timrot_regs *)MXS_TIMROT_BASE;

	/* Reset Timers and Rotary Encoder module */
	mx28_reset_block(&timrot_regs->hw_timrot_rotctrl_reg);

	/* Set fixed_count to 0 */
	writel(0, &timrot_regs->hw_timrot_fixed_count0);

	/* Set UPDATE bit and 1Khz frequency */
	writel(TIMROT_TIMCTRLn_UPDATE | TIMROT_TIMCTRLn_RELOAD |
		TIMROT_TIMCTRLn_SELECT_1KHZ_XTAL,
		&timrot_regs->hw_timrot_timctrl0);

	/* Set fixed_count to maximal value */
	writel(TIMER_LOAD_VAL, &timrot_regs->hw_timrot_fixed_count0);

	return 0;
}

unsigned long long get_ticks(void)
{
	struct mx28_timrot_regs *timrot_regs =
		(struct mx28_timrot_regs *)MXS_TIMROT_BASE;

	/* Current tick value */
	uint32_t now = readl(&timrot_regs->hw_timrot_running_count0);

	if (lastdec >= now) {
		/*
		 * normal mode (non roll)
		 * move stamp forward with absolut diff ticks
		 */
		timestamp += (lastdec - now);
	} else {
		/* we have rollover of decrementer */
		timestamp += (TIMER_LOAD_VAL - now) + lastdec;

	}
	lastdec = now;

	return timestamp;
}

ulong get_timer_masked(void)
{
	return tick_to_time(get_ticks());
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/* We use the HW_DIGCTL_MICROSECONDS register for sub-millisecond timer. */
#define	MX28_HW_DIGCTL_MICROSECONDS	0x8001c0c0

void __udelay(unsigned long usec)
{
	uint32_t old, new, incr;
	uint32_t counter = 0;

	old = readl(MX28_HW_DIGCTL_MICROSECONDS);

	while (counter < usec) {
		new = readl(MX28_HW_DIGCTL_MICROSECONDS);

		/* Check if the timer wrapped. */
		if (new < old) {
			incr = 0xffffffff - old;
			incr += new;
		} else {
			incr = new - old;
		}

		/*
		 * Check if we are close to the maximum time and the counter
		 * would wrap if incremented. If that's the case, break out
		 * from the loop as the requested delay time passed.
		 */
		if (counter + incr < counter)
			break;

		counter += incr;
		old = new;
	}
}

ulong get_tbclk(void)
{
	return MX28_INCREMENTER_HZ;
}
