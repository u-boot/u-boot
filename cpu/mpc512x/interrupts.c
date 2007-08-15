/*
 * (C) Copyright 2000-2007
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
 * Derived from the MPC83xx code.
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

struct irq_action {
	interrupt_handler_t *handler;
	void *arg;
	ulong count;
};

int interrupt_init_cpu (unsigned *decrementer_count)
{
	*decrementer_count = get_tbclk () / CFG_HZ;

	return 0;
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
