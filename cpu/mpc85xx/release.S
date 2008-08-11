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

	/* r10 has the base address for the entry */
	mfspr	r0,SPRN_PIR
	mr	r4,r0
	slwi	r8,r4,5
	add	r10,r3,r8

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

	/* setup mapping for AS = 1, and jump there */
	lis	r11,(MAS0_TLBSEL(1)|MAS0_ESEL(1))@h
	mtspr	SPRN_MAS0,r11
	lis	r11,(MAS1_VALID|MAS1_IPROT)@h
	ori	r11,r11,(MAS1_TS|MAS1_TSIZE(BOOKE_PAGESZ_4K))@l
	mtspr	SPRN_MAS1,r11
	lis	r11,(0xfffff000|MAS2_I)@h
	ori	r11,r11,(0xfffff000|MAS2_I)@l
	mtspr	SPRN_MAS2,r11
	lis	r11,(0xfffff000|MAS3_SX|MAS3_SW|MAS3_SR)@h
	ori	r11,r11,(0xfffff000|MAS3_SX|MAS3_SW|MAS3_SR)@l
	mtspr	SPRN_MAS3,r11
	tlbwe

	bl	1f
1:	mflr	r11
	addi	r11,r11,28
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

	.align L1_CACHE_SHIFT
	.globl __spin_table
__spin_table:
	.space CONFIG_NUM_CPUS*ENTRY_SIZE

	/* Fill in the empty space.  The actual reset vector is
	 * the last word of the page */
__secondary_start_code_end:
	.space 4092 - (__secondary_start_code_end - __secondary_start_page)
__secondary_reset_vector:
	b	__secondary_start_page
