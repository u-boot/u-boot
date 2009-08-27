/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
 *
 * (C) Copyright 2008
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
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
	int warmboot;
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

	warmboot = gd->bd->bi_bootflags & BOOTFLAG_WARM;

	mpc83xx_pci_init(1, reg, warmboot);
}
