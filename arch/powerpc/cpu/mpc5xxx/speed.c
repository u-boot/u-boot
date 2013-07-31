/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

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
	ulong val, vco;

#if !defined(CONFIG_SYS_MPC5XXX_CLKIN)
#error clock measuring not implemented yet - define CONFIG_SYS_MPC5XXX_CLKIN
#endif

	val = *(vu_long *)MPC5XXX_CDM_PORCFG;
	if (val & (1 << 6)) {
		vco = CONFIG_SYS_MPC5XXX_CLKIN * 12;
	} else {
		vco = CONFIG_SYS_MPC5XXX_CLKIN * 16;
	}
	if (val & (1 << 5)) {
		gd->bus_clk = vco / 8;
	} else {
		gd->bus_clk = vco / 4;
	}
	gd->cpu_clk = gd->bus_clk * bus2core[val & 0x1f] / 2;

	val = *(vu_long *)MPC5XXX_CDM_CFG;
	if (val & (1 << 8)) {
		gd->arch.ipb_clk = gd->bus_clk / 2;
	} else {
		gd->arch.ipb_clk = gd->bus_clk;
	}
	switch (val & 3) {
	case 0:
		gd->pci_clk = gd->arch.ipb_clk;
		break;
	case 1:
		gd->pci_clk = gd->arch.ipb_clk / 2;
		break;
	default:
		gd->pci_clk = gd->bus_clk / 4;
		break;
	}

	return (0);
}

int prt_mpc5xxx_clks (void)
{
	char buf1[32], buf2[32], buf3[32];

	printf ("       Bus %s MHz, IPB %s MHz, PCI %s MHz\n",
		strmhz(buf1, gd->bus_clk),
		strmhz(buf2, gd->arch.ipb_clk),
		strmhz(buf3, gd->pci_clk)
	);
	return (0);
}

/* ------------------------------------------------------------------------- */
