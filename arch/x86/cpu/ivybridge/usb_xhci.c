/*
 * From Coreboot
 * Copyright (C) 2008-2009 coresystems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>

void bd82x6x_usb_xhci_init(pci_dev_t dev)
{
	u32 reg32;

	debug("XHCI: Setting up controller.. ");

	/* lock overcurrent map */
	reg32 = x86_pci_read_config32(dev, 0x44);
	reg32 |= 1;
	x86_pci_write_config32(dev, 0x44, reg32);

	/* Enable clock gating */
	reg32 = x86_pci_read_config32(dev, 0x40);
	reg32 &= ~((1 << 20) | (1 << 21));
	reg32 |= (1 << 19) | (1 << 18) | (1 << 17);
	reg32 |= (1 << 10) | (1 << 9) | (1 << 8);
	reg32 |= (1 << 31); /* lock */
	x86_pci_write_config32(dev, 0x40, reg32);

	debug("done.\n");
}
