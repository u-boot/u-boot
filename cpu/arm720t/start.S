/*
 *  armboot - Startup Code for ARM720 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
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


#include <config.h>
#include <version.h>
#include <asm/hardware.h>

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */


.globl _start
_start: b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq

	.balignl 16,0xdeadbeef


/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from RAM!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif


/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0x13
	msr	cpsr,r0

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_crit
#endif

#ifndef CONFIG_SKIP_RELOCATE_UBOOT
relocate:				/* relocate U-Boot to RAM	    */
	adr	r0, _start		/* r0 <- current position of code   */
	ldr	r1, _TEXT_BASE		/* test if we run from flash or RAM */
	cmp	r0, r1			/* don't reloc during debug	    */
	beq	stack_setup

#if TEXT_BASE
	ldr	r2, =0x0		/* Relocate the exception vectors   */
	cmp	r1, r2			/* and associated data to address   */
	ldmneia r0!, {r3-r10}		/* 0x0. Do nothing if TEXT_BASE is  */
	stmneia r2!, {r3-r10}		/* 0x0. Copy the first 15 words.    */
	ldmneia r0, {r3-r9}
	stmneia r2, {r3-r9}
	adrne	r0, _start		/* restore r0			    */
#endif

	ldr	r2, _armboot_start
	ldr	r3, _bss_start
	sub	r2, r3, r2		/* r2 <- size of armboot	    */
	add	r2, r0, r2		/* r2 <- source end address	    */

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end addreee [r2]    */
	ble	copy_loop

#endif	/* CONFIG_SKIP_RELOCATE_UBOOT */

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN /* malloc area			    */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo			    */
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment	    */
	ldr	r1, _bss_end		/* stop here			    */
	mov	r2, #0x00000000		/* clear			    */

clbss_l:str	r2, [r0]		/* clear loop...		    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	ldr	pc, _start_armboot

_start_armboot: .word start_armboot

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */

#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_ARMADILLO)

/* Interupt-Controller base addresses */
INTMR1:		.word	0x80000280 @ 32 bit size
INTMR2:		.word	0x80001280 @ 16 bit size
INTMR3:		.word	0x80002280 @  8 bit size

/* SYSCONs */
SYSCON1:	.word	0x80000100
SYSCON2:	.word	0x80001100
SYSCON3:	.word	0x80002200

#define CLKCTL	       0x6  /* mask */
#define CLKCTL_18      0x0  /* 18.432 MHz */
#define CLKCTL_36      0x2  /* 36.864 MHz */
#define CLKCTL_49      0x4  /* 49.152 MHz */
#define CLKCTL_73      0x6  /* 73.728 MHz */

#endif

