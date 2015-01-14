/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <version.h>

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
