/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  crt0 - C-runtime startup Code for ARM U-Boot
 *
 *  Copyright (c) 2012  Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

#include <config.h>
#include <asm-offsets.h>
#include <linux/linkage.h>
#include <asm/assembler.h>

/*
 * This file handles the target-independent stages of the U-Boot
 * start-up where a C runtime environment is needed. Its entry point
 * is _main and is branched into from the target's start.S file.
 *
 * _main execution sequence is:
 *
 * 1. Set up initial environment for calling board_init_f().
 *    This environment only provides a stack and a place to store
 *    the GD ('global data') structure, both located in some readily
 *    available RAM (SRAM, locked cache...). In this context, VARIABLE
 *    global data, initialized or not (BSS), are UNAVAILABLE; only
 *    CONSTANT initialized data are available. GD should be zeroed
 *    before board_init_f() is called.
 *
 * 2. Call board_init_f(). This function prepares the hardware for
 *    execution from system RAM (DRAM, DDR...) As system RAM may not
 *    be available yet, , board_init_f() must use the current GD to
 *    store any data which must be passed on to later stages. These
 *    data include the relocation destination, the future stack, and
 *    the future GD location.
 *
 * 3. Set up intermediate environment where the stack and GD are the
 *    ones allocated by board_init_f() in system RAM, but BSS and
 *    initialized non-const data are still not available.
 *
 * 4a.For U-Boot proper (not SPL), call relocate_code(). This function
 *    relocates U-Boot from its current location into the relocation
 *    destination computed by board_init_f().
 *
 * 4b.For SPL, board_init_f() just returns (to crt0). There is no
 *    code relocation in SPL.
 *
 * 5. Set up final environment for calling board_init_r(). This
 *    environment has BSS (initialized to 0), initialized non-const
 *    data (initialized to their intended value), and stack in system
 *    RAM (for SPL moving the stack and GD into RAM is optional - see
 *    CONFIG_SPL_STACK_R). GD has retained values set by board_init_f().
 *
 * 6. For U-Boot proper (not SPL), some CPUs have some work left to do
 *    at this point regarding memory, so call c_runtime_cpu_setup.
 *
 * 7. Branch to board_init_r().
 *
 * For more information see 'Board Initialisation Flow in README.
 */

/*
 * Macro for clearing BSS during SPL execution. Usually called during the
 * relocation process for most boards before entering board_init_r(), but
 * can also be done early before entering board_init_f() on plaforms that
 * can afford it due to sufficient memory being available early.
 */

.macro CLEAR_BSS
	ldr	r0, =__bss_start	/* this is auto-relocated! */

#ifdef CONFIG_USE_ARCH_MEMSET
	ldr	r3, =__bss_end		/* this is auto-relocated! */
	mov	r1, #0x00000000		/* prepare zero to clear BSS */

	subs	r2, r3, r0		/* r2 = memset len */
	bl	memset
#else
	ldr	r1, =__bss_end		/* this is auto-relocated! */
	mov	r2, #0x00000000		/* prepare zero to clear BSS */

clbss_l:cmp	r0, r1			/* while not at end of BSS */
	strlo	r2, [r0]		/* clear 32-bit BSS word */
	addlo	r0, r0, #4		/* move to next */
	blo	clbss_l
#endif
.endm

/*
 * entry point of crt0 sequence
 */

ENTRY(_main)

/* Enable Domain 2 RAMs. SRAM1 through SRAM3 are where .data, /bss. the stack, etc. are located, so it has to be turned on first thing. */

    ldr r0,=0x580244DC      /* RCC_AHB2ENR |= SRAM1EN | SRAM2EN | SRAM3 EN; */
    ldr r1, [r0]
    orr r1, #0xE0000000
    str r1, [r0]

/*
 * Set up initial C runtime environment and call board_init_f(0).
 */

	ldr	r0, =(CONFIG_SYS_INIT_SP_ADDR)
	bic	r0, r0, #7	/* 8-byte alignment for ABI compliance */
	mov	sp, r0

 	bl	relocate_data               /* relocate initialized data in on-chip ROM to on-chip SRAM */

	CLEAR_BSS

    mov r0, sp
	bl	board_init_f_alloc_reserve
	mov	sp, r0
	/* set up gd here, outside any C code */
    ldr r1, =CONFIG_SYS_GLOBAL_DATA_POINTER
	str r0, [r1]
	bl	board_init_f_init_reserve

	mov	r0, #0x800                  /* GD_FLG_SKIP_RELOC */
	bl	board_init_f

    ldr r2, =CONFIG_SYS_GLOBAL_DATA_POINTER
    ldr r1, [r2]
	ldr	r0, [r1, #GD_START_ADDR_SP]	/* sp = gd->start_addr_sp */
	bic	r0, r0, #7	                /* 8-byte alignment for ABI compliance */
	mov	sp, r0
	ldr	r0, [r1, #GD_NEW_GD]		/* global data pointer <- gd->new_gd */
	str r0, [r2]


/*
 * now relocate vectors
 */

	bl	relocate_vectors

/* Set up final (full) environment */

	bl	c_runtime_cpu_setup	/* we still call old routine here */

	bl coloured_LED_init
	bl red_led_on

	/* call board_init_r(gd_t *id, ulong dest_addr) */
    ldr r2, =CONFIG_SYS_GLOBAL_DATA_POINTER
    ldr r0, [r2]
	ldr	r1, [r2, #GD_RELOCADDR]	/* dest_addr */
	/* call board_init_r */
	ldr	lr, =board_init_r	/* this is auto-relocated! */
	bx	lr
	/* we should not return here. */

ENDPROC(_main)

