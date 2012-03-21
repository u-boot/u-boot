/*
 * (C) Copyright 2009
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd.eu
 *
 * (C) Copyright 2006
 * Sylvie Gohl,             AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,           AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,           AMCC/IBM, rob.snyder@fr.ibm.com
 *
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/* define DEBUG for debug output */
#undef DEBUG

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/ppc440.h>

extern int denali_wait_for_dlllock(void);
extern void denali_core_search_data_eye(void);

struct sdram_conf_s {
	ulong size;
	int rows;
	int banks;
};

struct sdram_conf_s sdram_conf[] = {
	{(1024 << 20), 14, 8}, /* 1GByte: 4x2GBit, 14x10, 8 banks */
	{(512 << 20),  13, 8}, /* 512MByte: 4x1GBit, 13x10, 8 banks */
	{(256 << 20),  13, 4}, /* 256MByte: 4x512MBit, 13x10, 4 banks */
};

/*
 * initdram -- 440EPx's DDR controller is a DENALI Core
 */
int initdram_by_rb(int rows, int banks)
{
	ulong speed = get_bus_freq(0);

	mtsdram(DDR0_02, 0x00000000);

	mtsdram(DDR0_00, 0x0000190A);
	mtsdram(DDR0_01, 0x01000000);
	mtsdram(DDR0_03, 0x02030602);
	mtsdram(DDR0_04, 0x0A020200);
	mtsdram(DDR0_05, 0x02020308);
	mtsdram(DDR0_06, 0x0102C812);
	mtsdram(DDR0_07, 0x000D0100);
	mtsdram(DDR0_08, 0x02430001);
	mtsdram(DDR0_09, 0x00011D5F);
	mtsdram(DDR0_10, 0x00000100);
	mtsdram(DDR0_11, 0x0027C800);
	mtsdram(DDR0_12, 0x00000003);
	mtsdram(DDR0_14, 0x00000000);
	mtsdram(DDR0_17, 0x19000000);
	mtsdram(DDR0_18, 0x19191919);
	mtsdram(DDR0_19, 0x19191919);
	mtsdram(DDR0_20, 0x0B0B0B0B);
	mtsdram(DDR0_21, 0x0B0B0B0B);
	mtsdram(DDR0_22, 0x00267F0B);
	mtsdram(DDR0_23, 0x00000000);
	mtsdram(DDR0_24, 0x01010002);
	if (speed > 133333334)
		mtsdram(DDR0_26, 0x5B26050C);
	else
		mtsdram(DDR0_26, 0x5B260408);
	mtsdram(DDR0_27, 0x0000682B);
	mtsdram(DDR0_28, 0x00000000);
	mtsdram(DDR0_31, 0x00000000);

	mtsdram(DDR0_42,
		DDR0_42_ADDR_PINS_DECODE(14 - rows) |
		0x00000006);
	mtsdram(DDR0_43,
		DDR0_43_EIGHT_BANK_MODE_ENCODE(8 == banks ? 1 : 0) |
		0x030A0200);

	mtsdram(DDR0_44, 0x00000003);
	mtsdram(DDR0_02, 0x00000001);

	denali_wait_for_dlllock();

#ifdef CONFIG_DDR_DATA_EYE
	/*
	 * Perform data eye search if requested.
	 */
	denali_core_search_data_eye();
#endif
	/*
	 * Clear possible errors resulting from data-eye-search.
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	set_mcsr(get_mcsr());

	return 0;
}

phys_size_t initdram(int board_type)
{
	phys_size_t size;
	int n;

	/* go through supported memory configurations */
	for (n = 0; n < ARRAY_SIZE(sdram_conf); n++) {
		size = sdram_conf[n].size;

		/* program TLB entries */
		program_tlb(0, CONFIG_SYS_SDRAM_BASE, size,
			    TLB_WORD2_I_ENABLE);

		/*
		 * setup denali core
		 */
		initdram_by_rb(sdram_conf[n].rows,
			       sdram_conf[n].banks);

		/* check for suitable configuration */
		if (get_ram_size(CONFIG_SYS_SDRAM_BASE, size) == size)
			return size;

		/* delete TLB entries */
		remove_tlb(CONFIG_SYS_SDRAM_BASE, size);
	}

	return 0;
}
