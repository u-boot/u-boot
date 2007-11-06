/*
 * Copyright 2004, 2007 Freescale Semiconductor.
 * Copyright (C) 2003  Motorola,Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* U-Boot Startup Code for Motorola 85xx PowerPC based Embedded Boards
 *
 * The processor starts at 0xfffffffc and the code is first executed in the
 * last 4K page(0xfffff000-0xffffffff) in flash/rom.
 *
 */

#include <config.h>
#include <mpc85xx.h>
#include <version.h>

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#ifndef	 CONFIG_IDENT_STRING
#define	 CONFIG_IDENT_STRING ""
#endif

#undef	MSR_KERNEL
#define MSR_KERNEL ( MSR_ME )	/* Machine Check */

/*
 * Set up GOT: Global Offset Table
 *
 * Use r14 to access the GOT
 */
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(_FIXUP_TABLE_)

	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)

	GOT_ENTRY(__init_end)
	GOT_ENTRY(_end)
	GOT_ENTRY(__bss_start)
	END_GOT

/*
 * e500 Startup -- after reset only the last 4KB of the effective
 * address space is mapped in the MMU L2 TLB1 Entry0. The .bootpg
 * section is located at THIS LAST page and basically does three
 * things: clear some registers, set up exception tables and
 * add more TLB entries for 'larger spaces'(e.g. the boot rom) to
 * continue the boot procedure.

 * Once the boot rom is mapped by TLB entries we can proceed
 * with normal startup.
 *
 */

	.section .bootpg,"ax"
	.globl _start_e500

_start_e500:

/* clear registers/arrays not reset by hardware */

	/* L1 */
	li	r0,2
	mtspr	L1CSR0,r0	/* invalidate d-cache */
	mtspr	L1CSR1,r0 	/* invalidate i-cache */

	mfspr	r1,DBSR
	mtspr	DBSR,r1		/* Clear all valid bits */

	/*
	 *	Enable L1 Caches early
	 *
	 */

	lis	r2,L1CSR0_CPE@H	/* enable parity */
	ori	r2,r2,L1CSR0_DCE
	mtspr	L1CSR0,r2	/* enable L1 Dcache */
	isync
	mtspr	L1CSR1,r2	/* enable L1 Icache */
	isync
	msync

	/* Setup interrupt vectors */
	lis	r1,TEXT_BASE@h
	mtspr	IVPR,r1

	li	r1,0x0100
	mtspr	IVOR0,r1	/* 0: Critical input */
	li	r1,0x0200
	mtspr	IVOR1,r1	/* 1: Machine check */
	li	r1,0x0300
	mtspr	IVOR2,r1	/* 2: Data storage */
	li	r1,0x0400
	mtspr	IVOR3,r1	/* 3: Instruction storage */
	li	r1,0x0500
	mtspr	IVOR4,r1	/* 4: External interrupt */
	li	r1,0x0600
	mtspr	IVOR5,r1	/* 5: Alignment */
	li	r1,0x0700
	mtspr	IVOR6,r1	/* 6: Program check */
	li	r1,0x0800
	mtspr	IVOR7,r1	/* 7: floating point unavailable */
	li	r1,0x0900
	mtspr	IVOR8,r1	/* 8: System call */
	/* 9: Auxiliary processor unavailable(unsupported) */
	li	r1,0x0a00
	mtspr	IVOR10,r1	/* 10: Decrementer */
	li	r1,0x0b00
	mtspr	IVOR11,r1	/* 11: Interval timer */
	li	r1,0x0c00
	mtspr	IVOR12,r1	/* 12: Watchdog timer */
	li	r1,0x0d00
	mtspr	IVOR13,r1	/* 13: Data TLB error */
	li	r1,0x0e00
	mtspr	IVOR14,r1	/* 14: Instruction TLB error */
	li	r1,0x0f00
	mtspr	IVOR15,r1	/* 15: Debug */


	/*
	 * After reset, CCSRBAR is located at CFG_CCSRBAR_DEFAULT, i.e.
	 * 0xff700000-0xff800000. We need add a TLB1 entry for this 1MB
	 * region before we can access any CCSR registers such as L2
	 * registers, Local Access Registers,etc. We will also re-allocate
	 * CFG_CCSRBAR_DEFAULT to CFG_CCSRBAR immediately after TLB1 setup.
	 *
	 * Please refer to board-specif directory for TLB1 entry configuration.
	 * (e.g. board/<yourboard>/init.S)
	 *
	 */
	bl	tlb1_entry
	mr	r5,r0
	lwzu	r4,0(r5)	/* how many TLB1 entries we actually use */
	mtctr	r4

