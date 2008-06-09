/*
 * (C) Copyright 2000-2003
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

DECLARE_GLOBAL_DATA_PTR;

int checkboard (void)
{
	puts ("Board: Freescale M5282EVB Evaluation Board\n");
	return 0;
}

phys_size_t initdram (int board_type)
{
	u32 dramsize, i, dramclk;

	dramsize = CFG_SDRAM_SIZE * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	if (!(MCFSDRAMC_DACR0 & MCFSDRAMC_DACR_RE))
	{
		dramclk = gd->bus_clk / (CFG_HZ * CFG_HZ);

		/* Initialize DRAM Control Register: DCR */
		MCFSDRAMC_DCR = (0
			| MCFSDRAMC_DCR_RTIM_6
			| MCFSDRAMC_DCR_RC((15 * dramclk)>>4));

		/* Initialize DACR0 */
		MCFSDRAMC_DACR0 = (0
			| MCFSDRAMC_DACR_BASE(CFG_SDRAM_BASE)
			| MCFSDRAMC_DACR_CASL(1)
			| MCFSDRAMC_DACR_CBM(3)
			| MCFSDRAMC_DACR_PS_32);

		/* Initialize DMR0 */
		MCFSDRAMC_DMR0 = (0
			| ((dramsize - 1) & 0xFFFC0000)
			| MCFSDRAMC_DMR_V);

		/* Set IP (bit 3) in DACR */
		MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IP;

		/* Wait 30ns to allow banks to precharge */
		for (i = 0; i < 5; i++) {
			asm ("nop");
		}

		/* Write to this block to initiate precharge */
		*(u32 *)(CFG_SDRAM_BASE) = 0xA5A59696;

		/* Set RE (bit 15) in DACR */
		MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_RE;

		/* Wait for at least 8 auto refresh cycles to occur */
		for (i = 0; i < 2000; i++) {
			asm(" nop");
		}

		/* Finish the configuration by issuing the IMRS. */
		MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IMRS;

		/* Write to the SDRAM Mode Register */
		*(u32 *)(CFG_SDRAM_BASE + 0x400) = 0xA5A59696;
	}
	return dramsize;
}
