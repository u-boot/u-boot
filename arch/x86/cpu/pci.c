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
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <asm/pci.h>

static struct pci_controller x86_hose;

int pci_early_init_hose(struct pci_controller **hosep)
{
	struct pci_controller *hose;

	hose = calloc(1, sizeof(struct pci_controller));
	if (!hose)
		return -ENOMEM;

	board_pci_setup_hose(hose);
	pci_setup_type1(hose);
	gd->arch.hose = hose;
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
	gd->arch.hose = NULL;

	board_pci_setup_hose(hose);
	pci_setup_type1(hose);
	pci_register_hose(hose);

	board_pci_pre_scan(hose);
	hose->last_busno = pci_hose_scan(hose);
	board_pci_post_scan(hose);
}

static struct pci_controller *get_hose(void)
{
	if (gd->arch.hose)
		return gd->arch.hose;

	return pci_bus_to_hose(0);
}

unsigned int pci_read_config8(pci_dev_t dev, unsigned where)
{
	uint8_t value;

	pci_hose_read_config_byte(get_hose(), dev, where, &value);

	return value;
}

unsigned int pci_read_config16(pci_dev_t dev, unsigned where)
{
	uint16_t value;

	pci_hose_read_config_word(get_hose(), dev, where, &value);

	return value;
}

unsigned int pci_read_config32(pci_dev_t dev, unsigned where)
{
	uint32_t value;

	pci_hose_read_config_dword(get_hose(), dev, where, &value);

	return value;
}

void pci_write_config8(pci_dev_t dev, unsigned where, unsigned value)
{
	pci_hose_write_config_byte(get_hose(), dev, where, value);
}

void pci_write_config16(pci_dev_t dev, unsigned where, unsigned value)
{
	pci_hose_write_config_word(get_hose(), dev, where, value);
}

void pci_write_config32(pci_dev_t dev, unsigned where, unsigned value)
{
	pci_hose_write_config_dword(get_hose(), dev, where, value);
}
