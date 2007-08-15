/*
 * Copyright (C) 1995-1996  Gary Thomas (gdt@linuxppc.org)
 *
 * Modified by Cort Dougan (cort@cs.nmt.edu)
 * and Paul Mackerras (paulus@cs.anu.edu.au)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * This file handles the architecture-dependent parts of hardware exceptions
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>

#if defined(CONFIG_CMD_KGDB)
int (*debugger_exception_handler)(struct pt_regs *) = 0;
#endif

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

#define END_OF_MEM (gd->bd->bi_memstart + gd->bd->bi_memsize)

/*
 * Trap & Exception support
 */

void
print_backtrace(unsigned long *sp)
{
	DECLARE_GLOBAL_DATA_PTR;

	int cnt = 0;
	unsigned long i;

	printf("Call backtrace: ");
	while (sp) {
		if ((uint) sp > END_OF_MEM)
			break;

		i = sp[1];
		if (cnt++ % 7 == 0)
			printf("\n");
		printf("%08lX ", i);
		if (cnt > 32)
			break;
		sp = (unsigned long *)*sp;
	}
	printf("\n");
}

void
show_regs(struct pt_regs *regs)
{
	int i;

	printf("NIP: %08lX XER: %08lX LR: %08lX REGS:"
	       " %p TRAP: %04lx DAR: %08lX\n",
	       regs->nip, regs->xer, regs->link, regs, regs->trap, regs->dar);
	printf("MSR: %08lx EE: %01x PR: %01x FP:"
	       " %01x ME: %01x IR/DR: %01x%01x\n",
	       regs->msr, regs->msr & MSR_EE ? 1 : 0,
	       regs->msr & MSR_PR ? 1 : 0, regs->msr & MSR_FP ? 1 : 0,
	       regs->msr & MSR_ME ? 1 : 0, regs->msr & MSR_IR ? 1 : 0,
	       regs->msr & MSR_DR ? 1 : 0);

	printf("\n");
	for (i = 0; i < 32; i++) {
		if ((i % 8) == 0) {
			printf("GPR%02d: ", i);
		}

		printf("%08lX ", regs->gpr[i]);
		if ((i % 8) == 7) {
			printf("\n");
		}
	}
}


void
_exception(int signr, struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Exception in kernel pc %lx signal %d", regs->nip, signr);
}

void
MachineCheckException(struct pt_regs *regs)
{
	unsigned long fixup;

	/* Probing PCI using config cycles cause this exception
	 * when a device is not present.  Catch it and return to
	 * the PCI exception handler.
	 */
	if ((fixup = search_exception_table(regs->nip)) != 0) {
		regs->nip = fixup;
		return;
	}

#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler) (regs))
		return;
#endif

	printf("Machine check in kernel mode.\n");
	printf("Caused by (from msr): ");
	printf("regs %p ", regs);
	switch ( regs->msr & 0x001F0000) {
	case (0x80000000>>11):
		printf("MSS error. MSSSR0: %08x\n", mfspr(SPRN_MSSSR0));
		break;
	case (0x80000000>>12):
		printf("Machine check signal - probably due to mm fault\n"
		       "with mmu off\n");
		break;
	case (0x80000000 >> 13):
		printf("Transfer error ack signal\n");
		break;
	case (0x80000000 >> 14):
		printf("Data parity signal\n");
		break;
	case (0x80000000 >> 15):
		printf("Address parity signal\n");
		break;
	default:
		printf("Unknown values in msr\n");
	}
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("machine check");
}

void
AlignmentException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler) (regs))
		return;
#endif
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Alignment Exception");
}

void
ProgramCheckException(struct pt_regs *regs)
{
	unsigned char *p = regs ? (unsigned char *)(regs->nip) : NULL;
	int i, j;

#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler) (regs))
		return;
#endif
	show_regs(regs);

	p = (unsigned char *)((unsigned long)p & 0xFFFFFFE0);
	p -= 32;
	for (i = 0; i < 256; i += 16) {
		printf("%08x: ", (unsigned int)p + i);
		for (j = 0; j < 16; j++) {
			printf("%02x ", p[i + j]);
		}
		printf("\n");
	}

	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Program Check Exception");
}

void
SoftEmuException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler) (regs))
		return;
#endif
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Software Emulation Exception");
}

void
UnknownException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler) (regs))
		return;
#endif
	printf("UnknownException regs@%x\n", regs);
	printf("Bad trap at PC: %lx, SR: %lx, vector=%lx\n",
	       regs->nip, regs->msr, regs->trap);
	_exception(0, regs);
}

/*
 * Probe an address by reading.
 * If not present, return -1,
 * otherwise return 0.
 */
int
addr_probe(uint *addr)
{
	return 0;
}
