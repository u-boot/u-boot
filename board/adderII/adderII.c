/*
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
#include <config.h>
#include <mpc8xx.h>

/*
 * Check Board Identity:
 */

int checkboard( void )
{
	puts("Board: ");
	puts("AdderII(MPC852T)\n" );

  	return 0;
}

#if defined( CONFIG_SDRAM_50MHZ )

/******************************************************************************
** for chip Samsung K4S643232F - T70
** this table is for 32-50MHz operation
*******************************************************************************/

#define SDRAM_MPTPRVALUE 	0x0200

#define SDRAM_MAMRVALUE0 	0x00802114   /* refresh at 32MHz */
#define SDRAM_MAMRVALUE1 	0x00802118

#define SDRAM_OR1VALUE   	0xff800e00
#define SDRAM_BR1VALUE   	0x00000081

#define SDRAM_MARVALUE		94

#define SDRAM_MCRVALUE0  	0x80808105
#define SDRAM_MCRVALUE1  	0x80808130

const uint sdram_table[] = {

	/* single read   (offset 0x00 in upm ram) */
       	0x1f07fc24, 0xe0aefc04, 0x10adfc04, 0xe0bbbc00,
       	0x10f77c44, 0xf3fffc07, 0xfffffc04, 0xfffffc04,

	/* burst read    (offset 0x08 in upm ram) */
       	0x1f07fc24, 0xe0aefc04, 0x10adfc04, 0xf0affc00,
       	0xf0affc00, 0xf0affc00, 0xf0affc00, 0x10a77c44,
       	0xf7bffc47, 0xfffffc35, 0xfffffc34, 0xfffffc35,
       	0xfffffc35, 0x1ff77c35, 0xfffffc34, 0x1fb57c35,

	/* single write  (offset 0x18 in upm ram) */
       	0x1f27fc24, 0xe0aebc04, 0x00b93c00, 0x13f77c47,
       	0xfffdfc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* burst write   (offset 0x20 in upm ram) */
       	0x1f07fc24, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
       	0xf0affc00, 0xe0abbc00, 0x1fb77c47, 0xfffffc04,
       	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,
       	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* refresh       (offset 0x30 in upm ram) */
       	0x1ff5fca4, 0xfffffc04, 0xfffffc04, 0xfffffc04,
       	0xfffffc84, 0xfffffc07, 0xfffffc04, 0xfffffc04,
       	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* exception     (offset 0x3C in upm ram) */
       	0xfffffc27, 0xfffffc04, 0xfffffc04, 0xfffffc04,
};

#else
#error SDRAM not correctly configured
#endif

int _initsdram (uint base, uint noMbytes)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	if (noMbytes != 8) {
		return -1;
	}

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	memctl->memc_mptpr = SDRAM_MPTPRVALUE;

	/* Configure the refresh (mostly).  This needs to be
	 * based upon processor clock speed and optimized to provide
	 * the highest level of performance.  For multiple banks,
	 * this time has to be divided by the number of banks.
	 * Although it is not clear anywhere, it appears the
	 * refresh steps through the chip selects for this UPM
	 * on each refresh cycle.
	 * We have to be careful changing
	 * UPM registers after we ask it to run these commands.
	 */

	memctl->memc_mamr = (SDRAM_MAMRVALUE0 | (SDRAM_MARVALUE << 24));
	memctl->memc_mar = 0x0;
	udelay (200);

	/* Now run the precharge/nop/mrs commands.
	 */
	memctl->memc_mcr = 0x80002115;
	udelay (200);

	/* Run 8 refresh cycles */
	memctl->memc_mcr = 0x80002380;
	udelay (200);

	memctl->memc_mar = 0x88;
	udelay (200);

	memctl->memc_mcr = 0x80002116;
	udelay (200);

	memctl->memc_or1 = SDRAM_OR1VALUE;
	memctl->memc_br1 = SDRAM_BR1VALUE | base;

	return 0;
}

void _sdramdisable( void )
{
	volatile immap_t     *immap = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_br1 = 0x00000000;

	/* maybe we should turn off upma here or something */
}

int initsdram (uint base, uint * noMbytes)
{
	uint m = 8;

	*noMbytes = m;

	if (!_initsdram (base, m)) {
		return 0;
	} else {
		_sdramdisable ();
		return -1;
	}
}

long int initdram (int board_type)
{
	/* AdderII: has 8MB SDRAM */
	uint sdramsz;
	uint m = 0;

	if (!initsdram (0x00000000, &sdramsz)) {
		m += sdramsz;
	} else {
		return -1;
	}
	return (m << 20);
}

int testdram (void)
{
	/* TODO: XXX XXX XXX not an actual SDRAM test */
	printf ("Test: 8MB SDRAM\n");

	return (0);
}