cpu_init_crit:
#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_ARMADILLO)

	/*
	 * mask all IRQs by clearing all bits in the INTMRs
	 */
	mov	r1, #0x00
	ldr	r0, INTMR1
	str	r1, [r0]
	ldr	r0, INTMR2
	str	r1, [r0]
	ldr	r0, INTMR3
	str	r1, [r0]

	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15,0,r0,c1,c0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x0000008f	@ clear bits 7, 3:0 (B--- WCAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	mcr	p15,0,r0,c1,c0
#elif defined(CONFIG_NETARM)
	/*
	 * prior to software reset : need to set pin PORTC4 to be *HRESET
	 */
	ldr	r0, =NETARM_GEN_MODULE_BASE
	ldr	r1, =(NETARM_GEN_PORT_MODE(0x10) | \
			NETARM_GEN_PORT_DIR(0x10))
	str	r1, [r0, #+NETARM_GEN_PORTC]
	/*
	 * software reset : see HW Ref. Guide 8.2.4 : Software Service register
	 *		    for an explanation of this process
	 */
	ldr	r0, =NETARM_GEN_MODULE_BASE
	ldr	r1, =NETARM_GEN_SW_SVC_RESETA
	str	r1, [r0, #+NETARM_GEN_SOFTWARE_SERVICE]
	ldr	r1, =NETARM_GEN_SW_SVC_RESETB
	str	r1, [r0, #+NETARM_GEN_SOFTWARE_SERVICE]
	ldr	r1, =NETARM_GEN_SW_SVC_RESETA
	str	r1, [r0, #+NETARM_GEN_SOFTWARE_SERVICE]
	ldr	r1, =NETARM_GEN_SW_SVC_RESETB
	str	r1, [r0, #+NETARM_GEN_SOFTWARE_SERVICE]
	/*
	 * setup PLL and System Config
	 */
	ldr	r0, =NETARM_GEN_MODULE_BASE

	ldr	r1, =(	NETARM_GEN_SYS_CFG_LENDIAN | \
			NETARM_GEN_SYS_CFG_BUSFULL | \
			NETARM_GEN_SYS_CFG_USER_EN | \
			NETARM_GEN_SYS_CFG_ALIGN_ABORT | \
			NETARM_GEN_SYS_CFG_BUSARB_INT | \
			NETARM_GEN_SYS_CFG_BUSMON_EN )

	str	r1, [r0, #+NETARM_GEN_SYSTEM_CONTROL]

#ifndef CONFIG_NETARM_PLL_BYPASS
	ldr	r1, =(	NETARM_GEN_PLL_CTL_PLLCNT(NETARM_PLL_COUNT_VAL) | \
			NETARM_GEN_PLL_CTL_POLTST_DEF | \
			NETARM_GEN_PLL_CTL_INDIV(1) | \
			NETARM_GEN_PLL_CTL_ICP_DEF | \
			NETARM_GEN_PLL_CTL_OUTDIV(2) )
	str	r1, [r0, #+NETARM_GEN_PLL_CONTROL]
#endif

	/*
	 * mask all IRQs by clearing all bits in the INTMRs
	 */
	mov	r1, #0
	ldr	r0, =NETARM_GEN_MODULE_BASE
	str	r1, [r0, #+NETARM_GEN_INTR_ENABLE]

#elif defined(CONFIG_S3C4510B)

	/*
	 * Mask off all IRQ sources
	 */
	ldr	r1, =REG_INTMASK
	ldr	r0, =0x3FFFFF
	str	r0, [r1]

	/*
	 * Disable Cache
	 */
	ldr r0, =REG_SYSCFG
	ldr r1, =0x83ffffa0	/* cache-disabled  */
	str r1, [r0]

#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No specific initialisation for IntegratorAP/CM720T as yet */
#else
#error No cpu_init_crit() defined for current CPU type
#endif

#ifdef CONFIG_ARM7_REVD
	/* set clock speed */
	/* !!! we run @ 36 MHz due to a hardware flaw in Rev. D processors */
	/* !!! not doing DRAM refresh properly! */
	ldr	r0, SYSCON3
	ldr	r1, [r0]
	bic	r1, r1, #CLKCTL
	orr	r1, r1, #CLKCTL_36
	str	r1, [r0]
#endif

	mov	ip, lr
	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependent, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	bl	lowlevel_init
	mov	lr, ip

	mov	pc, lr


/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */

@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13
#define I_BIT	 0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add	r8, sp, #S_PC

	ldr	r2, _armboot_start
	sub	r2, r2, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r2, r2, #(CFG_GBL_DATA_SIZE+8)	@ set base 2 words into abort stack
	ldmia	r2, {r2 - r4}			@ get pc, cpsr, old_r0
	add	r0, sp, #S_FRAME_SIZE		@ restore sp_SVC

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r4}			@ save sp_SVC, lr_SVC, pc, cpsr, old_r
	mov	r0, sp
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}^			@ Calling SP, LR
	str	lr, [r8, #0]			@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]			@ Save CPSR
	str	r0, [r8, #8]			@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack
	sub	r13, r13, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r13, r13, #(CFG_GBL_DATA_SIZE+8) @ reserved a couple spots in abort stack

	str	lr, [r13]			@ save caller lr / spsr
	mrs	lr, spsr
	str	lr, [r13, #4]

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	msr	spsr_c, r13
	mov	lr, pc
	movs	pc, lr
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align	5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	bl	do_not_used

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	bl	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl	do_fiq
	irq_restore_user_regs

#else

	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl	do_fiq

#endif

#if defined(CONFIG_IMPA7) || defined(CONFIG_EP7312) || defined(CONFIG_ARMADILLO)
	.align	5
.globl reset_cpu
reset_cpu:
	mov	ip, #0
	mcr	p15, 0, ip, c7, c7, 0		@ invalidate cache
	mcr	p15, 0, ip, c8, c7, 0		@ flush TLB (v4)
	mrc	p15, 0, ip, c1, c0, 0		@ get ctrl register
	bic	ip, ip, #0x000f			@ ............wcam
	bic	ip, ip, #0x2100			@ ..v....s........
	mcr	p15, 0, ip, c1, c0, 0		@ ctrl register
	mov	pc, r0
#elif defined(CONFIG_NETARM)
	.align	5
.globl reset_cpu
reset_cpu:
	ldr	r1, =NETARM_MEM_MODULE_BASE
	ldr	r0, [r1, #+NETARM_MEM_CS0_BASE_ADDR]
	ldr	r1, =0xFFFFF000
	and	r0, r1, r0
	ldr	r1, =(relocate-TEXT_BASE)
	add	r0, r1, r0
	ldr	r4, =NETARM_GEN_MODULE_BASE
	ldr	r1, =NETARM_GEN_SW_SVC_RESETA
	str	r1, [r4, #+NETARM_GEN_SOFTWARE_SERVICE]
	ldr	r1, =NETARM_GEN_SW_SVC_RESETB
	str	r1, [r4, #+NETARM_GEN_SOFTWARE_SERVICE]
	ldr	r1, =NETARM_GEN_SW_SVC_RESETA
	str	r1, [r4, #+NETARM_GEN_SOFTWARE_SERVICE]
	ldr	r1, =NETARM_GEN_SW_SVC_RESETB
	str	r1, [r4, #+NETARM_GEN_SOFTWARE_SERVICE]
	mov	pc, r0
#elif defined(CONFIG_S3C4510B)
/* Nothing done here as reseting the CPU is board specific, depending
 * on external peripherals such as watchdog timers, etc. */
#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No specific reset actions for IntegratorAP/CM720T as yet */
#else
#error No reset_cpu() defined for current CPU type
#endif
