/*
 * U-boot - traps.c Routines related to interrupts and exceptions
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <linux/types.h>
#include <asm/errno.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/traps.h>
#include <asm/machdep.h>
#include "cpu.h"
#include <asm/arch/anomaly.h>
#include <asm/cplb.h>
#include <asm/io.h>

void init_IRQ(void)
{
	blackfin_init_IRQ();
	return;
}

void process_int(unsigned long vec, struct pt_regs *fp)
{
	printf("interrupt\n");
	return;
}

extern unsigned int icplb_table[page_descriptor_table_size][2];
extern unsigned int dcplb_table[page_descriptor_table_size][2];

unsigned long last_cplb_fault_retx;

static unsigned int cplb_sizes[4] =
    { 1024, 4 * 1024, 1024 * 1024, 4 * 1024 * 1024 };

void trap_c(struct pt_regs *regs)
{
	unsigned int addr;
	unsigned long trapnr = (regs->seqstat) & SEQSTAT_EXCAUSE;
	unsigned int i, j, size, *I0, *I1;
	unsigned short data = 0;

	switch (trapnr) {
		/* 0x26 - Data CPLB Miss */
	case VEC_CPLB_M:

#ifdef ANOMALY_05000261
		/*
		 * Work around an anomaly: if we see a new DCPLB fault, return
		 * without doing anything.  Then, if we get the same fault again,
		 * handle it.
		 */
		addr = last_cplb_fault_retx;
		last_cplb_fault_retx = regs->retx;
		printf("this time, curr = 0x%08x last = 0x%08x\n", addr,
		       last_cplb_fault_retx);
		if (addr != last_cplb_fault_retx)
			goto trap_c_return;
#endif
		data = 1;

	case VEC_CPLB_I_M:

		if (data)
			addr = *pDCPLB_FAULT_ADDR;
		else
			addr = *pICPLB_FAULT_ADDR;

		for (i = 0; i < page_descriptor_table_size; i++) {
			if (data) {
				size = cplb_sizes[dcplb_table[i][1] >> 16];
				j = dcplb_table[i][0];
			} else {
				size = cplb_sizes[icplb_table[i][1] >> 16];
				j = icplb_table[i][0];
			}
			if ((j <= addr) && ((j + size) > addr)) {
				debug("found %i 0x%08x\n", i, j);
				break;
			}
		}
		if (i == page_descriptor_table_size) {
			printf("something is really wrong\n");
			do_reset(NULL, 0, 0, NULL);
		}

		/* Turn the cache off */
		if (data) {
			sync();
			asm(" .align 8; ");
			*(unsigned int *)DMEM_CONTROL &=
			    ~(ACACHE_BCACHE | ENDCPLB | PORT_PREF0);
			sync();
		} else {
			sync();
			asm(" .align 8; ");
			*(unsigned int *)IMEM_CONTROL &= ~(IMC | ENICPLB);
			sync();
		}

		if (data) {
			I0 = (unsigned int *)DCPLB_ADDR0;
			I1 = (unsigned int *)DCPLB_DATA0;
		} else {
			I0 = (unsigned int *)ICPLB_ADDR0;
			I1 = (unsigned int *)ICPLB_DATA0;
		}

		j = 0;
		while (*I1 & CPLB_LOCK) {
			debug("skipping %i %08p - %08x\n", j, I1, *I1);
			*I0++;
			*I1++;
			j++;
		}

		debug("remove %i 0x%08x  0x%08x\n", j, *I0, *I1);

		for (; j < 15; j++) {
			debug("replace %i 0x%08x  0x%08x\n", j, I0, I0 + 1);
			*I0++ = *(I0 + 1);
			*I1++ = *(I1 + 1);
		}

		if (data) {
			*I0 = dcplb_table[i][0];
			*I1 = dcplb_table[i][1];
			I0 = (unsigned int *)DCPLB_ADDR0;
			I1 = (unsigned int *)DCPLB_DATA0;
		} else {
			*I0 = icplb_table[i][0];
			*I1 = icplb_table[i][1];
			I0 = (unsigned int *)ICPLB_ADDR0;
			I1 = (unsigned int *)ICPLB_DATA0;
		}

		for (j = 0; j < 16; j++) {
			debug("%i 0x%08x  0x%08x\n", j, *I0++, *I1++);
		}

		/* Turn the cache back on */
		if (data) {
			j = *(unsigned int *)DMEM_CONTROL;
			sync();
			asm(" .align 8; ");
			*(unsigned int *)DMEM_CONTROL =
			    ACACHE_BCACHE | ENDCPLB | PORT_PREF0 | j;
			sync();
		} else {
			sync();
			asm(" .align 8; ");
			*(unsigned int *)IMEM_CONTROL = IMC | ENICPLB;
			sync();
		}

		break;
	default:
		/* All traps come here */
		printf("code=[0x%x], ", (unsigned int)(regs->seqstat & 0x3f));
		printf("stack frame=0x%x, ", (unsigned int)regs);
		printf("bad PC=0x%04x\n", (unsigned int)regs->pc);
		dump(regs);
		printf("\n\n");

		printf("Unhandled IRQ or exceptions!\n");
		printf("Please reset the board \n");
		do_reset(NULL, 0, 0, NULL);
	}

