/*
 * Copyright (C) 2004-2006 Atmel Corporation
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
#include <div64.h>

#include <asm/errno.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/sysreg.h>

#include <asm/arch/hardware.h>

#define HANDLER_MASK	0x00ffffff
#define INTLEV_SHIFT	30
#define INTLEV_MASK	0x00000003

DECLARE_GLOBAL_DATA_PTR;

/* Incremented whenever COUNT reaches 0xffffffff by timer_interrupt_handler */
volatile unsigned long timer_overflow;

/*
 * Instead of dividing by get_tbclk(), multiply by this constant and
 * right-shift the result by 32 bits.
 */
static unsigned long tb_factor;

unsigned long get_tbclk(void)
{
	return gd->cpu_hz;
}

unsigned long long get_ticks(void)
{
	unsigned long lo, hi_now, hi_prev;

	do {
		hi_prev = timer_overflow;
		lo = sysreg_read(COUNT);
		hi_now = timer_overflow;
	} while (hi_prev != hi_now);

	return ((unsigned long long)hi_now << 32) | lo;
}

unsigned long get_timer(unsigned long base)
{
	u64 now = get_ticks();

	now *= tb_factor;
	return (unsigned long)(now >> 32) - base;
}

/*
 * For short delays only. It will overflow after a few seconds.
 */
void __udelay(unsigned long usec)
{
	unsigned long cycles;
	unsigned long base;
	unsigned long now;

	base = sysreg_read(COUNT);
	cycles = ((usec * (get_tbclk() / 10000)) + 50) / 100;

	do {
		now = sysreg_read(COUNT);
	} while ((now - base) < cycles);
}

static int set_interrupt_handler(unsigned int nr, void (*handler)(void),
				 unsigned int priority)
{
	extern void _evba(void);
	unsigned long intpr;
	unsigned long handler_addr = (unsigned long)handler;

	handler_addr -= (unsigned long)&_evba;

	if ((handler_addr & HANDLER_MASK) != handler_addr
	    || (priority & INTLEV_MASK) != priority)
		return -EINVAL;

	intpr = (handler_addr & HANDLER_MASK);
	intpr |= (priority & INTLEV_MASK) << INTLEV_SHIFT;
	writel(intpr, (void *)ATMEL_BASE_INTC + 4 * nr);

	return 0;
}

int timer_init(void)
{
	extern void timer_interrupt_handler(void);
	u64 tmp;

	sysreg_write(COUNT, 0);

	tmp = (u64)CONFIG_SYS_HZ << 32;
	tmp += gd->cpu_hz / 2;
	do_div(tmp, gd->cpu_hz);
	tb_factor = (u32)tmp;

	if (set_interrupt_handler(0, &timer_interrupt_handler, 3))
		return -EINVAL;

	/* For all practical purposes, this gives us an overflow interrupt */
	sysreg_write(COMPARE, 0xffffffff);
	return 0;
}
