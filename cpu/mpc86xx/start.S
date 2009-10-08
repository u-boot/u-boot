/*
 * Copyright 2004, 2007 Freescale Semiconductor.
 * Srikanth Srinivasan <srikanth.srinivaan@freescale.com>
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

/*  U-Boot - Startup Code for 86xx PowerPC based Embedded Boards
 *
 *
 *  The processor starts at 0xfff00100 and the code is executed
 *  from flash. The code is organized to be at an other address
 *  in memory, but as long we don't jump around before relocating.
 *  board_init lies at a quite high address and when the cpu has
 *  jumped there, everything is ok.
 */
#include <config.h>
#include <mpc86xx.h>
#include <timestamp.h>
#include <version.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#ifndef	CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING ""
#endif

/*
 * Need MSR_DR | MSR_IR enabled to access I/O (printf) in exceptions
 */

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
	.long	0x27051956		/* U-Boot Magic Number */
	.globl	version_string
version_string:
	.ascii	U_BOOT_VERSION
	.ascii	" (", U_BOOT_DATE, " - ", U_BOOT_TIME, ")"
	.ascii	CONFIG_IDENT_STRING, "\0"

	. = EXC_OFF_SYS_RESET
	.globl	_start
_start:
	li	r21, BOOTFLAG_COLD	/* Normal Power-On: Boot from FLASH */
	b	boot_cold
	sync

	. = EXC_OFF_SYS_RESET + 0x10

	.globl	_start_warm
_start_warm:
	li	r21, BOOTFLAG_WARM	/* Software reboot */
	b	boot_warm
	sync

	/* the boot code is located below the exception table */

	.globl	_start_of_vectors
_start_of_vectors:

/* Machine check */
	STD_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x500, ExtInterrupt, external_interrupt)

/* Alignment exception. */
	. = 0x600
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
	.long	AlignmentException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_ProgramCheck:
	.long	ProgramCheckException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

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
	STD_EXCEPTION(0x1d00, InstructionBreakpoint, UnknownException)
	STD_EXCEPTION(0x1e00, PeripheralBreakpoint, UnknownException)
	STD_EXCEPTION(0x1f00, DevPortBreakpoint, UnknownException)

	.globl	_end_of_vectors
_end_of_vectors:

	. = 0x2000

boot_cold:
boot_warm:
	/*
	 * NOTE: Only Cpu 0 will ever come here.  Other cores go to an
	 * address specified by the BPTR
	 */
1:
#ifdef CONFIG_SYS_RAMBOOT
	/* disable everything */
	li	r0, 0
	mtspr	HID0, r0
	sync
	mtmsr	0
#endif

	/* Invalidate BATs */
	bl	invalidate_bats
	sync
	/* Invalidate all of TLB before MMU turn on */
	bl      clear_tlbs
	sync

#ifdef CONFIG_SYS_L2
	/* init the L2 cache */
	lis	r3, L2_INIT@h
	ori	r3, r3, L2_INIT@l
	mtspr	l2cr, r3
	/* invalidate the L2 cache */
	bl	l2cache_invalidate
	sync
#endif

	/*
	 * Calculate absolute address in FLASH and jump there
	 *------------------------------------------------------*/
	lis	r3, CONFIG_SYS_MONITOR_BASE_EARLY@h
	ori	r3, r3, CONFIG_SYS_MONITOR_BASE_EARLY@l
	addi	r3, r3, in_flash - _start + EXC_OFF_SYS_RESET
	mtlr	r3
	blr

in_flash:
	/* let the C-code set up the rest			*/
	/*							*/
	/* Be careful to keep code relocatable !		*/
	/*------------------------------------------------------*/
	/* perform low-level init */

	/* enable extended addressing */
	bl	enable_ext_addr

	/* setup the bats */
	bl	early_bats

	/*
	 * Cache must be enabled here for stack-in-cache trick.
	 * This means we need to enable the BATS.
	 * Cache should be turned on after BATs, since by default
	 * everything is write-through.
	 */

	/* enable address translation */
	mfmsr	r5
	ori	r5, r5, (MSR_IR | MSR_DR)
	lis	r3,addr_trans_enabled@h
	ori	r3, r3, addr_trans_enabled@l
	mtspr	SPRN_SRR0,r3
	mtspr	SPRN_SRR1,r5
	rfi

addr_trans_enabled:
	/* enable and invalidate the data cache */
/*	bl	l1dcache_enable */
	bl	dcache_enable
	sync

#if 1
	bl	icache_enable
#endif

#ifdef CONFIG_SYS_INIT_RAM_LOCK
	bl	lock_ram_in_cache
	sync
#endif

#if (CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR)
	bl      setup_ccsrbar
