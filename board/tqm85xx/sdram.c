/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <spd.h>

struct sdram_conf_s {
	unsigned long size;
	unsigned long reg;
};

typedef struct sdram_conf_s sdram_conf_t;

sdram_conf_t ddr_cs_conf[] = {
	{(512 << 20), 0x80000202},	/* 512MB, 14x10(4)	*/
	{(256 << 20), 0x80000102},	/* 256MB, 13x10(4)	*/
	{(128 << 20), 0x80000101},	/* 128MB, 13x9(4)	*/
	{(64  << 20), 0x80000001},	/* 64MB,  12x9(4)	*/
};

#define	N_DDR_CS_CONF (sizeof(ddr_cs_conf) / sizeof(ddr_cs_conf[0]))

int cas_latency(void);

/*
 * Autodetect onboard DDR SDRAM on 85xx platforms
 *
 * NOTE: Some of the hardcoded values are hardware dependant,
 *       so this should be extended for other future boards
 *       using this routine!
 */
long int sdram_setup(int casl)
{
	int i;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_ddr_t *ddr = &immap->im_ddr;
	unsigned long cfg_ddr_timing1;
	unsigned long cfg_ddr_mode;

	/*
	 * Disable memory controller.
	 */
	ddr->cs0_config = 0;
	ddr->sdram_cfg = 0;

	switch (casl) {
	case 20:
		cfg_ddr_timing1 = 0x47405331 | (3 << 16);
		cfg_ddr_mode = 0x40020002 | (2 << 4);
		break;

	case 25:
		cfg_ddr_timing1 = 0x47405331 | (4 << 16);
		cfg_ddr_mode = 0x40020002 | (6 << 4);
		break;

	case 30:
	default:
		cfg_ddr_timing1 = 0x47405331 | (5 << 16);
		cfg_ddr_mode = 0x40020002 | (3 << 4);
		break;
	}

	ddr->cs0_bnds = (ddr_cs_conf[0].size - 1) >> 24;
	ddr->cs0_config = ddr_cs_conf[0].reg;
	ddr->timing_cfg_1 = cfg_ddr_timing1;
	ddr->timing_cfg_2 = 0x00000800;		/* P9-45,may need tuning */
	ddr->sdram_mode = cfg_ddr_mode;
	ddr->sdram_interval = 0x05160100;	/* autocharge,no open page */
	ddr->err_disable = 0x0000000D;

	asm ("sync;isync;msync");
	udelay(1000);

	ddr->sdram_cfg = 0xc2000000;		/* unbuffered,no DYN_PWR */
	asm ("sync; isync; msync");
	udelay(1000);

	for (i=0; i<N_DDR_CS_CONF; i++) {
		ddr->cs0_config = ddr_cs_conf[i].reg;

		if (get_ram_size(0, ddr_cs_conf[i].size) == ddr_cs_conf[i].size) {
			/*
			 * OK, size detected -> all done
			 */
			return ddr_cs_conf[i].size;
		}
	}

	return 0;				/* nothing found !		*/
}

void board_add_ram_info(int use_default)
{
	int casl;

	if (use_default)
		casl = CONFIG_DDR_DEFAULT_CL;
	else
		casl = cas_latency();

	puts(" (CL=");
	switch (casl) {
	case 20:
		puts("2)");
		break;

	case 25:
		puts("2.5)");
		break;

	case 30:
		puts("3)");
		break;
	}
}

long int initdram (int board_type)
{
	long dram_size = 0;
	int casl;

#if defined(CONFIG_DDR_DLL)
	/*
	 * This DLL-Override only used on TQM8540 and TQM8560
	 */
	{
		volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
		int i,x;

		x = 10;

		/*
		 * Work around to stabilize DDR DLL
		 */
		gur->ddrdllcr = 0x81000000;
		asm("sync;isync;msync");
		udelay (200);
		while (gur->ddrdllcr != 0x81000100) {
			gur->devdisr = gur->devdisr | 0x00010000;
			asm("sync;isync;msync");
			for (i=0; i<x; i++)
				;
			gur->devdisr = gur->devdisr & 0xfff7ffff;
			asm("sync;isync;msync");
			x++;
		}
	}
#endif

	casl = cas_latency();
	dram_size = sdram_setup(casl);
	if ((dram_size == 0) && (casl != CONFIG_DDR_DEFAULT_CL)) {
		/*
		 * Try again with default CAS latency
		 */
		puts("Problem with CAS lantency");
		board_add_ram_info(1);
		puts(", using default CL!\n");
		casl = CONFIG_DDR_DEFAULT_CL;
		dram_size = sdram_setup(casl);
		puts("       ");
	}

	return dram_size;
}

#if defined(CFG_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
	uint *p;

	printf ("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf ("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf ("SDRAM test passed.\n");
	return 0;
}
#endif