0:	lwzu	r6,4(r5)
	lwzu	r7,4(r5)
	lwzu	r8,4(r5)
	lwzu	r9,4(r5)
	mtspr	MAS0,r6
	mtspr	MAS1,r7
	mtspr	MAS2,r8
	mtspr	MAS3,r9
	isync
	msync
	tlbwe
	isync
	bdnz	0b

1:
#if (CFG_CCSRBAR_DEFAULT != CFG_CCSRBAR)
	/* Special sequence needed to update CCSRBAR itself */
	lis	r4,CFG_CCSRBAR_DEFAULT@h
	ori	r4,r4,CFG_CCSRBAR_DEFAULT@l

	lis	r5,CFG_CCSRBAR@h
	ori	r5,r5,CFG_CCSRBAR@l
	srwi	r6,r5,12
	stw	r6,0(r4)
	isync

	lis	r5,0xffff
	ori	r5,r5,0xf000
	lwz	r5,0(r5)
	isync

	lis	r3,CFG_CCSRBAR@h
	lwz	r5,CFG_CCSRBAR@l(r3)
	isync
#endif


	/* set up local access windows, defined at board/<boardname>/init.S */
	lis	r7,CFG_CCSRBAR@h
	ori	r7,r7,CFG_CCSRBAR@l

	bl	law_entry
	mr	r6,r0
	lwzu	r5,0(r6)	/* how many windows we actually use */
	mtctr	r5

	li	r2,0x0c28	/* the first pair is reserved for */
	li	r1,0x0c30	/* boot-over-rio-or-pci */

0:	lwzu	r4,4(r6)
	lwzu	r3,4(r6)
	stwx	r4,r7,r2
	stwx	r3,r7,r1
	addi	r2,r2,0x0020
	addi	r1,r1,0x0020
	bdnz	0b

	/* Clear and set up some registers. */
	li      r0,0
	mtmsr   r0
	li	r0,0x0000
	lis	r1,0xffff
	mtspr	DEC,r0			/* prevent dec exceptions */
	mttbl	r0			/* prevent fit & wdt exceptions */
	mttbu	r0
	mtspr	TSR,r1			/* clear all timer exception status */
	mtspr	TCR,r0			/* disable all */
	mtspr	ESR,r0			/* clear exception syndrome register */
	mtspr	MCSR,r0			/* machine check syndrome register */
	mtxer	r0			/* clear integer exception register */
	lis	r1,0x0002		/* set CE bit (Critical Exceptions) */
	ori	r1,r1,0x1200		/* set ME/DE bit */
	mtmsr	r1			/* change MSR */
	isync

	/* Enable Time Base and Select Time Base Clock */
	lis	r0,HID0_EMCP@h		/* Enable machine check */
#if defined(CONFIG_ENABLE_36BIT_PHYS)
	ori	r0,r0,(HID0_TBEN|HID0_ENMAS7)@l	/* Enable Timebase & MAS7 */
#else
	ori	r0,r0,HID0_TBEN@l	/* enable Timebase */
#endif
	mtspr	HID0,r0

	li	r0,(HID1_ASTME|HID1_ABE)@l	/* Addr streaming & broadcast */
	mtspr	HID1,r0

	/* Enable Branch Prediction */
#if defined(CONFIG_BTB)
	li	r0,0x201		/* BBFI = 1, BPEN = 1 */
	mtspr	BUCSR,r0
#endif

#if defined(CFG_INIT_DBCR)
	lis	r1,0xffff
	ori	r1,r1,0xffff
	mtspr	DBSR,r1			/* Clear all status bits */
	lis	r0,CFG_INIT_DBCR@h	/* DBCR0[IDM] must be set */
	ori	r0,r0,CFG_INIT_DBCR@l
	mtspr	DBCR0,r0
#endif