#endif

	/* set up the stack pointer in our newly created
	 * cache-ram (r1) */
	lis	r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@h
	ori	r1, r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@l

	li	r0, 0		/* Make room for stack frame header and */
	stwu	r0, -4(r1)	/* clear final stack frame so that	*/
	stwu	r0, -4(r1)	/* stack backtraces terminate cleanly	*/

	GET_GOT			/* initialize GOT access	*/

	/* run low-level CPU init code	   (from Flash) */
	bl	cpu_init_f
	sync

#ifdef	RUN_DIAG

	/* Load PX_AUX register address in r4 */
	lis	r4, PIXIS_BASE@h
	ori	r4, r4, 0x6
	/* Load contents of PX_AUX in r3 bits 24 to 31*/
	lbz	r3, 0(r4)

	/* Mask and obtain the bit in r3 */
	rlwinm. r3, r3, 0, 24, 24
	/* If not zero, jump and continue with u-boot */
	bne	diag_done

	/* Load back contents of PX_AUX in r3 bits 24 to 31 */
	lbz	r3, 0(r4)
	/* Set the MSB of the register value */
	ori	r3, r3, 0x80
	/* Write value in r3 back to PX_AUX */
	stb	r3, 0(r4)

	/* Get the address to jump to in r3*/
	lis	r3, CONFIG_SYS_DIAG_ADDR@h
	ori	r3, r3, CONFIG_SYS_DIAG_ADDR@l

	/* Load the LR with the branch address */
	mtlr	r3

	/* Branch to diagnostic */
	blr

diag_done:
#endif

/*	bl	l2cache_enable */
	mr	r3, r21

	/* r3: BOOTFLAG */
	/* run 1st part of board init code (from Flash)	  */
	bl	board_init_f
	sync

	/* NOTREACHED */

	.globl	invalidate_bats
invalidate_bats:

	li	r0, 0
	/* invalidate BATs */
	mtspr	IBAT0U, r0
	mtspr	IBAT1U, r0
	mtspr	IBAT2U, r0
	mtspr	IBAT3U, r0
	mtspr	IBAT4U, r0
	mtspr	IBAT5U, r0
	mtspr	IBAT6U, r0
	mtspr	IBAT7U, r0

	isync
	mtspr	DBAT0U, r0
	mtspr	DBAT1U, r0
	mtspr	DBAT2U, r0
	mtspr	DBAT3U, r0
	mtspr	DBAT4U, r0
	mtspr	DBAT5U, r0
	mtspr	DBAT6U, r0
	mtspr	DBAT7U, r0

	isync
	sync
	blr

/*
 * early_bats:
 *
 * Set up bats needed early on - this is usually the BAT for the
 * stack-in-cache, the Flash, and CCSR space
 */
	.globl  early_bats
early_bats:
	/* IBAT 3 */
	lis	r4, CONFIG_SYS_IBAT3L@h
	ori     r4, r4, CONFIG_SYS_IBAT3L@l
	lis	r3, CONFIG_SYS_IBAT3U@h
	ori     r3, r3, CONFIG_SYS_IBAT3U@l
	mtspr   IBAT3L, r4
	mtspr   IBAT3U, r3
	isync

	/* DBAT 3 */
	lis	r4, CONFIG_SYS_DBAT3L@h
	ori     r4, r4, CONFIG_SYS_DBAT3L@l
	lis	r3, CONFIG_SYS_DBAT3U@h
	ori     r3, r3, CONFIG_SYS_DBAT3U@l
	mtspr   DBAT3L, r4
	mtspr   DBAT3U, r3
	isync

	/* IBAT 5 */
	lis	r4, CONFIG_SYS_IBAT5L@h
	ori     r4, r4, CONFIG_SYS_IBAT5L@l
	lis	r3, CONFIG_SYS_IBAT5U@h
	ori     r3, r3, CONFIG_SYS_IBAT5U@l
	mtspr   IBAT5L, r4
	mtspr   IBAT5U, r3
	isync

	/* DBAT 5 */
	lis	r4, CONFIG_SYS_DBAT5L@h
	ori     r4, r4, CONFIG_SYS_DBAT5L@l
	lis	r3, CONFIG_SYS_DBAT5U@h
	ori     r3, r3, CONFIG_SYS_DBAT5U@l
	mtspr   DBAT5L, r4
	mtspr   DBAT5U, r3
	isync

	/* IBAT 6 */
	lis	r4, CONFIG_SYS_IBAT6L_EARLY@h
	ori     r4, r4, CONFIG_SYS_IBAT6L_EARLY@l
	lis	r3, CONFIG_SYS_IBAT6U_EARLY@h
	ori     r3, r3, CONFIG_SYS_IBAT6U_EARLY@l
	mtspr   IBAT6L, r4
	mtspr   IBAT6U, r3
	isync

	/* DBAT 6 */
	lis	r4, CONFIG_SYS_DBAT6L_EARLY@h
	ori     r4, r4, CONFIG_SYS_DBAT6L_EARLY@l
	lis	r3, CONFIG_SYS_DBAT6U_EARLY@h
	ori     r3, r3, CONFIG_SYS_DBAT6U_EARLY@l
	mtspr   DBAT6L, r4
	mtspr   DBAT6U, r3
	isync

