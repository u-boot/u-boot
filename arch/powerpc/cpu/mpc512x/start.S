/*
 * Copyright (C) 1998  Dan Malek <dmalek@jlc.net>
 * Copyright (C) 1999  Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 * Copyright (C) 2000-2009 Wolfgang Denk <wd@denx.de>
 * Copyright Freescale Semiconductor, Inc. 2004, 2006.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Based on the MPC83xx code.
 */

/*
 *  U-Boot - Startup Code for MPC512x based Embedded Boards
 */

#include <asm-offsets.h>
#include <config.h>
#ifndef  CONFIG_IDENT_STRING
#define  CONFIG_IDENT_STRING "MPC512X"
#endif
#include <version.h>

#define CONFIG_521X	1		/* needed for Linux kernel header files*/

#include <asm/immap_512x.h>
#include "asm-offsets.h"

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>
#include <asm/u-boot.h>

/*
 * Floating Point enable, Machine Check and Recoverable Interr.
 */
#undef	MSR_KERNEL
#ifdef DEBUG
#define MSR_KERNEL (MSR_FP|MSR_RI)
#else
#define MSR_KERNEL (MSR_FP|MSR_ME|MSR_RI)
#endif

/* Macros for manipulating CSx_START/STOP */
#define START_REG(start)	((start) >> 16)
#define STOP_REG(start, size)	(((start) + (size) - 1) >> 16)

/*
 * Set up GOT: Global Offset Table
 *
 * Use r12 to access the GOT
 */
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(_FIXUP_TABLE_)

	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)

	GOT_ENTRY(__init_end)
	GOT_ENTRY(__bss_end)
	GOT_ENTRY(__bss_start)
	END_GOT

/*
 * Magic number and version string
 */
	.long	0x27051956		/* U-Boot Magic Number */
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION_STRING, "\0"

/*
 * Vector Table
 */
	.text
	. = EXC_OFF_SYS_RESET

	.globl	_start
	/* Start from here after reset/power on */
_start:
	b	boot_cold

	.globl	_start_of_vectors
_start_of_vectors:

/* Machine check */
	STD_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x500, ExtInterrupt, UnknownException)

/* Alignment exception. */
	. = 0x600
Alignment:
	EXCEPTION_PROLOG(SRR0, SRR1)
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(Alignment, AlignmentException, MSR_KERNEL, COPY_EE)

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(ProgramCheck, ProgramCheckException,
		MSR_KERNEL, COPY_EE)

/* Floating Point Unit unavailable exception */
	STD_EXCEPTION(0x800, FPUnavailable, UnknownException)

/* Decrementer */
	STD_EXCEPTION(0x900, Decrementer, timer_interrupt)

/* Critical interrupt */
	STD_EXCEPTION(0xa00, Critical, UnknownException)

/* System Call */
	STD_EXCEPTION(0xc00, SystemCall, UnknownException)

/* Trace interrupt */
	STD_EXCEPTION(0xd00, Trace, UnknownException)

/* Performance Monitor interrupt */
	STD_EXCEPTION(0xf00, PerfMon, UnknownException)

/* Intruction Translation Miss */
	STD_EXCEPTION(0x1000, InstructionTLBMiss, UnknownException)

/* Data Load Translation Miss */
	STD_EXCEPTION(0x1100, DataLoadTLBMiss, UnknownException)

/* Data Store Translation Miss */
	STD_EXCEPTION(0x1200, DataStoreTLBMiss, UnknownException)

/* Instruction Address Breakpoint */
	STD_EXCEPTION(0x1300, InstructionAddrBreakpoint, DebugException)

/* System Management interrupt */
	STD_EXCEPTION(0x1400, SystemMgmtInterrupt, UnknownException)

	.globl	_end_of_vectors
_end_of_vectors:

	. = 0x3000
