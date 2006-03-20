/*
 * U-boot - ptrace.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
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

#ifndef _BLACKFIN_PTRACE_H
#define _BLACKFIN_PTRACE_H

#define NEW_PT_REGS

/*
 * GCC defines register number like this:
 * -----------------------------
 *       0 - 7 are data registers R0-R7
 *       8 - 15 are address registers P0-P7
 *      16 - 31 dsp registers I/B/L0 -- I/B/L3 & M0--M3
 *      32 - 33 A registers A0 & A1
 *      34 -    status register
 *
 * We follows above, except:
 *      32-33 --- Low 32-bit of A0&1
 *      34-35 --- High 8-bit of A0&1
 */

#if defined(NEW_PT_REGS)

#define PT_IPEND	0
#define PT_SYSCFG	(PT_IPEND+4)
#define PT_SEQSTAT	(PT_SYSCFG+4)
#define PT_RETE		(PT_SEQSTAT+4)
#define PT_RETN		(PT_RETE+4)
#define PT_RETX		(PT_RETN+4)
#define PT_RETI		(PT_RETX+4)
#define PT_PC		PT_RETI
#define PT_RETS		(PT_RETI+4)
#define PT_RESERVED	(PT_RETS+4)
#define PT_ASTAT	(PT_RESERVED+4)
#define PT_LB1		(PT_ASTAT+4)
#define PT_LB0		(PT_LB1+4)
#define PT_LT1		(PT_LB0+4)
#define PT_LT0		(PT_LT1+4)
#define PT_LC1		(PT_LT0+4)
#define PT_LC0		(PT_LC1+4)
#define PT_A1W		(PT_LC0+4)
#define PT_A1X		(PT_A1W+4)
#define PT_A0W		(PT_A1X+4)
#define PT_A0X		(PT_A0W+4)
#define PT_B3		(PT_A0X+4)
#define PT_B2		(PT_B3+4)
#define PT_B1		(PT_B2+4)
#define PT_B0		(PT_B1+4)
#define PT_L3		(PT_B0+4)
#define PT_L2		(PT_L3+4)
#define PT_L1		(PT_L2+4)
#define PT_L0		(PT_L1+4)
#define PT_M3		(PT_L0+4)
#define PT_M2		(PT_M3+4)
#define PT_M1		(PT_M2+4)
#define PT_M0		(PT_M1+4)
#define PT_I3		(PT_M0+4)
#define PT_I2		(PT_I3+4)
#define PT_I1		(PT_I2+4)
#define PT_I0		(PT_I1+4)
#define PT_USP		(PT_I0+4)
#define PT_FP		(PT_USP+4)
#define PT_P5		(PT_FP+4)
#define PT_P4		(PT_P5+4)
#define PT_P3		(PT_P4+4)
#define PT_P2		(PT_P3+4)
#define PT_P1		(PT_P2+4)
#define PT_P0		(PT_P1+4)
#define PT_R7		(PT_P0+4)
#define PT_R6		(PT_R7+4)
#define PT_R5		(PT_R6+4)
#define PT_R4		(PT_R5+4)
#define PT_R3		(PT_R4+4)
#define PT_R2		(PT_R3+4)
#define PT_R1		(PT_R2+4)
#define PT_R0		(PT_R1+4)
#define PT_ORIG_R0	(PT_R0+4)
#define PT_SR		PT_SEQSTAT

#else
/*
 * Here utilize blackfin : dpregs = [pregs + imm16s4]
 *                     [pregs + imm16s4] = dpregs
 * to access defferent saved reg in stack
 */
#define PT_R3		0
#define PT_R4		4
#define PT_R2		8
#define PT_R1		12
#define PT_P5		16
#define PT_P4		20
#define PT_P3		24
#define PT_P2		28
#define PT_P1		32
#define PT_P0		36
#define PT_R7		40
#define PT_R6		44
#define PT_R5		48
#define PT_PC		52
#define PT_SEQSTAT	56	/* so-called SR reg */
#define PT_SR		PT_SEQSTAT
#define PT_ASTAT	60
#define PT_RETS		64
#define PT_A1w		68
#define PT_A0w		72
#define PT_A1x		76
#define PT_A0x		80
#define PT_ORIG_R0	84
#define PT_R0		88
#define PT_USP		92
#define PT_FP		96
#define PT_SP		100

/* Added by HuTao, May26 2003 3:18PM */
#define PT_IPEND	100

/* Add SYSCFG register for single stepping support */
#define PT_SYSCFG	104

#endif

#ifndef __ASSEMBLY__

#if defined(NEW_PT_REGS)
/* this struct defines the way the registers are stored on the
 * stack during a system call.
 */
struct pt_regs {
	long ipend;
	long syscfg;
	long seqstat;
	long rete;
	long retn;
	long retx;
	long pc;
	long rets;
	long reserved;
	long astat;
	long lb1;
	long lb0;
	long lt1;
	long lt0;
	long lc1;
	long lc0;
	long a1w;
	long a1x;
	long a0w;
	long a0x;
	long b3;
	long b2;
	long b1;
	long b0;
	long l3;
	long l2;
	long l1;
	long l0;
	long m3;
	long m2;
	long m1;
	long m0;
	long i3;
	long i2;
	long i1;
	long i0;
	long usp;
	long fp;
	long p5;
	long p4;
	long p3;
	long p2;
	long p1;
	long p0;
	long r7;
	long r6;
	long r5;
	long r4;
	long r3;
	long r2;
	long r1;
	long r0;
	long orig_r0;
};

#else
/* now we don't know what regs the system call will use	*/
struct pt_regs {
	long r3;
	long r4;
	long r2;
	long r1;
	long p5;
	long p4;
	long p3;
	long p2;
	long p1;
	long p0;
	long r7;
	long r6;
	long r5;
	unsigned long pc;
	unsigned long seqstat;
	unsigned long astat;
	unsigned long rets;
	long a1w;
	long a0w;
	long a1x;
	long a0x;
	long orig_r0;
	long r0;
	long usp;
	long fp;
/*
 * Added for supervisor/user mode switch.
 *
 * HuTao May26 03 3:23PM
 */
	long ipend;
	long syscfg;
};

#endif

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
#define PTRACE_GETREGS		12
#define PTRACE_SETREGS		13	/* ptrace signal */

#ifdef __KERNEL__

#ifndef PS_S
#define PS_S			(0x0c00)

/* Bit 11:10 of SEQSTAT defines user/supervisor/debug mode
 *        00: user
 *        01: supervisor
 *        1x: debug
 */

#define PS_M			(0x1000)	/* I am not sure why this is required here Akbar */
#endif

#define user_mode(regs)			(!((regs)->seqstat & PS_S))
#define instruction_pointer(regs)	((regs)->pc)
extern void show_regs(struct pt_regs *);

#endif
#endif
#endif
