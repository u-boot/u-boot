/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>
#include <linux/sizes.h>
#include <asm/system.h>
#include <mach/led.h>
#include <mach/arm-mpcore.h>
#include <mach/sbc-regs.h>
#include <mach/ssc-regs.h>

ENTRY(lowlevel_init)
	mov	r8, lr			@ persevere link reg across call

	/*
	 * The UniPhier Boot ROM loads SPL code to the L2 cache.
	 * But CPUs can only do instruction fetch now because start.S has
	 * cleared C and M bits.
	 * First we need to turn on MMU and Dcache again to get back
	 * data access to L2.
	 */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r0, r0, #(CR_C | CR_M)	@ enable MMU and Dcache
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
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	bic	r0, r0, #(CR_C | CR_M)	@ disable MMU and Dcache
	mcr	p15, 0, r0, c1, c0, 0

	bl	enable_mmu

#ifdef CONFIG_UNIPHIER_SMP
secondary_startup:
	/*
	 * Entry point for secondary CPUs
	 *
	 * The Boot ROM has already enabled MMU for the secondary CPUs as well
	 * as for the primary one.  The MMU table embedded in the Boot ROM
	 * prohibits the DRAM access, so it is impossible to bring the
	 * secondary CPUs into DRAM directly.  They must jump here into SPL,
	 * which is run on L2 cache.
	 *
	 * Boot Sequence
	 *  [primary CPU]                    [secondary CPUs]
	 *  start from Boot ROM             start from Boot ROM
	 *     jump to SPL                    sleep in Boot ROM
	 *  kick secondaries   ---(sev)--->    jump to SPL
	 *  jump to U-Boot main               sleep in SPL
	 *  jump to Linux
	 *  kick secondaries   ---(sev)--->    jump to Linux
	 */

	/* branch by CPU ID */
	mrc	p15, 0, r0, c0, c0, 5	@ MPIDR (Multiprocessor Affinity Register)
	and  	r0, r0, #0x3
	cmp	r0, #0x0
	beq	primary_cpu
	/* only for secondary CPUs */
	ldr	r1, =ROM_BOOT_ROMRSV2	@ The last data access to L2 cache
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r0, r0, #CR_I		@ Enable ICache
	bic	r0, r0, #(CR_C | CR_M)	@ MMU and Dcache must be disabled
	mcr	p15, 0, r0, c1, c0, 0	@ before jumping to Linux
	mov	r0, #0
	str	r0, [r1]
	b	1f
	/*
	 * L2 cache is shared among all the CPUs and it might be disabled by
	 * the primary one.  Before that, the following 5 lines must be cached
	 * on the Icaches of the secondary CPUs.
	 */
0:	wfe				@ kicked by Linux
1:	ldr	r0, [r1]
	cmp	r0, #0
	bxne	r0			@ r0: Linux entry for secondary CPUs
	b	0b
primary_cpu:
	ldr	r1, =ROM_BOOT_ROMRSV2
	ldr	r0, =secondary_startup
	str	r0, [r1]
	ldr	r0, [r1]		@ make sure str is complete before sev
	sev				@ kick the secondary CPU
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
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r0, r0, #(CR_C | CR_M)	@ MMU and Dcache enable
	mcr	p15, 0, r0, c1, c0, 0

	mov	pc, lr
ENDPROC(enable_mmu)

/*
 * For PH1-Pro4 or older SoCs, the size of WAY is 32KB.
 * It is large enough for tmp RAM.
 */
#define BOOT_RAM_SIZE    (SZ_32K)
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
	ldr	r0, = (CONFIG_SPL_STACK - BOOT_RAM_SIZE)	@ base address
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
	bne	0b			@ try again if an error occurs

	ldr	r1, = SSCOLPQS
1:
	ldr	r0, [r1]
	cmp	r0, #0x4
	bne	1b			@ wait until the operation is completed
	str	r0, [r1]		@ clear the complete notification flag

	mov	pc, lr
ENDPROC(setup_init_ram)
