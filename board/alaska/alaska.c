/*
 * (C) Copyright 2004, Freescale Inc.
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com
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
#include <mpc8220.h>
#include <asm/processor.h>
#include <asm/mmu.h>

void setupBat (ulong size)
{
	ulong batu, batl;
	int blocksize = 0;

	/* Flash 0 */
#if defined (CFG_AMD_BOOT)
	batu = CFG_FLASH0_BASE | (BL_512K << 2) | BPP_RW | BPP_RX;
#else
	batu = CFG_FLASH0_BASE | (BL_16M << 2) | BPP_RW | BPP_RX;
#endif
	batl = CFG_FLASH0_BASE | 0x22;
	write_bat (IBAT0, batu, batl);
	write_bat (DBAT0, batu, batl);

	/* Flash 1 */
#if defined (CFG_AMD_BOOT)
	batu = CFG_FLASH1_BASE | (BL_16M << 2) | BPP_RW | BPP_RX;
#else
	batu = CFG_FLASH1_BASE | (BL_512K << 2) | BPP_RW | BPP_RX;
#endif
	batl = CFG_FLASH1_BASE | 0x22;
	write_bat (IBAT1, batu, batl);
	write_bat (DBAT1, batu, batl);

	/* CPLD */
	batu = CFG_CPLD_BASE | (BL_512K << 2) | BPP_RW | BPP_RX;
	batl = CFG_CPLD_BASE | 0x22;
	write_bat (IBAT2, 0, 0);
	write_bat (DBAT2, batu, batl);

	/* FPGA */
	batu = CFG_FPGA_BASE | (BL_512K << 2) | BPP_RW | BPP_RX;
	batl = CFG_FPGA_BASE | 0x22;
	write_bat (IBAT3, 0, 0);
	write_bat (DBAT3, batu, batl);

	/* MBAR - Data only */
	batu = CFG_MBAR | BPP_RW | BPP_RX;
	batl = CFG_MBAR | 0x22;
	mtspr (IBAT4L, 0);
	mtspr (IBAT4U, 0);
	mtspr (DBAT4L, batl);
	mtspr (DBAT4U, batu);

	/* MBAR - SRAM */
	batu = CFG_SRAM_BASE | BPP_RW | BPP_RX;
	batl = CFG_SRAM_BASE | 0x42;
	mtspr (IBAT5L, batl);
	mtspr (IBAT5U, batu);
	mtspr (DBAT5L, batl);
	mtspr (DBAT5U, batu);

	if (size <= 0x800000)	/* 8MB */
		blocksize = BL_8M << 2;
	else if (size <= 0x1000000)	/* 16MB */
		blocksize = BL_16M << 2;
	else if (size <= 0x2000000)	/* 32MB */
		blocksize = BL_32M << 2;
	else if (size <= 0x4000000)	/* 64MB */
		blocksize = BL_64M << 2;
	else if (size <= 0x8000000)	/* 128MB */
		blocksize = BL_128M << 2;
	else if (size <= 0x10000000)	/* 256MB */
		blocksize = BL_256M << 2;

	/* Memory */
	batu = CFG_SDRAM_BASE | blocksize | BPP_RW | BPP_RX;
	batl = CFG_SDRAM_BASE | 0x42;
	mtspr (IBAT6L, batl);
	mtspr (IBAT6U, batu);
	mtspr (DBAT6L, batl);
	mtspr (DBAT6U, batu);

	/* memory size is less than 256MB */
	if (size <= 0x10000000) {
		/* Nothing */
		batu = 0;
		batl = 0;
	} else {
		size -= 0x10000000;
		if (size <= 0x800000)	/* 8MB */
			blocksize = BL_8M << 2;
		else if (size <= 0x1000000)	/* 16MB */
			blocksize = BL_16M << 2;
		else if (size <= 0x2000000)	/* 32MB */
			blocksize = BL_32M << 2;
		else if (size <= 0x4000000)	/* 64MB */
			blocksize = BL_64M << 2;
		else if (size <= 0x8000000)	/* 128MB */
			blocksize = BL_128M << 2;
		else if (size <= 0x10000000)	/* 256MB */
			blocksize = BL_256M << 2;

		batu = (CFG_SDRAM_BASE +
			0x10000000) | blocksize | BPP_RW | BPP_RX;
		batl = (CFG_SDRAM_BASE + 0x10000000) | 0x42;
	}

	mtspr (IBAT7L, batl);
	mtspr (IBAT7U, batu);
	mtspr (DBAT7L, batl);
	mtspr (DBAT7U, batu);
}

long int initdram (int board_type)
{
	ulong size;

	size = dramSetup ();

/* if iCache ad dCache is defined */
#if (CONFIG_COMMANDS & CFG_CMD_CACHE)
/*    setupBat(size);*/
#endif

	return size;
}

int checkboard (void)
{
	puts ("Board: Alaska MPC8220 Evaluation Board\n");

	return 0;
}
