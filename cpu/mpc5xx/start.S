/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000, 2001, 2002 Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2003  Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
 * File:		start.S
 *
 * Discription:		startup code
 *
 */

#include <config.h>
#include <mpc5xx.h>
#include <version.h>

#define CONFIG_5xx 1		/* needed for Linux kernel header files */
#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <linux/config.h>
#include <asm/processor.h>

#ifndef  CONFIG_IDENT_STRING
#define  CONFIG_IDENT_STRING ""
#endif

/* We don't have a MMU.
*/
#undef	MSR_KERNEL
#define MSR_KERNEL ( MSR_ME | MSR_RI )		/* Machine Check and Recoverable Interr. */

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
 * r3 - 1st arg to board_init(): IMMP pointer
 * r4 - 2nd arg to board_init(): boot flag
 */
	.text
	.long	0x27051956			/* U-Boot Magic Number */
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION
	.ascii " (", __DATE__, " - ", __TIME__, ")"
	.ascii CONFIG_IDENT_STRING, "\0"

	. = EXC_OFF_SYS_RESET
	.globl	_start
_start:
	mfspr	r3, 638
	li	r4, CFG_ISB			/* Set ISB bit */
	or	r3, r3, r4
	mtspr	638, r3
	li	r21, BOOTFLAG_COLD		/* Normal Power-On: Boot from FLASH	*/
	b	boot_cold

	. = EXC_OFF_SYS_RESET + 0x20

	.globl	_start_warm
_start_warm:
	li	r21, BOOTFLAG_WARM		/* Software reboot */
	b	boot_warm

boot_cold:
boot_warm:

	/* Initialize machine status; enable machine check interrupt		*/
	/*----------------------------------------------------------------------*/
	li	r3, MSR_KERNEL			/* Set ME, RI flags */
	mtmsr	r3
	mtspr	SRR1, r3			/* Make SRR1 match MSR */

	/* Initialize debug port registers					*/
	/*----------------------------------------------------------------------*/
	xor	r0, r0, r0			/* Clear R0 */
	mtspr	LCTRL1, r0			/* Initialize debug port regs */
	mtspr	LCTRL2, r0
	mtspr	COUNTA, r0
	mtspr	COUNTB, r0

#if defined(CONFIG_PATI)
	/* the external flash access on PATI fails if programming the PLL to 40MHz.
	 * Copy the PLL programming code to the internal RAM and execute it
	 *----------------------------------------------------------------------*/
	lis	r3, CFG_MONITOR_BASE@h
	ori	r3, r3, CFG_MONITOR_BASE@l
	addi	r3, r3, pll_prog_code_start - _start + EXC_OFF_SYS_RESET

	lis	r4, CFG_INIT_RAM_ADDR@h
	ori	r4, r4, CFG_INIT_RAM_ADDR@l
	mtlr	r4
	addis	r5,0,0x0
	ori	r5,r5,((pll_prog_code_end - pll_prog_code_start) >>2)
	mtctr	r5
	addi	r3, r3, -4
	addi	r4, r4, -4
0:
	lwzu	r0,4(r3)
	stwu	r0,4(r4)
	bdnz	0b                /* copy loop */
	blrl
#endif

	/*
	 * Calculate absolute address in FLASH and jump there
	 *----------------------------------------------------------------------*/

	lis	r3, CFG_MONITOR_BASE@h
	ori	r3, r3, CFG_MONITOR_BASE@l
	addi	r3, r3, in_flash - _start + EXC_OFF_SYS_RESET
	mtlr	r3
	blr

in_flash:

	/* Initialize some SPRs that are hard to access from C			*/
	/*----------------------------------------------------------------------*/

	lis     r3, CFG_IMMR@h          	/* Pass IMMR as arg1 to C routine */
	lis	r2, CFG_INIT_SP_ADDR@h
	ori	r1, r2, CFG_INIT_SP_ADDR@l	/* Set up the stack in internal SRAM */
	/* Note: R0 is still 0 here */
	stwu	r0, -4(r1)			/* Clear final stack frame so that	*/
	stwu	r0, -4(r1)			/* stack backtraces terminate cleanly	*/

	/*
	 * Disable serialized ifetch and show cycles
	 * (i.e. set processor to normal mode) for maximum
	 * performance.
	 */

	li	r2, 0x0007
	mtspr	ICTRL, r2

	/* Set up debug mode entry */

	lis	r2, CFG_DER@h
	ori	r2, r2, CFG_DER@l
	mtspr	DER, r2

	/* Let the C-code set up the rest					*/
	/*									*/
	/* Be careful to keep code relocatable !				*/
	/*----------------------------------------------------------------------*/

	GET_GOT			/* initialize GOT access			*/

	/* r3: IMMR */
	bl	cpu_init_f	/* run low-level CPU init code     (from Flash)	*/

	mr	r3, r21
	/* r3: BOOTFLAG */
	bl	board_init_f	/* run 1st part of board init code (from Flash) */


	.globl	_start_of_vectors
_start_of_vectors:

/* Machine check */
	STD_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception.  "Never" generated on the 860. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception.  "Never" generated on the 860. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x500, ExtInterrupt, external_interrupt)

/* Alignment exception. */
	. = 0x600