/* L1 DCache is used for initial RAM */

	/* Allocate Initial RAM in data cache.
	 */
	lis	r3,CFG_INIT_RAM_ADDR@h
	ori	r3,r3,CFG_INIT_RAM_ADDR@l
	li	r2,(CFG_DCACHE_SIZE / (2 * CFG_CACHELINE_SIZE))
	mtctr	r2
	li	r0,0
1:
	dcbz	r0,r3
	dcbtls	0,r0,r3
	addi	r3,r3,CFG_CACHELINE_SIZE
	bdnz	1b

	/* Jump out the last 4K page and continue to 'normal' start */
#ifdef CFG_RAMBOOT
	b	_start_cont
#else
	/* Calculate absolute address in FLASH and jump there		*/
	/*--------------------------------------------------------------*/
	lis	r3,CFG_MONITOR_BASE@h
	ori	r3,r3,CFG_MONITOR_BASE@l
	addi	r3,r3,_start_cont - _start + _START_OFFSET
	mtlr	r3
	blr
#endif

	.text
	.globl	_start
_start:
	.long	0x27051956		/* U-BOOT Magic Number */
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION
	.ascii " (", __DATE__, " - ", __TIME__, ")"
	.ascii CONFIG_IDENT_STRING, "\0"

	.align	4
	.globl	_start_cont
_start_cont:
	/* Setup the stack in initial RAM,could be L2-as-SRAM or L1 dcache*/
	lis	r1,CFG_INIT_RAM_ADDR@h
	ori	r1,r1,CFG_INIT_SP_OFFSET@l

	li	r0,0
	stwu	r0,-4(r1)
	stwu	r0,-4(r1)		/* Terminate call chain */

	stwu	r1,-8(r1)		/* Save back chain and move SP */
	lis	r0,RESET_VECTOR@h	/* Address of reset vector */
	ori	r0,r0,RESET_VECTOR@l
	stwu	r1,-8(r1)		/* Save back chain and move SP */
	stw	r0,+12(r1)		/* Save return addr (underflow vect) */

	GET_GOT
	bl	cpu_init_f
	bl	board_init_f
	isync

	. = EXC_OFF_SYS_RESET
	.globl	_start_of_vectors
_start_of_vectors:

/* Critical input. */
	CRIT_EXCEPTION(0x0100, CriticalInput, CritcalInputException)

/* Machine check */
	MCK_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception. */
	STD_EXCEPTION(0x0300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x0400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x0500, ExtInterrupt, ExtIntException)

/* Alignment exception. */
	. = 0x0600
Alignment:
	EXCEPTION_PROLOG(SRR0, SRR1)
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_Alignment:
	.long	AlignmentException - _start + _START_OFFSET
	.long	int_return - _start + _START_OFFSET

/* Program check exception */
	. = 0x0700
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_ProgramCheck:
	.long	ProgramCheckException - _start + _START_OFFSET
	.long	int_return - _start + _START_OFFSET

	/* No FPU on MPC85xx.  This exception is not supposed to happen.
	*/
	STD_EXCEPTION(0x0800, FPUnavailable, UnknownException)

	. = 0x0900
/*
 * r0 - SYSCALL number
 * r3-... arguments
 */
SystemCall:
	addis	r11,r0,0	/* get functions table addr */
	ori	r11,r11,0	/* Note: this code is patched in trap_init */
	addis	r12,r0,0	/* get number of functions */
	ori	r12,r12,0

	cmplw	0,r0,r12
	bge	1f

	rlwinm	r0,r0,2,0,31	/* fn_addr = fn_tbl[r0] */
	add	r11,r11,r0
	lwz	r11,0(r11)

	li	r20,0xd00-4	/* Get stack pointer */
	lwz	r12,0(r20)
	subi	r12,r12,12	/* Adjust stack pointer */
	li	r0,0xc00+_end_back-SystemCall
	cmplw	0,r0,r12	/* Check stack overflow */
	bgt	1f
	stw	r12,0(r20)

	mflr	r0
	stw	r0,0(r12)
	mfspr	r0,SRR0
	stw	r0,4(r12)
	mfspr	r0,SRR1
	stw	r0,8(r12)

	li	r12,0xc00+_back-SystemCall
	mtlr	r12
	mtspr	SRR0,r11

1:	SYNC
	rfi
