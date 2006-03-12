/*
 * U-boot - traps.c Routines related to interrupts and exceptions
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * This file is based on
 * No original Copyright holder listed,
 * Probabily original (C) Roman Zippel (assigned DJD, 1999)
 *
 * Copyright 2003 Metrowerks - for Blackfin
 * Copyright 2000-2001 Lineo, Inc. D. Jeff Dionne <jeff@lineo.ca>
 * Copyright 1999-2000 D. Jeff Dionne, <jeff@uclinux.org>
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <linux/types.h>
#include <asm/errno.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/traps.h>
#include <asm/page.h>
#include <asm/machdep.h>
#include "cpu.h"

void init_IRQ(void)
{
	blackfin_init_IRQ();
	return;
}

void process_int(unsigned long vec, struct pt_regs *fp)
{
	return;
}

void dump(struct pt_regs *fp)
{
	printf("PC: %08lx\n", fp->pc);
	printf("SEQSTAT: %08lx    SP: %08lx\n", (long) fp->seqstat,
		(long) fp);
	printf("R0: %08lx    R1: %08lx    R2: %08lx    R3: %08lx\n",
		fp->r0, fp->r1, fp->r2, fp->r3);
	printf("R4: %08lx    R5: %08lx    R6: %08lx    R7: %08lx\n",
		fp->r4, fp->r5, fp->r6, fp->r7);
	printf("P0: %08lx    P1: %08lx    P2: %08lx    P3: %08lx\n",
		fp->p0, fp->p1, fp->p2, fp->p3);
	printf("P4: %08lx    P5: %08lx    FP: %08lx\n", fp->p4, fp->p5,
		fp->fp);
	printf("A0.w: %08lx    A0.x: %08lx    A1.w: %08lx    A1.x: %08lx\n",
		fp->a0w, fp->a0x, fp->a1w, fp->a1x);
	printf("\n");
}