Alignment:
	EXCEPTION_PROLOG
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
	.long	AlignmentException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_ProgramCheck:
	.long	ProgramCheckException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

	/* FPU on MPC5xx available. We will use it later.
	*/
	STD_EXCEPTION(0x800, FPUnavailable, UnknownException)

	/* I guess we could implement decrementer, and may have
	 * to someday for timekeeping.
	 */
	STD_EXCEPTION(0x900, Decrementer, timer_interrupt)
	STD_EXCEPTION(0xa00, Trap_0a, UnknownException)
	STD_EXCEPTION(0xb00, Trap_0b, UnknownException)
	STD_EXCEPTION(0xc00, SystemCall, UnknownException)
	STD_EXCEPTION(0xd00, SingleStep, UnknownException)

	STD_EXCEPTION(0xe00, Trap_0e, UnknownException)
	STD_EXCEPTION(0xf00, Trap_0f, UnknownException)

	/* On the MPC8xx, this is a software emulation interrupt.  It occurs
	 * for all unimplemented and illegal instructions.
	 */
	STD_EXCEPTION(0x1000, SoftEmu, SoftEmuException)
	STD_EXCEPTION(0x1100, InstructionTLBMiss, UnknownException)
	STD_EXCEPTION(0x1200, DataTLBMiss, UnknownException)
	STD_EXCEPTION(0x1300, InstructionTLBError, UnknownException)
	STD_EXCEPTION(0x1400, DataTLBError, UnknownException)

	STD_EXCEPTION(0x1500, Reserved5, UnknownException)
	STD_EXCEPTION(0x1600, Reserved6, UnknownException)
	STD_EXCEPTION(0x1700, Reserved7, UnknownException)
	STD_EXCEPTION(0x1800, Reserved8, UnknownException)
	STD_EXCEPTION(0x1900, Reserved9, UnknownException)
	STD_EXCEPTION(0x1a00, ReservedA, UnknownException)
	STD_EXCEPTION(0x1b00, ReservedB, UnknownException)

	STD_EXCEPTION(0x1c00, DataBreakpoint, UnknownException)
	STD_EXCEPTION(0x1d00, InstructionBreakpoint, DebugException)
	STD_EXCEPTION(0x1e00, PeripheralBreakpoint, UnknownException)
	STD_EXCEPTION(0x1f00, DevPortBreakpoint, UnknownException)


	.globl	_end_of_vectors
_end_of_vectors:


	. = 0x2000

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
	mfmsr	r28			/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC				/* Some chip revs need this... */
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
	lwz	r2,_NIP(r1)		/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SRR0,r2
	mtspr	SRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfi


/*
 * unsigned int get_immr (unsigned int mask)
 *
 * return (mask ? (IMMR & mask) : IMMR);
 */
	.globl	get_immr
get_immr:
	mr	r4,r3		/* save mask */
	mfspr	r3, IMMR	/* IMMR */
	cmpwi	0,r4,0		/* mask != 0 ? */
	beq	4f
	and	r3,r3,r4	/* IMMR & mask */
4:
	blr

	.globl get_pvr
get_pvr:
	mfspr	r3, PVR
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
	mr	r1,  r3		/* Set new stack pointer in SRAM	*/
	mr	r9,  r4		/* Save copy of global data pointer in SRAM */
	mr	r10, r5		/* Save copy of monitor destination Address in SRAM */

	mr	r3,  r5				/* Destination Address	*/
	lis	r4, CFG_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4, r4, CFG_MONITOR_BASE@l
	lwz	r5, GOT(__init_end)
	sub	r5, r5, r4

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CFG_MONITOR_BASE) + Destination Address
	 *
	 * Offset:
	 */
	sub	r15, r10, r4

	/* First our own GOT */
	add	r14, r14, r15
	/* the the one used by the C code */
	add	r30, r30, r15

	/*
	 * Now relocate code
	 */

	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary	*/
	beq	4f		/* Protect against 0 count		*/
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

4:	sync
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
	lwz	r7, GOT(_start)
	lwz	r8, GOT(_end_of_vectors)

	li	r9, 0x100		/* reset vector always at 0x100 */

	cmplw	0, r7, r8
	bgelr				/* return if r7>=r8 - just in case */

	mflr	r4			/* save link register		*/
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
	addi	r7, r7, 0x100		/* next exception vector	*/
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
	addi	r7, r7, 0x100		/* next exception vector	*/
	cmplw	0, r7, r8
	blt	3b

	li	r7, .L_SingleStep - _start + EXC_OFF_SYS_RESET
	li	r8, _end_of_vectors - _start + EXC_OFF_SYS_RESET
4:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector	*/
	cmplw	0, r7, r8
	blt	4b

	mtlr	r4			/* restore link register	*/
	blr

	/*
	 * Function: relocate entries for one exception vector
	 */
trap_reloc:
	lwz	r0, 0(r7)		/* hdlr ...			*/
	add	r0, r0, r3		/*  ... += dest_addr		*/
	stw	r0, 0(r7)

	lwz	r0, 4(r7)		/* int_return ...		*/
	add	r0, r0, r3		/*  ... += dest_addr		*/
	stw	r0, 4(r7)

	sync
	isync

	blr


#if defined(CONFIG_PATI)
/* Program the PLL */
pll_prog_code_start:
	lis	r4, (CFG_IMMR + 0x002fc384)@h
	ori	r4, r4, (CFG_IMMR + 0x002fc384)@l
	lis	r3, (0x55ccaa33)@h
	ori	r3, r3, (0x55ccaa33)@l
	stw	r3, 0(r4)
	lis	r4, (CFG_IMMR + 0x002fc284)@h
	ori	r4, r4, (CFG_IMMR + 0x002fc284)@l
	lis	r3, CFG_PLPRCR@h
	ori	r3, r3, CFG_PLPRCR@l
	stw	r3, 0(r4)
	addis	r3,0,0x0
	ori	r3,r3,0xA000
	mtctr	r3
..spinlp:
  bdnz    ..spinlp                /* spin loop */
	blr
pll_prog_code_end:
	nop
	blr
#endif
