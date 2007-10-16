/*
 * Copyright 2007 Freescale Semiconductor.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include <config.h>
#include <mpc86xx.h>

#define LAWAR_TRGT_PCI1		0x00000000
#define LAWAR_TRGT_PCIE1	0x00200000
#define LAWAR_TRGT_PCIE2	0x00100000
#define LAWAR_TRGT_LBC		0x00400000
#define LAWAR_TRGT_DDR		0x00f00000

#if !defined(CONFIG_SPD_EEPROM)
#define LAWBAR1 ((CFG_DDR_SDRAM_BASE>>12) & 0xffffff)
#define LAWAR1	(LAWAR_EN | LAWAR_TRGT_DDR | (LAWAR_SIZE & LAWAR_SIZE_512M))
#else
#define LAWBAR1 0
#define LAWAR1	((LAWAR_TRGT_DDR | (LAWAR_SIZE & LAWAR_SIZE_512M)) & ~LAWAR_EN)
#endif

#define LAWBAR2 ((CFG_PCIE1_MEM_BASE>>12) & 0xffffff)
#define LAWAR2	(LAWAR_EN | LAWAR_TRGT_PCIE1 | (LAWAR_SIZE & LAWAR_SIZE_256M))

#define LAWBAR3 ((CFG_PCIE2_MEM_BASE>>12) & 0xffffff)
#define LAWAR3	(LAWAR_EN | LAWAR_TRGT_PCIE2 | (LAWAR_SIZE & LAWAR_SIZE_256M))

#define LAWBAR4 ((PIXIS_BASE>>12) & 0xffffff)
#define LAWAR4	(LAWAR_EN | LAWAR_TRGT_LBC | (LAWAR_SIZE & LAWAR_SIZE_2M))

#define LAWBAR5 ((CFG_PCIE1_IO_PHYS>>12) & 0xffffff)
#define LAWAR5	(LAWAR_EN | LAWAR_TRGT_PCIE1 | (LAWAR_SIZE & LAWAR_SIZE_1M))

#define LAWBAR6 ((CFG_PCIE2_IO_PHYS>>12) & 0xffffff)
#define LAWAR6	(LAWAR_EN | LAWAR_TRGT_PCIE2 | (LAWAR_SIZE & LAWAR_SIZE_1M))

#define LAWBAR7 ((CFG_FLASH_BASE >>12) & 0xffffff)
#define LAWAR7	(LAWAR_EN | LAWAR_TRGT_LBC | (LAWAR_SIZE & LAWAR_SIZE_256M))

#define LAWBAR8 ((CFG_PCI1_MEM_PHYS>>12) & 0xffffff)
#define LAWAR8	(LAWAR_EN | LAWAR_TRGT_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_256M))

#define LAWBAR9 ((CFG_PCI1_IO_PHYS>>12) & 0xffffff)
#define LAWAR9	(LAWAR_EN | LAWAR_TRGT_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_1M))


	.section .bootpg, "ax"
	.globl	law_entry
law_entry:
	lis	r7,CFG_CCSRBAR@h
	ori	r7,r7,CFG_CCSRBAR@l

	addi	r4,r7,0
	addi	r5,r7,0

	/* Skip LAWAR0, start at LAWAR1 */
	lis	r6,LAWBAR1@h
	ori	r6,r6,LAWBAR1@l
	stwu	r6, 0xc28(r4)

	lis	r6,LAWAR1@h
	ori	r6,r6,LAWAR1@l
	stwu	r6, 0xc30(r5)

	/* LAWBAR2, LAWAR2 */
	lis	r6,LAWBAR2@h
	ori	r6,r6,LAWBAR2@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR2@h
	ori	r6,r6,LAWAR2@l
	stwu	r6, 0x20(r5)

	/* LAWBAR3, LAWAR3 */
	lis	r6,LAWBAR3@h
	ori	r6,r6,LAWBAR3@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR3@h
	ori	r6,r6,LAWAR3@l
	stwu	r6, 0x20(r5)

	/* LAWBAR4, LAWAR4 */
	lis	r6,LAWBAR4@h
	ori	r6,r6,LAWBAR4@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR4@h
	ori	r6,r6,LAWAR4@l
	stwu	r6, 0x20(r5)
	/* LAWBAR5, LAWAR5 */
	lis	r6,LAWBAR5@h
	ori	r6,r6,LAWBAR5@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR5@h
	ori	r6,r6,LAWAR5@l
	stwu	r6, 0x20(r5)

	/* LAWBAR6, LAWAR6 */
	lis	r6,LAWBAR6@h
	ori	r6,r6,LAWBAR6@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR6@h
	ori	r6,r6,LAWAR6@l
	stwu	r6, 0x20(r5)

	/* LAWBAR7, LAWAR7 */
	lis	r6,LAWBAR7@h
	ori	r6,r6,LAWBAR7@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR7@h
	ori	r6,r6,LAWAR7@l
	stwu	r6, 0x20(r5)

	/* LAWBAR8, LAWAR8 */
	lis	r6,LAWBAR8@h
	ori	r6,r6,LAWBAR8@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR8@h
	ori	r6,r6,LAWAR8@l
	stwu	r6, 0x20(r5)

	/* LAWBAR9, LAWAR9 */
	lis	r6,LAWBAR9@h
	ori	r6,r6,LAWBAR9@l
	stwu	r6, 0x20(r4)

	lis	r6,LAWAR9@h
	ori	r6,r6,LAWAR9@l
	stwu	r6, 0x20(r5)

	blr
