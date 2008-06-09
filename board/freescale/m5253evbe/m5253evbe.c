/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * Hayden Fraser (Hayden.Fraser@freescale.com)
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

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale MCF5253 EVBE\n");
	return 0;
};

phys_size_t initdram(int board_type)
{
	int i;

	/*
	 * Check to see if the SDRAM has already been initialized
	 * by a run control tool
	 */
	if (!(mbar_readLong(MCFSIM_DCR) & 0x8000)) {
		u32 RC, dramsize;

		RC = (CFG_CLK / 1000000) >> 1;
		RC = (RC * 15) >> 4;

		/* Initialize DRAM Control Register: DCR */
		mbar_writeShort(MCFSIM_DCR, (0x8400 | RC));

		mbar_writeLong(MCFSIM_DACR0, 0x00003224);

		/* Initialize DMR0 */
		dramsize = ((CFG_SDRAM_SIZE << 20) - 1) & 0xFFFC0000;
		mbar_writeLong(MCFSIM_DMR0, dramsize | 1);

		mbar_writeLong(MCFSIM_DACR0, 0x0000322c);

		/* Write to this block to initiate precharge */
		*(u32 *) (CFG_SDRAM_BASE) = 0xa5a5a5a5;

		/* Set RE bit in DACR */
		mbar_writeLong(MCFSIM_DACR0,
			       mbar_readLong(MCFSIM_DACR0) | 0x8000);

		/* Wait for at least 8 auto refresh cycles to occur */
		udelay(500);

		/* Finish the configuration by issuing the MRS */
		mbar_writeLong(MCFSIM_DACR0,
			       mbar_readLong(MCFSIM_DACR0) | 0x0040);

		*(u32 *) (CFG_SDRAM_BASE + 0x800) = 0xa5a5a5a5;
	}

	return CFG_SDRAM_SIZE * 1024 * 1024;
}

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

#ifdef CONFIG_CMD_IDE
#include <ata.h>
int ide_preinit(void)
{
	return (0);
}

void ide_set_reset(int idereset)
{
	volatile atac_t *ata = (atac_t *) CFG_ATA_BASE_ADDR;
	long period;
	/*  t1,  t2,  t3,  t4,  t5,  t6,  t9, tRD,  tA */
	int piotms[5][9] = { {70, 165, 60, 30, 50, 5, 20, 0, 35},	/* PIO 0 */
	{50, 125, 45, 20, 35, 5, 15, 0, 35},	/* PIO 1 */
	{30, 100, 30, 15, 20, 5, 10, 0, 35},	/* PIO 2 */
	{30, 80, 30, 10, 20, 5, 10, 0, 35},	/* PIO 3 */
	{25, 70, 20, 10, 20, 5, 10, 0, 35}	/* PIO 4 */
	};

	if (idereset) {
		ata->cr = 0;	/* control reset */
		udelay(100);
	} else {
		mbar2_writeLong(CIM_MISCCR, CIM_MISCCR_CPUEND);

#define CALC_TIMING(t) (t + period - 1) / period
		period = 1000000000 / (CFG_CLK / 2);	/* period in ns */

		/*ata->ton = CALC_TIMING (180); */
		ata->t1 = CALC_TIMING(piotms[2][0]);
		ata->t2w = CALC_TIMING(piotms[2][1]);
		ata->t2r = CALC_TIMING(piotms[2][1]);
		ata->ta = CALC_TIMING(piotms[2][8]);
		ata->trd = CALC_TIMING(piotms[2][7]);
		ata->t4 = CALC_TIMING(piotms[2][3]);
		ata->t9 = CALC_TIMING(piotms[2][6]);

		ata->cr = 0x40;	/* IORDY enable */
		udelay(2000);
		ata->cr |= 0x01;	/* IORDY enable */
	}
}
#endif				/* CONFIG_CMD_IDE */
