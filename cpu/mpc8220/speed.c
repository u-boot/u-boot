/*
 * (C) Copyright 2004, Freescale, Inc
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com.
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
#include <mpc8220.h>
#include <asm/processor.h>

typedef struct pllmultiplier {
	u8 hid1;
	int multi;
	int vco_div;
} pllcfg_t;

/* ------------------------------------------------------------------------- */

/*
 *
 */

int get_clocks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	pllcfg_t bus2core[] = {
		{0x10, 2, 8},	/* 1 */
		{0x08, 2, 4},
		{0x60, 3, 8},	/* 1.5 */
		{0x00, 3, 4},
		{0xc0, 3, 2},
		{0x28, 4, 4},	/* 2 */
		{0x20, 4, 2},
		{0x88, 5, 4},	/* 2.5 */
		{0x30, 5, 2},
		{0x80, 6, 4},	/* 3 */
		{0x40, 6, 2},
		{0x70, 7, 2},	/* 3.5 */
		{0x50, 8, 2},	/* 4 */
		{0x38, 9, 2},	/* 4.5 */
		{0x58, 10, 2},	/* 5 */
		{0x48, 11, 2},	/* 5.5 */
		{0x68, 12, 2},	/* 6 */
		{0x90, 13, 2},	/* 6.5 */
		{0xa0, 14, 2},	/* 7 */
		{0xb0, 15, 2},	/* 7.5 */
		{0xe0, 16, 2}	/* 8 */
	};
	u32 hid1;
	int i, size;

#if !defined(CFG_MPC8220_CLKIN)
#error clock measuring not implemented yet - define CFG_MPC8220_CLKIN
#endif

	gd->inp_clk = CFG_MPC8220_CLKIN;

	/* Bus clock is fixed at 120Mhz for now */
	/* will do dynamic in the future */
	gd->bus_clk = CFG_MPC8220_CLKIN * 4;

	/* PCI clock is same as input clock */
	gd->pci_clk = CFG_MPC8220_CLKIN;

	/* FlexBus is temporary set as the same as input clock */
	/* will do dynamic in the future */
	gd->flb_clk = CFG_MPC8220_CLKIN;

	/* CPU Clock - Read HID1 */
	asm volatile ("mfspr %0, 1009":"=r" (hid1):);

	size = sizeof (bus2core) / sizeof (pllcfg_t);
	hid1 >>= 24;

	for (i = 0; i < size; i++)
		if (hid1 == bus2core[i].hid1) {
			gd->cpu_clk = (bus2core[i].multi * gd->bus_clk) >> 1;
			/* Input Multiplier is determined by MPLL,
			   hardcoded for now at 16 */
			gd->vco_clk = gd->pci_clk * 16;
			break;
		}

	/* hardcoded 81MHz for now */
	gd->pev_clk = 81000000;

	return (0);
}

int prt_mpc8220_clks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	printf ("       Bus %ld MHz, CPU %ld MHz, PCI %ld MHz, VCO %ld MHz\n",
		gd->bus_clk / 1000000, gd->cpu_clk / 1000000,
		gd->pci_clk / 1000000, gd->vco_clk / 1000000);

	return (0);
}

/* ------------------------------------------------------------------------- */
