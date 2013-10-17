/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
 *
 * (C) Copyright 2008
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif
#include <pci.h>
#include <mpc83xx.h>
#include <fpga.h>
#include "mvblm7.h"
#include "fpga.h"
#include "../common/mv_common.h"

DECLARE_GLOBAL_DATA_PTR;

static struct pci_region pci_regions[] = {
	{
		bus_start: CONFIG_SYS_PCI1_MEM_BASE,
		phys_start: CONFIG_SYS_PCI1_MEM_PHYS,
		size: CONFIG_SYS_PCI1_MEM_SIZE,
		flags: PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		bus_start: CONFIG_SYS_PCI1_MMIO_BASE,
		phys_start: CONFIG_SYS_PCI1_MMIO_PHYS,
		size: CONFIG_SYS_PCI1_MMIO_SIZE,
		flags: PCI_REGION_MEM
	},
	{
		bus_start: CONFIG_SYS_PCI1_IO_BASE,
		phys_start: CONFIG_SYS_PCI1_IO_PHYS,
		size: CONFIG_SYS_PCI1_IO_SIZE,
		flags: PCI_REGION_IO
	}
};

void pci_init_board(void)
{
	int i;
	volatile immap_t *immr;
	volatile pcictrl83xx_t *pci_ctrl;
	volatile gpio83xx_t *gpio;
	volatile clk83xx_t *clk;
	volatile law83xx_t *pci_law;
	struct pci_region *reg[] = { pci_regions };

	immr = (immap_t *) CONFIG_SYS_IMMR;
	clk = (clk83xx_t *) &immr->clk;
	pci_ctrl = immr->pci_ctrl;
	pci_law = immr->sysconf.pcilaw;
	gpio  = (volatile gpio83xx_t *)&immr->gpio[0];

	gpio->dat = MV_GPIO_DAT;
	gpio->odr = MV_GPIO_ODE;
	gpio->dir = MV_GPIO_OUT;

	printf("SICRH / SICRL : 0x%08x / 0x%08x\n", immr->sysconf.sicrh,
		immr->sysconf.sicrl);

	mvblm7_init_fpga();
	mv_load_fpga();

	gpio->dir = MV_GPIO_OUT & ~(FPGA_DIN|FPGA_CCLK);

	/* Enable PCI_CLK_OUTPUTs 0 and 1 with 1:1 clocking */
	clk->occr = 0xc0000000;

	pci_ctrl[0].gcr = 0;
	udelay(2000);
	pci_ctrl[0].gcr = 1;

	for (i = 0; i < 1000; ++i)
		udelay(1000);

	pci_law[0].bar = CONFIG_SYS_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_1GB;

	pci_law[1].bar = CONFIG_SYS_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	mpc83xx_pci_init(1, reg);
}
