// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 * Copyright 2007-2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <init.h>
#include <pci.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_pci_setup(void *blob, struct bd_info *bd)
{
	FT_FSL_PCI_SETUP;
}
#endif /* CONFIG_OF_BOARD_SETUP */
