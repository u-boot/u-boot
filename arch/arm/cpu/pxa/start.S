/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for XScale CPU-core
 *
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2001	Alex Zuepke <azu@sysgo.de>
 *  Copyright (C) 2001	Marius Groger <mag@sysgo.de>
 *  Copyright (C) 2002	Alex Zupke <azu@sysgo.de>
 *  Copyright (C) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (C) 2002	Kyle Harris <kharris@nexus-tech.net>
 *  Copyright (C) 2003	Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>
 *  Copyright (C) 2003	Kshitij <kshitij@ti.com>
 *  Copyright (C) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (C) 2003	Robert Schwebel <r.schwebel@pengutronix.de>
 *  Copyright (C) 2004	Texas Instruments <r-woodruff2@ti.com>
 *  Copyright (C) 2010	Marek Vasut <marek.vasut@gmail.com>
 */

#include <asm-offsets.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

	.globl	reset

reset:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	bl  cpu_init_crit
#endif

#ifdef	CONFIG_CPU_PXA27X
	/*
	 * enable clock for SRAM
	 */
	ldr	r0,=CKEN
	ldr	r1,[r0]
	orr	r1,r1,#(1 << 20)
	str	r1,[r0]
#endif
	bl	_main

/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:
	bx	lr

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
#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* Invalidate I+D+BTB caches */
	mcr	p15, 0, r0, c8, c7, 0	/* Invalidate Unified TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00003300	@ clear bits 13:12, 9:8 (--VI --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (A) Align
	mcr	p15, 0, r0, c1, c0, 0

	mov	pc, lr		/* back to my caller */
#endif /* !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT) */
