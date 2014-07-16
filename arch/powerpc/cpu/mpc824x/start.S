/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000,2001,2002 Wolfgang Denk <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* U-Boot - Startup Code for PowerPC based Embedded Boards
 *
 *
 * The processor starts at 0x00000100 and the code is executed
 * from flash. The code is organized to be at an other address
 * in memory, but as long we don't jump around before relocating.
 * board_init lies at a quite high address and when the cpu has
 * jumped there, everything is ok.
 * This works because the cpu gives the FLASH (CS0) the whole
 * address space at startup, and board_init lies as a echo of
 * the flash somewhere up there in the memorymap.
 *
 * board_init will change CS0 to be positioned at the correct
 * address and (s)dram will be positioned at address 0
 */
#include <asm-offsets.h>
#include <config.h>
#include <mpc824x.h>
#include <version.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>
#include <asm/u-boot.h>

/* We don't want the MMU yet.
*/
#undef	MSR_KERNEL
/* FP, Machine Check and Recoverable Interr. */
#define MSR_KERNEL ( MSR_FP | MSR_ME | MSR_RI )

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
 * r3 - 1st arg to board_init(): IMMP pointer
 * r4 - 2nd arg to board_init(): boot flag
 */
	.text
	.long	0x27051956		/* U-Boot Magic Number			*/
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION_STRING, "\0"

	. = EXC_OFF_SYS_RESET
	.globl	_start
_start:
	/* Initialize machine status; enable machine check interrupt		*/
	/*----------------------------------------------------------------------*/
	li	r3, MSR_KERNEL		/* Set FP, ME, RI flags */
	mtmsr	r3
	mtspr	SRR1, r3		/* Make SRR1 match MSR */

	addis	r0,0,0x0000		/* lets make sure that r0 is really 0 */
	mtspr   HID0, r0		/* disable I and D caches */

	mfspr	r3, ICR			/* clear Interrupt Cause Register */

	mfmsr	r3			/* turn off address translation */
	addis	r4,0,0xffff
	ori	r4,r4,0xffcf
	and	r3,r3,r4
	mtmsr	r3
	isync
	sync				/* the MMU should be off... */


in_flash:
	/*
	 * Setup BATs - cannot be done in C since we don't have a stack yet
	 */
	bl	setup_bats

	/* Enable MMU.
	 */
	mfmsr	r3
	ori	r3, r3, (MSR_IR | MSR_DR)
	mtmsr	r3

	/* Enable and invalidate data cache.
	 */
	mfspr	r3, HID0
	mr	r2, r3
	ori	r3, r3, HID0_DCE | HID0_DCI
	ori	r2, r2, HID0_DCE
	sync
	mtspr	HID0, r3
	mtspr	HID0, r2
	sync

	/* Allocate Initial RAM in data cache.
	 */
	lis	r3, CONFIG_SYS_INIT_RAM_ADDR@h
	ori	r3, r3, CONFIG_SYS_INIT_RAM_ADDR@l
	li	r2, 128
	mtctr	r2
1:
	dcbz	r0, r3
	addi	r3, r3, 32
	bdnz	1b

	/* Lock way0 in data cache.
	 */
	mfspr	r3, 1011
	lis	r2, 0xffff
	ori	r2, r2, 0xff1f
	and	r3, r3, r2
	ori	r3, r3, 0x0080
	sync
	mtspr	1011, r3

	/*
	 * Thisk the stack pointer *somewhere* sensible. Doesnt
	 * matter much where as we'll move it when we relocate
	 */
	lis	r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@h
	ori	r1, r1, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@l

	li	r0, 0			/* Make room for stack frame header and	*/
	stwu	r0, -4(r1)		/* clear final stack frame so that	*/
	stwu	r0, -4(r1)		/* stack backtraces terminate cleanly	*/

	/* let the C-code set up the rest					*/
	/*									*/
	/* Be careful to keep code relocatable !				*/
	/*----------------------------------------------------------------------*/

	GET_GOT			/* initialize GOT access			*/

	/* r3: IMMR */
	bl	cpu_init_f	/* run low-level CPU init code     (from Flash)	*/

	bl	board_init_f	/* run 1st part of board init code (from Flash) */

	/* NOTREACHED - board_init_f() does not return */


	.globl	_start_of_vectors
