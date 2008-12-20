/*
 * (C) Copyright 2007-2008
 * Nobobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <asm/processor.h>
#include <asm/io.h>

#define TMU_MAX_COUNTER (~0UL)
static int clk_adj = 1;

static void tmu_timer_start (unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(TSTR) | (1 << timer), TSTR);
}

static void tmu_timer_stop (unsigned int timer)
{
	if (timer > 2)
		return;
	writeb(readb(TSTR) & ~(1 << timer), TSTR);
}

int timer_init (void)
{
	/* Divide clock by TMU_CLK_DIVIDER */
	u16 bit = 0;

	switch (TMU_CLK_DIVIDER) {
	case 1024:
		bit = 4;
		break;
	case 256:
		bit = 3;
		break;
	case 64:
		bit = 2;
		break;
	case 16:
		bit = 1;
		break;
	case 4:
	default:
		bit = 0;
		break;
	}
	writew(readw(TCR0) | bit, TCR0);

	/* Clock adjustment calc */
	clk_adj = (int)(1.0 / ((1.0 / CONFIG_SYS_HZ) * 1000000));
	if (clk_adj < 1)
		clk_adj = 1;

	tmu_timer_stop(0);
	tmu_timer_start(0);

	return 0;
}

unsigned long long get_ticks (void)
{
	return 0 - readl(TCNT0);
}

static unsigned long get_usec (void)
{
	return (0 - readl(TCNT0));
}

void udelay (unsigned long usec)
{
	unsigned int start = get_usec();
	unsigned int end = start + (usec * clk_adj);

	while (get_usec() < end)
		continue;
}

unsigned long get_timer (unsigned long base)
{
	/* return msec */
	return ((get_usec() / clk_adj) / 1000) - base;
}

void set_timer (unsigned long t)
{
	writel((0 - t), TCNT0);
}

void reset_timer (void)
{
	tmu_timer_stop(0);
	set_timer (0);
	tmu_timer_start(0);
}

unsigned long get_tbclk (void)
{
	return CONFIG_SYS_HZ;
}