_back:

	mfmsr	r11			/* Disable interrupts */
	li	r12,0
	ori	r12,r12,MSR_EE
	andc	r11,r11,r12
	SYNC				/* Some chip revs need this... */
	mtmsr	r11
	SYNC

	li	r12,0xd00-4		/* restore regs */
	lwz	r12,0(r12)

	lwz	r11,0(r12)
	mtlr	r11
	lwz	r11,4(r12)
	mtspr	SRR0,r11
	lwz	r11,8(r12)
	mtspr	SRR1,r11

	addi	r12,r12,12		/* Adjust stack pointer */
	li	r20,0xd00-4
	stw	r12,0(r20)

	SYNC
	rfi
_end_back:

	STD_EXCEPTION(0x0a00, Decrementer, timer_interrupt)
	STD_EXCEPTION(0x0b00, IntervalTimer, UnknownException)
	STD_EXCEPTION(0x0c00, WatchdogTimer, UnknownException)

	STD_EXCEPTION(0x0d00, DataTLBError, UnknownException)
	STD_EXCEPTION(0x0e00, InstructionTLBError, UnknownException)

	CRIT_EXCEPTION(0x0f00, DebugBreakpoint, DebugException )

	.globl	_end_of_vectors
_end_of_vectors:


	. = . + (0x100 - ( . & 0xff ))	/* align for debug */

/*
 * This code finishes saving the registers to the exception frame
 * and jumps to the appropriate handler for the exception.
 * Register r21 is pointer into trap frame, r1 has new stack pointer.
 */
	.globl	transfer_to_handler
transfer_to_handler:
	stw	r22,_NIP(r21)
	lis	r22,MSR_POW@h
	andc	r23,r23,r22
	stw	r23,_MSR(r21)
	SAVE_GPR(7, r21)
	SAVE_4GPRS(8, r21)
	SAVE_8GPRS(12, r21)
	SAVE_8GPRS(24, r21)

	mflr	r23
	andi.	r24,r23,0x3f00		/* get vector offset */
	stw	r24,TRAP(r21)
	li	r22,0
	stw	r22,RESULT(r21)
	mtspr	SPRG2,r22		/* r1 is now kernel sp */

	lwz	r24,0(r23)		/* virtual address of handler */
	lwz	r23,4(r23)		/* where to go when done */
	mtspr	SRR0,r24
	mtspr	SRR1,r20
	mtlr	r23
	SYNC
	rfi				/* jump to handler, enable MMU */

int_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SRR0,r2
	mtspr	SRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfi

crit_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SPRN_CSRR0,r2
	mtspr	SPRN_CSRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfci

mck_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SPRN_MCSRR0,r2
	mtspr	SPRN_MCSRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfmci

/* Cache functions.
*/
invalidate_icache:
	mfspr	r0,L1CSR1
	ori	r0,r0,L1CSR1_ICFI
	msync
	isync
	mtspr	L1CSR1,r0
	isync
	blr				/* entire I cache */

invalidate_dcache:
	mfspr	r0,L1CSR0
	ori	r0,r0,L1CSR0_DCFI
	msync
	isync
	mtspr	L1CSR0,r0
	isync
	blr

	.globl	icache_enable
icache_enable:
	mflr	r8
	bl	invalidate_icache
	mtlr	r8
	isync
	mfspr	r4,L1CSR1
	ori	r4,r4,0x0001
	oris	r4,r4,0x0001
	mtspr	L1CSR1,r4
	isync
	blr

	.globl	icache_disable
icache_disable:
	mfspr	r0,L1CSR1
	lis	r3,0
	ori	r3,r3,L1CSR1_ICE
	andc	r0,r0,r3
	mtspr	L1CSR1,r0
	isync
	blr

	.globl	icache_status
icache_status:
	mfspr	r3,L1CSR1
	andi.	r3,r3,L1CSR1_ICE
	blr

	.globl	dcache_enable
dcache_enable:
	mflr	r8
	bl	invalidate_dcache
	mtlr	r8
	isync
	mfspr	r0,L1CSR0
	ori	r0,r0,0x0001
	oris	r0,r0,0x0001
	msync
	isync
	mtspr	L1CSR0,r0
	isync
	blr

	.globl	dcache_disable
dcache_disable:
	mfspr	r3,L1CSR0
	lis	r4,0
	ori	r4,r4,L1CSR0_DCE
	andc	r3,r3,r4
	mtspr	L1CSR0,r0
	isync
	blr

	.globl	dcache_status
