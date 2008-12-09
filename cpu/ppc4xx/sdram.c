/*
 * (C) Copyright 2005-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * DAVE Srl <www.dave-tech.it>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#include "sdram.h"
#include "ecc.h"

#ifdef CONFIG_SDRAM_BANK0

#ifndef CONFIG_440

#ifndef CONFIG_SYS_SDRAM_TABLE
sdram_conf_t mb0cf[] = {
	{(128 << 20), 13, 0x000A4001},	    /* (0-128MB) Address Mode 3, 13x10(4) */
	{(64 << 20),  13, 0x00084001},	    /* (0-64MB) Address Mode 3, 13x9(4)	  */
	{(32 << 20),  12, 0x00062001},	    /* (0-32MB) Address Mode 2, 12x9(4)	  */
	{(16 << 20),  12, 0x00046001},	    /* (0-16MB) Address Mode 4, 12x8(4)	  */
	{(4 << 20),   11, 0x00008001},	    /* (0-4MB) Address Mode 5, 11x8(2)	  */
};
#else
sdram_conf_t mb0cf[] = CONFIG_SYS_SDRAM_TABLE;
#endif

#define N_MB0CF (sizeof(mb0cf) / sizeof(mb0cf[0]))

#ifdef CONFIG_SYS_SDRAM_CASL
static ulong ns2clks(ulong ns)
{
	ulong bus_period_x_10 = ONE_BILLION / (get_bus_freq(0) / 10);

	return ((ns * 10) + bus_period_x_10) / bus_period_x_10;
}
#endif /* CONFIG_SYS_SDRAM_CASL */

static ulong compute_sdtr1(ulong speed)
{
#ifdef CONFIG_SYS_SDRAM_CASL
	ulong tmp;
	ulong sdtr1 = 0;

	/* CASL */
	if (CONFIG_SYS_SDRAM_CASL < 2)
		sdtr1 |= (1 << SDRAM0_TR_CASL);
	else
		if (CONFIG_SYS_SDRAM_CASL > 4)
			sdtr1 |= (3 << SDRAM0_TR_CASL);
		else
			sdtr1 |= ((CONFIG_SYS_SDRAM_CASL-1) << SDRAM0_TR_CASL);

	/* PTA */
	tmp = ns2clks(CONFIG_SYS_SDRAM_PTA);
	if ((tmp >= 2) && (tmp <= 4))
		sdtr1 |= ((tmp-1) << SDRAM0_TR_PTA);
	else
		sdtr1 |= ((4-1) << SDRAM0_TR_PTA);

	/* CTP */
	tmp = ns2clks(CONFIG_SYS_SDRAM_CTP);
	if ((tmp >= 2) && (tmp <= 4))
		sdtr1 |= ((tmp-1) << SDRAM0_TR_CTP);
	else
		sdtr1 |= ((4-1) << SDRAM0_TR_CTP);

	/* LDF */
	tmp = ns2clks(CONFIG_SYS_SDRAM_LDF);
	if ((tmp >= 2) && (tmp <= 4))
		sdtr1 |= ((tmp-1) << SDRAM0_TR_LDF);
	else
		sdtr1 |= ((2-1) << SDRAM0_TR_LDF);

	/* RFTA */
	tmp = ns2clks(CONFIG_SYS_SDRAM_RFTA);
	if ((tmp >= 4) && (tmp <= 10))
		sdtr1 |= ((tmp-4) << SDRAM0_TR_RFTA);
	else
		sdtr1 |= ((10-4) << SDRAM0_TR_RFTA);

	/* RCD */
	tmp = ns2clks(CONFIG_SYS_SDRAM_RCD);
	if ((tmp >= 2) && (tmp <= 4))
		sdtr1 |= ((tmp-1) << SDRAM0_TR_RCD);
	else
		sdtr1 |= ((4-1) << SDRAM0_TR_RCD);

	return sdtr1;
#else /* CONFIG_SYS_SDRAM_CASL */
	/*
	 * If no values are configured in the board config file
	 * use the default values, which seem to be ok for most
	 * boards.
	 *
	 * REMARK:
	 * For new board ports we strongly recommend to define the
	 * correct values for the used SDRAM chips in your board
	 * config file (see PPChameleonEVB.h)
	 */
	if (speed > 100000000) {
		/*
		 * 133 MHz SDRAM
		 */
		return 0x01074015;
	} else {
		/*
		 * default: 100 MHz SDRAM
		 */
		return 0x0086400d;
	}
#endif /* CONFIG_SYS_SDRAM_CASL */
}

/* refresh is expressed in ms */
static ulong compute_rtr(ulong speed, ulong rows, ulong refresh)
{
#ifdef CONFIG_SYS_SDRAM_CASL
	ulong tmp;

	tmp = ((refresh*1000*1000) / (1 << rows)) * (speed / 1000);
	tmp /= 1000000;

	return ((tmp & 0x00003FF8) << 16);
#else /* CONFIG_SYS_SDRAM_CASL */
	if (speed > 100000000) {
		/*
		 * 133 MHz SDRAM
		 */
		return 0x07f00000;
	} else {
		/*
		 * default: 100 MHz SDRAM
		 */
		return 0x05f00000;
	}
#endif /* CONFIG_SYS_SDRAM_CASL */
}