boot_cold:
	/* Save msr contents */
	mfmsr	r5

	/* Set IMMR area to our preferred location */
	lis	r4, CONFIG_DEFAULT_IMMR@h
	lis	r3, CONFIG_SYS_IMMR@h
	ori	r3, r3, CONFIG_SYS_IMMR@l
	stw	r3, IMMRBAR(r4)
	mtspr	MBAR, r3		/* IMMRBAR is mirrored into the MBAR SPR (311) */

	/* Initialise the machine */
	bl	cpu_early_init

	/*
	 * Set up Local Access Windows:
	 *
	 * 1) Boot/CS0 (boot FLASH)
	 * 2) On-chip SRAM (initial stack purposes)
	 */

	/* Boot CS/CS0 window range */
	lis     r3, CONFIG_SYS_IMMR@h
	ori     r3, r3, CONFIG_SYS_IMMR@l

	lis	r4, START_REG(CONFIG_SYS_FLASH_BASE)
	ori	r4, r4, STOP_REG(CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_SIZE)
	stw	r4, LPCS0AW(r3)

	/*
	 * The SRAM window has a fixed size (256K), so only the start address
	 * is necessary
	 */
	lis	r4, START_REG(CONFIG_SYS_SRAM_BASE) & 0xff00
	stw	r4, SRAMBAR(r3)

	/*
	 * According to MPC5121e RM, configuring local access windows should
	 * be followed by a dummy read of the config register that was
	 * modified last and an isync
	 */
	lwz	r4, SRAMBAR(r3)
	isync

	/*
	 * Set configuration of the Boot/CS0, the SRAM window does not have a
	 * config register so no params can be set for it
	 */
	lis     r3, (CONFIG_SYS_IMMR + LPC_OFFSET)@h
	ori     r3, r3, (CONFIG_SYS_IMMR + LPC_OFFSET)@l

	lis     r4, CONFIG_SYS_CS0_CFG@h
	ori     r4, r4, CONFIG_SYS_CS0_CFG@l
	stw     r4, CS0_CONFIG(r3)

	/* Master enable all CS's */
	lis	r4, CS_CTRL_ME@h
	ori	r4, r4, CS_CTRL_ME@l
	stw	r4, CS_CTRL(r3)

	lis	r4, (CONFIG_SYS_MONITOR_BASE)@h
	ori	r4, r4, (CONFIG_SYS_MONITOR_BASE)@l
	addi	r5, r4, in_flash - _start + EXC_OFF_SYS_RESET
	mtlr	r5
	blr

in_flash:
	lis	r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@h
	ori	r1, r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@l

	li	r0, 0		/* Make room for stack frame header and	*/
	stwu	r0, -4(r1)	/* clear final stack frame so that	*/
	stwu	r0, -4(r1)	/* stack backtraces terminate cleanly	*/

	/* let the C-code set up the rest			*/
	/*							*/
	/* Be careful to keep code relocatable & stack humble	*/
	/*------------------------------------------------------*/

	GET_GOT			/* initialize GOT access	*/

	/* r3: IMMR */
	lis	r3, CONFIG_SYS_IMMR@h
	/* run low-level CPU init code (in Flash) */
	bl	cpu_init_f

	/* run 1st part of board init code (in Flash) */
	bl	board_init_f

	/* NOTREACHED - board_init_f() does not return */

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

/*
 * This code initialises the machine, it expects original MSR contents to be in r5.
 */
cpu_early_init:
	/* Initialize machine status; enable machine check interrupt */
	/*-----------------------------------------------------------*/

	li	r3, MSR_KERNEL			/* Set ME and RI flags */
	rlwimi	r3, r5, 0, 25, 25		/* preserve IP bit */
#ifdef DEBUG
	rlwimi	r3, r5, 0, 21, 22		/* debugger might set SE, BE bits */
#endif
	mtmsr	r3
	SYNC
	mtspr	SRR1, r3			/* Mirror current MSR state in SRR1 */

	lis	r3, CONFIG_SYS_IMMR@h

#if defined(CONFIG_WATCHDOG)
	/* Initialise the watchdog and reset it */
	/*--------------------------------------*/
	lis r4, CONFIG_SYS_WATCHDOG_VALUE
	ori r4, r4, (SWCRR_SWEN | SWCRR_SWRI | SWCRR_SWPR)
	stw r4, SWCRR(r3)

	/* reset */
	li	r4, 0x556C
	sth	r4, SWSRR@l(r3)
	li	r4, 0x0
	ori	r4, r4, 0xAA39
	sth	r4, SWSRR@l(r3)
