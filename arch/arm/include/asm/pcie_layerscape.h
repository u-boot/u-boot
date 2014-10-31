/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PCIE_LAYERSCAPE_H_
#define __PCIE_LAYERSCAPE_H_

void pci_init_board(void);
void ft_pcie_setup(void *blob, bd_t *bd);

#endif
