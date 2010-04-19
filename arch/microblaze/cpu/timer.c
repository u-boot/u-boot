/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
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
#include <asm/microblaze_timer.h>
#include <asm/microblaze_intc.h>

volatile int timestamp = 0;

void reset_timer (void)
{
	timestamp = 0;
}

#ifdef CONFIG_SYS_TIMER_0
ulong get_timer (ulong base)
{
	return (timestamp - base);
}
#else
ulong get_timer (ulong base)
{
	return (timestamp++ - base);
}
#endif

void set_timer (ulong t)
{
	timestamp = t;
}

#ifdef CONFIG_SYS_INTC_0
#ifdef CONFIG_SYS_TIMER_0
microblaze_timer_t *tmr = (microblaze_timer_t *) (CONFIG_SYS_TIMER_0_ADDR);

void timer_isr (void *arg)
{
	timestamp++;
	tmr->control = tmr->control | TIMER_INTERRUPT;
}

void timer_init (void)
{
	tmr->loadreg = CONFIG_SYS_TIMER_0_PRELOAD;
	tmr->control = TIMER_INTERRUPT | TIMER_RESET;
	tmr->control =
	    TIMER_ENABLE | TIMER_ENABLE_INTR | TIMER_RELOAD | TIMER_DOWN_COUNT;
	reset_timer ();
	install_interrupt_handler (CONFIG_SYS_TIMER_0_IRQ, timer_isr, (void *)tmr);
}
#endif
#endif
