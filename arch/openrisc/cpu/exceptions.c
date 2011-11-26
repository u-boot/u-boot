/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
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

#include <common.h>
#include <stdio_dev.h>
#include <asm/system.h>

static const char * const excp_table[] = {
	"Unknown exception",
	"Reset",
	"Bus Error",
	"Data Page Fault",
	"Instruction Page Fault",
	"Tick Timer",
	"Alignment",
	"Illegal Instruction",
	"External Interrupt",
	"D-TLB Miss",
	"I-TLB Miss",
	"Range",
	"System Call",
	"Floating Point",
	"Trap",
};

static void (*handlers[32])(void);

void exception_install_handler(int exception, void (*handler)(void))
{
	if (exception < 0 || exception > 31)
		return;

	handlers[exception] = handler;
}

void exception_free_handler(int exception)
{
	if (exception < 0 || exception > 31)
		return;

	handlers[exception] = 0;
}

static void exception_hang(int vect)
{
	printf("Unhandled exception at 0x%x ", vect & 0xff00);

	vect = ((vect >> 8) & 0xff);
	if (vect < ARRAY_SIZE(excp_table))
		printf("(%s)\n", excp_table[vect]);
	else
		printf("(%s)\n", excp_table[0]);

	printf("EPCR: 0x%08lx\n", mfspr(SPR_EPCR_BASE));
	printf("EEAR: 0x%08lx\n", mfspr(SPR_EEAR_BASE));
	printf("ESR:  0x%08lx\n", mfspr(SPR_ESR_BASE));
	hang();
}

void exception_handler(int vect)
{
	int exception = vect >> 8;

	if (handlers[exception])
		handlers[exception]();
	else
		exception_hang(vect);
}
