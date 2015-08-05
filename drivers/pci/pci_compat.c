/*
 * Compatibility functions for pre-driver-model code
 *
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUG
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

#define PCI_HOSE_OP(rw, name, size, type)				\
int pci_hose_##rw##_config_##name(struct pci_controller *hose,		\
				  pci_dev_t dev,			\
				  int offset, type value)		\
{									\
	return pci_##rw##_config##size(dev, offset, value);		\
}

PCI_HOSE_OP(read, byte, 8, u8 *)
PCI_HOSE_OP(read, word, 16, u16 *)
PCI_HOSE_OP(read, dword, 32, u32 *)
PCI_HOSE_OP(write, byte, 8, u8)
PCI_HOSE_OP(write, word, 16, u16)
PCI_HOSE_OP(write, dword, 32, u32)

pci_dev_t pci_find_devices(struct pci_device_id *ids, int index)
{
	struct pci_child_platdata *pplat;
	struct udevice *bus, *dev;

	if (pci_find_device_id(ids, index, &dev))
		return -1;
	bus = dev->parent;
	pplat = dev_get_parent_platdata(dev);

	return PCI_ADD_BUS(bus->seq, pplat->devfn);
}
