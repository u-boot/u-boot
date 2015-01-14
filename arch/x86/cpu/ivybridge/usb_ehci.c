/*
 * From Coreboot
 * Copyright (C) 2008-2009 coresystems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>

void bd82x6x_usb_ehci_init(pci_dev_t dev)
{
	u32 reg32;

	/* Disable Wake on Disconnect in RMH */
	reg32 = readl(RCB_REG(0x35b0));
	reg32 |= 0x22;
	writel(reg32, RCB_REG(0x35b0));

	debug("EHCI: Setting up controller.. ");
	reg32 = pci_read_config32(dev, PCI_COMMAND);
	reg32 |= PCI_COMMAND_MASTER;
	/* reg32 |= PCI_COMMAND_SERR; */
	pci_write_config32(dev, PCI_COMMAND, reg32);

	debug("done.\n");
}