dcache_status:
	mfspr	r3,L1CSR0
	andi.	r3,r3,L1CSR0_DCE
	blr

	.globl get_pir
get_pir:
	mfspr	r3,PIR
	blr

	.globl get_pvr
get_pvr:
	mfspr	r3,PVR
	blr

	.globl get_svr
get_svr:
	mfspr	r3,SVR
	blr

	.globl wr_tcr
wr_tcr:
	mtspr	TCR,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in8 */
/* Description:	 Input 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	in8
in8:
	lbz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out8 */
/* Description:	 Output 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	out8
out8:
	stb	r4,0x0000(r3)
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16 */
/* Description:	 Output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16
out16:
	sth	r4,0x0000(r3)
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16r */
/* Description:	 Byte reverse and output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16r
out16r:
	sthbrx	r4,r0,r3
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32 */
/* Description:	 Output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32
out32:
	stw	r4,0x0000(r3)
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32r */
/* Description:	 Byte reverse and output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32r
out32r:
	stwbrx	r4,r0,r3
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16 */
/* Description:	 Input 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	in16
in16:
	lhz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16r */
/* Description:	 Input 16 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in16r
in16r:
	lhbrx	r3,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32 */
/* Description:	 Input 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	in32
in32:
	lwz	3,0x0000(3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32r */
/* Description:	 Input 32 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in32r
in32r:
	lwbrx	r3,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbf */
/* Description:	 Data Cache block flush */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbf
ppcDcbf:
	dcbf	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbi */
/* Description:	 Data Cache block Invalidate */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbi
ppcDcbi:
	dcbi	r0,r3
	blr

/*--------------------------------------------------------------------------
 * Function:	 ppcDcbz
 * Description:	 Data Cache block zero.
 * Input:	 r3 = effective address
 * Output:	 none.
 *-------------------------------------------------------------------------- */

	.globl	ppcDcbz
ppcDcbz:
	dcbz	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcSync */
/* Description:	 Processor Synchronize */
/* Input:	 none. */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcSync
ppcSync:
	sync
	blr

/*------------------------------------------------------------------------------*/

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r3 = dest
 * r4 = src
 * r5 = length in bytes
 * r6 = cachelinesize
 */
	.globl	relocate_code
relocate_code:
	mr	r1,r3		/* Set new stack pointer		*/
	mr	r9,r4		/* Save copy of Init Data pointer	*/
	mr	r10,r5		/* Save copy of Destination Address	*/

	mr	r3,r5				/* Destination Address	*/
	lis	r4,CFG_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4,r4,CFG_MONITOR_BASE@l
	lwz	r5,GOT(__init_end)
	sub	r5,r5,r4
	li	r6,CFG_CACHELINE_SIZE		/* Cache Line Size	*/

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CFG_MONITOR_BASE) + Destination Address
	 *
	 * Offset:
	 */
	sub	r15,r10,r4

	/* First our own GOT */
	add	r14,r14,r15
	/* the the one used by the C code */
	add	r30,r30,r15

	/*
	 * Now relocate code
	 */

	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary	*/
	beq	7f		/* Protect against 0 count		*/
	mtctr	r0
	bge	cr1,2f

	la	r8,-4(r4)
	la	r7,-4(r3)
1:	lwzu	r0,4(r8)
	stwu	r0,4(r7)
	bdnz	1b
	b	4f

2:	slwi	r0,r0,2
	add	r8,r4,r0
	add	r7,r3,r0
3:	lwzu	r0,-4(r8)
	stwu	r0,-4(r7)
	bdnz	3b

/*
 * Now flush the cache: note that we must start from a cache aligned
 * address. Otherwise we might miss one cache line.
 */
4:	cmpwi	r6,0
	add	r5,r3,r5
	beq	7f		/* Always flush prefetch queue in any case */
	subi	r0,r6,1
	andc	r3,r3,r0
	mr	r4,r3
5:	dcbst	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	5b
	sync			/* Wait for all dcbst to complete on bus */
	mr	r4,r3
6:	icbi	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	6b
7:	sync			/* Wait for all icbi to complete on bus */
	isync

	/*
	 * Re-point the IVPR at RAM
	 */
	mtspr	IVPR,r10

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */

	addi	r0,r10,in_ram - _start + _START_OFFSET
	mtlr	r0
	blr				/* NEVER RETURNS! */
	.globl	in_ram
