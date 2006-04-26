/*
 * Copyright 2004 Freescale Semiconductor.
 * Jeff Brown (jeffrey@freescale.com)
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>
#include <config.h>
#include <mpc86xx.h>

/*
 * LAW(Local Access Window) configuration:
 *
 * 0x0000_0000     0x7fff_ffff     DDR                     2G
 * 0x8000_0000     0x9fff_ffff     PCI1 MEM                512M
 * 0xa000_0000     0xbfff_ffff     PCI2 MEM                512M
 * 0xc000_0000     0xdfff_ffff     RapidIO                 512M
 * 0xe000_0000     0xe000_ffff     CCSR                    1M
 * 0xe200_0000     0xe2ff_ffff     PCI1 IO                 16M
 * 0xe300_0000     0xe3ff_ffff     PCI2 IO                 16M
 * 0xf000_0000     0xf7ff_ffff     SDRAM                   128M
 * 0xf800_0000     0xf80f_ffff     BCSR                    1M
 * 0xfe00_0000     0xffff_ffff     FLASH (boot bank)       32M
 *
 * Notes:
 *    CCSRBAR don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 */

#if !defined(CONFIG_SPD_EEPROM)
#define LAWBAR1 ((CFG_DDR_SDRAM_BASE>>12) & 0xffffff)
#define LAWAR1	(LAWAR_EN | LAWAR_TRGT_IF_DDR1 | (LAWAR_SIZE & LAWAR_SIZE_256M))
#else
#define LAWBAR1 0
#define LAWAR1  ((LAWAR_TRGT_IF_DDR1 | (LAWAR_SIZE & LAWAR_SIZE_512M)) & ~LAWAR_EN)
#endif

#define LAWBAR2 ((CFG_PCI1_MEM_BASE>>12) & 0xffffff)
#define LAWAR2	(LAWAR_EN | LAWAR_TRGT_IF_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_512M))

#define LAWBAR3 ((CFG_PCI2_MEM_BASE>>12) & 0xffffff)
/*#define LAWAR3	(LAWAR_EN | LAWAR_TRGT_IF_PCI2 | (LAWAR_SIZE & LAWAR_SIZE_512M)) */
#define LAWAR3	(~LAWAR_EN & (LAWAR_TRGT_IF_PCI2 | (LAWAR_SIZE & LAWAR_SIZE_512M)))

/*
 * This is not so much the SDRAM map as it is the whole localbus map.
 */
#define LAWBAR4 ((0xf8100000>>12) & 0xffffff)
#define LAWAR4	(LAWAR_EN | LAWAR_TRGT_IF_LBC | (LAWAR_SIZE & LAWAR_SIZE_2M))

#define LAWBAR5 ((CFG_PCI1_IO_BASE>>12) & 0xffffff)
#define LAWAR5	(LAWAR_EN | LAWAR_TRGT_IF_PCI1 | (LAWAR_SIZE & LAWAR_SIZE_16M))

#define LAWBAR6 ((CFG_PCI2_IO_BASE>>12) & 0xffffff)
/*#define LAWAR6	(LAWAR_EN | LAWAR_TRGT_IF_PCI2 | (LAWAR_SIZE & LAWAR_SIZE_16M)) */
#define LAWAR6	(~LAWAR_EN &( LAWAR_TRGT_IF_PCI2 | (LAWAR_SIZE & LAWAR_SIZE_16M)))

 #define LAWBAR7 ((0xfe000000 >>12) & 0xffffff)
 #define LAWAR7	(LAWAR_EN | LAWAR_TRGT_IF_LBC | (LAWAR_SIZE & LAWAR_SIZE_32M))


/*
 * Rapid IO at 0xc000_0000 for 512 M
 */
/*
#ifdef CFG_INIT_RAM_LOCK
#define LAWBAR7 ((CFG_RIO_MEM_BASE>>12) & 0xffffff)
#define LAWAR7	(LAWAR_EN | LAWAR_TRGT_IF_RIO | (LAWAR_SIZE & LAWAR_SIZE_512M))
#endif
*/
/*
 * Stack at 0xfc00_0000 for 32M on LBC
 */
#if !defined(CFG_INIT_RAM_LOCK)
#define LAWBAR7 ((CFG_INIT_RAM_ADDR>>12) & 0xffffff)
#define LAWAR7 (LAWAR_EN | LAWAR_TRGT_IF_LBC | (LAWAR_SIZE & LAWAR_SIZE_32M))
#endif

      	.section .bootpg, "ax"
	.globl	law_entry
law_entry:
	lis	r7,CFG_CCSRBAR@h
	ori	r7,r7,CFG_CCSRBAR@l

	addi    r4,r7,0
	addi    r5,r7,0

	/* Skip LAWAR0, start at LAWAR1 */
	lis     r6,LAWBAR1@h
	ori     r6,r6,LAWBAR1@l
	stwu    r6, 0xc28(r4)

	lis     r6,LAWAR1@h
	ori     r6,r6,LAWAR1@l
	stwu    r6, 0xc30(r5)

	/* LAWBAR2, LAWAR2 */
	lis     r6,LAWBAR2@h
	ori     r6,r6,LAWBAR2@l
	stwu    r6, 0x20(r4)

	lis     r6,LAWAR2@h
        ori     r6,r6,LAWAR2@l
        stwu    r6, 0x20(r5)

	/* LAWBAR3, LAWAR3 */
	lis     r6,LAWBAR3@h
	ori     r6,r6,LAWBAR3@l
	stwu    r6, 0x20(r4)

	lis     r6,LAWAR3@h
	ori     r6,r6,LAWAR3@l
	stwu    r6, 0x20(r5)

	/* LAWBAR4, LAWAR4 */
	lis     r6,LAWBAR4@h
	ori     r6,r6,LAWBAR4@l
	stwu    r6, 0x20(r4)

        lis     r6,LAWAR4@h
	ori     r6,r6,LAWAR4@l
	stwu    r6, 0x20(r5)
	/* LAWBAR5, LAWAR5 */
	lis     r6,LAWBAR5@h
	ori     r6,r6,LAWBAR5@l
	stwu    r6, 0x20(r4)

	lis     r6,LAWAR5@h
	ori     r6,r6,LAWAR5@l
	stwu    r6, 0x20(r5)

	/* LAWBAR6, LAWAR6 */
	lis     r6,LAWBAR6@h
	ori     r6,r6,LAWBAR6@l
	stwu    r6, 0x20(r4)

	lis     r6,LAWAR6@h
	ori     r6,r6,LAWAR6@l
	stwu    r6, 0x20(r5)

	/* LAWBAR7, LAWAR7 */
	lis     r6,LAWBAR7@h
	ori     r6,r6,LAWBAR7@l
	stwu    r6, 0x20(r4)

	lis     r6,LAWAR7@h
	ori     r6,r6,LAWAR7@l
	stwu    r6, 0x20(r5)

	blr

