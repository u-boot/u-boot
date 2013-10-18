/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 *
 * Copyright 2007-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/fsl_serdes.h>

#include "kmp204x.h"

#define PCIE_SW_RST	14
#define HOOPER_SW_RST	12

void pci_init_board(void)
{
	qrio_prst(PCIE_SW_RST, false, false);
	qrio_prst(HOOPER_SW_RST, false, false);
	/* Hooper is not direcly PCIe capable */
	mdelay(50);
	fsl_pcie_init_board(0);
}

void pci_of_setup(void *blob, bd_t *bd)
{
	FT_FSL_PCI_SETUP;
}
