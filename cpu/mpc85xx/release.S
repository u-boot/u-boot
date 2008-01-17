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
	ori	r3,r3,HID0_TBEN@l	/* enable Timebase */
#ifdef CONFIG_PHYS_64BIT
	ori	r3,r3,HID0_ENMAS7@l	/* enable MAS7 updates */
#endif
	mtspr	SPRN_HID0,r3

	li	r3,(HID1_ASTME|HID1_ABE)@l	/* Addr streaming & broadcast */
	mtspr	SPRN_HID1,r3

	/* Enable branch prediction */
	li	r3,0x201
	mtspr	SPRN_BUCSR,r3

	/* Enable/invalidate the I-Cache */
	mfspr	r0,SPRN_L1CSR1
	ori	r0,r0,(L1CSR1_ICFI|L1CSR1_ICE)
	mtspr	SPRN_L1CSR1,r0
	isync

	/* Enable/invalidate the D-Cache */
	mfspr	r0,SPRN_L1CSR0
	ori	r0,r0,(L1CSR0_DCFI|L1CSR0_DCE)
	msync
	isync
	mtspr	SPRN_L1CSR0,r0
	isync

#define toreset(x) (x - __secondary_start_page + 0xfffff000)

	/* get our PIR to figure out our table entry */
	lis	r3,toreset(__spin_table)@h
	ori	r3,r3,toreset(__spin_table)@l

	/* r9 has the base address for the entry */
	mfspr	r0,SPRN_PIR
	mr	r4,r0
	slwi	r8,r4,4
	slwi	r9,r4,3
	add	r8,r8,r9
	add	r9,r3,r8

#define EPAPR_MAGIC	(0x65504150)
#define ENTRY_ADDR	0
#define ENTRY_PIR	4
#define ENTRY_R3	8
#define ENTRY_R4	12
#define ENTRY_R6	16
#define ENTRY_R7	20

	/* setup the entry */
	li	r4,0
	li	r8,1
	lis	r6,EPAPR_MAGIC@h
	ori	r6,r6,EPAPR_MAGIC@l
	stw	r0,ENTRY_PIR(r9)
	stw	r8,ENTRY_ADDR(r9)
	stw	r4,ENTRY_R3(r9)
	stw	r4,ENTRY_R4(r9)
	stw	r6,ENTRY_R6(r9)
	stw	r4,ENTRY_R7(r9)

	/* spin waiting for addr */
1:	lwz	r4,ENTRY_ADDR(r9)
	andi.	r11,r4,1
	bne	1b

	/* setup branch addr */
	mtctr	r4

	/* mark the entry as released */
	li	r8,3
	stw	r8,ENTRY_ADDR(r9)

	/* mask by ~64M to setup our tlb we will jump to */
	rlwinm	r8,r4,0,0,5

	/* setup r3, r5, r6, r7 */
	lwz	r3,ENTRY_R3(r9)
	lwz	r4,ENTRY_R4(r9)
	li	r5,0
	lwz	r6,ENTRY_R6(r9)
	lwz	r7,ENTRY_R7(r9)

	/* load up the pir */
	lwz	r0,ENTRY_PIR(r9)
	mtspr	SPRN_PIR,r0
	mfspr	r0,SPRN_PIR
	stw	r0,ENTRY_PIR(r9)

/*
 * Coming here, we know the cpu has one TLB mapping in TLB1[0]
 * which maps 0xfffff000-0xffffffff one-to-one.  We set up a
 * second mapping that maps addr 1:1 for 64M, and then we jump to
 * addr
 */
	lis	r9,(MAS0_TLBSEL(1)|MAS0_ESEL(1))@h
	mtspr	SPRN_MAS0,r9
	lis	r9,(MAS1_VALID|MAS1_IPROT)@h
	ori	r9,r9,(MAS1_TSIZE(BOOKE_PAGESZ_64M))@l
	mtspr	SPRN_MAS1,r9
	/* WIMGE = 0b00000 for now */
	mtspr	SPRN_MAS2,r8
	ori	r8,r8,(MAS3_SX|MAS3_SW|MAS3_SR)
	mtspr	SPRN_MAS3,r8
	tlbwe

/* Now we have another mapping for this page, so we jump to that
 * mapping
 */
	bctr

	.align 3
	.globl __spin_table
__spin_table:
	.space CONFIG_NR_CPUS*24

	/* Fill in the empty space.  The actual reset vector is
	 * the last word of the page */
__secondary_start_code_end:
	.space 4092 - (__secondary_start_code_end - __secondary_start_page)
__secondary_reset_vector:
	b	__secondary_start_page