in_ram:

	/*
	 * Relocation Function, r14 point to got2+0x8000
	 *
	 * Adjust got2 pointers, no need to check for 0, this code
	 * already puts a few entries in the table.
	 */
	li	r0,__got2_entries@sectoff@l
	la	r3,GOT(_GOT2_TABLE_)
	lwz	r11,GOT(_GOT2_TABLE_)
	mtctr	r0
	sub	r11,r3,r11
	addi	r3,r3,-4
1:	lwzu	r0,4(r3)
	add	r0,r0,r11
	stw	r0,0(r3)
	bdnz	1b

	/*
	 * Now adjust the fixups and the pointers to the fixups
	 * in case we need to move ourselves again.
	 */
2:	li	r0,__fixup_entries@sectoff@l
	lwz	r3,GOT(_FIXUP_TABLE_)
	cmpwi	r0,0
	mtctr	r0
	addi	r3,r3,-4
	beq	4f
3:	lwzu	r4,4(r3)
	lwzux	r0,r4,r11
	add	r0,r0,r11
	stw	r10,0(r3)
	stw	r0,0(r4)
	bdnz	3b
4:
clear_bss:
	/*
	 * Now clear BSS segment
	 */
	lwz	r3,GOT(__bss_start)
	lwz	r4,GOT(_end)

	cmplw	0,r3,r4
	beq	6f

	li	r0,0
5:
	stw	r0,0(r3)
	addi	r3,r3,4
	cmplw	0,r3,r4
	bne	5b
6:

	mr	r3,r9		/* Init Data pointer		*/
	mr	r4,r10		/* Destination Address		*/
	bl	board_init_r

	/*
	 * Copy exception vector code to low memory
	 *
	 * r3: dest_addr
	 * r7: source address, r8: end address, r9: target address
	 */
	.globl	trap_init
trap_init:
	lwz	r7,GOT(_start_of_vectors)
	lwz	r8,GOT(_end_of_vectors)

	li	r9,0x100		/* reset vector always at 0x100 */

	cmplw	0,r7,r8
	bgelr				/* return if r7>=r8 - just in case */

	mflr	r4			/* save link register		*/
1:
	lwz	r0,0(r7)
	stw	r0,0(r9)
	addi	r7,r7,4
	addi	r9,r9,4
	cmplw	0,r7,r8
	bne	1b

	/*
	 * relocate `hdlr' and `int_return' entries
	 */
	li	r7,.L_CriticalInput - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_MachineCheck - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_DataStorage - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_InstStorage - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_ExtInterrupt - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_Alignment - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_ProgramCheck - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_FPUnavailable - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_Decrementer - _start + _START_OFFSET
	bl	trap_reloc
	li	r7,.L_IntervalTimer - _start + _START_OFFSET
	li	r8,_end_of_vectors - _start + _START_OFFSET
2:
	bl	trap_reloc
	addi	r7,r7,0x100		/* next exception vector	*/
	cmplw	0,r7,r8
	blt	2b

	lis	r7,0x0
	mtspr	IVPR,r7

	mtlr	r4			/* restore link register	*/
	blr

	/*
	 * Function: relocate entries for one exception vector
	 */
trap_reloc:
	lwz	r0,0(r7)		/* hdlr ...			*/
	add	r0,r0,r3		/*  ... += dest_addr		*/
	stw	r0,0(r7)

	lwz	r0,4(r7)		/* int_return ...		*/
	add	r0,r0,r3		/*  ... += dest_addr		*/
	stw	r0,4(r7)

	blr

#ifdef CFG_INIT_RAM_LOCK
.globl unlock_ram_in_cache
unlock_ram_in_cache:
	/* invalidate the INIT_RAM section */
	lis	r3,(CFG_INIT_RAM_ADDR & ~31)@h
	ori	r3,r3,(CFG_INIT_RAM_ADDR & ~31)@l
	li	r4,(CFG_DCACHE_SIZE / (2 * CFG_CACHELINE_SIZE))
	mtctr	r4
1:	icbi	r0,r3
	dcbi	r0,r3
	addi	r3,r3,CFG_CACHELINE_SIZE
	bdnz	1b
	sync			/* Wait for all icbi to complete on bus */
	isync
	blr
#endif
