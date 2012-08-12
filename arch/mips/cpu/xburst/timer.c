/*
 *  Copyright (c) 2006
 *  Ingenic Semiconductor, <jlwei@ingenic.cn>
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

#include <config.h>
#include <common.h>
#include <asm/io.h>

#include <asm/jz4740.h>

#define TIMER_CHAN  0
#define TIMER_FDATA 0xffff  /* Timer full data value */

DECLARE_GLOBAL_DATA_PTR;

static struct jz4740_tcu *tcu = (struct jz4740_tcu *)JZ4740_TCU_BASE;

void reset_timer_masked(void)
{
	/* reset time */
	gd->lastinc = readl(&tcu->tcnt0);
	gd->tbl = 0;
}

ulong get_timer_masked(void)
{
	ulong now = readl(&tcu->tcnt0);

	if (gd->lastinc <= now)
		gd->tbl += now - gd->lastinc; /* normal mode */
	else {
		/* we have an overflow ... */
		gd->tbl += TIMER_FDATA + now - gd->lastinc;
	}

	gd->lastinc = now;

	return gd->tbl;
}

void udelay_masked(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	/* normalize */
	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
	} else {
		if (usec > 1) {
			tmo = usec * CONFIG_SYS_HZ;
			tmo /= 1000*1000;
		} else
			tmo = 1;
	}

	endtime = get_timer_masked() + tmo;

	do {
		ulong now = get_timer_masked();
		diff = endtime - now;
	} while (diff >= 0);
}

int timer_init(void)
{
	writel(TCU_TCSR_PRESCALE256 | TCU_TCSR_EXT_EN, &tcu->tcsr0);

	writel(0, &tcu->tcnt0);
	writel(0, &tcu->tdhr0);
	writel(TIMER_FDATA, &tcu->tdfr0);

	/* mask irqs */
	writel((1 << TIMER_CHAN) | (1 << (TIMER_CHAN + 16)), &tcu->tmsr);
	writel(1 << TIMER_CHAN, &tcu->tscr); /* enable timer clock */
	writeb(1 << TIMER_CHAN, &tcu->tesr); /* start counting up */

	gd->lastinc = 0;
	gd->tbl = 0;

	return 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	gd->tbl = t;
}

void __udelay(unsigned long usec)
{
	ulong tmo, tmp;

	/* normalize */
	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
	} else {
		if (usec >= 1) {
			tmo = usec * CONFIG_SYS_HZ;
			tmo /= 1000 * 1000;
		} else
			tmo = 1;
	}

	/* check for rollover during this delay */
	tmp = get_timer(0);
	if ((tmp + tmo) < tmp)
		reset_timer_masked();  /* timer would roll over */
	else
		tmo += tmp;

	while (get_timer_masked() < tmo)
		;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On MIPS it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On MIPS it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
