/*
 *	Andesboot - Startup Code for Whitiger core
 *
 *	Copyright (C) 2006	Andes Technology Corporation
 *	Copyright (C) 2006	Shawn Lin <nobuhiro@andestech.com>
 *	Copyright (C) 2011	Macpaul Lin <macpaul@andestech.com>
 *				Greentime Hu <greentime@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <common.h>
#include <asm/macro.h>

/*
 * Jump vector table for EVIC mode
 */
#define ENA_DCAC		2UL
#define DIS_DCAC		~ENA_DCAC
#define ICAC_MEM_KBF_ISET	(0x07)	 	! I Cache sets per way
#define ICAC_MEM_KBF_IWAY	(0x07<<3)	! I cache ways
#define ICAC_MEM_KBF_ISZ	(0x07<<6)	! I cache line size
#define DCAC_MEM_KBF_DSET	(0x07)		! D Cache sets per way
#define DCAC_MEM_KBF_DWAY	(0x07<<3)	! D cache ways
#define DCAC_MEM_KBF_DSZ	(0x07<<6)	! D cache line size

#define PSW			$ir0
#define EIT_INTR_PSW		$ir1		! interruption $PSW
#define EIT_PREV_IPSW		$ir2		! previous $IPSW
#define EIT_IVB			$ir3		! intr vector base address
#define EIT_EVA			$ir4		! MMU related Exception VA reg
#define EIT_PREV_EVA		$ir5		! previous $eva
#define EIT_ITYPE		$ir6		! interruption type
#define EIT_PREV_ITYPE		$ir7		! prev intr type
#define EIT_MACH_ERR		$ir8		! machine error log
#define EIT_INTR_PC		$ir9		! Interruption PC
#define EIT_PREV_IPC		$ir10		! previous $IPC
#define EIT_OVL_INTR_PC		$ir11		! overflow interruption PC
#define EIT_PREV_P0		$ir12		! prev $P0
#define EIT_PREV_P1		$ir13		! prev $p1
#define CR_ICAC_MEM		$cr1		! I-cache/memory config reg
#define CR_DCAC_MEM		$cr2		! D-cache/memory config reg
#define MR_CAC_CTL		$mr8

.globl _start

_start:	j	reset
	j	tlb_fill
	j	tlb_not_present
	j	tlb_misc
	j	tlb_vlpt_miss
	j	machine_error
	j	debug
	j	general_exception
	j	syscall
	j	internal_interrupt		! H0I
	j	internal_interrupt		! H1I
	j	internal_interrupt		! H2I
	j	internal_interrupt		! H3I
	j	internal_interrupt		! H4I
	j	internal_interrupt		! H5I
	j	software_interrupt		! S0I

	.balign 16

/*
 * Andesboot Startup Code (reset vector)
 *
 *	1.	bootstrap
 *		1.1 reset - start of u-boot
 *		1.2 to superuser mode - as is when reset
 *		1.4 Do lowlevel_init
 *			- (this will jump out to lowlevel_init.S in SoC)
 *			- (lowlevel_init)
 *		1.3 Turn off watchdog timer
 *			- (this will jump out to watchdog.S in SoC)
 *			- (turnoff_watchdog)
 *	2.	Do critical init when reboot (not from mem)
 *	3.	Relocate andesboot to ram
 *	4.	Setup stack
 *	5.	Jump to second stage (board_init_r)
 */

/* Note: TEXT_BASE is defined by the (board-dependent) linker script */
.globl _TEXT_BASE
_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

/*
 * These are defined in the board-specific linker script.
 * Subtracting _start from them lets the linker put their
 * relative position in the executable instead of leaving
 * them null.
 */
#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word 0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif

/* IRQ stack memory (calculated at run-time) + 8 bytes */
.globl IRQ_STACK_START_IN
IRQ_STACK_START_IN:
	.word 0x0badc0de

/*
 * The bootstrap code of nds32 core
 */

reset:
set_ivb:
	li	$r0, 0x0

	/* turn on BTB */
	mtsr	$r0, $misc_ctl
	/* set IVIC, vector size: 4 bytes, base: 0x0 */
	mtsr	$r0, $ivb

