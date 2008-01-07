/*
 * (C) Copyright 2007
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>

void sdram_init(void)
{
	return;
}

long int initdram(int board_type)
{
	/*
	 * Same as on Kilauea, Makalu generates exception 0x200
	 * (machine check) after trap_init() in board_init_f,
	 * when SDRAM is initialized here (late) and d-cache is
	 * used earlier as INIT_RAM.
	 * So for now, initialize DDR2 in init.S very early and
	 * also use it for INIT_RAM. Then this exception doesn't
	 * occur.
	 */
#if 0
	u32 val;

	/* base=00000000, size=128MByte (5), mode=2 (n*10*4) */
	mtsdram(SDRAM_MB0CF, 0x00005201);

	/* SET SDRAM_MB1CF - Not enabled */
	mtsdram(SDRAM_MB1CF, 0x00000000);

	/* SET SDRAM_MB2CF  - Not enabled */
	mtsdram(SDRAM_MB2CF, 0x00000000);

	/* SET SDRAM_MB3CF  - Not enabled */
	mtsdram(SDRAM_MB3CF, 0x00000000);

	/* SDRAM_CLKTR: Adv Addr clock by 90 deg */
	mtsdram(SDRAM_CLKTR, 0x80000000);

	/* Refresh Time register (0x30) Refresh every 7.8125uS */
	mtsdram(SDRAM_RTR, 0x06180000);

	/* SDRAM_SDTR1 */
	mtsdram(SDRAM_SDTR1, 0x80201000);

	/* SDRAM_SDTR2	*/
	mtsdram(SDRAM_SDTR2, 0x32204232);

	/* SDRAM_SDTR3	*/
	mtsdram(SDRAM_SDTR3, 0x080b0d1a);

	mtsdram(SDRAM_MMODE, 0x00000442);
	mtsdram(SDRAM_MEMODE, 0x00000404);

	/* SDRAM0_MCOPT1 (0X20) No ECC Gen */
	mtsdram(SDRAM_MCOPT1, 0x04322000);

	/* NOP */
	mtsdram(SDRAM_INITPLR0, 0xa8380000);
	/* precharge 3 DDR clock cycle */
	mtsdram(SDRAM_INITPLR1, 0x81900400);
	/* EMR2 twr = 2tck */
	mtsdram(SDRAM_INITPLR2, 0x81020000);
	/* EMR3  twr = 2tck */
	mtsdram(SDRAM_INITPLR3, 0x81030000);
	/* EMR DLL ENABLE twr = 2tck */
	mtsdram(SDRAM_INITPLR4, 0x81010404);
	/* MR w/ DLL reset
	 * Note: 5 is CL.  May need to be changed
	 */
	mtsdram(SDRAM_INITPLR5, 0x81000542);
	/* precharge 3 DDR clock cycle */
	mtsdram(SDRAM_INITPLR6, 0x81900400);
	/* Auto-refresh trfc = 26tck */
	mtsdram(SDRAM_INITPLR7, 0x8D080000);
	/* Auto-refresh trfc = 26tck */
	mtsdram(SDRAM_INITPLR8, 0x8D080000);
	/* Auto-refresh */
	mtsdram(SDRAM_INITPLR9, 0x8D080000);
	/* Auto-refresh */
	mtsdram(SDRAM_INITPLR10, 0x8D080000);
	/* MRS - normal operation; wait 2 cycle (set wait to tMRD) */
	mtsdram(SDRAM_INITPLR11, 0x81000442);
	mtsdram(SDRAM_INITPLR12, 0x81010780);
	mtsdram(SDRAM_INITPLR13, 0x81010400);
	mtsdram(SDRAM_INITPLR14, 0x00000000);
	mtsdram(SDRAM_INITPLR15, 0x00000000);

	/* SET MCIF0_CODT   Die Termination On */
	mtsdram(SDRAM_CODT, 0x0080f837);
	mtsdram(SDRAM_MODT0, 0x01800000);
	mtsdram(SDRAM_MODT1, 0x00000000);

	mtsdram(SDRAM_WRDTR, 0x00000000);

	/* SDRAM0_MCOPT2 (0X21) Start initialization */
	mtsdram(SDRAM_MCOPT2, 0x20000000);

	/* Step 5 */
	do {
		mfsdram(SDRAM_MCSTAT, val);
	} while ((val & SDRAM_MCSTAT_MIC_COMP) != SDRAM_MCSTAT_MIC_COMP);

	/* Step 6 */

	/* SDRAM_DLCR */
	mtsdram(SDRAM_DLCR, 0x030000a5);

	/* SDRAM_RDCC */
	mtsdram(SDRAM_RDCC, 0x40000000);

	/* SDRAM_RQDC */
	mtsdram(SDRAM_RQDC, 0x80000038);

	/* SDRAM_RFDC */
	mtsdram(SDRAM_RFDC, 0x00000209);

	/* Enable memory controller */
	mfsdram(SDRAM_MCOPT2, val);
	val |= SDRAM_MCOPT2_DCEN_ENABLE;
	mtsdram(SDRAM_MCOPT2, val);
#endif
	return (CFG_MBYTES_SDRAM << 20);
}

#if defined(CFG_DRAM_TEST)
int testdram (void)
{
    printf ("testdram\n");
#if defined (CONFIG_NAND_U_BOOT)
    return 0;
#endif
	uint *pstart = (uint *) 0x00000000;
	uint *pend = (uint *) 0x00001000;
	uint *p;

	for (p = pstart; p < pend; p++) {
		*p = 0xaaaaaaaa;
	}

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
#if !defined (CONFIG_NAND_SPL)
			printf ("SDRAM test fails at: %08x\n", (uint) p);
#endif
			return 1;
		}
	}

	for (p = pstart; p < pend; p++) {
		*p = 0x55555555;
	}

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
#if !defined (CONFIG_NAND_SPL)
			printf ("SDRAM test fails at: %08x\n", (uint) p);
#endif
			return 1;
		}
	}
#if !defined (CONFIG_NAND_SPL)
	printf ("SDRAM test passed!!!\n");
#endif
	return 0;
}
#endif
