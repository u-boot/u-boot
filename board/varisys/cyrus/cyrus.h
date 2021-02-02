/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __CYRUS_H
#define __CYRUS_H

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, struct bd_info *bd);

#endif