load_lli:
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	jal	load_lowlevel_init
	jral	$p0
#endif

/*
 * Set the N1213 (Whitiger) core to superuser mode
 * According to spec, it is already when reset
 */
turnoff_wtdog:
#ifndef CONFIG_SKIP_TRUNOFF_WATCHDOG
	jal	load_turnoff_watchdog
	jral	$p0
#endif

/*
 * Do CPU critical regs init only at reboot,
 * not when booting from ram
 */
#ifdef CONFIG_INIT_CRITICAL
	bal	cpu_init_crit		! Do CPU critical regs init
#endif

/*
 * Set stackpointer in internal RAM to call board_init_f
 * $sp must be 8-byte alignment for ABI compliance.
 */
call_board_init_f:
	li		$sp, CONFIG_SYS_INIT_SP_ADDR
	li		$r10, GD_SIZE	/* get GD size */
	sub		$sp, $sp, $r10	/* GD start addr */
	move	$r10, $sp
	li		$r0, 0x00000000

#ifdef __PIC__
#ifdef __NDS32_N1213_43U1H__
/* __NDS32_N1213_43U1H__ implies NDS32 V0 ISA */
	la	$r15, board_init_f	! store function address into $r15
#endif
#endif
	j	board_init_f		! jump to board_init_f() in lib/board.c

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 */
.globl	relocate_code
relocate_code:
	move	$r4, $r0		/* save addr_sp */
	move	$r5, $r1		/* save addr of gd */
	move	$r6, $r2		/* save addr of destination */

/* Set up the stack */
stack_setup:
	move	$sp, $r4

	la	$r0, _start

	beq	$r0, $r6, clear_bss	/* skip relocation */

	move	$r1, $r6		/* r1 <- scratch for copy_loop */
	la	$r3, __bss_start
	sub	$r3, $r3, $r0		/* r3 <- __bss_start_ofs */
	add	$r2, $r0, $r3		/* r2 <- source end address */

copy_loop:
	lwi.p	$r7, [$r0], #4
	swi.p	$r7, [$r1], #4
	blt	$r0, $r2, copy_loop

/*
 * fix relocations related issues
 */
fix_relocations:
	l.w	$r0, _TEXT_BASE		/* r0 <- Text base */
	sub	$r9, $r6, $r0		/* r9 <- relocation offset */

fix_got:
/*
 * Now we want to update GOT.
 *
 * GOT[0] is reserved. GOT[1] is also reserved for the dynamic object
 * generated by GNU ld. Skip these reserved entries from relocation.
 */
	la	$r2, __got_start	/* r2 <- rel __got_start in FLASH */
	add	$r2, $r2, $r9		/* r2 <- rel __got_start in RAM */
	la	$r3, __got_end		/* r3 <- rel __got_end in FLASH */
	add	$r3, $r3, $r9		/* r3 <- rel __got_end in RAM */
	addi	$r2, $r2, #8		/* skipping first two entries */
fix_got_loop:
	lwi	$r0, [$r2]		/* r0 <- location in FLASH to fix up */
	add	$r0, $r0, $r9		/* r0 <- location fix up to RAM */
	swi.p	$r0, [$r2], #4		/* r0 <- store fix into .got in RAM */
	blt	$r2, $r3, fix_got_loop

clear_bss:
	la	$r0, __bss_start	/* r0 <- rel __bss_start in FLASH */
	add	$r0, $r0, $r9		/* r0 <- rel __bss_start in FLASH */
	la	$r1, __bss_end		/* r1 <- rel __bss_end in RAM */
	add	$r1, $r1, $r9		/* r0 <- rel __bss_end in RAM */
	li	$r2, 0x00000000		/* clear */

clbss_l:
	sw	$r2, [$r0]		/* clear loop... */
	addi	$r0, $r0, #4
	bne	$r0, $r1, clbss_l

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
call_board_init_r:
	la	$r0, board_init_r
	move	$lp, $r0		/* offset of board_init_r() */
	add	$lp, $lp, $r9		/* real address of board_init_r() */
	/* setup parameters for board_init_r */
	move	$r0, $r5		/* gd_t */
	move	$r1, $r6		/* dest_addr */

