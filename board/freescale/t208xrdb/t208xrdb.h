/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#ifndef __CORENET_DS_H__
#define __CORENET_DS_H__

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, struct bd_info *bd);
void fdt_fixup_board_fman_ethernet(void *blob);
void fdt_fixup_board_phy(void *blob);

#endif
