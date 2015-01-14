/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/pcie_layerscape.h>

#ifdef CONFIG_OF_BOARD_SETUP
#include <libfdt.h>
#include <fdt_support.h>

static void ft_pcie_ls_setup(void *blob, const char *pci_compat,
			     unsigned long ctrl_addr, enum srds_prtcl dev)
{
	int off;

	off = fdt_node_offset_by_compat_reg(blob, pci_compat,
					    (phys_addr_t)ctrl_addr);
	if (off < 0)
		return;

	if (!is_serdes_configured(dev))
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);
}

void ft_pcie_setup(void *blob, bd_t *bd)
{
	#ifdef CONFIG_PCIE1
	ft_pcie_ls_setup(blob, FSL_PCIE_COMPAT, CONFIG_SYS_PCIE1_ADDR, PCIE1);
	#endif

	#ifdef CONFIG_PCIE2
	ft_pcie_ls_setup(blob, FSL_PCIE_COMPAT, CONFIG_SYS_PCIE2_ADDR, PCIE2);
	#endif
}

#else
void ft_pcie_setup(void *blob, bd_t *bd)
{
}
#endif

void pci_init_board(void)
{
}
