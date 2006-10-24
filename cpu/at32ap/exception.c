/*
 * Copyright (C) 2005-2006 Atmel Corporation
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
#include <common.h>

#include <asm/sysreg.h>
#include <asm/ptrace.h>

static const char * const cpu_modes[8] = {
	"Application", "Supervisor", "Interrupt level 0", "Interrupt level 1",
	"Interrupt level 2", "Interrupt level 3", "Exception", "NMI"
};

static void dump_mem(const char *str, unsigned long bottom, unsigned long top)
{
	unsigned long p;
	int i;

	printf("%s(0x%08lx to 0x%08lx)\n", str, bottom, top);

	for (p = bottom & ~31; p < top; ) {
		printf("%04lx: ", p & 0xffff);

		for (i = 0; i < 8; i++, p += 4) {
			unsigned int val;

			if (p < bottom || p >= top)
				printf("         ");
			else {
				val = *(unsigned long *)p;
				printf("%08x ", val);
			}
		}
		printf("\n");
	}
}

void do_unknown_exception(unsigned int ecr, struct pt_regs *regs)
{
	unsigned int mode;

	printf("\n *** Unhandled exception %u at PC=0x%08lx\n", ecr, regs->pc);

	switch (ecr) {
	case ECR_BUS_ERROR_WRITE:
	case ECR_BUS_ERROR_READ:
		printf("Bus error at address 0x%08lx\n",
		       sysreg_read(BEAR));
		break;
	case ECR_TLB_MULTIPLE:
	case ECR_ADDR_ALIGN_X:
	case ECR_PROTECTION_X:
	case ECR_ADDR_ALIGN_R:
	case ECR_ADDR_ALIGN_W:
	case ECR_PROTECTION_R:
	case ECR_PROTECTION_W:
	case ECR_DTLB_MODIFIED:
	case ECR_TLB_MISS_X:
	case ECR_TLB_MISS_R:
	case ECR_TLB_MISS_W:
		printf("MMU exception at address 0x%08lx\n",
		       sysreg_read(TLBEAR));
		break;
	}

	printf("   pc: %08lx    lr: %08lx    sp: %08lx   r12: %08lx\n",
	       regs->pc, regs->lr, regs->sp, regs->r12);
	printf("  r11: %08lx   r10: %08lx    r9: %08lx    r8: %08lx\n",
	       regs->r11, regs->r10, regs->r9, regs->r8);
	printf("   r7: %08lx    r6: %08lx    r5: %08lx    r4: %08lx\n",
	       regs->r7, regs->r6, regs->r5, regs->r4);
	printf("   r3: %08lx    r2: %08lx    r1: %08lx    r0: %08lx\n",
	       regs->r3, regs->r2, regs->r1, regs->r0);
	printf("Flags: %c%c%c%c%c\n",
	       regs->sr & SR_Q ? 'Q' : 'q',
	       regs->sr & SR_V ? 'V' : 'v',
	       regs->sr & SR_N ? 'N' : 'n',
	       regs->sr & SR_Z ? 'Z' : 'z',
	       regs->sr & SR_C ? 'C' : 'c');
	printf("Mode bits: %c%c%c%c%c%c%c%c%c\n",
	       regs->sr & SR_H ? 'H' : 'h',
	       regs->sr & SR_R ? 'R' : 'r',
	       regs->sr & SR_J ? 'J' : 'j',
	       regs->sr & SR_EM ? 'E' : 'e',
	       regs->sr & SR_I3M ? '3' : '.',
	       regs->sr & SR_I2M ? '2' : '.',
	       regs->sr & SR_I1M ? '1' : '.',
	       regs->sr & SR_I0M ? '0' : '.',
	       regs->sr & SR_GM ? 'G' : 'g');
	mode = (regs->sr >> SYSREG_M0_OFFSET) & 7;
	printf("CPU Mode: %s\n", cpu_modes[mode]);

	/* Avoid exception loops */
	if (regs->sp >= CFG_INIT_SP_ADDR
	    || regs->sp < (CFG_INIT_SP_ADDR - CONFIG_STACKSIZE))
		printf("\nStack pointer seems bogus, won't do stack dump\n");
	else
		dump_mem("\nStack: ", regs->sp, CFG_INIT_SP_ADDR);

	panic("Unhandled exception\n");
}
