/*
 * (C) Copyright 2000 - 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 1995-1996  Gary Thomas (gdt@linuxppc.org)
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

/*
 * This file handles the architecture-dependent parts of hardware
 * exceptions
 */

#include <common.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

extern unsigned long search_exception_table(unsigned long);

/*
 * End of addressable memory.  This may be less than the actual
 * amount of memory on the system if we're unable to keep all
 * the memory mapped in.
 */
extern ulong get_effective_memsize(void);
#define END_OF_MEM (gd->bd->bi_memstart + get_effective_memsize())

/*
 * Trap & Exception support
 */

void
print_backtrace (unsigned long *sp)
{
	int cnt = 0;
	unsigned long i;

	puts ("Call backtrace: ");
	while (sp) {
		if ((uint)sp > END_OF_MEM)
			break;

		i = sp[1];
		if (cnt++ % 7 == 0)
			putc ('\n');
		printf ("%08lX ", i);
		if (cnt > 32) break;
		sp = (unsigned long *) *sp;
	}
	putc ('\n');
}

void show_regs (struct pt_regs * regs)
{
	int i;

	printf ("NIP: %08lX XER: %08lX LR: %08lX REGS: %p TRAP: %04lx DAR: %08lX\n",
	       regs->nip, regs->xer, regs->link, regs, regs->trap, regs->dar);
	printf ("MSR: %08lx EE: %01x PR: %01x FP: %01x ME: %01x IR/DR: %01x%01x\n",
	       regs->msr, regs->msr & MSR_EE ? 1 : 0, regs->msr & MSR_PR ? 1 : 0,
	       regs->msr & MSR_FP ? 1 : 0,regs->msr & MSR_ME ? 1 : 0,
	       regs->msr & MSR_IR ? 1 : 0,
	       regs->msr & MSR_DR ? 1 : 0);

	putc ('\n');
	for (i = 0;  i < 32;  i++) {
		if ((i % 8) == 0) {
			printf ("GPR%02d: ", i);
		}

		printf ("%08lX ", regs->gpr[i]);
		if ((i % 8) == 7) {
			putc ('\n');
		}
	}
}


void
_exception (int signr, struct pt_regs *regs)
{
	show_regs (regs);
	print_backtrace ((unsigned long *)regs->gpr[1]);
	panic ("Exception at pc %lx signal %d", regs->nip,signr);
}


void
MachineCheckException (struct pt_regs *regs)
{
	unsigned long fixup;

	if ((fixup = search_exception_table (regs->nip)) != 0) {
		regs->nip = fixup;
		return;
	}

#ifdef CONFIG_CMD_KGDB
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	puts ("Machine check.\nCaused by (from msr): ");
	printf ("regs %p ",regs);
	switch (regs->msr & 0x00FF0000) {
	case (0x80000000 >> 10):
		puts ("Instruction cache parity signal\n");
		break;
	case (0x80000000 >> 11):
		puts ("Data cache parity signal\n");
		break;
	case (0x80000000 >> 12):
		puts ("Machine check signal\n");
		break;
	case (0x80000000 >> 13):
		puts ("Transfer error ack signal\n");
		break;
	case (0x80000000 >> 14):
		puts ("Data parity signal\n");
		break;
	case (0x80000000 >> 15):
		puts ("Address parity signal\n");
		break;
	default:
		puts ("Unknown values in msr\n");
	}
	show_regs (regs);
	print_backtrace ((unsigned long *)regs->gpr[1]);

	panic ("machine check");
}

void
AlignmentException (struct pt_regs *regs)
{
#ifdef CONFIG_CMD_KGDB
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs (regs);
	print_backtrace ((unsigned long *)regs->gpr[1]);
	panic ("Alignment Exception");
}

void
ProgramCheckException (struct pt_regs *regs)
{
#ifdef CONFIG_CMD_KGDB
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs (regs);
	print_backtrace ((unsigned long *)regs->gpr[1]);
	panic ("Program Check Exception");
}

void
SoftEmuException (struct pt_regs *regs)
{
#ifdef CONFIG_CMD_KGDB
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs (regs);
	print_backtrace ((unsigned long *)regs->gpr[1]);
	panic ("Software Emulation Exception");
}


void
UnknownException (struct pt_regs *regs)
{
#ifdef CONFIG_CMD_KGDB
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	printf ("Bad trap at PC: %lx, SR: %lx, vector=%lx\n",
	       regs->nip, regs->msr, regs->trap);
	_exception (0, regs);
}

#ifdef CONFIG_CMD_BEDBUG
extern void do_bedbug_breakpoint (struct pt_regs *);
#endif

void
DebugException (struct pt_regs *regs)
{
	printf ("Debugger trap at @ %lx\n", regs->nip );
	show_regs (regs);
#ifdef CONFIG_CMD_BEDBUG
	do_bedbug_breakpoint (regs);
#endif
}
