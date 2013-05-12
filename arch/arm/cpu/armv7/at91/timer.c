/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2013
 * Bo Shen <voice.shen@atmel.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>
#include <div64.h>

#if !defined(CONFIG_AT91FAMILY)
# error You need to define CONFIG_AT91FAMILY in your board config!
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * We're using the SAMA5D3x PITC in 32 bit mode, by
 * setting the 20 bit counter period to its maximum (0xfffff).
 * (See the relevant data sheets to understand that this really works)
 *
 * We do also mimic the typical powerpc way of incrementing
 * two 32 bit registers called tbl and tbu.
 *
 * Those registers increment at 1/16 the main clock rate.
 */

#define TIMER_LOAD_VAL	0xfffff

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, gd->arch.timer_rate_hz);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= gd->arch.timer_rate_hz;
	do_div(usec, 1000000);

	return usec;
}

/*
 * Use the PITC in full 32 bit incrementing mode
 */
int timer_init(void)
{
	at91_pit_t *pit = (at91_pit_t *)ATMEL_BASE_PIT;

	/* Enable PITC Clock */
	at91_periph_clk_enable(ATMEL_ID_SYS);

	/* Enable PITC */
	writel(TIMER_LOAD_VAL | AT91_PIT_MR_EN , &pit->mr);

	gd->arch.timer_rate_hz = gd->arch.mck_rate_hz / 16;
	gd->arch.tbu = 0;
	gd->arch.tbl = 0;

	return 0;
}

/*
 * Get the current 64 bit timer tick count
 */
unsigned long long get_ticks(void)
{
	at91_pit_t *pit = (at91_pit_t *)ATMEL_BASE_PIT;

	ulong now = readl(&pit->piir);

	/* increment tbu if tbl has rolled over */
	if (now < gd->arch.tbl)
		gd->arch.tbu++;
	gd->arch.tbl = now;
	return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}

void __udelay(unsigned long usec)
{
	unsigned long long start;
	ulong tmo;

	start = get_ticks();		/* get current timestamp */
	tmo = usec_to_tick(usec);	/* convert usecs to ticks */
	while ((get_ticks() - start) < tmo)
		;			/* loop till time has passed */
}

/*
 * get_timer(base) can be used to check for timeouts or
 * to measure elasped time relative to an event:
 *
 * ulong start_time = get_timer(0) sets start_time to the current
 * time value.
 * get_timer(start_time) returns the time elapsed since then.
 *
 * The time is used in CONFIG_SYS_HZ units!
 */
ulong get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

/*
 * Return the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
