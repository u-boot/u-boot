
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

void pci_setup_type1(struct pci_controller *hose);
#endif
