/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __T1024_RDB_H__
#define __T1024_RDB_H__

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, bd_t *bd);

#endif
