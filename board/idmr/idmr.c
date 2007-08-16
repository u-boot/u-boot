/*
 * (C) Copyright 2000-2006
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
#include <asm/immap.h>

int checkboard (void) {
	puts ("Board: iDMR\n");
	return 0;
};

long int initdram (int board_type) {
	int i;

	/*
	 * After reset, CS0 is configured to cover entire address space. We
	 * need to configure it to its proper values, so that writes to
	 * CFG_SDRAM_BASE and vicinity during SDRAM controller setup below do
	 * now fall under CS0 (see 16.3.1 of the MCF5271 Reference Manual).
	 */

	/* Flash chipselect, CS0 */
	/* ;CSAR0: Flash at 0xFF800000 */
	mbar_writeShort(0x0080, 0xFF80);

	/* CSCR0: Flash 6 waits, 16bit */
	mbar_writeShort(0x008A, 0x1980);

	/* CSMR0: Flash 8MB, R/W, valid */
	mbar_writeLong(0x0084, 0x007F0001);


	/*
	 * SDRAM configuration proper
	 */

	/*
	 * Address/Data Pin Assignment Reg.: enable address lines 23-21; do
	 * not enable data pins D[15:0], as we have 16 bit port to SDRAM
	 */
	mbar_writeByte(MCF_GPIO_PAR_AD,
			MCF_GPIO_AD_ADDR23 |
			MCF_GPIO_AD_ADDR22 |
			MCF_GPIO_AD_ADDR21);

	/* No need to configure BS pins - reset values are OK */

	/* Chip Select Pin Assignment Reg.: set CS[1-7] to GPIO */
	mbar_writeByte(MCF_GPIO_PAR_CS, 0x00);

	/* SDRAM Control Pin Assignment Reg. */
	mbar_writeByte(MCF_GPIO_PAR_SDRAM,
			MCF_GPIO_SDRAM_CSSDCS_00 | /* no matter: PAR_CS=0 */
			MCF_GPIO_SDRAM_SDWE |
			MCF_GPIO_SDRAM_SCAS |
			MCF_GPIO_SDRAM_SRAS |
			MCF_GPIO_SDRAM_SCKE |
			MCF_GPIO_SDRAM_SDCS_01);

	/*
	 * Wait 100us.  We run the bus at 50Mhz, one cycle is 20ns. So 5
	 * iterations will do, but we do 10 just to be safe.
	 */
	for (i = 0; i < 10; ++i)
		asm(" nop");


	/* 1. Initialize DRAM Control Register: DCR */
	mbar_writeShort(MCF_SDRAMC_DCR,
			MCF_SDRAMC_DCR_RTIM(0x10) |	/* 65ns */
			MCF_SDRAMC_DCR_RC(0x60));	/* 1562 cycles */


	/*
	 * 2. Initialize DACR0
	 *
	 * CL: 11 (CL=3: 0x03, 0x02; CL=2: 0x1)
	 * CBM: cmd at A20, bank select bits 21 and up
	 * PS: 16 bit
	 */
	mbar_writeLong(MCF_SDRAMC_DACR0,
			MCF_SDRAMC_DACRn_BA(CFG_SDRAM_BASE>>18) |
			MCF_SDRAMC_DACRn_BA(0x00) |
			MCF_SDRAMC_DACRn_CASL(0x03) |
			MCF_SDRAMC_DACRn_CBM(0x03) |
			MCF_SDRAMC_DACRn_PS(0x03));

	/* Initialize DMR0 */
	mbar_writeLong(MCF_SDRAMC_DMR0,
			MCF_SDRAMC_DMRn_BAM_16M |
			MCF_SDRAMC_DMRn_V);


	/* 3. Set IP bit in DACR to initiate PALL command */
	mbar_writeLong(MCF_SDRAMC_DACR0,
			mbar_readLong(MCF_SDRAMC_DACR0) |
			MCF_SDRAMC_DACRn_IP);

	/* Write to this block to initiate precharge */
	*(volatile u16 *)(CFG_SDRAM_BASE) = 0xa5a5;

	/*
	 * Wait at least 20ns to allow banks to precharge (t_RP = 20ns). We
	 * wait a wee longer, just to be safe.
	 */
	for (i = 0; i < 5; ++i)
		asm(" nop");


	/* 4. Set RE bit in DACR */
	mbar_writeLong(MCF_SDRAMC_DACR0,
			mbar_readLong(MCF_SDRAMC_DACR0) |
			MCF_SDRAMC_DACRn_RE);

	/*
	 * Wait for at least 8 auto refresh cycles to occur, i.e. at least
	 * 781 bus cycles.
	 */
	for (i = 0; i < 1000; ++i)
		asm(" nop");

	/* Finish the configuration by issuing the MRS */
	mbar_writeLong(MCF_SDRAMC_DACR0,
			mbar_readLong(MCF_SDRAMC_DACR0) |
			MCF_SDRAMC_DACRn_MRS);

	/*
	 * Write to the SDRAM Mode Register A0-A11 = 0x400
	 *
	 * Write Burst Mode = Programmed Burst Length
	 * Op Mode = Standard Op
	 * CAS Latency = 3
	 * Burst Type = Sequential
	 * Burst Length = 1
	 */
	*(volatile u32 *)(CFG_SDRAM_BASE + 0x1800) = 0xa5a5a5a5;

	return CFG_SDRAM_SIZE * 1024 * 1024;
};


int testdram (void) {

	/* TODO: XXX XXX XXX */
	printf ("DRAM test not implemented!\n");

	return (0);
}
