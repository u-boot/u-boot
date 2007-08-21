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
	puts ("Board: Freescale M5271EVB\n");
	return 0;
};

long int initdram (int board_type) {

	int i;

	/* Enable Address lines 23-21 and lower 16bits of data path */
	mbar_writeByte(MCF_GPIO_PAR_AD, MCF_GPIO_AD_ADDR23 |
			MCF_GPIO_AD_ADDR22 | MCF_GPIO_AD_ADDR21 |
			MCF_GPIO_AD_DATAL);

	/* Set CS2 pin to be SD_CS0 */
	mbar_writeByte(MCF_GPIO_PAR_CS, mbar_readByte(MCF_GPIO_PAR_CS)
			| MCF_GPIO_PAR_CS_PAR_CS2);

	/* Configure SDRAM Control Pin Assignemnt Register */
	mbar_writeByte(MCF_GPIO_PAR_SDRAM, MCF_GPIO_SDRAM_CSSDCS_00 |
			MCF_GPIO_SDRAM_SDWE | MCF_GPIO_SDRAM_SCAS |
			MCF_GPIO_SDRAM_SRAS | MCF_GPIO_SDRAM_SCKE |
			MCF_GPIO_SDRAM_SDCS_11);

	/*
	 * Check to see if the SDRAM has already been initialized
	 * by a run control tool
	 */
	if (!(mbar_readLong(MCF_SDRAMC_DACR0) & MCF_SDRAMC_DACRn_RE)) {
		/* Initialize DRAM Control Register: DCR */
		mbar_writeShort(MCF_SDRAMC_DCR,
				MCF_SDRAMC_DCR_RTIM(0x01)
				| MCF_SDRAMC_DCR_RC(0x30));

		/*
		 * Initialize DACR0
		 *
		 * CASL: 01
		 * CBM: cmd at A20, bank select bits 21 and up
		 * PS: 32bit port size
		 */
		mbar_writeLong(MCF_SDRAMC_DACR0,
				MCF_SDRAMC_DACRn_BA(CFG_SDRAM_BASE>>18)
				| MCF_SDRAMC_DACRn_CASL(1)
				| MCF_SDRAMC_DACRn_CBM(3)
				| MCF_SDRAMC_DACRn_PS(0));

		/* Initialize DMR0 */
		mbar_writeLong(MCF_SDRAMC_DMR0,
				MCF_SDRAMC_DMRn_BAM_16M
				| MCF_SDRAMC_DMRn_V);

		/* Set IP bit in DACR */
		mbar_writeLong(MCF_SDRAMC_DACR0, mbar_readLong(MCF_SDRAMC_DACR0)
				| MCF_SDRAMC_DACRn_IP);

		/* Wait at least 20ns to allow banks to precharge */
		for (i = 0; i < 5; i++)
			asm(" nop");

		/* Write to this block to initiate precharge */
		*(u32 *)(CFG_SDRAM_BASE) = 0xa5a5a5a5;

		/* Set RE bit in DACR */
		mbar_writeLong(MCF_SDRAMC_DACR0, mbar_readLong(MCF_SDRAMC_DACR0)
				| MCF_SDRAMC_DACRn_RE);

		/* Wait for at least 8 auto refresh cycles to occur */
		for (i = 0; i < 2000; i++)
			asm(" nop");

		/* Finish the configuration by issuing the MRS */
		mbar_writeLong(MCF_SDRAMC_DACR0, mbar_readLong(MCF_SDRAMC_DACR0)
				| MCF_SDRAMC_DACRn_MRS);

		/*
		 * Write to the SDRAM Mode Register A0-A11 = 0x400
		 *
		 * Write Burst Mode = Programmed Burst Length
		 * Op Mode = Standard Op
		 * CAS Latency = 2
		 * Burst Type = Sequential
		 * Burst Length = 1
		 */
		*(u32 *)(CFG_SDRAM_BASE + 0x400) = 0xa5a5a5a5;
	}

	return CFG_SDRAM_SIZE * 1024 * 1024;
};

int testdram (void) {

	/* TODO: XXX XXX XXX */
	printf ("DRAM test not implemented!\n");

	return (0);
}