trap_c_return:
	return;

}

void dump(struct pt_regs *fp)
{
	debug("RETE:  %08lx  RETN: %08lx  RETX: %08lx  RETS: %08lx\n", fp->rete,
	      fp->retn, fp->retx, fp->rets);
	debug("IPEND: %04lx  SYSCFG: %04lx\n", fp->ipend, fp->syscfg);
	debug("SEQSTAT: %08lx    SP: %08lx\n", (long)fp->seqstat, (long)fp);
	debug("R0: %08lx    R1: %08lx    R2: %08lx    R3: %08lx\n", fp->r0,
	      fp->r1, fp->r2, fp->r3);
	debug("R4: %08lx    R5: %08lx    R6: %08lx    R7: %08lx\n", fp->r4,
	      fp->r5, fp->r6, fp->r7);
	debug("P0: %08lx    P1: %08lx    P2: %08lx    P3: %08lx\n", fp->p0,
	      fp->p1, fp->p2, fp->p3);
	debug("P4: %08lx    P5: %08lx    FP: %08lx\n", fp->p4, fp->p5, fp->fp);
	debug("A0.w: %08lx    A0.x: %08lx    A1.w: %08lx    A1.x: %08lx\n",
	      fp->a0w, fp->a0x, fp->a1w, fp->a1x);

	debug("LB0: %08lx  LT0: %08lx  LC0: %08lx\n", fp->lb0, fp->lt0,
	      fp->lc0);
	debug("LB1: %08lx  LT1: %08lx  LC1: %08lx\n", fp->lb1, fp->lt1,
	      fp->lc1);
	debug("B0: %08lx  L0: %08lx  M0: %08lx  I0: %08lx\n", fp->b0, fp->l0,
	      fp->m0, fp->i0);
	debug("B1: %08lx  L1: %08lx  M1: %08lx  I1: %08lx\n", fp->b1, fp->l1,
	      fp->m1, fp->i1);
	debug("B2: %08lx  L2: %08lx  M2: %08lx  I2: %08lx\n", fp->b2, fp->l2,
	      fp->m2, fp->i2);
	debug("B3: %08lx  L3: %08lx  M3: %08lx  I3: %08lx\n", fp->b3, fp->l3,
	      fp->m3, fp->i3);

	debug("DCPLB_FAULT_ADDR=%p\n", *pDCPLB_FAULT_ADDR);
	debug("ICPLB_FAULT_ADDR=%p\n", *pICPLB_FAULT_ADDR);

}
