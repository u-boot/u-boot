/*
 * (C) Copyright 2002
 * Lineo, Inc. <www.lineo.com>
 * Bernhard Kuhn <bkuhn@lineo.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
#include <asm/arch/hardware.h>
#include <asm/arch/at91_tc.h>
#include <asm/arch/at91_pmc.h>

DECLARE_GLOBAL_DATA_PTR;

/* the number of clocks per CONFIG_SYS_HZ */
#define TIMER_LOAD_VAL (CONFIG_SYS_HZ_CLOCK/CONFIG_SYS_HZ)

int timer_init(void)
{
	at91_tc_t *tc = (at91_tc_t *) ATMEL_BASE_TC;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	/* enables TC1.0 clock */
	writel(1 << ATMEL_ID_TC0, &pmc->pcer);	/* enable clock */

	writel(0, &tc->bcr);
	writel(AT91_TC_BMR_TC0XC0S_NONE | AT91_TC_BMR_TC1XC1S_NONE |
		AT91_TC_BMR_TC2XC2S_NONE , &tc->bmr);

	writel(AT91_TC_CCR_CLKDIS, &tc->tc[0].ccr);
	/* set to MCLK/2 and restart the timer
	when the value in TC_RC is reached */
	writel(AT91_TC_CMR_TCCLKS_CLOCK1 | AT91_TC_CMR_CPCTRG, &tc->tc[0].cmr);

	writel(0xFFFFFFFF, &tc->tc[0].idr); /* disable interrupts */
	writel(TIMER_LOAD_VAL, &tc->tc[0].rc);

	writel(AT91_TC_CCR_SWTRG | AT91_TC_CCR_CLKEN, &tc->tc[0].ccr);
	gd->lastinc = 0;
	gd->tbl = 0;

	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void __udelay(unsigned long usec)
{
	udelay_masked(usec);
}

ulong get_timer_raw(void)
{
	at91_tc_t *tc = (at91_tc_t *) ATMEL_BASE_TC;
	u32 now;

	now = readl(&tc->tc[0].cv) & 0x0000ffff;

	if (now >= gd->lastinc) {
		/* normal mode */
		gd->tbl += now - gd->lastinc;
	} else {
		/* we have an overflow ... */
		gd->tbl += now + TIMER_LOAD_VAL - gd->lastinc;
	}
	gd->lastinc = now;

	return gd->tbl;
}

ulong get_timer_masked(void)
{
	return get_timer_raw()/TIMER_LOAD_VAL;
}

void udelay_masked(unsigned long usec)
{
	u32 tmo;
	u32 endtime;
	signed long diff;

	tmo = CONFIG_SYS_HZ_CLOCK / 1000;
	tmo *= usec;
	tmo /= 1000;

	endtime = get_timer_raw() + tmo;

	do {
		u32 now = get_timer_raw();
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
