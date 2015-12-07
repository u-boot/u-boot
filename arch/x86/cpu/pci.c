/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2008,2009
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/pci.h>

DECLARE_GLOBAL_DATA_PTR;

static struct pci_controller x86_hose;

int pci_early_init_hose(struct pci_controller **hosep)
{
	struct pci_controller *hose;

	hose = calloc(1, sizeof(struct pci_controller));
	if (!hose)
		return -ENOMEM;

	board_pci_setup_hose(hose);
	pci_setup_type1(hose);
	hose->last_busno = pci_hose_scan(hose);
	gd->hose = hose;
	*hosep = hose;

	return 0;
}

__weak int board_pci_pre_scan(struct pci_controller *hose)
{
	return 0;
}

__weak int board_pci_post_scan(struct pci_controller *hose)
{
	return 0;
}

void pci_init_board(void)
{
	struct pci_controller *hose = &x86_hose;

	/* Stop using the early hose */
	gd->hose = NULL;

	board_pci_setup_hose(hose);
	pci_setup_type1(hose);
	pci_register_hose(hose);

	board_pci_pre_scan(hose);
	hose->last_busno = pci_hose_scan(hose);
	board_pci_post_scan(hose);
}

static struct pci_controller *get_hose(void)
{
	if (gd->hose)
		return gd->hose;

	return pci_bus_to_hose(0);
}

unsigned int x86_pci_read_config8(pci_dev_t dev, unsigned where)
{
	uint8_t value;

	if (pci_hose_read_config_byte(get_hose(), dev, where, &value))
		return -1U;

	return value;
}

unsigned int x86_pci_read_config16(pci_dev_t dev, unsigned where)
{
	uint16_t value;

	if (pci_hose_read_config_word(get_hose(), dev, where, &value))
		return -1U;

	return value;
}

unsigned int x86_pci_read_config32(pci_dev_t dev, unsigned where)
{
	uint32_t value;

	if (pci_hose_read_config_dword(get_hose(), dev, where, &value))
		return -1U;

	return value;
}

void x86_pci_write_config8(pci_dev_t dev, unsigned where, unsigned value)
{
	pci_hose_write_config_byte(get_hose(), dev, where, value);
}

void x86_pci_write_config16(pci_dev_t dev, unsigned where, unsigned value)
{
	pci_hose_write_config_word(get_hose(), dev, where, value);
}

void x86_pci_write_config32(pci_dev_t dev, unsigned where, unsigned value)
{
	pci_hose_write_config_dword(get_hose(), dev, where, value);
}

int pci_x86_read_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			ulong *valuep, enum pci_size_t size)
{
	outl(bdf | (offset & 0xfc) | PCI_CFG_EN, PCI_REG_ADDR);
	switch (size) {
	case PCI_SIZE_8:
		*valuep = inb(PCI_REG_DATA + (offset & 3));
		break;
	case PCI_SIZE_16:
		*valuep = inw(PCI_REG_DATA + (offset & 2));
		break;
	case PCI_SIZE_32:
		*valuep = inl(PCI_REG_DATA);
		break;
	}

	return 0;
}

int pci_x86_write_config(struct udevice *bus, pci_dev_t bdf, uint offset,
			 ulong value, enum pci_size_t size)
{
	outl(bdf | (offset & 0xfc) | PCI_CFG_EN, PCI_REG_ADDR);
	switch (size) {
	case PCI_SIZE_8:
		outb(value, PCI_REG_DATA + (offset & 3));
		break;
	case PCI_SIZE_16:
		outw(value, PCI_REG_DATA + (offset & 2));
		break;
	case PCI_SIZE_32:
		outl(value, PCI_REG_DATA);
		break;
	}

	return 0;
}

void pci_assign_irqs(int bus, int device, u8 irq[4])
{
	pci_dev_t bdf;
	int func;
	u16 vendor;
	u8 pin, line;

	for (func = 0; func < 8; func++) {
		bdf = PCI_BDF(bus, device, func);
		vendor = x86_pci_read_config16(bdf, PCI_VENDOR_ID);
		if (vendor == 0xffff || vendor == 0x0000)
			continue;

		pin = x86_pci_read_config8(bdf, PCI_INTERRUPT_PIN);

		/* PCI spec says all values except 1..4 are reserved */
		if ((pin < 1) || (pin > 4))
			continue;

		line = irq[pin - 1];
		if (!line)
			continue;

		debug("Assigning IRQ %d to PCI device %d.%x.%d (INT%c)\n",
		      line, bus, device, func, 'A' + pin - 1);

		x86_pci_write_config8(bdf, PCI_INTERRUPT_LINE, line);
	}
}
