/*
 * (C) Copyright 2002-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
#include <ppc4xx.h>
#include <asm/processor.h>


#ifdef CONFIG_SDRAM_BANK0


#define mtsdram0(reg, data)  mtdcr(memcfga,reg);mtdcr(memcfgd,data)


struct sdram_conf_s {
	unsigned long size;
	unsigned long reg;
};

typedef struct sdram_conf_s sdram_conf_t;

sdram_conf_t mb0cf[] = {
	{(128 << 20), 0x000A4001},      /* (0-128MB) Address Mode 3, 13x10(4) */
	{(64 << 20),  0x00084001},      /* (0-64MB) Address Mode 3, 13x9(4)   */
	{(32 << 20),  0x00062001},      /* (0-32MB) Address Mode 2, 12x9(4)   */
	{(16 << 20),  0x00046001},      /* (0-16MB) Address Mode 4, 12x8(4)   */
	{(4 << 20),   0x00008001},      /* (0-4MB) Address Mode 5, 11x8(2)    */
};
#define	N_MB0CF (sizeof(mb0cf) / sizeof(mb0cf[0]))


void sdram_init(void)
{
	ulong sdtr1;
	ulong rtr;
	int i;

	/*
	 * Support for 100MHz and 133MHz SDRAM
	 */
	if (get_bus_freq(0) > 100000000) {
		/*
		 * 133 MHz SDRAM
		 */
		sdtr1 = 0x01074015;
		rtr = 0x07f00000;
	} else {
		/*
		 * default: 100 MHz SDRAM
		 */
		sdtr1 = 0x0086400d;
		rtr = 0x05f00000;
	}

	for (i=0; i<N_MB0CF; i++) {
		/*
		 * Disable memory controller.
		 */
		mtsdram0(mem_mcopt1, 0x00000000);

		/*
		 * Set MB0CF for bank 0.
		 */
		mtsdram0(mem_mb0cf, mb0cf[i].reg);
		mtsdram0(mem_sdtr1, sdtr1);
		mtsdram0(mem_rtr, rtr);

		udelay(200);

		/*
		 * Set memory controller options reg, MCOPT1.
		 * Set DC_EN to '1' and BRD_PRF to '01' for 16 byte PLB Burst
		 * read/prefetch.
		 */
		mtsdram0(mem_mcopt1, 0x80800000);

		udelay(10000);

		if (get_ram_size(0, mb0cf[i].size) == mb0cf[i].size) {
			/*
			 * OK, size detected -> all done
			 */
			return;
		}
	}
}

#endif /* CONFIG_SDRAM_BANK0 */
