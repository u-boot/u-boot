/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
 * CPU specific code
 */

#include <common.h>
#include <command.h>

int cpu_init(void)
{
	/* initialize FPU, reset EM, set MP and NE */
	asm ("fninit\n" \
	     "movl %cr0, %eax\n" \
	     "andl $~0x4, %eax\n" \
	     "orl  $0x22, %eax\n" \
	     "movl %eax, %cr0\n" );

	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf ("resetting ...\n");
	udelay(50000);				/* wait 50 ms */
	disable_interrupts();
	reset_cpu(0);

	/*NOTREACHED*/
	return 0;
}

void  flush_cache (unsigned long dummy1, unsigned long dummy2)
{
	asm("wbinvd\n");
	return;
}
