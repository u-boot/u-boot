/* SPARC stack layout Macros and structures,
 * mainly taken from BCC (the Bare C compiler for
 * SPARC LEON2/3) sources.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SPARC_STACK_H__
#define __SPARC_STACK_H__

#include <asm/ptrace.h>

#ifndef __ASSEMBLER__

#ifdef __cplusplus
extern "C" {
#endif

#define PT_REGS_SZ   sizeof(struct pt_regs)

/* A Sparc stack frame */
	struct sparc_stackframe_regs {
		unsigned long sf_locals[8];
		unsigned long sf_ins[6];
		struct sparc_stackframe_regs *sf_fp;
		unsigned long sf_callers_pc;
		char *sf_structptr;
		unsigned long sf_xargs[6];
		unsigned long sf_xxargs[1];
	};
#define SF_REGS_SZ sizeof(struct sparc_stackframe_regs)

/* A register window */
	struct sparc_regwindow_regs {
		unsigned long locals[8];
		unsigned long ins[8];
	};
#define RW_REGS_SZ sizeof(struct sparc_regwindow_regs)

/* A fpu window */
	struct sparc_fpuwindow_regs {
		unsigned long locals[32];
		unsigned long fsr;
		unsigned long lastctx;
	};
#define FW_REGS_SZ sizeof(struct sparc_fpuwindow_regs)

#ifdef __cplusplus
}
#endif
#else
#define PT_REGS_SZ	0x50	/* 20*4 */
#define SF_REGS_SZ	0x60	/* 24*4 */
#define RW_REGS_SZ	0x20	/* 16*4 */
#define FW_REGS_SZ	0x88	/* 34*4 */
#endif				/* !ASM */

/* These are for pt_regs. */
#define PT_PSR		0x0
#define PT_PC		0x4
#define PT_NPC		0x8
#define PT_Y		0xc
#define PT_G0		0x10
#define PT_WIM		PT_G0
#define PT_G1		0x14
#define PT_G2		0x18
#define PT_G3		0x1c
#define PT_G4		0x20
#define PT_G5		0x24
#define PT_G6		0x28
#define PT_G7		0x2c
#define PT_I0		0x30
#define PT_I1		0x34
#define PT_I2		0x38
#define PT_I3		0x3c
#define PT_I4		0x40
#define PT_I5		0x44
#define PT_I6		0x48
#define PT_FP		PT_I6
#define PT_I7		0x4c

/* Stack_frame offsets */
#define SF_L0		0x00
#define SF_L1		0x04
#define SF_L2		0x08
#define SF_L3		0x0c
#define SF_L4		0x10
#define SF_L5		0x14
#define SF_L6		0x18
#define SF_L7		0x1c
#define SF_I0		0x20
#define SF_I1		0x24
#define SF_I2		0x28
#define SF_I3		0x2c
#define SF_I4		0x30
#define SF_I5		0x34
#define SF_FP		0x38
#define SF_PC		0x3c
#define SF_RETP		0x40
#define SF_XARG0	0x44
#define	SF_XARG1	0x48
#define	SF_XARG2	0x4c
#define	SF_XARG3	0x50
#define	SF_XARG4	0x54
#define	SF_XARG5	0x58
#define	SF_XXARG	0x5c

/* Reg_window offsets */
#define RW_L0		0x00
#define RW_L1		0x04
#define RW_L2		0x08
#define RW_L3		0x0c
#define RW_L4		0x10
#define RW_L5		0x14
#define RW_L6		0x18
#define RW_L7		0x1c
#define RW_I0		0x20
#define RW_I1		0x24
#define RW_I2		0x28
#define RW_I3		0x2c
#define RW_I4		0x30
#define RW_I5		0x34
#define RW_I6		0x38
#define RW_I7		0x3c

/* Fpu_window offsets */
#define FW_F0		0x00
#define FW_F2		0x08
#define FW_F4		0x10
#define FW_F6		0x18
#define FW_F8		0x20
#define FW_F10		0x28
#define FW_F12		0x30
#define FW_F14		0x38
#define FW_F16		0x40
#define FW_F18		0x48
#define FW_F20		0x50
#define FW_F22		0x58
#define FW_F24		0x60
#define FW_F26		0x68
#define FW_F28		0x70
#define FW_F30		0x78
#define FW_FSR		0x80

#endif
