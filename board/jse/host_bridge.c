/*
 * Copyright (c) 2004 Picture Elements, Inc.
 *    Stephen Williams (steve@icarus.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ident "$Id:$"

# include  <common.h>
# include  <pci.h>
# include  "jse_priv.h"

/*
 * The JSE board has an Intel 21555 non-transparent bridge for
 * communication with the host. We need to render it harmless on the
 * JSE side, but leave it alone on the host (primary) side. Normally,
 * this will all be done before the host BIOS can gain access to the
 * board, due to the Primary Access Lockout bit.
 *
 * The host_bridge_init function is called as a late initialization
 * function, after most of the board is set up, including a PCI scan.
 */

void host_bridge_init (void)
{
	/* The bridge chip is at a fixed location. */
	pci_dev_t dev = PCI_BDF (0, 10, 0);

	/* Set PCI Class code --
	   The primary side sees this class code at 0x08 in the
	   primary config space. This must be something other then a
	   bridge, or MS Windows starts doing weird stuff to me. */
	pci_write_config_dword (dev, 0x48, 0x04800000);

	/* Set subsystem ID --
	   The primary side sees this value at 0x2c. We set it here so
	   that the host can tell what sort of device this is:
	   We are a Picture Elements [0x12c5] JSE [0x008a]. */
	pci_write_config_dword (dev, 0x6c, 0x008a12c5);

	/* Downstream (Primary-to-Secondary) BARs are set up mostly
	   off. We need only the Memory-0 Bar so that the host can get
	   at the CSR region to set up tables and the lot. */

	/* Downstream Memory 0 setup (4K for CSR) */
	pci_write_config_dword (dev, 0xac, 0xfffff000);
	/* Downstream Memory 1 setup (off) */
	pci_write_config_dword (dev, 0xb0, 0x00000000);
	/* Downstream Memory 2 setup (off) */
	pci_write_config_dword (dev, 0xb4, 0x00000000);
	/* Downstream Memory 3 setup (off) */
	pci_write_config_dword (dev, 0xb8, 0x00000000);

	/* Upstream (Secondary-to-Primary) BARs are used to get at
	   host memory from the JSE card. Create two regions: a small
	   one to manage individual word reads/writes, and a larger
	   one for doing bulk frame moves. */

	/* Upstream Memory 0 Setup -- (BAR2) 4K non-prefetchable */
	pci_write_config_dword (dev, 0xc4, 0xfffff000);
	/* Upstream Memory 1 setup -- (BAR3) 4K non-prefetchable */
	pci_write_config_dword (dev, 0xc8, 0xfffff000);

	/* Upstream Memory 2 (BAR4) uses page translation, and is set
	   up in CCR1. Configure for 4K pages. */

	/* Set CCR1,0 reigsters. This clears the Primary PCI Lockout
	   bit as well, so we are done configuring after this
	   point. Therefore, this must be the last step.

	   CC1[15:12]= 0  (disable I2O message unit)
	   CC1[11:8] = 0x5 (4K page size)
	   CC0[11]   = 1  (Secondary Clock Disable: disable clock)
	   CC0[10]   = 0  (Primary Access Lockout: allow primary access)
	 */
	pci_write_config_dword (dev, 0xcc, 0x05000800);
}