_start_of_vectors:

/* Machine check */
	STD_EXCEPTION(EXC_OFF_MACH_CHCK, MachineCheck, MachineCheckException)

/* Data Storage exception.  "Never" generated on the 860. */
	STD_EXCEPTION(EXC_OFF_DATA_STOR, DataStorage, UnknownException)

/* Instruction Storage exception.  "Never" generated on the 860. */
	STD_EXCEPTION(EXC_OFF_INS_STOR, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(EXC_OFF_EXTERNAL, ExtInterrupt, external_interrupt)

/* Alignment exception. */
	. = EXC_OFF_ALIGN
Alignment:
	EXCEPTION_PROLOG(SRR0, SRR1)
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(Alignment, AlignmentException, MSR_KERNEL, COPY_EE)

/* Program check exception */
	. = EXC_OFF_PROGRAM
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(ProgramCheck, ProgramCheckException,
		MSR_KERNEL, COPY_EE)

	/* No FPU on MPC8xx. This exception is not supposed to happen.
	*/
	STD_EXCEPTION(EXC_OFF_FPUNAVAIL, FPUnavailable, UnknownException)

	/* I guess we could implement decrementer, and may have
	 * to someday for timekeeping.
	 */
	STD_EXCEPTION(EXC_OFF_DECR, Decrementer, timer_interrupt)
	STD_EXCEPTION(0xa00, Trap_0a, UnknownException)
	STD_EXCEPTION(0xb00, Trap_0b, UnknownException)
	STD_EXCEPTION(0xc00, SystemCall, UnknownException)

	STD_EXCEPTION(EXC_OFF_TRACE, SingleStep, UnknownException)

	STD_EXCEPTION(EXC_OFF_FPUNASSIST, Trap_0e, UnknownException)
	STD_EXCEPTION(EXC_OFF_PMI, Trap_0f, UnknownException)

	STD_EXCEPTION(EXC_OFF_ITME, InstructionTransMiss, UnknownException)
	STD_EXCEPTION(EXC_OFF_DLTME, DataLoadTransMiss, UnknownException)
	STD_EXCEPTION(EXC_OFF_DSTME, DataStoreTransMiss, UnknownException)
	STD_EXCEPTION(EXC_OFF_IABE, InstructionBreakpoint, DebugException)
	STD_EXCEPTION(EXC_OFF_SMIE, SysManageInt, UnknownException)
	STD_EXCEPTION(0x1500, Reserved5, UnknownException)
	STD_EXCEPTION(0x1600, Reserved6, UnknownException)
	STD_EXCEPTION(0x1700, Reserved7, UnknownException)
	STD_EXCEPTION(0x1800, Reserved8, UnknownException)
	STD_EXCEPTION(0x1900, Reserved9, UnknownException)
	STD_EXCEPTION(0x1a00, ReservedA, UnknownException)
	STD_EXCEPTION(0x1b00, ReservedB, UnknownException)
	STD_EXCEPTION(0x1c00, ReservedC, UnknownException)
	STD_EXCEPTION(0x1d00, ReservedD, UnknownException)
	STD_EXCEPTION(0x1e00, ReservedE, UnknownException)
	STD_EXCEPTION(0x1f00, ReservedF, UnknownException)

	STD_EXCEPTION(EXC_OFF_RMTE, RunModeTrace, UnknownException)

	.globl	_end_of_vectors
_end_of_vectors:


	. = 0x3000

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
#if 0
	andi.	r23,r23,MSR_PR
	mfspr	r23,SPRG3		/* if from user, fix up tss.regs */
	beq	2f
	addi	r24,r1,STACK_FRAME_OVERHEAD
	stw	r24,PT_REGS(r23)
2:	addi	r2,r23,-TSS		/* set r2 to current */
	tovirt(r2,r2,r23)
#endif
	mflr	r23
	andi.	r24,r23,0x3f00		/* get vector offset */
	stw	r24,TRAP(r21)
	li	r22,0
	stw	r22,RESULT(r21)
	mtspr	SPRG2,r22		/* r1 is now kernel sp */
#if 0
	addi	r24,r2,TASK_STRUCT_SIZE /* check for kernel stack overflow */
	cmplw	0,r1,r2
	cmplw	1,r1,r24
	crand	1,1,4
	bgt	stack_ovf		/* if r2 < r1 < r2+TASK_STRUCT_SIZE */
#endif
	lwz	r24,0(r23)		/* virtual address of handler */
	lwz	r23,4(r23)		/* where to go when done */
	mtspr	SRR0,r24
	ori	r20,r20,0x30		/* enable IR, DR */
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

/* Cache functions.
*/
	.globl	icache_enable
icache_enable:
	mfspr	r5,HID0		/* turn on the I cache. */
	ori	r5,r5,0x8800	/* Instruction cache only! */
	addis	r6,0,0xFFFF
	ori	r6,r6,0xF7FF
	and	r6,r5,r6	/* clear the invalidate bit */
	sync
	mtspr	HID0,r5
	mtspr	HID0,r6
	isync
	sync
	blr

	.globl	icache_disable
icache_disable:
	mfspr	r5,HID0
	addis	r6,0,0xFFFF
	ori	r6,r6,0x7FFF
	and	r5,r5,r6
	sync
	mtspr	HID0,r5
	isync
	sync
	blr

	.globl	icache_status
icache_status:
	mfspr	r3, HID0
	srwi	r3, r3, 15	/* >>15 & 1=> select bit 16 */
	andi.	r3, r3, 1
	blr

	.globl	dcache_enable
dcache_enable:
	mfspr	r5,HID0		/* turn on the D cache. */
	ori	r5,r5,0x4400	/* Data cache only! */
	mfspr	r4, PVR		/* read PVR */
	srawi	r3, r4, 16	/* shift off the least 16 bits */
	cmpi	0, 0, r3, 0xC	/* Check for Max pvr */
	bne	NotMax
	ori	r5,r5,0x0040	/* setting the DCFA bit, for Max rev 1 errata */
NotMax:
	addis	r6,0,0xFFFF
	ori	r6,r6,0xFBFF
	and	r6,r5,r6	/* clear the invalidate bit */
	sync
	mtspr	HID0,r5
	mtspr	HID0,r6
	isync
	sync
	blr

	.globl	dcache_disable
dcache_disable:
	mfspr	r5,HID0
	addis	r6,0,0xFFFF
	ori	r6,r6,0xBFFF
	and	r5,r5,r6
	sync
	mtspr	HID0,r5
	isync
	sync
	blr

	.globl	dcache_status
dcache_status:
	mfspr	r3, HID0
	srwi	r3, r3, 14	/* >>14 & 1=> select bit 17 */
	andi.	r3, r3, 1
	blr

	.globl	dc_read
dc_read:
/*TODO : who uses this, what should it do?
*/
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

	mr	r1,  r3		/* Set new stack pointer		*/
	mr	r9,  r4		/* Save copy of Global Data pointer	*/
	mr	r10, r5		/* Save copy of Destination Address	*/

	GET_GOT
	mr	r3,  r5				/* Destination Address	*/
#ifdef CONFIG_SYS_RAMBOOT
	lis	r4, CONFIG_SYS_SDRAM_BASE@h		/* Source      Address	*/
	ori	r4, r4, CONFIG_SYS_SDRAM_BASE@l
#else
	lis	r4, CONFIG_SYS_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4, r4, CONFIG_SYS_MONITOR_BASE@l
#endif
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
	add	r12, r12, r15
	/* the the one used by the C code */
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

4:
/* Unlock the data cache and invalidate locked area */
	xor	r0, r0, r0
	mtspr	1011, r0
	lis	r4, CONFIG_SYS_INIT_RAM_ADDR@h
	ori	r4, r4, CONFIG_SYS_INIT_RAM_ADDR@l
	li	r0, 128
	mtctr	r0
41:
	dcbi	r0, r4
	addi	r4, r4, 32
	bdnz	41b

/*
 * Now flush the cache: note that we must start from a cache aligned
 * address. Otherwise we might miss one cache line.
 */
	cmpwi	r6,0
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
	blt	5b
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
	mflr	r4			/* save link register		*/
	GET_GOT
	lwz	r7, GOT(_start)
	lwz	r8, GOT(_end_of_vectors)

	li	r9, 0x100		/* reset vector always at 0x100 */

	cmplw	0, r7, r8
	bgelr				/* return if r7>=r8 - just in case */
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

	/* Setup the BAT registers.
	 */
setup_bats:
	lis	r4, CONFIG_SYS_IBAT0L@h
	ori	r4, r4, CONFIG_SYS_IBAT0L@l
	lis	r3, CONFIG_SYS_IBAT0U@h
	ori	r3, r3, CONFIG_SYS_IBAT0U@l
	mtspr	IBAT0L, r4
	mtspr	IBAT0U, r3
	isync

	lis	r4, CONFIG_SYS_DBAT0L@h
	ori	r4, r4, CONFIG_SYS_DBAT0L@l
	lis	r3, CONFIG_SYS_DBAT0U@h
	ori	r3, r3, CONFIG_SYS_DBAT0U@l
	mtspr	DBAT0L, r4
	mtspr	DBAT0U, r3
	isync

	lis	r4, CONFIG_SYS_IBAT1L@h
	ori	r4, r4, CONFIG_SYS_IBAT1L@l
	lis	r3, CONFIG_SYS_IBAT1U@h
	ori	r3, r3, CONFIG_SYS_IBAT1U@l
	mtspr	IBAT1L, r4
	mtspr	IBAT1U, r3
	isync

	lis	r4, CONFIG_SYS_DBAT1L@h
	ori	r4, r4, CONFIG_SYS_DBAT1L@l
	lis	r3, CONFIG_SYS_DBAT1U@h
	ori	r3, r3, CONFIG_SYS_DBAT1U@l
	mtspr	DBAT1L, r4
	mtspr	DBAT1U, r3
	isync

	lis	r4, CONFIG_SYS_IBAT2L@h
	ori	r4, r4, CONFIG_SYS_IBAT2L@l
	lis	r3, CONFIG_SYS_IBAT2U@h
	ori	r3, r3, CONFIG_SYS_IBAT2U@l
	mtspr	IBAT2L, r4
	mtspr	IBAT2U, r3
	isync

	lis	r4, CONFIG_SYS_DBAT2L@h
	ori	r4, r4, CONFIG_SYS_DBAT2L@l
	lis	r3, CONFIG_SYS_DBAT2U@h
	ori	r3, r3, CONFIG_SYS_DBAT2U@l
	mtspr	DBAT2L, r4
	mtspr	DBAT2U, r3
	isync

	lis	r4, CONFIG_SYS_IBAT3L@h
	ori	r4, r4, CONFIG_SYS_IBAT3L@l
	lis	r3, CONFIG_SYS_IBAT3U@h
	ori	r3, r3, CONFIG_SYS_IBAT3U@l
	mtspr	IBAT3L, r4
	mtspr	IBAT3U, r3
	isync

	lis	r4, CONFIG_SYS_DBAT3L@h
	ori	r4, r4, CONFIG_SYS_DBAT3L@l
	lis	r3, CONFIG_SYS_DBAT3U@h
	ori	r3, r3, CONFIG_SYS_DBAT3U@l
	mtspr	DBAT3L, r4
	mtspr	DBAT3U, r3
	isync

	/* Invalidate TLBs.
	 * -> for (val = 0; val < 0x20000; val+=0x1000)
	 * ->   tlbie(val);
	 */
	lis	r3, 0
	lis	r5, 2

1:
	tlbie	r3
	addi	r3, r3, 0x1000
	cmp	0, 0, r3, r5
	blt	1b

	blr
