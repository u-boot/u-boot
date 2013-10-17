/*
 * (C) Copyright 2009 Magnus Lilja <lilja.magnus@gmail.com>
 * (C) Copyright 2011 Helmut Raiger <helmut.raiger@hale.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <config.h>
#include <asm/arch/imx-regs.h>
#include <asm/macro.h>

.globl lowlevel_init
lowlevel_init:
	/* Also setup the Peripheral Port Remap register inside the core */
	ldr	r0, =ARM_PPMRR      /* start from AIPS 2GB region */
	mcr	p15, 0, r0, c15, c2, 4
	mov	pc, lr
