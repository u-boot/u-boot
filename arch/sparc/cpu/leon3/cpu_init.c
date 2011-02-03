/* Initializes CPU and basic hardware such as memory
 * controllers, IRQ controller and system timer 0.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
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
 *
 */

#include <common.h>
#include <asm/asi.h>
#include <asm/leon.h>
#include <ambapp.h>

#include <config.h>

DECLARE_GLOBAL_DATA_PTR;

/* reset CPU (jump to 0, without reset) */
void start(void);

/* find & initialize the memory controller */
int init_memory_ctrl(void);

ambapp_dev_irqmp *irqmp = NULL;
ambapp_dev_mctrl memctrl;
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
 * Set up the memory map,
 * initialize a bunch of registers.
 *
 * Run from FLASH/PROM:
 *  - until memory controller is set up, only registers available
 *  - no global variables available for writing
 *  - constants available
 */

void cpu_init_f(void)
{
	/* these varaiable must not be initialized */
	ambapp_dev_irqmp *irqmp;
	ambapp_apbdev apbdev;
	register unsigned int apbmst;

	/* find AMBA APB Master */
	apbmst = (unsigned int)
	    ambapp_ahb_next_nomem(VENDOR_GAISLER, GAISLER_APBMST, 1, 0);
	if (!apbmst) {
		/*
		 * no AHB/APB bridge, something is wrong
		 * ==> jump to start (or hang)
		 */
		while (1) ;
	}
	/* Init memory controller */
	if (init_memory_ctrl()) {
		while (1) ;
	}

	/****************************************************
	 * From here we can use the main memory and the stack.
	 */

	/* Find AMBA APB IRQMP Controller */
	if (ambapp_apb_first(VENDOR_GAISLER, GAISLER_IRQMP, &apbdev) != 1) {
		/* no IRQ controller, something is wrong
		 * ==> jump to start (or hang)
		 */
		while (1) ;
	}
	irqmp = (ambapp_dev_irqmp *) apbdev.address;

	/* initialize the IRQMP */
	irqmp->ilevel = 0xf;	/* all IRQ off */
	irqmp->iforce = 0;
	irqmp->ipend = 0;
	irqmp->iclear = 0xfffe;	/* clear all old pending interrupts */
	irqmp->cpu_mask[0] = 0;	/* mask all IRQs on CPU 0 */
	irqmp->cpu_force[0] = 0;	/* no force IRQ on CPU 0 */

	/* cache */
}

void cpu_init_f2(void)
{

}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r(void)
{
	ambapp_apbdev apbdev;

	/*
	 * Find AMBA APB IRQMP Controller,
	 * When we come so far we know there is a IRQMP available
	 */
	ambapp_apb_first(VENDOR_GAISLER, GAISLER_IRQMP, &apbdev);
	irqmp = (ambapp_dev_irqmp *) apbdev.address;

	/* timer */
	if (ambapp_apb_first(VENDOR_GAISLER, GAISLER_GPTIMER, &apbdev) != 1) {
		printf("cpu_init_r: gptimer not found!\n");
		return 1;
	}
	gptimer = (ambapp_dev_gptimer *) apbdev.address;
	gptimer_irq = apbdev.irq;

	/* initialize prescaler common to all timers to 1MHz */
	gptimer->scalar = gptimer->scalar_reload =
	    (((CONFIG_SYS_CLK_FREQ / 1000) + 500) / 1000) - 1;

	return (0);
}

/* find & setup memory controller */
int init_memory_ctrl()
{
	register ambapp_dev_mctrl *mctrl;
	register ambapp_dev_sdctrl *sdctrl;
	register ambapp_dev_ddrspa *ddrspa;
	register ambapp_dev_ddr2spa *ddr2spa;
	register ahbctrl_pp_dev *ahb;
	register unsigned int base;
	register int not_found_mctrl = -1;

	/* find ESA Memory controller */
	base = ambapp_apb_next_nomem(VENDOR_ESA, ESA_MCTRL, 0);
	if (base) {
		mctrl = (ambapp_dev_mctrl *) base;

		/* config MCTRL memory controller */
		mctrl->mcfg1 = CONFIG_SYS_GRLIB_MEMCFG1 | (mctrl->mcfg1 & 0x300);
		mctrl->mcfg2 = CONFIG_SYS_GRLIB_MEMCFG2;
		mctrl->mcfg3 = CONFIG_SYS_GRLIB_MEMCFG3;
		not_found_mctrl = 0;
	}

	/* find Gaisler Fault Tolerant Memory controller */
	base = ambapp_apb_next_nomem(VENDOR_GAISLER, GAISLER_FTMCTRL, 0);
	if (base) {
		mctrl = (ambapp_dev_mctrl *) base;

		/* config MCTRL memory controller */
		mctrl->mcfg1 = CONFIG_SYS_GRLIB_FT_MEMCFG1 | (mctrl->mcfg1 & 0x300);
		mctrl->mcfg2 = CONFIG_SYS_GRLIB_FT_MEMCFG2;
		mctrl->mcfg3 = CONFIG_SYS_GRLIB_FT_MEMCFG3;
		not_found_mctrl = 0;
	}

	/* find SDRAM controller */
	base = ambapp_apb_next_nomem(VENDOR_GAISLER, GAISLER_SDCTRL, 0);
	if (base) {
		sdctrl = (ambapp_dev_sdctrl *) base;

		/* config memory controller */
		sdctrl->sdcfg = CONFIG_SYS_GRLIB_SDRAM;
		not_found_mctrl = 0;
	}

	ahb = ambapp_ahb_next_nomem(VENDOR_GAISLER, GAISLER_DDR2SPA, 1, 0);
	if (ahb) {
		ddr2spa = (ambapp_dev_ddr2spa *) ambapp_ahb_get_info(ahb, 1);

		/* Config DDR2 memory controller */
		ddr2spa->cfg1 = CONFIG_SYS_GRLIB_DDR2_CFG1;
		ddr2spa->cfg3 = CONFIG_SYS_GRLIB_DDR2_CFG3;
		not_found_mctrl = 0;
	}

	ahb = ambapp_ahb_next_nomem(VENDOR_GAISLER, GAISLER_DDRSPA, 1, 0);
	if (ahb) {
		ddrspa = (ambapp_dev_ddrspa *) ambapp_ahb_get_info(ahb, 1);

		/* Config DDR memory controller */
		ddrspa->ctrl = CONFIG_SYS_GRLIB_DDR_CFG;
		not_found_mctrl = 0;
	}

	/* failed to find any memory controller */
	return not_found_mctrl;
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

/* initiate and setup timer0 interrupt to 1MHz
 * Return irq number for timer int or a negative number for
 * dealing with self
 */
int timer_interrupt_init_cpu(void)
{
	/* 1ms ticks */
	gptimer->e[0].val = 0;
	gptimer->e[0].rld = 999;	/* (((1000000 / 100) - 1)) */
	gptimer->e[0].ctrl =
	    (LEON3_GPTIMER_EN |
	     LEON3_GPTIMER_RL | LEON3_GPTIMER_LD | LEON3_GPTIMER_IRQEN);

	return gptimer_irq;
}

/*
 * This function is intended for SHORT delays only.
 */
unsigned long cpu_usec2ticks(unsigned long usec)
{
	/* timer set to 1kHz ==> 1 clk tick = 1 msec */
	if (usec < 1000)
		return 1;
	return (usec / 1000);
}

unsigned long cpu_ticks2usec(unsigned long ticks)
{
	/* 1tick = 1usec */
	return ticks * 1000;
}
