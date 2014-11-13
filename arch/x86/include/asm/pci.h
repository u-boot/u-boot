
/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PCI_I386_H_
#define _PCI_I386_H_

#define DEFINE_PCI_DEVICE_TABLE(_table) \
	const struct pci_device_id _table[]

struct pci_controller;

void pci_setup_type1(struct pci_controller *hose);

/**
 * board_pci_setup_hose() - Set up the PCI hose
 *
 * This is called by the common x86 PCI code to set up the PCI controller
 * hose. It may be called when no memory/BSS is available so should just
 * store things in 'hose' and not in BSS variables.
 */
void board_pci_setup_hose(struct pci_controller *hose);
#endif
