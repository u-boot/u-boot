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
#include <pci.h>
#include <asm/pci.h>

static struct pci_controller x86_hose;

void pci_init_board(void)
{
	struct pci_controller *hose = &x86_hose;

	board_pci_setup_hose(hose);
	pci_setup_type1(hose);
	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);
}
