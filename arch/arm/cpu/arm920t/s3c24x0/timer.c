/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
#ifdef CONFIG_S3C24X0

#include <asm/io.h>
#include <asm/arch/s3c24x0_cpu.h>

DECLARE_GLOBAL_DATA_PTR;

int timer_init(void)
{
	struct s3c24x0_timers *timers = s3c24x0_get_base_timers();
	ulong tmr;

	/* use PWM Timer 4 because it has no output */
	/* prescaler for Timer 4 is 16 */
	writel(0x0f00, &timers->tcfg0);
	if (gd->tbu == 0) {
		/*
		 * for 10 ms clock period @ PCLK with 4 bit divider = 1/2
		 * (default) and prescaler = 16. Should be 10390
		 * @33.25MHz and 15625 @ 50 MHz
		 */
		gd->tbu = get_PCLK() / (2 * 16 * 100);
		gd->timer_rate_hz = get_PCLK() / (2 * 16);
	}
	/* load value for 10 ms timeout */
	writel(gd->tbu, &timers->tcntb4);
	/* auto load, manual update of timer 4 */
	tmr = (readl(&timers->tcon) & ~0x0700000) | 0x0600000;
	writel(tmr, &timers->tcon);
	/* auto load, start timer 4 */
	tmr = (tmr & ~0x0700000) | 0x0500000;
	writel(tmr, &timers->tcon);
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

void __udelay (unsigned long usec)
{
	ulong tmo;
	ulong start = get_ticks();

	tmo = usec / 1000;
	tmo *= (gd->tbu * 100);
	tmo /= 1000;

	while ((ulong) (get_ticks() - start) < tmo)
		/*NOP*/;
}

ulong get_timer_masked(void)
{
	ulong tmr = get_ticks();

	return tmr / (gd->timer_rate_hz / CONFIG_SYS_HZ);
}

void udelay_masked(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= (gd->tbu * 100);
		tmo /= 1000;
	} else {
		tmo = usec * (gd->tbu * 100);
		tmo /= (1000 * 1000);
	}

	endtime = get_ticks() + tmo;

	do {
		ulong now = get_ticks();
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	struct s3c24x0_timers *timers = s3c24x0_get_base_timers();
	ulong now = readl(&timers->tcnto4) & 0xffff;

	if (gd->lastinc >= now) {
		/* normal mode */
		gd->tbl += gd->lastinc - now;
	} else {
		/* we have an overflow ... */
		gd->tbl += gd->lastinc + gd->tbu - now;
	}
	gd->lastinc = now;

	return gd->tbl;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

/*
 * reset the cpu by setting up the watchdog timer and let him time out
 */
void reset_cpu(ulong ignored)
{
	struct s3c24x0_watchdog *watchdog;

	watchdog = s3c24x0_get_base_watchdog();

	/* Disable watchdog */
	writel(0x0000, &watchdog->wtcon);

	/* Initialize watchdog timer count register */
	writel(0x0001, &watchdog->wtcnt);

	/* Enable watchdog timer; assert reset at timer timeout */
	writel(0x0021, &watchdog->wtcon);

	while (1)
		/* loop forever and wait for reset to happen */;

	/*NOTREACHED*/
}

#endif /* CONFIG_S3C24X0 */