#else
	/* Disable the watchdog */
	/*----------------------*/
	lwz r4, SWCRR(r3)
	/*
	 * Check to see if it's enabled for disabling: once disabled by s/w
	 * it's not possible to re-enable it
	 */
	andi. r4, r4, 0x4
	beq 1f
	xor r4, r4, r4
	stw r4, SWCRR(r3)
1:
#endif /* CONFIG_WATCHDOG */

	/* Initialize the Hardware Implementation-dependent Registers */
	/* HID0 also contains cache control			*/
	/*------------------------------------------------------*/
	lis	r3, CONFIG_SYS_HID0_INIT@h
	ori	r3, r3, CONFIG_SYS_HID0_INIT@l
	SYNC
	mtspr	HID0, r3

	lis	r3, CONFIG_SYS_HID0_FINAL@h
	ori	r3, r3, CONFIG_SYS_HID0_FINAL@l
	SYNC
	mtspr	HID0, r3

	lis	r3, CONFIG_SYS_HID2@h
	ori	r3, r3, CONFIG_SYS_HID2@l
	SYNC
	mtspr	HID2, r3
	sync
	blr


/* Cache functions.
 *
 * Note: requires that all cache bits in
 * HID0 are in the low half word.
 */
	.globl	icache_enable
icache_enable:
	mfspr	r3, HID0
	ori	r3, r3, HID0_ICE
	lis	r4, 0
	ori	r4, r4, HID0_ILOCK
	andc	r3, r3, r4
	ori	r4, r3, HID0_ICFI
	isync
	mtspr	HID0, r4    /* sets enable and invalidate, clears lock */
	isync
	mtspr	HID0, r3	/* clears invalidate */
	blr

	.globl	icache_disable
icache_disable:
	mfspr	r3, HID0
	lis	r4, 0
	ori	r4, r4, HID0_ICE|HID0_ILOCK
	andc	r3, r3, r4
	ori	r4, r3, HID0_ICFI
	isync
	mtspr	HID0, r4     /* sets invalidate, clears enable and lock*/
	isync
	mtspr	HID0, r3	/* clears invalidate */
	blr

	.globl	icache_status
icache_status:
	mfspr	r3, HID0
	rlwinm	r3, r3, (31 - HID0_ICE_SHIFT + 1), 31, 31
	blr

	.globl	dcache_enable
dcache_enable:
	mfspr	r3, HID0
	li	r5, HID0_DCFI|HID0_DLOCK
	andc	r3, r3, r5
	mtspr	HID0, r3		/* no invalidate, unlock */
	ori	r3, r3, HID0_DCE
	ori	r5, r3, HID0_DCFI
	mtspr	HID0, r5		/* enable + invalidate */
	mtspr	HID0, r3		/* enable */
	sync
	blr

	.globl	dcache_disable
dcache_disable:
	mfspr	r3, HID0
	lis	r4, 0
	ori	r4, r4, HID0_DCE|HID0_DLOCK
	andc	r3, r3, r4
	ori	r4, r3, HID0_DCI
	sync
	mtspr	HID0, r4	/* sets invalidate, clears enable and lock */
	sync
	mtspr	HID0, r3	/* clears invalidate */
	blr

	.globl	dcache_status
dcache_status:
	mfspr	r3, HID0
	rlwinm	r3, r3, (31 - HID0_DCE_SHIFT + 1), 31, 31
	blr

	.globl get_pvr
get_pvr:
	mfspr	r3, PVR
	blr

/*-------------------------------------------------------------------*/

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
	mr	r1,  r3		/* Set new stack pointer	*/
	mr	r9,  r4		/* Save copy of Global Data pointer */
	mr	r10, r5		/* Save copy of Destination Address */

	GET_GOT
	mr	r3,  r5				/* Destination Address */
	lis	r4, CONFIG_SYS_MONITOR_BASE@h		/* Source      Address */
	ori	r4, r4, CONFIG_SYS_MONITOR_BASE@l
	lwz	r5, GOT(__init_end)
	sub	r5, r5, r4
	li	r6, CONFIG_SYS_CACHELINE_SIZE		/* Cache Line Size */

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CONFIG_SYS_MONITOR_BASE)
	 *		+ Destination Address
	 *
	 * Offset:
	 */
	sub	r15, r10, r4

	/* First our own GOT */
	add	r12, r12, r15
	/* then the one used by the C code */
	add	r30, r30, r15

	/*
	 * Now relocate code
	 */
	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary */
	beq	7f		/* Protect against 0 count	  */
	mtctr	r0
	bge	cr1,2f
	la	r8,-4(r4)
	la	r7,-4(r3)

	/* copy */
