/*
 * (C) Copyright 2000-2002
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
#include <mpc5xxx.h>
#include <asm/processor.h>

/* ------------------------------------------------------------------------- */

/* Bus-to-Core Multipliers */

static int bus2core[] = {
	3, 2, 2, 2, 4, 4, 5, 9,
	6, 11, 8, 10, 3, 12, 7, 0,
	6, 5, 13, 2, 14, 4, 15, 9,
	0, 11, 8, 10, 16, 12, 7, 0
};
/* ------------------------------------------------------------------------- */

/*
 *
 */

int get_clocks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong val, vco;

#if !defined(CFG_MPC5XXX_CLKIN)
#error clock measuring not implemented yet - define CFG_MPC5XXX_CLKIN
#endif

	val = *(vu_long *)MPC5XXX_CDM_PORCFG;
	if (val & (1 << 6)) {
		vco = CFG_MPC5XXX_CLKIN * 12;
	} else {
		vco = CFG_MPC5XXX_CLKIN * 16;
	}
	if (val & (1 << 5)) {
		gd->bus_clk = vco / 8;
	} else {
		gd->bus_clk = vco / 4;
	}
	gd->cpu_clk = gd->bus_clk * bus2core[val & 0x1f] / 2;

	val = *(vu_long *)MPC5XXX_CDM_CFG;
	if (val & (1 << 8)) {
		gd->ipb_clk = gd->bus_clk / 2;
	} else {
		gd->ipb_clk = gd->bus_clk;
	}
	switch (val & 3) {
		case 0: gd->pci_clk = gd->ipb_clk; break;
		case 1: gd->pci_clk = gd->ipb_clk / 2; break;
		default: gd->pci_clk = gd->bus_clk / 4; break;
	}

	return (0);
}

int prt_mpc5xxx_clks (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	printf("       Bus %ld MHz, IPB %ld MHz, PCI %ld MHz\n",
			gd->bus_clk / 1000000, gd->ipb_clk / 1000000,
			gd->pci_clk / 1000000);

	return (0);
}

/* ------------------------------------------------------------------------- */
