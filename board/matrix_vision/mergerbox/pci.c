/*
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2011 Matrix Vision GmbH
 * Andre Schwarz <andre.schwarz@matrix-vision.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc83xx.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/fsl_mpc83xx_serdes.h>
#include "mergerbox.h"
#include "fpga.h"
#include "../common/mv_common.h"

static struct pci_region pci_regions[] = {
	{
		.bus_start = CONFIG_SYS_PCI_MEM_BASE,
		.phys_start = CONFIG_SYS_PCI_MEM_PHYS,
		.size = CONFIG_SYS_PCI_MEM_SIZE,
		.flags = PCI_REGION_MEM | PCI_REGION_PREFETCH
	},
	{
		.bus_start = CONFIG_SYS_PCI_MMIO_BASE,
		.phys_start = CONFIG_SYS_PCI_MMIO_PHYS,
		.size = CONFIG_SYS_PCI_MMIO_SIZE,
		.flags = PCI_REGION_MEM
	},
	{
		.bus_start = CONFIG_SYS_PCI_IO_BASE,
		.phys_start = CONFIG_SYS_PCI_IO_PHYS,
		.size = CONFIG_SYS_PCI_IO_SIZE,
		.flags = PCI_REGION_IO
	}
};

static struct pci_region pcie_regions_0[] = {
	{
		.bus_start = CONFIG_SYS_PCIE1_MEM_BASE,
		.phys_start = CONFIG_SYS_PCIE1_MEM_PHYS,
		.size = CONFIG_SYS_PCIE1_MEM_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CONFIG_SYS_PCIE1_IO_BASE,
		.phys_start = CONFIG_SYS_PCIE1_IO_PHYS,
		.size = CONFIG_SYS_PCIE1_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

static struct pci_region pcie_regions_1[] = {
	{
		.bus_start = CONFIG_SYS_PCIE2_MEM_BASE,
		.phys_start = CONFIG_SYS_PCIE2_MEM_PHYS,
		.size = CONFIG_SYS_PCIE2_MEM_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CONFIG_SYS_PCIE2_IO_BASE,
		.phys_start = CONFIG_SYS_PCIE2_IO_PHYS,
		.size = CONFIG_SYS_PCIE2_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

void pci_init_board(void)
{
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile sysconf83xx_t *sysconf = &immr->sysconf;
	volatile clk83xx_t *clk = (clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	volatile law83xx_t *pcie_law = sysconf->pcielaw;
	struct pci_region *reg[] = { pci_regions };
	struct pci_region *pcie_reg[] = { pcie_regions_0, pcie_regions_1, };

	volatile gpio83xx_t *gpio;
	gpio = (gpio83xx_t *)&immr->gpio[0];

	gpio->dat = MV_GPIO1_DAT;
	gpio->odr = MV_GPIO1_ODE;
	gpio->dir = MV_GPIO1_OUT;

	gpio = (gpio83xx_t *)&immr->gpio[1];

	gpio->dat = MV_GPIO2_DAT;
	gpio->odr = MV_GPIO2_ODE;
	gpio->dir = MV_GPIO2_OUT;

	printf("SICRH / SICRL : 0x%08x / 0x%08x\n", immr->sysconf.sicrh,
		immr->sysconf.sicrl);

	/* Enable PCI_CLK[0:1] */
	clk->occr |= 0xc0000000;
	udelay(2000);

	mergerbox_init_fpga();
	mv_load_fpga();

	mergerbox_tft_dim(0);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CONFIG_SYS_PCI_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_512MB;

	pci_law[1].bar = CONFIG_SYS_PCI_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	udelay(2000);

	mpc83xx_pci_init(1, reg);

	/* Deassert the resets in the control register */
	out_be32(&sysconf->pecr1, 0xE0008000);
	out_be32(&sysconf->pecr2, 0xE0008000);
	udelay(2000);

	out_be32(&pcie_law[0].bar, CONFIG_SYS_PCIE1_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[0].ar, LBLAWAR_EN | LBLAWAR_512MB);

	out_be32(&pcie_law[1].bar, CONFIG_SYS_PCIE2_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[1].ar, LBLAWAR_EN | LBLAWAR_512MB);

	mpc83xx_pcie_init(2, pcie_reg);
}
