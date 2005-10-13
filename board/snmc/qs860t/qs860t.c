/*
 * (C) Copyright 2003
 * MuLogic B.V.
 *
 * (C) Copyright 2002
 * Simple Network Magic Corporation, dnevil@snmc.com
 *
 * (C) Copyright 2000
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
#include <asm/u-boot.h>
#include <commproc.h>
#include "mpc8xx.h"

/* ------------------------------------------------------------------------- */

static long  int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x11ADFC04, 0xEFBBBC00,
	0x1FF77C47, 0x1FF77C35, 0xEFEABC34, 0x1FB57C35,
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x10ADFC04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBBBC00, 0x1FF77C47,
	0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04,
	0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F27FC04, 0xEEAEBC00, 0x01B93C04, 0x1FF77C47,
	0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x10AD7C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BBBC04, 0x1FF77C47, 0xFFFFEC04,
	0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04,
	0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FF5FC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, 0xFFFFEC04, 0xFFFFEC04,
	0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07, 0xFFFFEC04, 0xFFFFEC04, 0xFFFFEC04
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Test ID string (QS860T...)
 *
 * Always return 1
 */

int checkboard (void)
{
	char *s, *e;
	char buf[64];
	int i;

	i = getenv_r("serial#", buf, sizeof(buf));
	s = (i>0) ? buf : NULL;

	if (!s || strncmp(s, "QS860T", 6)) {
		puts ("### No HW ID - assuming QS860T");
	} else {
		for (e=s; *e; ++e) {
			if (*e == ' ')
			break;
		}

		for ( ; s<e; ++s) {
			putc (*s);
		}
	}
	putc ('\n');

	return (0);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size;

	upmconfig(UPMB, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

	/*
	* Prescaler for refresh
	*/
	memctl->memc_mptpr = 0x0400;

	/*
	* Map controller bank 2 to the SDRAM address
	*/
	memctl->memc_or2 = CFG_OR2;
	memctl->memc_br2 = CFG_BR2;
	udelay(200);

	/* perform SDRAM initialization sequence */
	memctl->memc_mbmr = CFG_16M_MBMR;
	udelay(100);

	memctl->memc_mar  = 0x00000088;
	memctl->memc_mcr  = 0x80804105;	/* run precharge pattern */
	udelay(1);

	/* Run two refresh cycles on SDRAM */
	memctl->memc_mbmr = 0x18802118;
	memctl->memc_mcr  = 0x80804130;
	memctl->memc_mbmr = 0x18802114;
	memctl->memc_mcr  = 0x80804106;

	udelay (1000);

#if 0
	/*
	* Check for 64M SDRAM Memory Size
	*/
	size = dram_size (CFG_64M_MBMR, (ulong *)SDRAM_BASE, SDRAM_64M_MAX_SIZE);
	udelay (1000);

	/*
	* Check for 16M SDRAM Memory Size
	*/
	if (size != SDRAM_64M_MAX_SIZE) {
#endif
	size = dram_size (CFG_16M_MBMR, (long *)SDRAM_BASE, SDRAM_16M_MAX_SIZE);
	udelay (1000);
#if 0
	}

	memctl->memc_or2 = ((-size) & 0xFFFF0000) | SDRAM_TIMING;
#endif


	udelay(10000);


#if 0

	/*
	* Also, map other memory to correct position
	*/

	/*
	* Map the 8M Intel Flash device to chip select 1
	*/
	memctl->memc_or1 = CFG_OR1;
	memctl->memc_br1 = CFG_BR1;


	/*
	* Map 64K NVRAM, Sipex Device, NAND Ctl Reg, and LED Ctl Reg
	* to chip select 3
	*/
	memctl->memc_or3 = CFG_OR3;
	memctl->memc_br3 = CFG_BR3;

	/*
	* Map chip selects 4, 5, 6, & 7 for external expansion connector
	*/
	memctl->memc_or4 = CFG_OR4;
	memctl->memc_br4 = CFG_BR4;

	memctl->memc_or5 = CFG_OR5;
	memctl->memc_br5 = CFG_BR5;

	memctl->memc_or6 = CFG_OR6;
	memctl->memc_br6 = CFG_BR6;

	memctl->memc_or7 = CFG_OR7;
	memctl->memc_br7 = CFG_BR7;

#endif

	return (size);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mbmr_value, long int *base, long int maxsize)
{
	volatile immap_t *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mbmr = mbmr_value;

	return (get_ram_size(base, maxsize));
}
