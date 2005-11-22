/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
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

#ifndef CFG_SDRAM_TABLE
sdram_conf_t mb0cf[] = {
	{(128 << 20), 0x000A4001},      /* (0-128MB) Address Mode 3, 13x10(4) */
	{(64 << 20),  0x00084001},      /* (0-64MB) Address Mode 3, 13x9(4)   */
	{(32 << 20),  0x00062001},      /* (0-32MB) Address Mode 2, 12x9(4)   */
	{(16 << 20),  0x00046001},      /* (0-16MB) Address Mode 4, 12x8(4)   */
	{(4 << 20),   0x00008001},      /* (0-4MB) Address Mode 5, 11x8(2)    */
};
#else
sdram_conf_t mb0cf[] = CFG_SDRAM_TABLE;
#endif

#define	N_MB0CF (sizeof(mb0cf) / sizeof(mb0cf[0]))


#ifndef CONFIG_440

/*
 * Autodetect onboard SDRAM on 405 platforms
 */
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

#else /* CONFIG_440 */

/*
 * Autodetect onboard DDR SDRAM on 440 platforms
 *
 * NOTE: Some of the hardcoded values are hardware dependant,
 *       so this should be extended for other future boards
 *       using this routine!
 */
long int initdram(int board_type)
{
	int i;

	for (i=0; i<N_MB0CF; i++) {
		/*
		 * Disable memory controller.
		 */
		mtsdram(mem_cfg0, 0x00000000);

		/*
		 * Setup some default
		 */
		mtsdram(mem_uabba, 0x00000000);	/* ubba=0 (default)             */
		mtsdram(mem_slio, 0x00000000);	/* rdre=0 wrre=0 rarw=0         */
		mtsdram(mem_devopt, 0x00000000); /* dll=0 ds=0 (normal)		*/
		mtsdram(mem_wddctr, 0x00000000); /* wrcp=0 dcd=0		*/
		mtsdram(mem_clktr, 0x40000000);	/* clkp=1 (90 deg wr) dcdt=0    */

		/*
		 * Following for CAS Latency = 2.5 @ 133 MHz PLB
		 */
		mtsdram(mem_b0cr, mb0cf[i].reg);
		mtsdram(mem_tr0, 0x41094012);
		mtsdram(mem_tr1, 0x80800800);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x00*/
		mtsdram(mem_rtr, 0x7e000000);	/* Interval 15.20µs @ 133MHz PLB*/
		mtsdram(mem_cfg1, 0x00000000);	/* Self-refresh exit, disable PM*/
		udelay(400);			/* Delay 200 usecs (min)	*/

		/*
		 * Enable the controller, then wait for DCEN to complete
		 */
		mtsdram(mem_cfg0, 0x86000000);	/* DCEN=1, PMUD=1, 64-bit       */
		udelay(10000);

		if (get_ram_size(0, mb0cf[i].size) == mb0cf[i].size) {
			/*
			 * OK, size detected -> all done
			 */
			return mb0cf[i].size;
		}
	}

	return 0;				/* nothing found !		*/
}

#endif /* CONFIG_440 */

#endif /* CONFIG_SDRAM_BANK0 */
