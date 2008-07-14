/*
 * (C) Copyright 2005
 * BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
 *
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
#include <command.h>
#include "asm/m5282.h"
#include "VCxK.h"

int checkboard (void)
{
	puts ("Board: MCF-EV1 + MCF-EV23 (BuS Elektronik GmbH & Co. KG)\n");
#if (TEXT_BASE ==  CFG_INT_FLASH_BASE)
	puts ("       Boot from Internal FLASH\n");
#endif

	return 0;
}

phys_size_t initdram (int board_type)
{
	int size, i;

	size = 0;
	MCFSDRAMC_DCR = MCFSDRAMC_DCR_RTIM_6
			| MCFSDRAMC_DCR_RC ((15 * CFG_CLK) >> 4);
#ifdef CFG_SDRAM_BASE0

	MCFSDRAMC_DACR0 = MCFSDRAMC_DACR_BASE (CFG_SDRAM_BASE0)
			| MCFSDRAMC_DACR_CASL (1)
			| MCFSDRAMC_DACR_CBM (3)
			| MCFSDRAMC_DACR_PS_16;

	MCFSDRAMC_DMR0 = MCFSDRAMC_DMR_BAM_16M | MCFSDRAMC_DMR_V;

	MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IP;

	*(unsigned short *) (CFG_SDRAM_BASE0) = 0xA5A5;
	MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_RE;
	for (i = 0; i < 2000; i++)
		asm (" nop");
	mbar_writeLong (MCFSDRAMC_DACR0,
			mbar_readLong (MCFSDRAMC_DACR0) | MCFSDRAMC_DACR_IMRS);
	*(unsigned int *) (CFG_SDRAM_BASE0 + 0x220) = 0xA5A5;
	size += CFG_SDRAM_SIZE * 1024 * 1024;
#endif
#ifdef CFG_SDRAM_BASE1
	MCFSDRAMC_DACR1 = MCFSDRAMC_DACR_BASE (CFG_SDRAM_BASE1)
			| MCFSDRAMC_DACR_CASL (1)
			| MCFSDRAMC_DACR_CBM (3)
			| MCFSDRAMC_DACR_PS_16;

	MCFSDRAMC_DMR1 = MCFSDRAMC_DMR_BAM_16M | MCFSDRAMC_DMR_V;

	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_IP;

	*(unsigned short *) (CFG_SDRAM_BASE1) = 0xA5A5;
	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_RE;

	for (i = 0; i < 2000; i++)
		asm (" nop");

	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_IMRS;
	*(unsigned int *) (CFG_SDRAM_BASE1 + 0x220) = 0xA5A5;
	size += CFG_SDRAM_SIZE1 * 1024 * 1024;
#endif
	return size;
}


#if defined(CFG_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
	uint *p;

	printf("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test passed.\n");
	return 0;
}
#endif

int misc_init_r(void)
{
	init_vcxk();
	return 1;
}

/*---------------------------------------------------------------------------*/

int do_vcimage (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode = 0;
	ulong source;

	switch (argc) {
	case 2:
		source = simple_strtoul(argv[1],NULL,16);
		vcxk_loadimage(source);
		rcode = 0;
		break;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		rcode = 1;
		break;
	}
	return rcode;
}

/***************************************************/

U_BOOT_CMD(
	vcimage,	2,	0,	do_vcimage,
	"vcimage - loads an image to Display\n",
	"vcimage addr\n"
);

/* EOF EB+MCF-EV123c */
