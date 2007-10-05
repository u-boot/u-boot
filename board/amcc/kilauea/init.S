/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Based on code provided from UDTech and AMCC
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

#define mtsdram_as(reg, value)	 	\
	addi    r4,0,reg	;     	\
	mtdcr   memcfga,r4	;	\
	addis   r4,0,value@h 	;	\
	ori     r4,r4,value@l	;	\
	mtdcr   memcfgd,r4	;

	.globl  ext_bus_cntlr_init
ext_bus_cntlr_init:

	/*
	 * DDR2 setup
	 */

	/* Following the DDR Core Manual, here is the initialization */

	/* Step 1 */

	/* Step 2 */

	/* Step 3 */

	/* SET SDRAM_MB0CF  base addr 00000000 - 128MB */
	mtsdram_as(SDRAM_MB0CF, 0x00007701);		/* 8 -- 7*/

	/* SET SDRAM_MB1CF - Not enabled */
	mtsdram_as(SDRAM_MB1CF, 0x00000000);

	/* SET SDRAM_MB2CF  - Not enabled */
	mtsdram_as(SDRAM_MB2CF, 0x00000000);

	/* SET SDRAM_MB3CF  - Not enabled */
	mtsdram_as(SDRAM_MB3CF, 0x00000000);

	/* SDRAM_CLKTR: Adv Addr clock by 90 deg */
	mtsdram_as(SDRAM_CLKTR,0x80000000);

	/* Refresh Time register (0x30) Refresh every 7.8125uS */
	mtsdram_as(SDRAM_RTR, 0x06180000);

	/* SDRAM_SDTR1 */
	mtsdram_as(SDRAM_SDTR1,0x80201000);

	/* SDRAM_SDTR2	*/
	mtsdram_as(SDRAM_SDTR2,0x32204232);

	/* SDRAM_SDTR3	*/
	mtsdram_as(SDRAM_SDTR3,0x080b0d1a);

	mtsdram_as(SDRAM_MMODE,    0x00000442);
	mtsdram_as(SDRAM_MEMODE,   0x00000404);

	/* SDRAM0_MCOPT1 (0X20) No ECC Gen */
	mtsdram_as(SDRAM_MCOPT1,   0x04322000);

	/* NOP */
	mtsdram_as(SDRAM_INITPLR0,  0xa8380000);
	/* precharge 3 DDR clock cycle */
	mtsdram_as(SDRAM_INITPLR1,  0x81900400);
	/* EMR2 twr = 2tck */
	mtsdram_as(SDRAM_INITPLR2,  0x81020000);
	/* EMR3  twr = 2tck */
	mtsdram_as(SDRAM_INITPLR3,  0x81030000);
	/* EMR DLL ENABLE twr = 2tck */
	mtsdram_as(SDRAM_INITPLR4,  0x81010404);
	/* MR w/ DLL reset
	 * Note: 5 is CL.  May need to be changed
	 */
	mtsdram_as(SDRAM_INITPLR5,  0x81000542);
	/* precharge 3 DDR clock cycle */
	mtsdram_as(SDRAM_INITPLR6,  0x81900400);
	/* Auto-refresh trfc = 26tck */
	mtsdram_as(SDRAM_INITPLR7,  0x8D080000);
	/* Auto-refresh trfc = 26tck */
	mtsdram_as(SDRAM_INITPLR8,  0x8D080000);
	/* Auto-refresh */
	mtsdram_as(SDRAM_INITPLR9,  0x8D080000);
	/* Auto-refresh */
	mtsdram_as(SDRAM_INITPLR10, 0x8D080000);
	/* MRS - normal operation; wait 2 cycle (set wait to tMRD) */
	mtsdram_as(SDRAM_INITPLR11, 0x81000442);
	mtsdram_as(SDRAM_INITPLR12, 0x81010780);
	mtsdram_as(SDRAM_INITPLR13, 0x81010400);
	mtsdram_as(SDRAM_INITPLR14, 0x00000000);
	mtsdram_as(SDRAM_INITPLR15, 0x00000000);

	/* SET MCIF0_CODT   Die Termination On */
	mtsdram_as(SDRAM_CODT,      0x0080f837);
	mtsdram_as(SDRAM_MODT0,     0x01800000);
	mtsdram_as(SDRAM_MODT1,     0x00000000);

	mtsdram_as(SDRAM_WRDTR, 0x00000000);

	/* SDRAM0_MCOPT2 (0X21) Start initialization */
	mtsdram_as(SDRAM_MCOPT2, 0x20000000);

	/* Step 5 */
	lis     r3,0x1	/* 400000 =  wait 100ms */
	mtctr   r3

pll_wait:
	bdnz	pll_wait

	/* Step 6 */

	/* SDRAM_DLCR */
	mtsdram_as(SDRAM_DLCR,0x030000a5);

	/* SDRAM_RDCC */
	mtsdram_as(SDRAM_RDCC,0x40000000);

	/* SDRAM_RQDC */
	mtsdram_as(SDRAM_RQDC,0x80000038);

	/* SDRAM_RFDC */
	mtsdram_as(SDRAM_RFDC,0x00000209);

	/* Enable memory controller */
	mtsdram_as(SDRAM_MCOPT2, 0x28000000);

	blr
