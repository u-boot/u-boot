/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 * Kumar Gala <kumar.gala@freescale.com>
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

#include <asm-offsets.h>
#include <config.h>
#include <mpc85xx.h>
#include <version.h>

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

/* To boot secondary cpus, we need a place for them to start up.
 * Normally, they start at 0xfffffffc, but that's usually the
 * firmware, and we don't want to have to run the firmware again.
 * Instead, the primary cpu will set the BPTR to point here to
 * this page.  We then set up the core, and head to
 * start_secondary.  Note that this means that the code below
 * must never exceed 1023 instructions (the branch at the end
 * would then be the 1024th).
 */
	.globl	__secondary_start_page
	.align	12
__secondary_start_page:
/* First do some preliminary setup */
	lis	r3, HID0_EMCP@h		/* enable machine check */
#ifndef CONFIG_E500MC
	ori	r3,r3,HID0_TBEN@l	/* enable Timebase */
#endif
#ifdef CONFIG_PHYS_64BIT
	ori	r3,r3,HID0_ENMAS7@l	/* enable MAS7 updates */
#endif
	mtspr	SPRN_HID0,r3

#ifndef CONFIG_E500MC
	li	r3,(HID1_ASTME|HID1_ABE)@l	/* Addr streaming & broadcast */
	mfspr   r0,PVR
	andi.	r0,r0,0xff
	cmpwi	r0,0x50@l	/* if we are rev 5.0 or greater set MBDD */
	blt 1f
	/* Set MBDD bit also */
	ori r3, r3, HID1_MBDD@l
1:
	mtspr	SPRN_HID1,r3
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_CPU_A003999
	mfspr	r3,977
	oris	r3,r3,0x0100
	mtspr	977,r3
#endif

	/* Enable branch prediction */
	lis	r3,BUCSR_ENABLE@h
	ori	r3,r3,BUCSR_ENABLE@l
	mtspr	SPRN_BUCSR,r3

	/* Ensure TB is 0 */
	li	r3,0
	mttbl	r3
	mttbu	r3

	/* Enable/invalidate the I-Cache */
	lis	r2,(L1CSR1_ICFI|L1CSR1_ICLFR)@h
	ori	r2,r2,(L1CSR1_ICFI|L1CSR1_ICLFR)@l
	mtspr	SPRN_L1CSR1,r2
1:
	mfspr	r3,SPRN_L1CSR1
	and.	r1,r3,r2
	bne	1b

	lis	r3,(L1CSR1_CPE|L1CSR1_ICE)@h
	ori	r3,r3,(L1CSR1_CPE|L1CSR1_ICE)@l
	mtspr	SPRN_L1CSR1,r3
	isync
2:
	mfspr	r3,SPRN_L1CSR1
	andi.	r1,r3,L1CSR1_ICE@l
	beq	2b

	/* Enable/invalidate the D-Cache */
	lis	r2,(L1CSR0_DCFI|L1CSR0_DCLFR)@h
	ori	r2,r2,(L1CSR0_DCFI|L1CSR0_DCLFR)@l
	mtspr	SPRN_L1CSR0,r2
1:
	mfspr	r3,SPRN_L1CSR0
	and.	r1,r3,r2
	bne	1b

	lis	r3,(L1CSR0_CPE|L1CSR0_DCE)@h
	ori	r3,r3,(L1CSR0_CPE|L1CSR0_DCE)@l
	mtspr	SPRN_L1CSR0,r3
	isync
2:
	mfspr	r3,SPRN_L1CSR0
	andi.	r1,r3,L1CSR0_DCE@l
	beq	2b

#define toreset(x) (x - __secondary_start_page + 0xfffff000)

	/* get our PIR to figure out our table entry */
	lis	r3,toreset(__spin_table)@h
	ori	r3,r3,toreset(__spin_table)@l

	/* r10 has the base address for the entry */
	mfspr	r0,SPRN_PIR
#ifdef CONFIG_E500MC
	rlwinm	r4,r0,27,27,31
#else
	mr	r4,r0
#endif
	slwi	r8,r4,5
	add	r10,r3,r8