/*
 * Autodetect onboard SDRAM on 405 platforms
 */
phys_size_t initdram(int board_type)
{
	ulong speed;
	ulong sdtr1;
	int i;

	/*
	 * Determine SDRAM speed
	 */
	speed = get_bus_freq(0); /* parameter not used on ppc4xx */

	/*
	 * sdtr1 (register SDRAM0_TR) must take into account timings listed
	 * in SDRAM chip datasheet. rtr (register SDRAM0_RTR) must take into
	 * account actual SDRAM size. So we can set up sdtr1 according to what
	 * is specified in board configuration file while rtr dependds on SDRAM
	 * size we are assuming before detection.
	 */
	sdtr1 = compute_sdtr1(speed);

	for (i=0; i<N_MB0CF; i++) {
		/*
		 * Disable memory controller.
		 */
		mtsdram(mem_mcopt1, 0x00000000);

		/*
		 * Set MB0CF for bank 0.
		 */
		mtsdram(mem_mb0cf, mb0cf[i].reg);
		mtsdram(mem_sdtr1, sdtr1);
		mtsdram(mem_rtr, compute_rtr(speed, mb0cf[i].rows, 64));

		udelay(200);

		/*
		 * Set memory controller options reg, MCOPT1.
		 * Set DC_EN to '1' and BRD_PRF to '01' for 16 byte PLB Burst
		 * read/prefetch.
		 */
		mtsdram(mem_mcopt1, 0x80800000);

		udelay(10000);

		if (get_ram_size(0, mb0cf[i].size) == mb0cf[i].size) {
			phys_size_t size = mb0cf[i].size;

			/*
			 * OK, size detected.  Enable second bank if
			 * defined (assumes same type as bank 0)
			 */
#ifdef CONFIG_SDRAM_BANK1
			mtsdram(mem_mcopt1, 0x00000000);
			mtsdram(mem_mb1cf, mb0cf[i].size | mb0cf[i].reg);
			mtsdram(mem_mcopt1, 0x80800000);
			udelay(10000);

			/*
			 * Check if 2nd bank is really available.
			 * If the size not equal to the size of the first
			 * bank, then disable the 2nd bank completely.
			 */
			if (get_ram_size((long *)mb0cf[i].size, mb0cf[i].size) !=
			    mb0cf[i].size) {
				mtsdram(mem_mb1cf, 0);
				mtsdram(mem_mcopt1, 0);
			} else {
				/*
				 * We have two identical banks, so the size
				 * is twice the bank size
				 */
				size = 2 * size;
			}
#endif

			/*
			 * OK, size detected -> all done
			 */
			return size;
		}
	}

	return 0;
}

#else /* CONFIG_440 */

/*
 * Define some default values. Those can be overwritten in the
 * board config file.
 */

#ifndef CONFIG_SYS_SDRAM_TABLE
sdram_conf_t mb0cf[] = {
	{(256 << 20), 13, 0x000C4001},	/* 256MB mode 3, 13x10(4)	*/
	{(128 << 20), 13, 0x000A4001},	/* 128MB mode 3, 13x10(4)	*/
	{(64 << 20),  12, 0x00082001}	/* 64MB mode 2, 12x9(4)		*/
};
#else
sdram_conf_t mb0cf[] = CONFIG_SYS_SDRAM_TABLE;
#endif

#ifndef CONFIG_SYS_SDRAM0_TR0
#define	CONFIG_SYS_SDRAM0_TR0		0x41094012
#endif

#ifndef CONFIG_SYS_SDRAM0_WDDCTR
#define CONFIG_SYS_SDRAM0_WDDCTR	0x00000000  /* wrcp=0 dcd=0	*/
#endif

#ifndef CONFIG_SYS_SDRAM0_RTR
#define CONFIG_SYS_SDRAM0_RTR 		0x04100000 /* 7.8us @ 133MHz PLB */
#endif

#ifndef CONFIG_SYS_SDRAM0_CFG0
#define CONFIG_SYS_SDRAM0_CFG0		0x82000000 /* DCEN=1, PMUD=0, 64-bit */
#endif

#define N_MB0CF (sizeof(mb0cf) / sizeof(mb0cf[0]))

#define NUM_TRIES 64
#define NUM_READS 10

