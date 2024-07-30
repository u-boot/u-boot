// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <vsprintf.h>
#include <dm/device.h>
#include <dm/ofnode.h>
#include <dm/read.h>
#include <dm/util.h>
#include <linux/libfdt.h>
#include <linux/list.h>

#if CONFIG_IS_ENABLED(OF_REAL)
int pci_get_devfn(struct udevice *dev)
{
	struct fdt_pci_addr addr;
	int ret;

	/* Extract the devfn from fdt_pci_addr */
	ret = ofnode_read_pci_addr(dev_ofnode(dev), FDT_PCI_SPACE_CONFIG,
				   "reg", &addr, NULL);
	if (ret) {
		if (ret != -ENOENT)
			return -EINVAL;
	}

	return addr.phys_hi & 0xff00;
}
#endif
