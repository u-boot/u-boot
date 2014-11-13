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

void pci_init_board(void)
{
	struct pci_controller *hose = &x86_hose;

	/* Stop using the early hose */
	gd->arch.hose = NULL;

	board_pci_setup_hose(hose);
	pci_setup_type1(hose);
	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);
}