#if defined(CONFIG_E500MC) && defined(CONFIG_SYS_CACHE_STASHING)
	/* set stash id to (coreID) * 2 + 32 + L1 CT (0) */
	slwi	r8,r4,1
	addi	r8,r8,32
	mtspr	L1CSR2,r8
#endif

#if defined(CONFIG_SYS_P4080_ERRATUM_CPU22) || \
	defined(CONFIG_SYS_FSL_ERRATUM_NMG_CPU_A011)
	/*
	 * CPU22 applies to P4080 rev 1.0, 2.0, fixed in 3.0
	 * NMG_CPU_A011 applies to P4080 rev 1.0, 2.0, fixed in 3.0
	 * also appleis to P3041 rev 1.0, 1.1, P2041 rev 1.0, 1.1
	 */
	mfspr   r3,SPRN_SVR
	rlwinm	r6,r3,24,~0x800		/* clear E bit */

	lis	r5,SVR_P4080@h
	ori	r5,r5,SVR_P4080@l
	cmpw	r6,r5
	bne	1f

	rlwinm  r3,r3,0,0xf0
	li      r5,0x30
	cmpw    r3,r5
	bge     2f
1:
	mfspr	r3,L1CSR2
	oris	r3,r3,(L1CSR2_DCWS)@h
	mtspr	L1CSR2,r3
2:
#endif

#ifdef CONFIG_BACKSIDE_L2_CACHE
	/* skip L2 setup on P2040/P2040E as they have no L2 */
	mfspr	r3,SPRN_SVR
	rlwinm	r6,r3,24,~0x800		/* clear E bit of SVR */

	lis	r3,SVR_P2040@h
	ori	r3,r3,SVR_P2040@l
	cmpw	r6,r3
	beq 3f

	/* Enable/invalidate the L2 cache */
	msync
	lis	r2,(L2CSR0_L2FI|L2CSR0_L2LFC)@h
	ori	r2,r2,(L2CSR0_L2FI|L2CSR0_L2LFC)@l
	mtspr	SPRN_L2CSR0,r2
1:
	mfspr	r3,SPRN_L2CSR0
	and.	r1,r3,r2
	bne	1b

#ifdef CONFIG_SYS_CACHE_STASHING
	/* set stash id to (coreID) * 2 + 32 + L2 (1) */
	addi	r3,r8,1
	mtspr	SPRN_L2CSR1,r3
#endif

	lis	r3,CONFIG_SYS_INIT_L2CSR0@h
	ori	r3,r3,CONFIG_SYS_INIT_L2CSR0@l
	mtspr	SPRN_L2CSR0,r3
	isync
2:
	mfspr	r3,SPRN_L2CSR0
	andis.	r1,r3,L2CSR0_L2E@h
	beq	2b
#endif
3:

#define EPAPR_MAGIC		(0x45504150)
#define ENTRY_ADDR_UPPER	0
#define ENTRY_ADDR_LOWER	4
#define ENTRY_R3_UPPER		8
#define ENTRY_R3_LOWER		12
#define ENTRY_RESV		16
#define ENTRY_PIR		20
#define ENTRY_R6_UPPER		24
#define ENTRY_R6_LOWER		28
#define ENTRY_SIZE		32

	/* setup the entry */
	li	r3,0
	li	r8,1
	stw	r0,ENTRY_PIR(r10)
	stw	r3,ENTRY_ADDR_UPPER(r10)
	stw	r8,ENTRY_ADDR_LOWER(r10)
	stw	r3,ENTRY_R3_UPPER(r10)
	stw	r4,ENTRY_R3_LOWER(r10)
	stw	r3,ENTRY_R6_UPPER(r10)
	stw	r3,ENTRY_R6_LOWER(r10)

	/* load r13 with the address of the 'bootpg' in SDRAM */
	lis	r13,toreset(__bootpg_addr)@h
	ori	r13,r13,toreset(__bootpg_addr)@l
	lwz	r13,0(r13)

	/* setup mapping for AS = 1, and jump there */
	lis	r11,(MAS0_TLBSEL(1)|MAS0_ESEL(1))@h
	mtspr	SPRN_MAS0,r11
	lis	r11,(MAS1_VALID|MAS1_IPROT)@h
	ori	r11,r11,(MAS1_TS|MAS1_TSIZE(BOOKE_PAGESZ_4K))@l
	mtspr	SPRN_MAS1,r11
	oris	r11,r13,(MAS2_I|MAS2_G)@h
	ori	r11,r13,(MAS2_I|MAS2_G)@l
	mtspr	SPRN_MAS2,r11
	oris	r11,r13,(MAS3_SX|MAS3_SW|MAS3_SR)@h
	ori	r11,r13,(MAS3_SX|MAS3_SW|MAS3_SR)@l
	mtspr	SPRN_MAS3,r11
	tlbwe

	bl	1f
