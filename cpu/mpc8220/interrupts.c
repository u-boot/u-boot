/*
 * (C) Copyright -2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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
 */

/*
 * interrupts.c - just enough support for the decrementer/timer
 */

#include <common.h>
#include <asm/processor.h>
#include <command.h>

int interrupt_init_cpu (ulong * decrementer_count)
{
	*decrementer_count = get_tbclk () / CFG_HZ;

	return (0);
}

/****************************************************************************/

/*
 * Handle external interrupts
 */
void external_interrupt (struct pt_regs *regs)
{
	puts ("external_interrupt (oops!)\n");
}

void timer_interrupt_cpu (struct pt_regs *regs)
{
	/* nothing to do here */
	return;
}

/****************************************************************************/

/*
 * Install and free a interrupt handler.
 */

void irq_install_handler (int vec, interrupt_handler_t * handler, void *arg)
{

}

void irq_free_handler (int vec)
{

}

/****************************************************************************/

void
do_irqinfo (cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char *argv[])
{
	puts ("IRQ related functions are unimplemented currently.\n");
}
