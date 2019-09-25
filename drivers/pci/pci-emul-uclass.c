// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <pci.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>

struct sandbox_pci_emul_priv {
	int dev_count;
};

int sandbox_pci_get_emul(struct udevice *bus, pci_dev_t find_devfn,
			 struct udevice **containerp, struct udevice **emulp)
{
	struct udevice *dev;
	int ret;

	*containerp = NULL;
	ret = pci_bus_find_devfn(bus, PCI_MASK_BUS(find_devfn), &dev);
	if (ret) {
		debug("%s: Could not find emulator for dev %x\n", __func__,
		      find_devfn);
		return ret;
	}
	*containerp = dev;

	/*
	 * See commit 4345998ae9df,
	 * "pci: sandbox: Support dynamically binding device driver"
	 */
	ret = uclass_find_device_by_phandle(UCLASS_PCI_EMUL, dev,
					    "sandbox,emul", emulp);
	if (ret && device_get_uclass_id(dev) != UCLASS_PCI_GENERIC)
		*emulp = dev;

	return *emulp ? 0 : -ENODEV;
}

static int sandbox_pci_emul_post_probe(struct udevice *dev)
{
	struct sandbox_pci_emul_priv *priv = dev->uclass->priv;

	priv->dev_count++;
	sandbox_set_enable_pci_map(true);

	return 0;
}

static int sandbox_pci_emul_pre_remove(struct udevice *dev)
{
	struct sandbox_pci_emul_priv *priv = dev->uclass->priv;

	priv->dev_count--;
	sandbox_set_enable_pci_map(priv->dev_count > 0);

	return 0;
}

UCLASS_DRIVER(pci_emul) = {
	.id		= UCLASS_PCI_EMUL,
	.name		= "pci_emul",
	.post_probe	= sandbox_pci_emul_post_probe,
	.pre_remove	= sandbox_pci_emul_pre_remove,
	.priv_auto_alloc_size	= sizeof(struct sandbox_pci_emul_priv),
};

/*
 * This uclass is a child of the pci bus. Its platdata is not defined here so
 * is defined by its parent, UCLASS_PCI, which uses struct pci_child_platdata.
 * See per_child_platdata_auto_alloc_size in UCLASS_DRIVER(pci).
 */
UCLASS_DRIVER(pci_emul_parent) = {
	.id		= UCLASS_PCI_EMUL_PARENT,
	.name		= "pci_emul_parent",
	.post_bind	= dm_scan_fdt_dev,
};

static const struct udevice_id pci_emul_parent_ids[] = {
	{ .compatible = "sandbox,pci-emul-parent" },
	{ }
};

U_BOOT_DRIVER(pci_emul_parent_drv) = {
	.name		= "pci_emul_parent_drv",
	.id		= UCLASS_PCI_EMUL_PARENT,
	.of_match	= pci_emul_parent_ids,
};
