/*
 * (C) Copyright 2000-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2004 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
	*decrementer_count = get_tbclk () / CONFIG_SYS_HZ;

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