1:	mflr	r11
	/*
	 * OR in 0xfff to create a mask of the bootpg SDRAM address.  We use
	 * this mask to fixup the cpu spin table and the address that we want
	 * to jump to, eg change them from 0xfffffxxx to 0x7ffffxxx if the
	 * bootpg is at 0x7ffff000 in SDRAM.
	 */
	ori	r13,r13,0xfff
	and	r11, r11, r13
	and	r10, r10, r13

	addi	r11,r11,(2f-1b)
	mfmsr	r13
	ori	r12,r13,MSR_IS|MSR_DS@l

	mtspr	SPRN_SRR0,r11
	mtspr	SPRN_SRR1,r12
	rfi

	/* spin waiting for addr */
2:
	lwz	r4,ENTRY_ADDR_LOWER(r10)
	andi.	r11,r4,1
	bne	2b
	isync

	/* setup IVORs to match fixed offsets */
#include "fixed_ivor.S"

	/* get the upper bits of the addr */
	lwz	r11,ENTRY_ADDR_UPPER(r10)

	/* setup branch addr */
	mtspr	SPRN_SRR0,r4

	/* mark the entry as released */
	li	r8,3
	stw	r8,ENTRY_ADDR_LOWER(r10)

	/* mask by ~64M to setup our tlb we will jump to */
	rlwinm	r12,r4,0,0,5

	/* setup r3, r4, r5, r6, r7, r8, r9 */
	lwz	r3,ENTRY_R3_LOWER(r10)
	li	r4,0
	li	r5,0
	lwz	r6,ENTRY_R6_LOWER(r10)
	lis	r7,(64*1024*1024)@h
	li	r8,0
	li	r9,0

	/* load up the pir */
	lwz	r0,ENTRY_PIR(r10)
	mtspr	SPRN_PIR,r0
	mfspr	r0,SPRN_PIR
	stw	r0,ENTRY_PIR(r10)

	mtspr	IVPR,r12
/*
 * Coming here, we know the cpu has one TLB mapping in TLB1[0]
 * which maps 0xfffff000-0xffffffff one-to-one.  We set up a
 * second mapping that maps addr 1:1 for 64M, and then we jump to
 * addr
 */
	lis	r10,(MAS0_TLBSEL(1)|MAS0_ESEL(0))@h
	mtspr	SPRN_MAS0,r10
	lis	r10,(MAS1_VALID|MAS1_IPROT)@h
	ori	r10,r10,(MAS1_TSIZE(BOOKE_PAGESZ_64M))@l
	mtspr	SPRN_MAS1,r10
	/* WIMGE = 0b00000 for now */
	mtspr	SPRN_MAS2,r12
	ori	r12,r12,(MAS3_SX|MAS3_SW|MAS3_SR)
	mtspr	SPRN_MAS3,r12
#ifdef CONFIG_ENABLE_36BIT_PHYS
	mtspr	SPRN_MAS7,r11
#endif
	tlbwe

/* Now we have another mapping for this page, so we jump to that
 * mapping
 */
	mtspr	SPRN_SRR1,r13
	rfi

	/*
	 * Allocate some space for the SDRAM address of the bootpg.
	 * This variable has to be in the boot page so that it can
	 * be accessed by secondary cores when they come out of reset.
	 */
	.globl __bootpg_addr
__bootpg_addr:
	.long	0

	.align L1_CACHE_SHIFT
	.globl __spin_table
__spin_table:
	.space CONFIG_MAX_CPUS*ENTRY_SIZE

	/* Fill in the empty space.  The actual reset vector is
	 * the last word of the page */
__secondary_start_code_end:
	.space 4092 - (__secondary_start_code_end - __secondary_start_page)
__secondary_reset_vector:
	b	__secondary_start_page
