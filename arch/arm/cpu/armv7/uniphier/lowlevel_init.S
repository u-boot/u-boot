/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/system.h>
#include <asm/arch/led.h>
#include <asm/arch/arm-mpcore.h>
#include <asm/arch/sbc-regs.h>

ENTRY(lowlevel_init)
	mov	r8, lr			@ persevere link reg across call

	/*
	 * The UniPhier Boot ROM loads SPL code to the L2 cache.
	 * But CPUs can only do instruction fetch now because start.S has
	 * cleared C and M bits.
	 * First we need to turn on MMU and Dcache again to get back
	 * data access to L2.
	 */
	mrc	p15, 0, r0, c1, c0, 0		@ SCTLR (System Contrl Register)
	orr	r0, r0, #(CR_C | CR_M)		@ enable MMU and Dcache
	mcr	p15, 0, r0, c1, c0, 0

#ifdef CONFIG_DEBUG_LL
	bl	setup_lowlevel_debug
#endif

	/*
	 * Now we are using the page table embedded in the Boot ROM.
	 * It is not handy since it is not a straight mapped table for sLD3.
	 * What we need to do next is to switch over to the page table in SPL.
	 */
	ldr	r3, =init_page_table	@ page table must be 16KB aligned

	/* Disable MMU and Dcache before switching Page Table */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Contrl Register)
	bic	r0, r0, #(CR_C | CR_M)	@ disable MMU and Dcache
	mcr	p15, 0, r0, c1, c0, 0

	bl	enable_mmu

#ifdef CONFIG_UNIPHIER_SMP
	/*
	 * ACTLR (Auxiliary Control Register) for Cortex-A9
	 * bit[9]  Parity on
	 * bit[8]  Alloc in one way
	 * bit[7]  EXCL (Exclusive cache bit)
	 * bit[6]  SMP
	 * bit[3]  Write full line of zeros mode
	 * bit[2]  L1 Prefetch enable
	 * bit[1]  L2 prefetch enable
	 * bit[0]  FW (Cache and TLB maintenance broadcast)
	 */
	mrc	p15, 0, r0, c1, c0, 1	@ ACTLR (Auxiliary Control Register)
	orr	r0, r0, #0x41		@ enable SMP, FW bit
	mcr	p15, 0, r0, c1, c0, 1

	/* branch by CPU ID */
	mrc	p15, 0, r0, c0, c0, 5	@ MPIDR (Multiprocessor Affinity Register)
	and  	r0, r0, #0x3
	cmp	r0, #0x0
	beq	primary_cpu
	ldr	r1, =ROM_BOOT_ROMRSV2
	mov	r0, #0
	str	r0, [r1]
0:	wfe
	ldr	r0, [r1]
	cmp	r0, #0
	beq	0b
	bx	r0			@ r0: entry point of U-Boot main for the secondary CPU
primary_cpu:
	ldr	r1, =ROM_BOOT_ROMRSV2
	ldr	r0, =_start		@ entry for the secondary CPU
	str	r0, [r1]
	ldr	r0, [r1]		@ make sure str is complete before sev
	sev				@ kick the sedoncary CPU
	mrc	p15, 4, r1, c15, c0, 0	@ Configuration Base Address Register
	bfc	r1, #0, #13		@ clear bit 12-0
	mov	r0, #-1
	str	r0, [r1, #SCU_INV_ALL]	@ SCU Invalidate All Register
	mov	r0, #1			@ SCU enable
	str	r0, [r1, #SCU_CTRL]	@ SCU Control Register
#endif

	bl	setup_init_ram		@ RAM area for temporary stack pointer

	mov	lr, r8			@ restore link
	mov	pc, lr			@ back to my caller
ENDPROC(lowlevel_init)

ENTRY(enable_mmu)
	mrc	p15, 0, r0, c2, c0, 2	@ TTBCR (Translation Table Base Control Register)
	bic	r0, r0, #0x37
	orr	r0, r0, #0x20		@ disable TTBR1
	mcr	p15, 0, r0, c2, c0, 2

	orr	r0, r3, #0x8		@ Outer Cacheability for table walks: WBWA
	mcr	p15, 0, r0, c2, c0, 0   @ TTBR0

	mov	r0, #0
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs

	mov	r0, #-1			@ manager for all domains (No permission check)
	mcr	p15, 0, r0, c3, c0, 0   @ DACR (Domain Access Control Register)

	dsb
	isb
	/*
	 * MMU on:
	 * TLBs was already invalidated in "../start.S"
	 * So, we don't need to invalidate it here.
	 */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Contrl Register)
	orr	r0, r0, #(CR_C | CR_M)	@ MMU and Dcache enable
	mcr	p15, 0, r0, c1, c0, 0

	mov	pc, lr
ENDPROC(enable_mmu)

#include <asm/arch/ssc-regs.h>

#define BOOT_RAM_SIZE    (SSC_WAY_SIZE)
#define BOOT_WAY_BITS    (0x00000100)   /* way 8 */

ENTRY(setup_init_ram)
	/*
	 * Touch to zero for the boot way
	 */
0:
	/*
	 * set SSCOQM, SSCOQAD, SSCOQSZ, SSCOQWN in this order
	 */
	ldr	r0, = 0x00408006	@ touch to zero with address range
	ldr	r1, = SSCOQM
	str	r0, [r1]
	ldr	r0, = (CONFIG_SYS_INIT_SP_ADDR - BOOT_RAM_SIZE)	@ base address
	ldr	r1, = SSCOQAD
	str	r0, [r1]
	ldr	r0, = BOOT_RAM_SIZE
	ldr	r1, = SSCOQSZ
	str	r0, [r1]
	ldr	r0, = BOOT_WAY_BITS
	ldr	r1, = SSCOQWN
	str	r0, [r1]
	ldr	r1, = SSCOPPQSEF
	ldr	r0, [r1]
	cmp	r0, #0			@ check if the command is successfully set
	bne	0b			@ try again if an error occurres

	ldr	r1, = SSCOLPQS
1:
	ldr	r0, [r1]
	cmp	r0, #0x4
	bne	1b			@ wait until the operation is completed
	str	r0, [r1]		@ clear the complete notification flag

	mov	pc, lr
ENDPROC(setup_init_ram)
