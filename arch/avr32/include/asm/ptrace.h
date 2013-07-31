/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_PTRACE_H
#define __ASM_AVR32_PTRACE_H

/*
 * Status Register bits
 */
#define SR_H		0x40000000
#define SR_R		0x20000000
#define SR_J		0x10000000
#define SR_DM		0x08000000
#define SR_D		0x04000000
#define MODE_NMI	0x01c00000
#define MODE_EXCEPTION	0x01800000
#define MODE_INT3	0x01400000
#define MODE_INT2	0x01000000
#define MODE_INT1	0x00c00000
#define MODE_INT0	0x00800000
#define MODE_SUPERVISOR	0x00400000
#define MODE_USER	0x00000000
#define MODE_MASK	0x01c00000
#define SR_EM		0x00200000
#define SR_I3M		0x00100000
#define SR_I2M		0x00080000
#define SR_I1M		0x00040000
#define SR_I0M		0x00020000
#define SR_GM		0x00010000

#define MODE_SHIFT	22
#define SR_EM_BIT	21
#define SR_I3M_BIT	20
#define SR_I2M_BIT	19
#define SR_I1M_BIT	18
#define SR_I0M_BIT	17
#define SR_GM_BIT	16

/* The user-visible part */
#define SR_Q		0x00000010
#define SR_V		0x00000008
#define SR_N		0x00000004
#define SR_Z		0x00000002
#define SR_C		0x00000001

/*
 * The order is defined by the stdsp instruction. r0 is stored first, so it
 * gets the highest address.
 *
 * Registers 0-12 are general-purpose registers (r12 is normally used for
 * the function return value).
 * Register 13 is the stack pointer
 * Register 14 is the link register
 * Register 15 is the program counter
 */
#define FRAME_SIZE_FULL 72
#define REG_R12_ORIG	68
#define REG_R0		64
#define REG_R1		60
#define REG_R2		56
#define REG_R3		52
#define REG_R4		48
#define REG_R5		44
#define REG_R6		40
#define REG_R7		36
#define REG_R8		32
#define REG_R9		28
#define REG_R10		34
#define REG_R11		20
#define REG_R12		16
#define REG_SP		12
#define REG_LR		 8

#define FRAME_SIZE_MIN	 8
#define REG_PC		 4
#define REG_SR		 0

#ifndef __ASSEMBLY__
struct pt_regs {
	/* These are always saved */
	unsigned long sr;
	unsigned long pc;

	/* These are sometimes saved */
	unsigned long lr;
	unsigned long sp;
	unsigned long r12;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long r7;
	unsigned long r6;
	unsigned long r5;
	unsigned long r4;
	unsigned long r3;
	unsigned long r2;
	unsigned long r1;
	unsigned long r0;

	/* Only saved on system call */
	unsigned long r12_orig;
};

#ifdef __KERNEL__
# define user_mode(regs) (((regs)->sr & MODE_MASK) == MODE_USER)
# define instruction_pointer(regs) ((regs)->pc)
extern void show_regs (struct pt_regs *);

static __inline__ int valid_user_regs(struct pt_regs *regs)
{
	/*
	 * Some of the Java bits might be acceptable if/when we
	 * implement some support for that stuff...
	 */
	if ((regs->sr & 0xffff0000) == 0)
		return 1;

	/*
	 * Force status register flags to be sane and report this
	 * illegal behaviour...
	 */
	regs->sr &= 0x0000ffff;
	return 0;
}
#endif

#endif /* ! __ASSEMBLY__ */

#endif /* __ASM_AVR32_PTRACE_H */