static void sdram_tr1_set(int ram_address, int* tr1_value)
{
	int i;
	int j, k;
	volatile unsigned int* ram_pointer = (unsigned int *)ram_address;
	int first_good = -1, last_bad = 0x1ff;

	unsigned long test[NUM_TRIES] = {
		0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
		0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55 };

	/* go through all possible SDRAM0_TR1[RDCT] values */
	for (i=0; i<=0x1ff; i++) {
		/* set the current value for TR1 */
		mtsdram(mem_tr1, (0x80800800 | i));

		/* write values */
		for (j=0; j<NUM_TRIES; j++) {
			ram_pointer[j] = test[j];

			/* clear any cache at ram location */
			__asm__("dcbf 0,%0": :"r" (&ram_pointer[j]));
		}

		/* read values back */
		for (j=0; j<NUM_TRIES; j++) {
			for (k=0; k<NUM_READS; k++) {
				/* clear any cache at ram location */
				__asm__("dcbf 0,%0": :"r" (&ram_pointer[j]));

				if (ram_pointer[j] != test[j])
					break;
			}

			/* read error */
			if (k != NUM_READS)
				break;
		}

		/* we have a SDRAM0_TR1[RDCT] that is part of the window */
		if (j == NUM_TRIES) {
			if (first_good == -1)
				first_good = i;		/* found beginning of window */
		} else { /* bad read */
			/* if we have not had a good read then don't care */
			if (first_good != -1) {
				/* first failure after a good read */
				last_bad = i-1;
				break;
			}
		}
	}

	/* return the current value for TR1 */
	*tr1_value = (first_good + last_bad) / 2;
}

/*
 * Autodetect onboard DDR SDRAM on 440 platforms
 *
 * NOTE: Some of the hardcoded values are hardware dependant,
 *	 so this should be extended for other future boards
 *	 using this routine!
 */
phys_size_t initdram(int board_type)
{
	int i;
	int tr1_bank1;

#if defined(CONFIG_440GX) || defined(CONFIG_440EP) || \
    defined(CONFIG_440GR) || defined(CONFIG_440SP)
	/*
	 * Soft-reset SDRAM controller.
	 */
	mtsdr(sdr_srst, SDR0_SRST_DMC);
	mtsdr(sdr_srst, 0x00000000);
#endif

	for (i=0; i<N_MB0CF; i++) {
		/*
		 * Disable memory controller.
		 */
		mtsdram(mem_cfg0, 0x00000000);

		/*
		 * Setup some default
		 */
		mtsdram(mem_uabba, 0x00000000); /* ubba=0 (default)		*/
		mtsdram(mem_slio, 0x00000000);	/* rdre=0 wrre=0 rarw=0		*/
		mtsdram(mem_devopt, 0x00000000); /* dll=0 ds=0 (normal)		*/
		mtsdram(mem_wddctr, CONFIG_SYS_SDRAM0_WDDCTR);
		mtsdram(mem_clktr, 0x40000000); /* clkp=1 (90 deg wr) dcdt=0	*/

		/*
		 * Following for CAS Latency = 2.5 @ 133 MHz PLB
		 */
		mtsdram(mem_b0cr, mb0cf[i].reg);
		mtsdram(mem_tr0, CONFIG_SYS_SDRAM0_TR0);
		mtsdram(mem_tr1, 0x80800800);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x00*/
		mtsdram(mem_rtr, CONFIG_SYS_SDRAM0_RTR);
		mtsdram(mem_cfg1, 0x00000000);	/* Self-refresh exit, disable PM*/
		udelay(400);			/* Delay 200 usecs (min)	*/

		/*
		 * Enable the controller, then wait for DCEN to complete
		 */
		mtsdram(mem_cfg0, CONFIG_SYS_SDRAM0_CFG0);
		udelay(10000);

		if (get_ram_size(0, mb0cf[i].size) == mb0cf[i].size) {
			phys_size_t size = mb0cf[i].size;
			/*
			 * Optimize TR1 to current hardware environment
			 */
			sdram_tr1_set(0x00000000, &tr1_bank1);
			mtsdram(mem_tr1, (tr1_bank1 | 0x80800800));


			/*
			 * OK, size detected.  Enable second bank if
			 * defined (assumes same type as bank 0)
			 */
#ifdef CONFIG_SDRAM_BANK1
			mtsdram(mem_cfg0, 0);
			mtsdram(mem_b1cr, mb0cf[i].size | mb0cf[i].reg);
			mtsdram(mem_cfg0, CONFIG_SYS_SDRAM0_CFG0);
			udelay(10000);

			/*
			 * Check if 2nd bank is really available.
			 * If the size not equal to the size of the first
			 * bank, then disable the 2nd bank completely.
			 */
			if (get_ram_size((long *)mb0cf[i].size, mb0cf[i].size)
			    != mb0cf[i].size) {
				mtsdram(mem_cfg0, 0);
				mtsdram(mem_b1cr, 0);
				mtsdram(mem_cfg0, CONFIG_SYS_SDRAM0_CFG0);
				udelay(10000);
			} else {
				/*
				 * We have two identical banks, so the size
				 * is twice the bank size
				 */
				size = 2 * size;
			}
#endif

#ifdef CONFIG_SDRAM_ECC
			ecc_init(0, size);
#endif

			/*
			 * OK, size detected -> all done
			 */
			return size;
		}
	}

	return 0;				/* nothing found !		*/
}

#endif /* CONFIG_440 */

#endif /* CONFIG_SDRAM_BANK0 */
