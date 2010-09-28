/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 * Copyright (C) 2010 Ilya Yanok, Emcraft Systems, yanok@emcraft.com
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
 * MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <i2c.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <pci.h>
#include <mpc83xx.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_mpc83xx_serdes.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: MPC8308 P1M\n");

	return 0;
}

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

void pci_init_board(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;
	law83xx_t *pcie_law = sysconf->pcielaw;
	struct pci_region *pcie_reg[] = { pcie_regions_0 };

	fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_PEX,
					FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);

	/* Deassert the resets in the control register */
	out_be32(&sysconf->pecr1, 0xE0008000);
	udelay(2000);

	/* Configure PCI Express Local Access Windows */
	out_be32(&pcie_law[0].bar, CONFIG_SYS_PCIE1_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[0].ar, LBLAWAR_EN | LBLAWAR_512MB);

	mpc83xx_pcie_init(1, pcie_reg);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_dr_usb(blob, bd);
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv, num_if = 0;

	/* Initialize TSECs first */
	rv = cpu_eth_init(bis);
	if (rv >= 0)
		num_if += rv;
	else
		printf("ERROR: failed to initialize TSECs.\n");

	rv = pci_eth_init(bis);
	if (rv >= 0)
		num_if += rv;
	else
		printf("ERROR: failed to initialize PCI Ethernet.\n");

	return num_if;
}
