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
		{0x02, 2, 8},	/* 1 */
		{0x01, 2, 4},
		{0x0C, 3, 8},	/* 1.5 */
		{0x00, 3, 4},
		{0x18, 3, 2},
		{0x05, 4, 4},	/* 2 */
		{0x04, 4, 2},
		{0x11, 5, 4},	/* 2.5 */
		{0x06, 5, 2},
		{0x10, 6, 4},	/* 3 */
		{0x08, 6, 2},
		{0x0E, 7, 2},	/* 3.5 */
		{0x0A, 8, 2},	/* 4 */
		{0x07, 9, 2},	/* 4.5 */
		{0x0B, 10, 2},	/* 5 */
		{0x09, 11, 2},	/* 5.5 */
		{0x0D, 12, 2},	/* 6 */
		{0x12, 13, 2},	/* 6.5 */
		{0x14, 14, 2},	/* 7 */
		{0x16, 15, 2},	/* 7.5 */
		{0x1C, 16, 2}	/* 8 */
	};
	u32 hid1;
	int i, size, pci2bus;

#if !defined(CFG_MPC8220_CLKIN)
#error clock measuring not implemented yet - define CFG_MPC8220_CLKIN
#endif

	gd->inp_clk = CFG_MPC8220_CLKIN;

	/* Read XLB to PCI(INP) clock multiplier */
	pci2bus = (*((volatile u32 *)PCI_REG_PCIGSCR) &
		PCI_REG_PCIGSCR_PCI2XLB_CLK_MASK)>>PCI_REG_PCIGSCR_PCI2XLB_CLK_BIT;

	/* XLB bus clock */
	gd->bus_clk = CFG_MPC8220_CLKIN * pci2bus;

	/* PCI clock is same as input clock */
	gd->pci_clk = CFG_MPC8220_CLKIN;

	/* FlexBus is temporary set as the same as input clock */
	/* will do dynamic in the future */
	gd->flb_clk = CFG_MPC8220_CLKIN;

	/* CPU Clock - Read HID1 */
	asm volatile ("mfspr %0, 1009":"=r" (hid1):);

	size = sizeof (bus2core) / sizeof (pllcfg_t);

	hid1 >>= 27;

	for (i = 0; i < size; i++)
		if (hid1 == bus2core[i].hid1) {
			gd->cpu_clk = (bus2core[i].multi * gd->bus_clk) >> 1;
			gd->vco_clk = CFG_MPC8220_SYSPLL_VCO_MULTIPLIER * (gd->pci_clk * bus2core[i].vco_div)/2;
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
