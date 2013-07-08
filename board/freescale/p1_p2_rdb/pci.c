/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <asm/fsl_pci.h>
#include <libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}

void ft_pci_board_setup(void *blob)
{
	FT_FSL_PCI_SETUP;
}
