/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2004 Freescale Semiconductor, Inc.
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
 * Change log:
 *
 * Hacked for MPC8260 by Murray.Jensen@cmst.csiro.au, 22-Oct-00
 *
 * 20050101: Eran Liberty (liberty@freescale.com)
 *           Initial file creating (porting from 85XX & 8260)
 */

#include <common.h>
#include <command.h>
#include <mpc83xx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	ulong count;
};

int interrupt_init_cpu (unsigned *decrementer_count)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMRBAR;

	*decrementer_count = (gd->bus_clk / 4) / CFG_HZ;

	/* Enable e300 time base */

	immr->sysconf.spcr |= 0x00400000;

	return 0;
}


/*
 * Handle external interrupts
 */

void external_interrupt (struct pt_regs *regs)
{
}


/*
 * Install and free an interrupt handler.
 */

void
irq_install_handler (int irq, interrupt_handler_t * handler, void *arg)
{
}


void irq_free_handler (int irq)
{
}


void timer_interrupt_cpu (struct pt_regs *regs)
{
	/* nothing to do here */
	return;
}


#if (CONFIG_COMMANDS & CFG_CMD_IRQ)

/* ripped this out of ppc4xx/interrupts.c */

/*
 * irqinfo - print information about PCI devices
 */

void
do_irqinfo(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
}

#endif		/* CONFIG_COMMANDS & CFG_CMD_IRQ */
