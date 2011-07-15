/*
 * Copyright (C) 2009 Samsung Electronics
 * Heungjun Kim <riverful.kim@samsung.com>
 * Inki Dae <inki.dae@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
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
#include <asm/arch/pwm.h>
#include <asm/arch/clk.h>
#include <pwm.h>

DECLARE_GLOBAL_DATA_PTR;

/* macro to read the 16 bit timer */
static inline struct s5p_timer *s5p_get_base_timer(void)
{
	return (struct s5p_timer *)samsung_get_base_timer();
}

int timer_init(void)
{
	/* PWM Timer 4 */
	pwm_init(4, MUX_DIV_2, 0);
	pwm_config(4, 0, 0);
	pwm_enable(4);

	return 0;
}

/*
 * timer without interrupts
 */
unsigned long get_timer(unsigned long base)
{
	return get_timer_masked() - base;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	struct s5p_timer *const timer = s5p_get_base_timer();
	unsigned long tmo, tmp, count_value;

	count_value = readl(&timer->tcntb4);

	if (usec >= 1000) {
		/*
		 * if "big" number, spread normalization
		 * to seconds
		 * 1. start to normalize for usec to ticks per sec
		 * 2. find number of "ticks" to wait to achieve target
		 * 3. finish normalize.
		 */
		tmo = usec / 1000;
		tmo *= (CONFIG_SYS_HZ * count_value / 10);
		tmo /= 1000;
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ * count_value / 10;
		tmo /= (1000 * 1000);
	}

	/* get current timestamp */
	tmp = get_timer(0);

	/* if setting this fordward will roll time stamp */
	/* reset "advancing" timestamp to 0, set lastinc value */
	/* else, set advancing stamp wake up time */
	if ((tmo + tmp + 1) < tmp)
		reset_timer_masked();
	else
		tmo += tmp;

	/* loop till event */
	while (get_timer_masked() < tmo)
		;	/* nop */
}

void reset_timer_masked(void)
{
	struct s5p_timer *const timer = s5p_get_base_timer();

	/* reset time */
	gd->lastinc = readl(&timer->tcnto4);
	gd->tbl = 0;
}

unsigned long get_timer_masked(void)
{
	struct s5p_timer *const timer = s5p_get_base_timer();
	unsigned long now = readl(&timer->tcnto4);
	unsigned long count_value = readl(&timer->tcntb4);

	if (gd->lastinc >= now)
		gd->tbl += gd->lastinc - now;
	else
		gd->tbl += gd->lastinc + count_value - now;

	gd->lastinc = now;

	return gd->tbl;
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
unsigned long get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
