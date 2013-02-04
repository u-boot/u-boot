/*
 * linux/arch/powerpc/kernel/traps.c
 *
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
#include <kgdb.h>
#include <asm/processor.h>
#include <asm/m8260_pci.h>

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

/* THIS NEEDS CHANGING to use the board info structure.
*/
#define END_OF_MEM	0x02000000

/*
 * Trap & Exception support
 */

static void print_backtrace(unsigned long *sp)
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
		printf("%08lX ", i);
		if (cnt > 32) break;
		sp = (unsigned long *)*sp;
	}
	putc ('\n');
}

void show_regs(struct pt_regs *regs)
{
	int i;

	printf("NIP: %08lX XER: %08lX LR: %08lX REGS: %p TRAP: %04lx DAR: %08lX\n",
	       regs->nip, regs->xer, regs->link, regs, regs->trap, regs->dar);
	printf("MSR: %08lx EE: %01x PR: %01x FP: %01x ME: %01x IR/DR: %01x%01x\n",
	       regs->msr, regs->msr&MSR_EE ? 1 : 0, regs->msr&MSR_PR ? 1 : 0,
	       regs->msr & MSR_FP ? 1 : 0,regs->msr&MSR_ME ? 1 : 0,
	       regs->msr&MSR_IR ? 1 : 0,
	       regs->msr&MSR_DR ? 1 : 0);

	putc ('\n');
	for (i = 0;  i < 32;  i++) {
		if ((i % 8) == 0) {
			printf("GPR%02d: ", i);
		}

		printf("%08lX ", regs->gpr[i]);
		if ((i % 8) == 7) {
			putc ('\n');
		}
	}
}


static void _exception(int signr, struct pt_regs *regs)
{
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Exception in kernel pc %lx signal %d",regs->nip,signr);
}

#ifdef CONFIG_PCI
void dump_pci (void)
{

	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	printf ("PCI: err status %x err mask %x err ctrl %x\n",
		le32_to_cpu (immap->im_pci.pci_esr),
		le32_to_cpu (immap->im_pci.pci_emr),
		le32_to_cpu (immap->im_pci.pci_ecr));
	printf ("     error address %x error data %x ctrl %x\n",
		le32_to_cpu (immap->im_pci.pci_eacr),
		le32_to_cpu (immap->im_pci.pci_edcr),
		le32_to_cpu (immap->im_pci.pci_eccr));

}
#endif

void MachineCheckException(struct pt_regs *regs)
{
	unsigned long fixup;

	/* Probing PCI using config cycles cause this exception
	 * when a device is not present.  Catch it and return to
	 * the PCI exception handler.
	 */
#ifdef CONFIG_PCI
	volatile immap_t *immap  = (immap_t *)CONFIG_SYS_IMMR;
#ifdef DEBUG
	dump_pci();
#endif
	/* clear the error in the error status register */
	if(immap->im_pci.pci_esr & cpu_to_le32(PCI_ERROR_PCI_NO_RSP)) {
		immap->im_pci.pci_esr = cpu_to_le32(PCI_ERROR_PCI_NO_RSP);
		return;
	}
#endif
	if ((fixup = search_exception_table(regs->nip)) != 0) {
		regs->nip = fixup;
		return;
	}

#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif

	puts ("Machine check in kernel mode.\n"
		"Caused by (from msr): ");
	printf("regs %p ",regs);
	switch( regs->msr & 0x000F0000) {
	case (0x80000000>>12):
		puts ("Machine check signal - probably due to mm fault\n"
			"with mmu off\n");
		break;
	case (0x80000000>>13):
		puts ("Transfer error ack signal\n");
		break;
	case (0x80000000>>14):
		puts ("Data parity signal\n");
		break;
	case (0x80000000>>15):
		puts ("Address parity signal\n");
		break;
	default:
		puts ("Unknown values in msr\n");
	}
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
#ifdef CONFIG_PCI
	dump_pci();
#endif
	panic("machine check");
}

void AlignmentException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Alignment Exception");
}

void ProgramCheckException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Program Check Exception");
}

void SoftEmuException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	show_regs(regs);
	print_backtrace((unsigned long *)regs->gpr[1]);
	panic("Software Emulation Exception");
}


void UnknownException(struct pt_regs *regs)
{
#if defined(CONFIG_CMD_KGDB)
	if (debugger_exception_handler && (*debugger_exception_handler)(regs))
		return;
#endif
	printf("Bad trap at PC: %lx, SR: %lx, vector=%lx\n",
	       regs->nip, regs->msr, regs->trap);
	_exception(0, regs);
}

#if defined(CONFIG_CMD_BEDBUG)
extern void do_bedbug_breakpoint(struct pt_regs *);
#endif

void DebugException(struct pt_regs *regs)
{

  printf("Debugger trap at @ %lx\n", regs->nip );
  show_regs(regs);
#if defined(CONFIG_CMD_BEDBUG)
  do_bedbug_breakpoint( regs );
#endif
}

/* Probe an address by reading.  If not present, return -1, otherwise
 * return 0.
 */
int addr_probe(uint *addr)
{
#if 0
	int	retval;

	__asm__ __volatile__(			\
		"1:	lwz %0,0(%1)\n"		\
		"	eieio\n"		\
		"	li %0,0\n"		\
		"2:\n"				\
		".section .fixup,\"ax\"\n"	\
		"3:	li %0,-1\n"		\
		"	b 2b\n"			\
		".section __ex_table,\"a\"\n"	\
		"	.align 2\n"		\
		"	.long 1b,3b\n"		\
		".text"				\
		: "=r" (retval) : "r"(addr));

	return (retval);
#endif
	return 0;
}
