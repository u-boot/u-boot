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
microblaze_timer_t *tmr;

ulong get_timer (ulong base)
{
	if (tmr)
		return timestamp - base;
	return timestamp++ - base;
}

void __udelay(unsigned long usec)
{
	u32 i;

	if (tmr) {
		i = get_timer(0);
		while ((get_timer(0) - i) < (usec / 1000))
			;
	} else {
		for (i = 0; i < (usec * XILINX_CLOCK_FREQ / 10000000); i++)
			;
	}
}

static void timer_isr(void *arg)
{
	timestamp++;
	tmr->control = tmr->control | TIMER_INTERRUPT;
}

int timer_init (void)
{
	int irq = -1;
	u32 preload = 0;
	u32 ret = 0;

#if defined(CONFIG_SYS_TIMER_0_ADDR) && defined(CONFIG_SYS_INTC_0_NUM)
	preload = XILINX_CLOCK_FREQ / CONFIG_SYS_HZ;
	irq = CONFIG_SYS_TIMER_0_IRQ;
	tmr = (microblaze_timer_t *) (CONFIG_SYS_TIMER_0_ADDR);
#endif

	if (tmr && preload && irq >= 0) {
		tmr->loadreg = preload;
		tmr->control = TIMER_INTERRUPT | TIMER_RESET;
		tmr->control = TIMER_ENABLE | TIMER_ENABLE_INTR |\
					TIMER_RELOAD | TIMER_DOWN_COUNT;
		timestamp = 0;
		ret = install_interrupt_handler (irq, timer_isr, (void *)tmr);
		if (ret)
			tmr = NULL;
	}

	/* No problem if timer is not found/initialized */
	return 0;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On Microblaze it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On Microblaze it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}
