/*
 * Copyright (c) 2008 Nuovation System Designs, LLC
 *   Grant Erickson <gerickson@nuovations.com>
 *
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Originally based on code provided from UDTech and AMCC
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

#include <config.h>
#include <ppc4xx.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#define mtsdram_as(reg, value)		\
	addi	r4,0,reg	;	\
	mtdcr	memcfga,r4	;	\
	addis	r4,0,value@h	;	\
	ori	r4,r4,value@l	;	\
	mtdcr	memcfgd,r4	;

#if defined(CONFIG_DDR_ECC)
	.extern ecc_init
#endif /* defined(CONFIG_DDR_ECC) */

	.globl  ext_bus_cntlr_init
ext_bus_cntlr_init:
#if !defined(CFG_INIT_DCACHE_CS)
#if !defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)

	/*
	 * DDR2 SDRAM Controller Setup
	 */

	/* Set Memory Bank Configuration Registers */
	mtsdram_as(SDRAM_MB0CF, CFG_SDRAM0_MB0CF);
	mtsdram_as(SDRAM_MB1CF, CFG_SDRAM0_MB1CF);
	mtsdram_as(SDRAM_MB2CF, CFG_SDRAM0_MB2CF);
	mtsdram_as(SDRAM_MB3CF, CFG_SDRAM0_MB3CF);

	/* Set Memory Clock Timing Register */
	mtsdram_as(SDRAM_CLKTR, CFG_SDRAM0_CLKTR);

	/* Set Refresh Time Register */
	mtsdram_as(SDRAM_RTR, CFG_SDRAM0_RTR);

	/* Set SDRAM Timing Registers */
	mtsdram_as(SDRAM_SDTR1, CFG_SDRAM0_SDTR1);
	mtsdram_as(SDRAM_SDTR2, CFG_SDRAM0_SDTR2);
	mtsdram_as(SDRAM_SDTR3, CFG_SDRAM0_SDTR3);

	/* Set Mode and Extended Mode Registers */
	mtsdram_as(SDRAM_MMODE, CFG_SDRAM0_MMODE);
	mtsdram_as(SDRAM_MEMODE, CFG_SDRAM0_MEMODE);

	/* Set Memory Controller Options 1 Register */
	mtsdram_as(SDRAM_MCOPT1, CFG_SDRAM0_MCOPT1);

	/* Set Manual Initialization Control Registers */
	mtsdram_as(SDRAM_INITPLR0, CFG_SDRAM0_INITPLR0);
	mtsdram_as(SDRAM_INITPLR1, CFG_SDRAM0_INITPLR1);
	mtsdram_as(SDRAM_INITPLR2, CFG_SDRAM0_INITPLR2);
	mtsdram_as(SDRAM_INITPLR3, CFG_SDRAM0_INITPLR3);
	mtsdram_as(SDRAM_INITPLR4, CFG_SDRAM0_INITPLR4);
	mtsdram_as(SDRAM_INITPLR5, CFG_SDRAM0_INITPLR5);
	mtsdram_as(SDRAM_INITPLR6, CFG_SDRAM0_INITPLR6);
	mtsdram_as(SDRAM_INITPLR7, CFG_SDRAM0_INITPLR7);
	mtsdram_as(SDRAM_INITPLR8, CFG_SDRAM0_INITPLR8);
	mtsdram_as(SDRAM_INITPLR9, CFG_SDRAM0_INITPLR9);
	mtsdram_as(SDRAM_INITPLR10, CFG_SDRAM0_INITPLR10);
	mtsdram_as(SDRAM_INITPLR11, CFG_SDRAM0_INITPLR11);
	mtsdram_as(SDRAM_INITPLR12, CFG_SDRAM0_INITPLR12);
	mtsdram_as(SDRAM_INITPLR13, CFG_SDRAM0_INITPLR13);
	mtsdram_as(SDRAM_INITPLR14, CFG_SDRAM0_INITPLR14);
	mtsdram_as(SDRAM_INITPLR15, CFG_SDRAM0_INITPLR15);

	/* Set On-Die Termination Registers */
	mtsdram_as(SDRAM_CODT, CFG_SDRAM0_CODT);
	mtsdram_as(SDRAM_MODT0, CFG_SDRAM0_MODT0);
	mtsdram_as(SDRAM_MODT1, CFG_SDRAM0_MODT1);

	/* Set Write Timing Register */
	mtsdram_as(SDRAM_WRDTR, CFG_SDRAM0_WRDTR);

	/*
	 * Start Initialization by SDRAM0_MCOPT2[SREN] = 0 and
	 * SDRAM0_MCOPT2[IPTR] = 1
	 */
	mtsdram_as(SDRAM_MCOPT2, SDRAM_MCOPT2_SREN_EXIT | \
				 SDRAM_MCOPT2_IPTR_EXECUTE);

	/*
	 * Poll SDRAM0_MCSTAT[MIC] for assertion to indicate the
	 * completion of initialization.
	 *
	 *   do {
	 *           mfsdram(SDRAM_MCSTAT, val);
	 *   } while ((val & SDRAM_MCSTAT_MIC_MASK) != SDRAM_MCSTAT_MIC_COMP);
	 */
	li	r4,SDRAM_MCSTAT
	lis	r2,SDRAM_MCSTAT_MIC_COMP@h
	ori	r2,r2,SDRAM_MCSTAT_MIC_COMP@l
0:	mtdcr	memcfga,r4
	mfdcr	r3,memcfgd
	clrrwi	r3,r3,31
	cmpw	cr7,r3,r2
	bne+	cr7,0b

	/* Set Delay Control Registers */
	mtsdram_as(SDRAM_DLCR, CFG_SDRAM0_DLCR);
	mtsdram_as(SDRAM_RDCC, CFG_SDRAM0_RDCC);
	mtsdram_as(SDRAM_RQDC, CFG_SDRAM0_RQDC);
	mtsdram_as(SDRAM_RFDC, CFG_SDRAM0_RFDC);

	/*
	 * Enable Controller by SDRAM0_MCOPT2[DCEN] = 1:
	 *
	 *   mcopt2 = mfsdram(SDRAM_MCOPT2);
	 */
	li	r4,SDRAM_MCOPT2
	mtdcr	memcfga,r4
	mfdcr	r3,memcfgd

	/*
	 *   mtsdram(SDRAM_MCOPT2, mcopt2 | SDRAM_MCOPT2_DCEN_ENABLE);
	 */
	mtdcr	memcfga,r4
	oris	r3,r3,SDRAM_MCOPT2_DCEN_ENABLE@h
	ori	r3,r3,SDRAM_MCOPT2_DCEN_ENABLE@l
	mtdcr	memcfgd,r3

#if defined(CONFIG_DDR_ECC)
	/*
	 *   ecc_init(CFG_SDRAM_BASE, CFG_MBYTES_SDRAM << 20);
	 */
	mflr	r13
	lis	r3,CFG_SDRAM_BASE@h
	ori	r3,r3,CFG_SDRAM_BASE@l
	lis	r4,(CFG_MBYTES_SDRAM << 20)@h
	ori	r4,r4,(CFG_MBYTES_SDRAM << 20)@l
	bl	ecc_init
	mtlr	r13
#endif /* defined(CONFIG_DDR_ECC) */
#endif /* !defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL) */
#endif /* !defined(CFG_INIT_DCACHE_CS) */

	blr
