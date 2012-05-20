/*
 * Copyright (C) 2011 Vladimir Zapolskiy <vz@mleia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/timer.h>
#include <asm/io.h>

static struct timer_regs  *timer0 = (struct timer_regs *)TIMER0_BASE;
static struct timer_regs  *timer1 = (struct timer_regs *)TIMER1_BASE;
static struct clk_pm_regs *clk    = (struct clk_pm_regs *)CLK_PM_BASE;

static void lpc32xx_timer_clock(u32 bit, int enable)
{
	if (enable)
		setbits_le32(&clk->timclk_ctrl1, bit);
	else
		clrbits_le32(&clk->timclk_ctrl1, bit);
}

static void lpc32xx_timer_reset(struct timer_regs *timer, u32 freq)
{
	writel(TIMER_TCR_COUNTER_RESET,   &timer->tcr);
	writel(TIMER_TCR_COUNTER_DISABLE, &timer->tcr);
	writel(0, &timer->tc);
	writel(0, &timer->pr);

	/* Count mode is every rising PCLK edge */
	writel(TIMER_CTCR_MODE_TIMER, &timer->ctcr);

	/* Set prescale counter value */
	writel((get_periph_clk_rate() / freq) - 1, &timer->pr);
}

static void lpc32xx_timer_count(struct timer_regs *timer, int enable)
{
	if (enable)
		writel(TIMER_TCR_COUNTER_ENABLE,  &timer->tcr);
	else
		writel(TIMER_TCR_COUNTER_DISABLE, &timer->tcr);
}

int timer_init(void)
{
	lpc32xx_timer_clock(CLK_TIMCLK_TIMER0, 1);
	lpc32xx_timer_reset(timer0, CONFIG_SYS_HZ);
	lpc32xx_timer_count(timer0, 1);

	return 0;
}

ulong get_timer(ulong base)
{
	return readl(&timer0->tc) - base;
}

void __udelay(unsigned long usec)
{
	lpc32xx_timer_clock(CLK_TIMCLK_TIMER1, 1);
	lpc32xx_timer_reset(timer1, CONFIG_SYS_HZ * 1000);
	lpc32xx_timer_count(timer1, 1);

	while (readl(&timer1->tc) < usec)
		/* NOP */;

	lpc32xx_timer_count(timer1, 0);
	lpc32xx_timer_clock(CLK_TIMCLK_TIMER1, 0);
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
