/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <mpc83xx.h>
#include <pci.h>

#if defined(CONFIG_PCI)
static struct pci_region pci_regions[] = {
	{
		bus_start: CFG_PCI_MEM_BASE,
		phys_start: CFG_PCI_MEM_PHYS,
		size: CFG_PCI_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CFG_PCI_MMIO_BASE,
		phys_start: CFG_PCI_MMIO_PHYS,
		size: CFG_PCI_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
	{
		bus_start: CFG_PCI_IO_BASE,
		phys_start: CFG_PCI_IO_PHYS,
		size: CFG_PCI_IO_SIZE,
		flags: PCI_REGION_IO
	}
};

void pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CFG_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	struct pci_region *reg[] = { pci_regions };

	/* Enable all 5 PCI_CLK_OUTPUTS */
	clk->occr |= 0xf8000000;
	udelay(2000);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CFG_PCI_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_512MB;

	pci_law[1].bar = CFG_PCI_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	mpc83xx_pci_init(1, reg, 0);
}
#endif	/* CONFIG_PCI */