#ifdef __PIC__
#ifdef __NDS32_N1213_43U1H__		/* NDS32 V0 ISA	*/
	move	$r15, $lp		/* store function address into $r15 */
#endif
#endif

	/* jump to it ... */
	jr	$lp			/* jump to board_init_r() */

/*
 * Initialize CPU critical registers
 *
 *	1.	Setup control registers
 *		1.1 Mask all IRQs
 *		1.2 Flush cache and TLB
 *		1.3 Disable MMU and cache
 *	2.	Setup memory timing
 */

cpu_init_crit:

	move	$r0, $lp		/* push	ra */

	/* Disable Interrupts by clear GIE in $PSW reg */
	setgie.d

	/* Flush caches and TLB */
	/* Invalidate caches */
	bal	invalidate_icac
	bal	invalidate_dcac

	/* Flush TLB */
	mfsr	$p0, $MMU_CFG
	andi	$p0, $p0, 0x3			! MMPS
	li	$p1, 0x2			! TLB MMU
	bne	$p0, $p1, 1f
	tlbop	flushall			! Flush TLB

1:
	! Disable MMU, Dcache
	! Whitiger is MMU disabled when reset
	! Disable the D$
	mfsr	$p0, MR_CAC_CTL			! Get the $CACHE_CTL reg
	li	$p1, DIS_DCAC
	and	$p0, $p0, $p1			! Set DC_EN bit
	mtsr	$p0, MR_CAC_CTL			! write back the $CACHE_CTL reg
	isb

	move	$lp, $r0
2:
	ret

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
load_lowlevel_init:
	la  $r6, lowlevel_init
	la  $r7, load_lli + 4
	sub $p0, $r6, $r7
	add $p0, $p0, $lp
ret
#endif

#ifndef CONFIG_SKIP_TRUNOFF_WATCHDOG
load_turnoff_watchdog:
	la  $r6, turnoff_watchdog
	la  $r7, turnoff_wtdog + 4
	sub $p0, $r6, $r7
	add $p0, $p0, $lp
ret
#endif

/*
 * Invalidate I$
 */
invalidate_icac:
	! read $cr1(I CAC/MEM cfg. reg.) configuration
	mfsr	$t0, CR_ICAC_MEM

	! Get the ISZ field
	andi	$p0, $t0, ICAC_MEM_KBF_ISZ

	! if $p0=0, then no I CAC existed
	beqz	$p0, end_flush_icache

	! get $p0 the index of I$ block
	srli	$p0, $p0, 6

	! $t1= bit width of I cache line size(ISZ)
	addi	$t1, $p0, 2

	li	$t4, 1
	sll	$t5, $t4, $t1			! get $t5 cache line size
	andi	$p1, $t0, ICAC_MEM_KBF_ISET	! get the ISET field
	addi	$t2, $p1, 6			! $t2= bit width of ISET
	andi	$p1, $t0, ICAC_MEM_KBF_IWAY	! get bitfield of Iway
	srli	$p1, $p1, 3
	addi	$p1, $p1, 1			! then $p1 is I way number
	add	$t3, $t2, $t1			! SHIFT
	sll	$p1, $p1, $t3			! GET the total cache size
ICAC_LOOP:
	sub	$p1, $p1, $t5
	cctl	$p1, L1I_IX_INVAL
	bnez	$p1, ICAC_LOOP
end_flush_icache:
	ret

/*
 * Invalidate D$
 */
invalidate_dcac:
	! read $cr2(D CAC/MEM cfg. reg.) configuration
	mfsr	$t0, CR_DCAC_MEM

	! Get the DSZ field
	andi	$p0, $t0, DCAC_MEM_KBF_DSZ

	! if $p0=0, then no D CAC existed
	beqz	$p0, end_flush_dcache

	! get $p0 the index of D$ block
	srli	$p0, $p0, 6

	! $t1= bit width of D cache line size(DSZ)
	addi	$t1, $p0, 2

	li	$t4, 1
	sll	$t5, $t4, $t1			! get $t5 cache line size
	andi	$p1, $t0, DCAC_MEM_KBF_DSET	! get the DSET field
	addi	$t2, $p1, 6			! $t2= bit width of DSET
	andi	$p1, $t0, DCAC_MEM_KBF_DWAY	! get bitfield of D way
	srli	$p1, $p1, 3
	addi	$p1, $p1, 1			! then $p1 is D way number
	add	$t3, $t2, $t1			! SHIFT
	sll	$p1, $p1, $t3			! GET the total cache size