1:	lwzu	r0,4(r8)
	stwu	r0,4(r7)
	bdnz	1b

	addi	r0,r5,3
	srwi.	r0,r0,2
	mtctr	r0
	la	r8,-4(r4)
	la	r7,-4(r3)

	/* and compare */
20:	lwzu	r20,4(r8)
	lwzu	r21,4(r7)
	xor. r22, r20, r21
	bne  30f
	bdnz	20b
	b 4f

	/* compare failed */
30:	li r3, 0
	blr

2:	slwi	r0,r0,2 /* re copy in reverse order ... y do we needed it? */
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
7:	sync			/* Wait for all icbi to complete on bus	*/
	isync

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
	addi	r0, r10, in_ram - _start + EXC_OFF_SYS_RESET
	mtlr	r0
	blr

in_ram:
	/*
	 * Relocation Function, r12 point to got2+0x8000
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
	cmpwi	r0,0
	beq-	2f
	add	r0,r0,r11
	stw	r0,0(r3)
2:	bdnz	1b

	/*
	 * Now adjust the fixups and the pointers to the fixups
	 * in case we need to move ourselves again.
	 */
	li	r0,__fixup_entries@sectoff@l
	lwz	r3,GOT(_FIXUP_TABLE_)
	cmpwi	r0,0
	mtctr	r0
	addi	r3,r3,-4
	beq	4f
3:	lwzu	r4,4(r3)
	lwzux	r0,r4,r11
	cmpwi	r0,0
	add	r0,r0,r11
	stw	r4,0(r3)
	beq-	5f
	stw	r0,0(r4)
5:	bdnz	3b
4:
clear_bss:
	/*
	 * Now clear BSS segment
	 */
	lwz	r3,GOT(__bss_start)
	lwz	r4,GOT(__bss_end)

	cmplw	0, r3, r4
	beq	6f

	li	r0, 0
5:
	stw	r0, 0(r3)
	addi	r3, r3, 4
	cmplw	0, r3, r4
	bne	5b
6:
	mr	r3, r9		/* Global Data pointer		*/
	mr	r4, r10		/* Destination Address		*/
	bl	board_init_r

	/*
	 * Copy exception vector code to low memory
	 *
	 * r3: dest_addr
	 * r7: source address, r8: end address, r9: target address
	 */
	.globl	trap_init
trap_init:
	mflr	r4		/* save link register */
	GET_GOT
	lwz	r7, GOT(_start)
	lwz	r8, GOT(_end_of_vectors)

	li	r9, 0x100	/* reset vector at 0x100 */

	cmplw	0, r7, r8
	bgelr			/* return if r7>=r8 - just in case */
1:
	lwz	r0, 0(r7)
	stw	r0, 0(r9)
	addi	r7, r7, 4
	addi	r9, r9, 4
	cmplw	0, r7, r8
	bne	1b

	/*
	 * relocate `hdlr' and `int_return' entries
	 */
	li	r7, .L_MachineCheck - _start + EXC_OFF_SYS_RESET
	li	r8, Alignment - _start + EXC_OFF_SYS_RESET
2:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	2b

	li	r7, .L_Alignment - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_ProgramCheck - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_FPUnavailable - _start + EXC_OFF_SYS_RESET
	li	r8, SystemCall - _start + EXC_OFF_SYS_RESET
3:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	3b

	li	r7, .L_Trace - _start + EXC_OFF_SYS_RESET
	li	r8, _end_of_vectors - _start + EXC_OFF_SYS_RESET
4:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	4b

	mfmsr	r3			/* now that the vectors have */
	lis	r7, MSR_IP@h		/* relocated into low memory */
	ori	r7, r7, MSR_IP@l	/* MSR[IP] can be turned off */
	andc	r3, r3, r7		/* (if it was on) */
	SYNC				/* Some chip revs need this... */
	mtmsr	r3
	SYNC

	mtlr	r4			/* restore link register    */
	blr
