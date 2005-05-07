/*
 * (C) Copyright 2005
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

#include <config.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include "test_burst.h"

	.text
/*
 *	void mmu_init(void);
 *
 *	This function turns the MMU on
 *
 *	Three 8 MByte regions are mapped 1:1, uncached
 *	- SDRAM lower 8 MByte
 *	- SDRAM higher 8 MByte
 *	- IMMR
 */
	.global mmu_init
mmu_init:
	tlbia			/* Invalidate all TLB entries */
	li	r8, 0
	mtspr	MI_CTR, r8	/* Set instruction control to zero */
	lis	r8, MD_RESETVAL@h
	mtspr	MD_CTR, r8	/* Set data TLB control */

	/* Now map the lower 8 Meg into the TLBs.  For this quick hack,
	 * we can load the instruction and data TLB registers with the
	 * same values.
	 */
	li	r8, MI_EVALID		/* Create EPN for address 0 */
	mtspr	MI_EPN, r8
	mtspr	MD_EPN, r8
	li	r8, MI_PS8MEG		/* Set 8M byte page */
	ori	r8, r8, MI_SVALID	/* Make it valid */
	mtspr	MI_TWC, r8
	mtspr	MD_TWC, r8
	li	r8, MI_BOOTINIT|0x2	/* Create RPN for address 0 */
	mtspr	MI_RPN, r8		/* Store TLB entry */
	mtspr	MD_RPN, r8
	lis	r8, MI_Kp@h		/* Set the protection mode */
	mtspr	MI_AP, r8
	mtspr	MD_AP, r8

	/* Now map the higher 8 Meg into the TLBs.  For this quick hack,
	 * we can load the instruction and data TLB registers with the
	 * same values.
	 */
	lwz	r9,20(r29)		/* gd->ram_size */
	addis	r9,r9,-0x80

	mr	r8, r9			/* Higher 8 Meg in SDRAM */
	ori	r8, r8, MI_EVALID	/* Mark page valid */
	mtspr	MI_EPN, r8
	mtspr	MD_EPN, r8
	li	r8, MI_PS8MEG		/* Set 8M byte page */
	ori	r8, r8, MI_SVALID	/* Make it valid */
	mtspr	MI_TWC, r8
	mtspr	MD_TWC, r8
	mr	r8, r9
	ori	r8, r8, MI_BOOTINIT|0x2
	mtspr	MI_RPN, r8		/* Store TLB entry */
	mtspr	MD_RPN, r8
	lis	r8, MI_Kp@h		/* Set the protection mode */
	mtspr	MI_AP, r8
	mtspr	MD_AP, r8

	/* Map another 8 MByte at the IMMR to get the processor
	 * internal registers (among other things).
	 */
	mfspr	r9, 638			/* Get current IMMR */
	andis.	r9, r9, 0xff80		/* Get 8Mbyte boundary */

	mr	r8, r9			/* Create vaddr for TLB */
	ori	r8, r8, MD_EVALID	/* Mark it valid */
	mtspr	MD_EPN, r8
	li	r8, MD_PS8MEG		/* Set 8M byte page */
	ori	r8, r8, MD_SVALID	/* Make it valid */
	mtspr	MD_TWC, r8
	mr	r8, r9			/* Create paddr for TLB */
	ori	r8, r8, MI_BOOTINIT|0x2 /* Inhibit cache -- Cort */
	mtspr	MD_RPN, r8

	/* We now have the lower and higher 8 Meg mapped into TLB entries,
	 * and the caches ready to work.
	 */
	mfmsr	r0
	ori	r0,r0,MSR_DR|MSR_IR
	mtspr	SRR1,r0
	mflr	r0
	mtspr	SRR0,r0
	SYNC
	rfi	/* enables MMU */

/*
 *	void caches_init(void);
 */
	.globl	caches_init
caches_init:
	sync

	mfspr	r3, IC_CST		/* Clear error bits */
	mfspr	r3, DC_CST

	lis	r3, IDC_UNALL@h		/* Unlock all */
	mtspr	IC_CST, r3
	mtspr	DC_CST, r3

	lis	r3, IDC_INVALL@h	/* Invalidate all */
	mtspr	IC_CST, r3
	mtspr	DC_CST, r3

	lis	r3, IDC_ENABLE@h	/* Enable all */
	mtspr	IC_CST, r3
	mtspr	DC_CST, r3

	blr

/*
 *	void flush_dcache_range(unsigned long start, unsigned long stop);
 */
	.global flush_dcache_range
flush_dcache_range:
	li	r5,CACHE_LINE_SIZE-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,LG_CACHE_LINE_SIZE
	beqlr
	mtctr	r4

1:	dcbf	0,r3
	addi	r3,r3,CACHE_LINE_SIZE
	bdnz	1b
	sync				/* wait for dcbf's to get to ram */
	blr

/*
 *	void disable_interrupts(void);
 */
	.global disable_interrupts
disable_interrupts:
	mfmsr	r0
	rlwinm	r0,r0,0,17,15
	mtmsr	r0
	blr
