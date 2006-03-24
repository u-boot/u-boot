/*
 * Copyright (C) 2005 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Support for Embedded Planet EP88x boards.
 * Tested on EP88xC with MPC885 CPU, 64MB SDRAM and 16MB flash.
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
#include <mpc8xx.h>

/*
 * SDRAM uses two Micron chips.
 * Minimal CPU frequency is 40MHz.
 */
static uint sdram_table[] = {
	/* Single read	(offset 0x00 in UPM RAM) */
	0xEFCBCC04, 0x0F37C804, 0x0EEEC004, 0x01B98404,
	0x1FF74C00, 0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05,

	/* Burst read	(offset 0x08 in UPM RAM) */
	0xEFCBCC04, 0x0F37C804, 0x0EEEC004, 0x00BDC404,
	0x00FFCC00, 0x00FFCC00, 0x01FB8C00, 0x1FF74C00,
	0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05,
	0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05,

	/* Single write (offset 0x18 in UPM RAM) */
	0xEFCBCC04, 0x0F37C804, 0x0EEE8002, 0x01B90404,
	0x1FF74C05, 0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05,

	/* Burst write	(offset 0x20 in UPM RAM) */
	0xEFCBCC04, 0x0F37C804, 0x0EEE8000, 0x00BD4400,
	0x00FFCC00, 0x00FFCC02, 0x01FB8C04, 0x1FF74C05,
	0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05,
	0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05, 0xFFFFCC05,

	/* Refresh	(offset 0x30 in UPM RAM) */
	0xEFFACC04, 0x0FF5CC04, 0x0FFFCC04, 0x1FFFCC04,
	0xFFFFCC05, 0xFFFFCC05, 0xEFFB8C34, 0x0FF74C34,
	0x0FFACCB4, 0x0FF5CC34, 0x0FFFC034, 0x0FFFC0B4,

	/* Exception	(offset 0x3C in UPM RAM) */
	0x0FEA8034, 0x1FB54034,	0xFFFFCC34, 0xFFFFCC05
};

int board_early_init_f (void)
{
	vu_char *bcsr = (vu_char *)CFG_BCSR;

	bcsr[0] |= 0x0C; /* Turn the LEDs off */
	bcsr[2] |= 0x08; /* Enable flash WE# line - necessary for
			    flash detection by CFI driver
			 */

#if defined(CONFIG_8xx_CONS_SMC1)
	bcsr[6] |= 0x10; /* Enables RS-232 transceiver */
#endif
#if defined(CONFIG_8xx_CONS_SCC2)
	bcsr[7] |= 0x10; /* Enables RS-232 transceiver */
#endif
#ifdef CONFIG_ETHER_ON_FEC1
	bcsr[8] |= 0xC0; /* Enable Ethernet 1 PHY */
#endif
#ifdef CONFIG_ETHER_ON_FEC2
	bcsr[8] |= 0x30; /* Enable Ethernet 2 PHY */
#endif

	return 0;
}

long int initdram (int board_type)
{
	long int msize;
	volatile immap_t     *immap  = (volatile immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	upmconfig(UPMA, sdram_table, sizeof(sdram_table) / sizeof(uint));

	/* Configure SDRAM refresh */
	memctl->memc_mptpr = MPTPR_PTP_DIV2; /* BRGCLK/2 */

	memctl->memc_mamr = (65 << 24) | CFG_MAMR; /* No refresh */
	udelay(100);

	/* Run MRS pattern from location 0x36 */
	memctl->memc_mar = 0x88;
	memctl->memc_mcr = 0x80002236;
	udelay(100);

	memctl->memc_mamr |=  MAMR_PTAE; /* Enable refresh */
	memctl->memc_or1   = ~(CFG_SDRAM_MAX_SIZE - 1) | OR_CSNT_SAM;
	memctl->memc_br1   =  CFG_SDRAM_BASE | BR_PS_32 | BR_MS_UPMA | BR_V;

	msize = get_ram_size(CFG_SDRAM_BASE, CFG_SDRAM_MAX_SIZE);
	memctl->memc_or1  |= ~(msize - 1);

	return msize;
}

int checkboard( void )
{
	vu_char *bcsr = (vu_char *)CFG_BCSR;

	puts("Board: ");
	switch (bcsr[15]) {
	case 0xE7:
		puts("EP88xC 1.0");
		break;
	default:
		printf("unknown ID=%02X", bcsr[15]);
	}
	printf("  CPLD revision %d\n", bcsr[14]);

	return 0;
}
