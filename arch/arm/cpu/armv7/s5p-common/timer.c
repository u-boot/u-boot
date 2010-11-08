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

#define PRESCALER_1		(16 - 1)	/* prescaler of timer 2, 3, 4 */
#define MUX_DIV_2		1		/* 1/2 period */
#define MUX_DIV_4		2		/* 1/4 period */
#define MUX_DIV_8		3		/* 1/8 period */
#define MUX_DIV_16		4		/* 1/16 period */
#define MUX4_DIV_SHIFT		16

#define TCON_TIMER4_SHIFT	20

static unsigned long count_value;

/* Internal tick units */
static unsigned long long timestamp;	/* Monotonic incrementing timer */
static unsigned long lastdec;		/* Last decremneter snapshot */

/* macro to read the 16 bit timer */
static inline struct s5p_timer *s5p_get_base_timer(void)
{
	return (struct s5p_timer *)samsung_get_base_timer();
}

int timer_init(void)
{
	struct s5p_timer *const timer = s5p_get_base_timer();
	u32 val;

	/*
	 * @ PWM Timer 4
	 * Timer Freq(HZ) =
	 *	PWM_CLK / { (prescaler_value + 1) * (divider_value) }
	 */

	/* set prescaler : 16 */
	/* set divider : 2 */
	writel((PRESCALER_1 & 0xff) << 8, &timer->tcfg0);
	writel((MUX_DIV_2 & 0xf) << MUX4_DIV_SHIFT, &timer->tcfg1);

	if (count_value == 0) {
		/* reset initial value */
		/* count_value = 2085937.5(HZ) (per 1 sec)*/
		count_value = get_pwm_clk() / ((PRESCALER_1 + 1) *
				(MUX_DIV_2 + 1));

		/* count_value / 100 = 20859.375(HZ) (per 10 msec) */
		count_value = count_value / 100;
	}

	/* set count value */
	writel(count_value, &timer->tcntb4);
	lastdec = count_value;

	val = (readl(&timer->tcon) & ~(0x07 << TCON_TIMER4_SHIFT)) |
		TCON4_AUTO_RELOAD;

	/* auto reload & manual update */
	writel(val | TCON4_UPDATE, &timer->tcon);

	/* start PWM timer 4 */
	writel(val | TCON4_START, &timer->tcon);

	timestamp = 0;

	return 0;
}

/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

unsigned long get_timer(unsigned long base)
{
	return get_timer_masked() - base;
}

void set_timer(unsigned long t)
{
	timestamp = t;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	unsigned long tmo, tmp;

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
	/* reset "advancing" timestamp to 0, set lastdec value */
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
	lastdec = readl(&timer->tcnto4);
	timestamp = 0;
}

unsigned long get_timer_masked(void)
{
	struct s5p_timer *const timer = s5p_get_base_timer();
	unsigned long now = readl(&timer->tcnto4);

	if (lastdec >= now)
		timestamp += lastdec - now;
	else
		timestamp += lastdec + count_value - now;

	lastdec = now;

	return timestamp;
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
