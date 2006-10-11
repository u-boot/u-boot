/*
 * (C) Copyright 2006
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
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

int interrupt (int argc, char *argv[])
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
