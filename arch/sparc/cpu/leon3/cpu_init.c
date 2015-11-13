/* Initializes CPU and basic hardware such as memory
 * controllers, IRQ controller and system timer 0.
 *
 * (C) Copyright 2007, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/asi.h>
#include <asm/leon.h>
#include <ambapp.h>
#include <grlib/irqmp.h>
#include <grlib/gptimer.h>
#include <debug_uart.h>

#include <config.h>

/* Default Plug&Play I/O area */
#ifndef CONFIG_AMBAPP_IOAREA
#define CONFIG_AMBAPP_IOAREA AMBA_DEFAULT_IOAREA
#endif

#define TIMER_BASE_CLK 1000000
#define US_PER_TICK (1000000 / CONFIG_SYS_HZ)

DECLARE_GLOBAL_DATA_PTR;

/* reset CPU (jump to 0, without reset) */
void start(void);

ambapp_dev_irqmp *irqmp = NULL;
ambapp_dev_gptimer *gptimer = NULL;
unsigned int gptimer_irq = 0;
int leon3_snooping_avail = 0;

struct {
	gd_t gd_area;
	bd_t bd;
} global_data;

/*
 * Breath some life into the CPU...
 *
 * Run from FLASH/PROM:
 *  - until memory controller is set up, only registers available
 *  - memory controller has already been setup up, stack can be used
 *  - no global variables available for writing
 *  - constants available
 */
void cpu_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
}

/* Routine called from start.S,
 *
 * Run from FLASH/PROM:
 *  - memory controller has already been setup up, stack can be used
 *  - global variables available for read/writing
 *  - constants avaiable
	 */
void cpu_init_f2(void)
{
	/* Initialize the AMBA Plug & Play bus structure, the bus
	 * structure represents the AMBA bus that the CPU is located at.
	 */
	ambapp_bus_init(CONFIG_AMBAPP_IOAREA, CONFIG_SYS_CLK_FREQ, &ambapp_plb);
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r(void)
{
	ambapp_apbdev apbdev;
	int index, cpu;
	ambapp_dev_gptimer *timer = NULL;
	unsigned int bus_freq;

	/*
	 * Find AMBA APB IRQMP Controller,
	 */
	if (ambapp_apb_find(&ambapp_plb, VENDOR_GAISLER,
		GAISLER_IRQMP, 0, &apbdev) != 1) {
		panic("%s: IRQ controller not found\n", __func__);
		return -1;
	}
	irqmp = (ambapp_dev_irqmp *)apbdev.address;

	/* initialize the IRQMP */
	irqmp->ilevel = 0xf;	/* all IRQ off */
	irqmp->iforce = 0;
	irqmp->ipend = 0;
	irqmp->iclear = 0xfffe;	/* clear all old pending interrupts */
	for (cpu = 0; cpu < 16; cpu++) {
		/* mask and clear force for all IRQs on CPU[N] */
		irqmp->cpu_mask[cpu] = 0;
		irqmp->cpu_force[cpu] = 0;
	}

	/* timer */
	index = 0;
	while (ambapp_apb_find(&ambapp_plb, VENDOR_GAISLER, GAISLER_GPTIMER,
		index, &apbdev) == 1) {
		timer = (ambapp_dev_gptimer *)apbdev.address;
		if (gptimer == NULL) {
			gptimer = timer;
			gptimer_irq = apbdev.irq;
		}

		/* Different buses may have different frequency, the
		 * frequency of the bus tell in which frequency the timer
		 * prescaler operates.
		 */
		bus_freq = ambapp_bus_freq(&ambapp_plb, apbdev.ahb_bus_index);

		/* initialize prescaler common to all timers to 1MHz */
		timer->scalar = timer->scalar_reload =
		    (((bus_freq / 1000) + 500) / 1000) - 1;

		index++;
	}
	if (!gptimer) {
		printf("%s: gptimer not found!\n", __func__);
		return 1;
	}
	return 0;
}

/* Uses Timer 0 to get accurate
 * pauses. Max 2 raised to 32 ticks
 *
 */
void cpu_wait_ticks(unsigned long ticks)
{
	unsigned long start = get_timer(0);
	while (get_timer(start) < ticks) ;
}

/* initiate and setup timer0 interrupt to configured HZ. Base clock is 1MHz.
 * Return irq number for timer int or a negative number for
 * dealing with self
 */
int timer_interrupt_init_cpu(void)
{
	/* SYS_HZ ticks per second */
	gptimer->e[0].val = 0;
	gptimer->e[0].rld = (TIMER_BASE_CLK / CONFIG_SYS_HZ) - 1;
	gptimer->e[0].ctrl =
	    (GPTIMER_CTRL_EN | GPTIMER_CTRL_RS |
	     GPTIMER_CTRL_LD | GPTIMER_CTRL_IE);

	return gptimer_irq;
}

ulong get_tbclk(void)
{
	return TIMER_BASE_CLK;
}

/*
 * This function is intended for SHORT delays only.
 */
unsigned long cpu_usec2ticks(unsigned long usec)
{
	if (usec < US_PER_TICK)
		return 1;
	return usec / US_PER_TICK;
}

unsigned long cpu_ticks2usec(unsigned long ticks)
{
	return ticks * US_PER_TICK;
}
