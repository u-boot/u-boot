/*
 *(C) Copyright 2005-2008 Netstal Maschinen AG
 *    Niklaus Giger (Niklaus.Giger@netstal.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <common.h>
#include  <ppc4xx.h>
#include  <asm/processor.h>
#include  "nm.h"

#if defined(DEBUG)
void show_sdram_registers(void)
{
	u32 value;

	printf("SDRAM Controller Registers --\n");
	mfsdram(mem_mcopt1, value);
	printf("    SDRAM0_CFG   : 0x%08x\n", value);
	mfsdram(mem_status, value);
	printf("    SDRAM0_STATUS: 0x%08x\n", value);
	mfsdram(mem_mb0cf, value);
	printf("    SDRAM0_B0CR  : 0x%08x\n", value);
	mfsdram(mem_mb1cf, value);
	printf("    SDRAM0_B1CR  : 0x%08x\n", value);
	mfsdram(mem_sdtr1, value);
	printf("    SDRAM0_TR    : 0x%08x\n", value);
	mfsdram(mem_rtr, value);
	printf("    SDRAM0_RTR   : 0x%08x\n", value);
}
#endif

long int fixed_hcu4_sdram (unsigned int dram_size)
{
#ifdef DEBUG
	printf(__FUNCTION__);
#endif
	/* disable memory controller */
	mtsdram(mem_mcopt1, 0x00000000);

	udelay (500);

	/* Clear SDRAM0_BESR0 (Bus Error Syndrome Register) */
	mtsdram(mem_besra, 0xffffffff);

	/* Clear SDRAM0_BESR1 (Bus Error Syndrome Register) */
	mtsdram(mem_besrb, 0xffffffff);

	/* Clear SDRAM0_ECCCFG (disable ECC) */
	mtsdram(mem_ecccf, 0x00000000);

	/* Clear SDRAM0_ECCESR (ECC Error Syndrome Register) */
	mtsdram(mem_eccerr, 0xffffffff);

	/* Timing register: CASL=2, PTA=2, CTP=2, LDF=1, RFTA=5, RCD=2
	 */
	mtsdram(mem_sdtr1, 0x008a4015);

	/* Memory Bank 0 Config == BA=0x00000000, SZ=64M, AM=3, BE=1
	 * and refresh timer
	 */
	switch (dram_size >> 20) {
	case 32:
		mtsdram(mem_mb0cf, 0x00062001);
		mtsdram(mem_rtr,   0x07F00000);
		break;
	case 64:
		mtsdram(mem_mb0cf, 0x00084001);
		mtsdram(mem_rtr,   0x04100000);
		break;
	case 128:
		mtsdram(mem_mb0cf, 0x000A4001);
		mtsdram(mem_rtr,   0x04100000);
		break;
	default:
		printf("Invalid memory size of %d MB given\n", dram_size >> 20);
	}

	/* Power management idle timer set to the default. */
	mtsdram(mem_pmit, 0x07c00000);

	udelay (500);

	/* Enable banks (DCE=1, BPRF=1, ECCDD=1, EMDUL=1) TODO */
	mtsdram(mem_mcopt1, 0x90800000);

#ifdef DEBUG
	printf("%s: done\n", __FUNCTION__);
#endif
	return dram_size;
}
