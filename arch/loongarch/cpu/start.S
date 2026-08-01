/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Startup Code for LoongArch CPU
 *
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <asm-offsets.h>
#include <config.h>
#include <elf.h>
#include <system-constants.h>
#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/loongarch.h>
#include <asm/arch/entry-init.h>

#define BOOTCORE_ID		0

.section .text
.globl _start
_start:
	/* Allow arch specific setup code for MCSR stuff */
	entry_setup

	/* Disable interrupt */
	LONG_LI		t0, CSR_CRMD_IE
	csrxchg		zero, t0, LOONGARCH_CSR_CRMD

	/* Configure reset ebase */
	la.pcrel	t0, exception_entry
	csrwr		t0, LOONGARCH_CSR_EENTRY

	/* Setup direct map window for later use */
	PTR_LI		t0, CSR_DMW0_INIT
	csrwr		t0, LOONGARCH_CSR_DMWIN0
	PTR_LI		t0, CSR_DMW1_INIT
	csrwr		t0, LOONGARCH_CSR_DMWIN1

	/* Branch out for nonboot core */
	csrrd		t0, LOONGARCH_CSR_CPUID
	andi		t0, t0, CSR_CPUID_COREID
	LONG_LI		t1, BOOTCORE_ID
	bne			t1, t0, secondary_core_loop

/*
 * Set stackpointer in internal/ex RAM to call board_init_f
 */
call_board_init_f:
	PTR_LI		t0, (SYS_INIT_SP_ADDR & STACK_ALIGN)
	move		sp, t0			/* save stack pointer */
/*
 * Now sp points to the right stack belonging to current CPU.
 * It's essential before any function call, otherwise, we get data-race.
 */

call_board_init_f_0:
	/* find top of reserve space */
	PTR_LI		t1, 1
	PTR_SLL		t1, t1, CONFIG_STACK_SIZE_SHIFT
	PTR_SUB		a0, t0, t1		/* t1 -> size of all CPU stacks */
	bl		board_init_f_alloc_reserve

	/* Set global pointer to u0 ($r21) */
	move		u0, a0
	bl		board_init_f_init_reserve

	/* Probe and enable cache */
	bl		probe_caches
	bl		enable_caches

#ifdef CONFIG_DEBUG_UART
	bl		debug_uart_init
#endif

	move		a0, zero		/* a0 <-- boot_flags = 0 */
	la.pcrel	t5, board_init_f
	jirl		ra, t5, 0		/* jump to board_init_f() */

	move		sp, s0

/*
 * void relocate_code(addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 */
.globl relocate_code
relocate_code:
	move		s2, a0			/* save addr_sp */
	move		s3, a1			/* save addr of gd */
	move		s4, a2			/* save addr of destination */

/*
 *Set up the stack
 */
stack_setup:
	move		sp, s2
	la.pcrel	t0, _start
	PTR_SUB		s5, s4, t0		/* s5 <- relocation offset */
	beq		t0, s4, clear_bss	/* skip relocation */

	move		t1, s4			/* t1 <- scratch for copy_loop */
	la.pcrel	t2, __bss_start		/* t2 <- source end address */

copy_loop:
	LONG_L		t5, t0, 0
	PTR_ADDI	t0, t0, LONGSIZE
	LONG_S		t5, t1, 0
	PTR_ADDI	t1, t1, LONGSIZE
	blt		t0, t2, copy_loop

/*
 * Update dynamic relocations after board_init_f
 */
fix_rela_dyn:
	la.pcrel	t1, __rel_dyn_start
	la.pcrel	t2, __rel_dyn_end
	beq		t1, t2, clear_bss
	PTR_ADD		t1, t1, s5		/* t1 <- rela_dyn_start in RAM */
	PTR_ADD		t2, t2, s5		/* t2 <- rela_dyn_end in RAM */

6:
	PTR_L		t5, t1, PTRSIZE		/* t5 <-- relocation info:type */
	PTR_LI		t3, R_LARCH_RELATIVE	/* reloc type R_LARCH_RELATIVE */
	bne		t5, t3, 8f		/* skip non-RELATIVE entries */
	PTR_L		t3, t1, 0
	PTR_L		t5, t1, (PTRSIZE * 2)	/* t5 <-- addend */
	PTR_ADD		t5, t5, s5		/* t5 <-- location to fix up in RAM */
	PTR_ADD		t3, t3, s5		/* t3 <-- location to fix up in RAM */
	PTR_S		t5, t3, 0
8:
	PTR_ADDI	t1, t1, (PTRSIZE * 3)
	blt		t1, t2, 6b


	/* Update exception entry */
	la.pcrel	t0, exception_entry
	PTR_ADD		t0, t0, s5
	csrwr		t0, LOONGARCH_CSR_EENTRY

clear_bss:
	la.pcrel	t0, __bss_start		/* t0 <- rel __bss_start in FLASH */
	PTR_ADD		t0, t0, s5		/* t0 <- rel __bss_start in RAM */
	la.pcrel	t1, __bss_end		/* t1 <- rel __bss_end in FLASH */
	PTR_ADD		t1, t1, s5		/* t1 <- rel __bss_end in RAM */

clbss_l:
	LONG_S		zero, t0, 0		/* clear loop... */
	PTR_ADDI	t0, t0, LONGSIZE
	blt		t0, t1, clbss_l


/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
call_board_init_r:
	bl		invalidate_icache_all
	bl		flush_dcache_all
	la.pcrel	t0, board_init_r        /* offset of board_init_r() */
	PTR_ADD		t4, t0, s5		/* real address of board_init_r() */
/*
 * setup parameters for board_init_r
 */
	move		a0, s3			/* gd_t */
	move		a1, s4			/* dest_addr */
	move		s0, zero		/* fp == NULL */
	jirl		ra, t4, 0		/* jump to board_init_r() */

