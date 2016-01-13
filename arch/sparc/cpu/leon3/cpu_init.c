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
#include <asm/io.h>
#include <ambapp.h>
#include <grlib/irqmp.h>
#include <grlib/gptimer.h>
#include <debug_uart.h>

#include <config.h>

/* Default Plug&Play I/O area */
#ifndef CONFIG_AMBAPP_IOAREA
#define CONFIG_AMBAPP_IOAREA AMBA_DEFAULT_IOAREA
#endif

/* Select which TIMER that will become the time base */
#ifndef CONFIG_SYS_GRLIB_GPTIMER_INDEX
#define CONFIG_SYS_GRLIB_GPTIMER_INDEX 0
#endif

DECLARE_GLOBAL_DATA_PTR;

ambapp_dev_irqmp *irqmp = NULL;

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

/* If cache snooping is available in hardware the result will be set
 * to 0x800000, otherwise 0.
 */
static unsigned int snoop_detect(void)
{
	unsigned int result;
	asm("lda [%%g0] 2, %0" : "=r"(result));
	return result & 0x00800000;
}

int arch_cpu_init(void)
{
	ambapp_apbdev apbdev;
	int index;

	gd->cpu_clk = CONFIG_SYS_CLK_FREQ;
	gd->bus_clk = CONFIG_SYS_CLK_FREQ;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	gd->arch.snooping_available = snoop_detect();

	/* Initialize the AMBA Plug & Play bus structure, the bus
	 * structure represents the AMBA bus that the CPU is located at.
	 */
	ambapp_bus_init(CONFIG_AMBAPP_IOAREA, CONFIG_SYS_CLK_FREQ, &ambapp_plb);

	/* Initialize/clear all the timers in the system.
	 */
	for (index = 0; ambapp_apb_find(&ambapp_plb, VENDOR_GAISLER,
		GAISLER_GPTIMER, index, &apbdev) == 1; index++) {
		ambapp_dev_gptimer *timer;
		unsigned int bus_freq;
		int i, ntimers;

		timer = (ambapp_dev_gptimer *)apbdev.address;

		/* Different buses may have different frequency, the
		 * frequency of the bus tell in which frequency the timer
		 * prescaler operates.
		 */
		bus_freq = ambapp_bus_freq(&ambapp_plb, apbdev.ahb_bus_index);

		/* Initialize prescaler common to all timers to 1MHz */
		timer->scalar = timer->scalar_reload =
			(((bus_freq / 1000) + 500) / 1000) - 1;

		/* Clear all timers */
		ntimers = timer->config & 0x7;
		for (i = 0; i < ntimers; i++) {
			timer->e[i].ctrl = GPTIMER_CTRL_IP;
			timer->e[i].rld = 0;
			timer->e[i].ctrl = GPTIMER_CTRL_LD;
		}
	}

	return 0;
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r(void)
{
	ambapp_apbdev apbdev;
	int cpu;

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

	return 0;
}

			;
int timer_init(void)
{
	ambapp_dev_gptimer_element *tmr;
	ambapp_dev_gptimer *gptimer;
	ambapp_apbdev apbdev;
	unsigned bus_freq;

	if (ambapp_apb_find(&ambapp_plb, VENDOR_GAISLER, GAISLER_GPTIMER,
		CONFIG_SYS_GRLIB_GPTIMER_INDEX, &apbdev) != 1) {
		panic("%s: gptimer not found!\n", __func__);
		return -1;
	}

	gptimer = (ambapp_dev_gptimer *) apbdev.address;

	/* Different buses may have different frequency, the
	 * frequency of the bus tell in which frequency the timer
	 * prescaler operates.
	 */
	bus_freq = ambapp_bus_freq(&ambapp_plb, apbdev.ahb_bus_index);

	/* initialize prescaler common to all timers to 1MHz */
	gptimer->scalar = gptimer->scalar_reload =
		(((bus_freq / 1000) + 500) / 1000) - 1;

	tmr = (ambapp_dev_gptimer_element *)&gptimer->e[0];

	tmr->val = 0;
	tmr->rld = ~0;
	tmr->ctrl = GPTIMER_CTRL_EN | GPTIMER_CTRL_RS | GPTIMER_CTRL_LD;

	CONFIG_SYS_TIMER_COUNTER = (void *)&tmr->val;
	return 0;
}
