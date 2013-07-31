/*
 * (C) Copyright 2006
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * This is a very simple standalone application demonstrating
 * catching IRQs on the MPC52xx architecture.
 *
 * The interrupt to be intercepted can be specified as an argument
 * to the application.  Specifying nothing will intercept IRQ1 on the
 * MPC5200 platform.  On the CR825 carrier board from MicroSys this
 * maps to the ABORT switch :)
 *
 * Note that the specified vector is only a logical number specified
 * by the respective header file.
 */

#include <common.h>
#include <exports.h>
#include <config.h>

#if defined(CONFIG_MPC5xxx)
#define DFL_IRQ MPC5XXX_IRQ1
#else
#define DFL_IRQ 0
#endif

static void irq_handler (void *arg);

int interrupt (int argc, char * const argv[])
{
	int c, irq = -1;

	app_startup (argv);

	if (argc > 1)
		irq = simple_strtoul (argv[1], NULL, 0);
	if ((irq < 0) || (irq > NR_IRQS))
		irq = DFL_IRQ;

	printf ("Installing handler for irq vector %d and doing busy wait\n",
		irq);
	printf ("Press 'q' to quit\n");

	/* Install interrupt handler */
	install_hdlr (irq, irq_handler, NULL);
	while ((c = getc ()) != 'q') {
		printf ("Ok, ok, I am still alive!\n");
	}

	free_hdlr (irq);
	printf ("\nInterrupt handler has been uninstalled\n");

	return (0);
}

/*
 * Handler for interrupt
 */
static void irq_handler (void *arg)
{
	/* just for demonstration */
	printf ("+");
}
