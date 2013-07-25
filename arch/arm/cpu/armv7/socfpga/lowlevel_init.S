/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <version.h>

/* Save the parameter pass in by previous boot loader */
.global save_boot_params
save_boot_params:
	/* save the parameter here */

	/*
	 * Setup stack for exception, which is located
	 * at the end of on-chip RAM. We don't expect exception prior to
	 * relocation and if that happens, we won't worry -- it will overide
	 * global data region as the code will goto reset. After relocation,
	 * this region won't be used by other part of program.
	 * Hence it is safe.
	 */
	ldr	r0, =(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE)
	ldr	r1, =IRQ_STACK_START_IN
	str	r0, [r1]

	bx	lr


/* Set up the platform, once the cpu has been initialized */
.globl lowlevel_init
lowlevel_init:

	/* Remap */
#ifdef CONFIG_SPL_BUILD
	/*
	 * SPL : configure the remap (L3 NIC-301 GPV)
	 * so the on-chip RAM at lower memory instead ROM.
	 */
	ldr	r0, =SOCFPGA_L3REGS_ADDRESS
	mov	r1, #0x19
	str	r1, [r0]
#else
	/*
	 * U-Boot : configure the remap (L3 NIC-301 GPV)
	 * so the SDRAM at lower memory instead on-chip RAM.
	 */
	ldr	r0, =SOCFPGA_L3REGS_ADDRESS
	mov	r1, #0x2
	str	r1, [r0]

	/* Private components security */

	/*
	 * U-Boot : configure private timer, global timer and cpu
	 * component access as non secure for kernel stage (as required
	 * by kernel)
	 */
	mrc	p15,4,r0,c15,c0,0
	add	r1, r0, #0x54
	ldr	r2, [r1]
	orr	r2, r2, #0xff
	orr	r2, r2, #0xf00
	str	r2, [r1]
#endif	/* #ifdef CONFIG_SPL_BUILD */
	mov	pc, lr
