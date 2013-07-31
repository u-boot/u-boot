/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
