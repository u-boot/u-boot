/*
 * Copyright (C) 2013 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/system.h>
#include <asm/arch/led.h>
#include <asm/arch/sbc-regs.h>

/* Entry point of U-Boot main program for the secondary CPU */
LENTRY(secondary_entry)
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Contrl Register)
	bic	r0, r0, #(CR_C | CR_M)	@ MMU and Dcache disable
	mcr	p15, 0, r0, c1, c0, 0
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	dsb
	led_write(C,0,,)
	ldr	r1, =ROM_BOOT_ROMRSV2
	mov	r0, #0
	str	r0, [r1]
0:	wfe
	ldr	r4, [r1]		@ r4: entry point for secondary CPUs
	cmp	r4, #0
	beq	0b
	led_write(C, P, U, 1)
	bx	r4			@ secondary CPUs jump to linux
ENDPROC(secondary_entry)

ENTRY(wakeup_secondary)
	ldr	r1, =ROM_BOOT_ROMRSV2
0:	ldr	r0, [r1]
	cmp	r0, #0
	bne	0b

	/* set entry address and send event to the secondary CPU */
	ldr	r0, =secondary_entry
	str	r0, [r1]
	ldr	r0, [r1]	@ make sure store is complete
	mov	r0, #0x100
0:	subs	r0, r0, #1	@ I don't know the reason, but without this wait
	bne	0b		@ fails to wake up the secondary CPU
	sev

	/* wait until the secondary CPU reach to secondary_entry */
0:	ldr	r0, [r1]
	cmp	r0, #0
	bne	0b
	bx	lr
ENDPROC(wakeup_secondary)