DCAC_LOOP:
	sub	$p1, $p1, $t5
	cctl	$p1, L1D_IX_INVAL
	bnez	$p1, DCAC_LOOP
end_flush_dcache:
	ret

/*
 * Interrupt handling
 */

/*
 * exception handlers
 */
	.align	5

.macro	SAVE_ALL
	! FIXME: Other way to get PC?
	! FIXME: Update according to the newest spec!!
1:
	li	 $r28, 1
	push $r28
	mfsr $r28, PSW			! $PSW
	push $r28
	mfsr $r28, EIT_EVA		! $ir1 $EVA
	push $r28
	mfsr $r28, EIT_ITYPE		! $ir2 $ITYPE
	push $r28
	mfsr $r28, EIT_MACH_ERR		! $ir3 Mach Error
	push $r28
	mfsr $r28, EIT_INTR_PSW		! $ir5 $IPSW
	push $r28
	mfsr $r28, EIT_PREV_IPSW	! $ir6 prev $IPSW
	push $r28
	mfsr $r28, EIT_PREV_EVA		! $ir7 prev $EVA
	push $r28
	mfsr $r28, EIT_PREV_ITYPE	! $ir8 prev $ITYPE
	push $r28
	mfsr $r28, EIT_INTR_PC		! $ir9 Interruption PC
	push $r28
	mfsr $r28, EIT_PREV_IPC		! $ir10 prev INTR_PC
	push $r28
	mfsr $r28, EIT_OVL_INTR_PC	! $ir11 Overflowed INTR_PC
	push $r28
	mfusr $r28, $d1.lo
	push $r28
	mfusr $r28, $d1.hi
	push $r28
	mfusr $r28, $d0.lo
	push $r28
	mfusr $r28, $d0.hi
	push $r28
	pushm $r0, $r30		! store $sp-$r31, ra-$r30, $gp-$r29, $r28-$fp
	addi	$sp, $sp, -4	! make room for implicit pt_regs parameters
.endm

	.align	5
tlb_fill:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 1				! Determine interruption type
	bal 	do_interruption

	.align	5
tlb_not_present:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 2				! Determine interruption type
	bal 	do_interruption

	.align	5
tlb_misc:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 3				! Determine interruption type
	bal 	do_interruption

	.align	5
tlb_vlpt_miss:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 4				! Determine interruption type
	bal	do_interruption

	.align	5
machine_error:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 5				! Determine interruption type
	bal	do_interruption

	.align	5
debug:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 6				! Determine interruption type
	bal	do_interruption

	.align	5
general_exception:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 7				! Determine interruption type
	bal	do_interruption

	.align	5
syscall:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 8				! Determine interruption type
	bal	do_interruption

	.align	5
internal_interrupt:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 9				! Determine interruption type
	bal	do_interruption

	.align	5
software_interrupt:
	SAVE_ALL
	move	$r0, $sp			! To get the kernel stack
	li	$r1, 10				! Determine interruption type
	bal	do_interruption

	.align	5

/*
 * void reset_cpu(ulong addr);
 * $r0: input address to jump to
 */
.globl reset_cpu
reset_cpu:
/* No need to disable MMU because we never enable it */

	bal	invalidate_icac
	bal	invalidate_dcac
	mfsr	$p0, $MMU_CFG
	andi	$p0, $p0, 0x3			! MMPS
	li	$p1, 0x2			! TLB MMU
	bne	$p0, $p1, 1f
	tlbop	flushall			! Flush TLB
1:
	mfsr	$p0, MR_CAC_CTL			! Get the $CACHE_CTL reg
	li	$p1, DIS_DCAC
	and	$p0, $p0, $p1			! Clear the DC_EN bit
	mtsr	$p0, MR_CAC_CTL			! Write back the $CACHE_CTL reg
	br	$r0				! Jump to the input address
