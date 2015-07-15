/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PCI_I386_H_
#define _PCI_I386_H_

#include <pci.h>

/* bus mapping constants (used for PCI core initialization) */
#define PCI_REG_ADDR	0xcf8
#define PCI_REG_DATA	0xcfc

#define PCI_CFG_EN	0x80000000

#ifndef __ASSEMBLY__

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

/**
 * pci_early_init_hose() - Set up PCI host before relocation
 *
 * This allocates memory for, sets up and returns the PCI hose. It can be
 * called before relocation. The hose will be stored in gd->hose for
 * later use, but will become invalid one DRAM is available.
 */
int pci_early_init_hose(struct pci_controller **hosep);

int board_pci_pre_scan(struct pci_controller *hose);
int board_pci_post_scan(struct pci_controller *hose);

/*
 * Simple PCI access routines - these work from either the early PCI hose
 * or the 'real' one, created after U-Boot has memory available
 */
unsigned int x86_pci_read_config8(pci_dev_t dev, unsigned where);
unsigned int x86_pci_read_config16(pci_dev_t dev, unsigned where);
unsigned int x86_pci_read_config32(pci_dev_t dev, unsigned where);

void x86_pci_write_config8(pci_dev_t dev, unsigned where, unsigned value);
void x86_pci_write_config16(pci_dev_t dev, unsigned where, unsigned value);
void x86_pci_write_config32(pci_dev_t dev, unsigned where, unsigned value);

int pci_x86_read_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			ulong *valuep, enum pci_size_t size);

int pci_x86_write_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			 ulong value, enum pci_size_t size);

/**
 * Assign IRQ number to a PCI device
 *
 * This function assigns IRQ for a PCI device. If the device does not exist
 * or does not require interrupts then this function has no effect.
 *
 * @bus:	PCI bus number
 * @device:	PCI device number
 * @irq:	An array of IRQ numbers that are assigned to INTA through
 *		INTD of this PCI device.
 */
void pci_assign_irqs(int bus, int device, u8 irq[4]);

#endif /* __ASSEMBLY__ */

#endif /* _PCI_I386_H_ */
