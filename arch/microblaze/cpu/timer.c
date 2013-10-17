/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