#if(CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR)
	/* IBAT 7 */
	lis	r4, CONFIG_SYS_CCSR_DEFAULT_IBATL@h
	ori     r4, r4, CONFIG_SYS_CCSR_DEFAULT_IBATL@l
	lis	r3, CONFIG_SYS_CCSR_DEFAULT_IBATU@h
	ori     r3, r3, CONFIG_SYS_CCSR_DEFAULT_IBATU@l
	mtspr   IBAT7L, r4
	mtspr   IBAT7U, r3
	isync

	/* DBAT 7 */
	lis	r4, CONFIG_SYS_CCSR_DEFAULT_DBATL@h
	ori     r4, r4, CONFIG_SYS_CCSR_DEFAULT_DBATL@l
	lis	r3, CONFIG_SYS_CCSR_DEFAULT_DBATU@h
	ori     r3, r3, CONFIG_SYS_CCSR_DEFAULT_DBATU@l
	mtspr   DBAT7L, r4
	mtspr   DBAT7U, r3
	isync
#endif
	blr

	.globl clear_tlbs
clear_tlbs:
	addis   r3, 0, 0x0000
	addis   r5, 0, 0x4
	isync
tlblp:
	tlbie   r3
	sync
	addi    r3, r3, 0x1000
	cmp     0, 0, r3, r5
	blt tlblp
	blr

	.globl disable_addr_trans
disable_addr_trans:
	/* disable address translation */
	mflr	r4
	mfmsr	r3
	andi.	r0, r3, (MSR_IR | MSR_DR)
	beqlr
	andc	r3, r3, r0
	mtspr	SRR0, r4
	mtspr	SRR1, r3
	rfi

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

	.globl	dc_read
dc_read:
	blr

	.globl get_pvr
get_pvr:
	mfspr	r3, PVR
	blr

	.globl get_svr
get_svr:
	mfspr	r3, SVR
	blr


/*
 * Function:	in8
 * Description:	Input 8 bits
 */
	.globl	in8
in8:
	lbz	r3,0x0000(r3)
	blr

/*
 * Function:	out8
 * Description:	Output 8 bits
 */
	.globl	out8
out8:
	stb	r4,0x0000(r3)
	blr

/*
 * Function:	out16
 * Description:	Output 16 bits
 */
	.globl	out16
out16:
	sth	r4,0x0000(r3)
	blr

/*
 * Function:	out16r
 * Description:	Byte reverse and output 16 bits
 */
	.globl	out16r
out16r:
	sthbrx	r4,r0,r3
	blr

/*
 * Function:	out32
 * Description:	Output 32 bits
 */
	.globl	out32
out32:
	stw	r4,0x0000(r3)
	blr

/*
 * Function:	out32r
 * Description:	Byte reverse and output 32 bits
 */
	.globl	out32r
out32r:
	stwbrx	r4,r0,r3
	blr

/*
 * Function:	in16
 * Description:	Input 16 bits
 */
	.globl	in16
in16:
	lhz	r3,0x0000(r3)
	blr

/*
 * Function:	in16r
 * Description:	Input 16 bits and byte reverse
 */
	.globl	in16r
in16r:
	lhbrx	r3,r0,r3
	blr

/*
 * Function:	in32
 * Description:	Input 32 bits
 */
	.globl	in32
in32:
	lwz	3,0x0000(3)
	blr

/*
 * Function:	in32r
 * Description:	Input 32 bits and byte reverse
 */
	.globl	in32r
in32r:
	lwbrx	r3,r0,r3
	blr

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

	mr	r1,  r3		/* Set new stack pointer		*/
	mr	r9,  r4		/* Save copy of Global Data pointer	*/
	mr	r10, r5		/* Save copy of Destination Address	*/

	mr	r3,  r5				/* Destination Address	*/
	lis	r4, CONFIG_SYS_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4, r4, CONFIG_SYS_MONITOR_BASE@l
	lwz	r5, GOT(__init_end)
	sub	r5, r5, r4
	li	r6, CONFIG_SYS_CACHELINE_SIZE		/* Cache Line Size	*/

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CONFIG_SYS_MONITOR_BASE) + Destination Address
	 *
	 * Offset:
	 */
	sub	r15, r10, r4

	/* First our own GOT */
	add	r14, r14, r15
	/* then the one used by the C code */
	add	r30, r30, r15

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
	add	r0,r0,r11
	stw	r10,0(r3)
	stw	r0,0(r4)
	bdnz	3b
