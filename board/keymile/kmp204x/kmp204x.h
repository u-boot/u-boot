/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define PRSTCFG_POWUP_UNIT_CORE_RST	0x0
#define PRSTCFG_POWUP_UNIT_RST		0x1
#define PRSTCFG_POWUP_RST		0x3

void qrio_prst(u8 bit, bool en, bool wden);
void qrio_prstcfg(u8 bit, u8 mode);

void pci_of_setup(void *blob, bd_t *bd);