4:
/* clear_bss: */
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
	mr	r3, r9		/* Init Date pointer		*/
	mr	r4, r10		/* Destination Address		*/
	bl	board_init_r

	/* not reached - end relocate_code */
/*-----------------------------------------------------------------------*/

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

	/* enable execptions from RAM vectors */
	mfmsr	r7
	li	r8,MSR_IP
	andc	r7,r7,r8
	ori	r7,r7,MSR_ME		/* Enable Machine Check */
	mtmsr	r7

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

.globl enable_ext_addr
enable_ext_addr:
	mfspr	r0, HID0
	lis	r0, (HID0_HIGH_BAT_EN | HID0_XBSEN | HID0_XAEN)@h
	ori	r0, r0, (HID0_HIGH_BAT_EN | HID0_XBSEN | HID0_XAEN)@l
	mtspr	HID0, r0
	sync
	isync
	blr

#if (CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR)
.globl setup_ccsrbar
setup_ccsrbar:
	/* Special sequence needed to update CCSRBAR itself */
	lis	r4, CONFIG_SYS_CCSRBAR_DEFAULT@h
	ori	r4, r4, CONFIG_SYS_CCSRBAR_DEFAULT@l

	lis	r5, CONFIG_SYS_CCSRBAR_PHYS_LOW@h
	ori	r5, r5, CONFIG_SYS_CCSRBAR_PHYS_LOW@l
	srwi	r5,r5,12
	li	r6, CONFIG_SYS_CCSRBAR_PHYS_HIGH@l
	rlwimi	r5,r6,20,8,11
	stw	r5, 0(r4) /* Store physical value of CCSR */
	isync

	lis	r5, TEXT_BASE@h
	ori	r5,r5,TEXT_BASE@l
	lwz	r5, 0(r5)
	isync

	/* Use VA of CCSR to do read */
	lis	r3, CONFIG_SYS_CCSRBAR@h
	lwz	r5, CONFIG_SYS_CCSRBAR@l(r3)
	isync

	blr
#endif

#ifdef CONFIG_SYS_INIT_RAM_LOCK
lock_ram_in_cache:
	/* Allocate Initial RAM in data cache.
	 */
	lis	r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@h
	ori	r3, r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@l
	li	r4, ((CONFIG_SYS_INIT_RAM_END & ~31) + \
		     (CONFIG_SYS_INIT_RAM_ADDR & 31) + 31) / 32
	mtctr	r4
1:
	dcbz	r0, r3
	addi	r3, r3, 32
	bdnz	1b
#if 1
/* Lock the data cache */
	mfspr	r0, HID0
	ori	r0, r0, 0x1000
	sync
	mtspr	HID0, r0
	sync
	blr
#endif
#if 0
	/* Lock the first way of the data cache */
	mfspr	r0, LDSTCR
	ori	r0, r0, 0x0080
#if defined(CONFIG_ALTIVEC)
	dssall
#endif
	sync
	mtspr	LDSTCR, r0
	sync
	isync
	blr
#endif

.globl unlock_ram_in_cache
unlock_ram_in_cache:
	/* invalidate the INIT_RAM section */
	lis	r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@h
	ori	r3, r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@l
	li	r4, ((CONFIG_SYS_INIT_RAM_END & ~31) + \
		     (CONFIG_SYS_INIT_RAM_ADDR & 31) + 31) / 32
	mtctr	r4
1:	icbi	r0, r3
	addi	r3, r3, 32
	bdnz	1b
	sync			/* Wait for all icbi to complete on bus */
	isync
#if 1
/* Unlock the data cache and invalidate it */
	mfspr	r0, HID0
	li	r3,0x1000
	andc	r0,r0,r3
	li	r3,0x0400
	or	r0,r0,r3
	sync
	mtspr	HID0, r0
	sync
	blr
#endif
#if 0
	/* Unlock the first way of the data cache */
	mfspr	r0, LDSTCR
	li	r3,0x0080
	andc	r0,r0,r3
#ifdef CONFIG_ALTIVEC
	dssall
#endif
	sync
	mtspr	LDSTCR, r0
	sync
	isync
	li	r3,0x0400
	or	r0,r0,r3
	sync
	mtspr	HID0, r0
	sync
	blr
#endif
#endif
