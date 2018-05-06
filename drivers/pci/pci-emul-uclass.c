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

struct sandbox_pci_priv {
	int dev_count;
};

int sandbox_pci_get_emul(struct udevice *bus, pci_dev_t find_devfn,
			 struct udevice **emulp)
{
	struct udevice *dev;
	int ret;

	ret = pci_bus_find_devfn(bus, find_devfn, &dev);
	if (ret) {
		debug("%s: Could not find emulator for dev %x\n", __func__,
		      find_devfn);
		return ret;
	}

	ret = device_find_first_child(dev, emulp);
	if (ret)
		return ret;

	return *emulp ? 0 : -ENODEV;
}

static int sandbox_pci_emul_post_probe(struct udevice *dev)
{
	struct sandbox_pci_priv *priv = dev->uclass->priv;

	priv->dev_count++;
	sandbox_set_enable_pci_map(true);

	return 0;
}

static int sandbox_pci_emul_pre_remove(struct udevice *dev)
{
	struct sandbox_pci_priv *priv = dev->uclass->priv;

	priv->dev_count--;
	sandbox_set_enable_pci_map(priv->dev_count > 0);

	return 0;
}

UCLASS_DRIVER(pci_emul) = {
	.id		= UCLASS_PCI_EMUL,
	.name		= "pci_emul",
	.post_probe	= sandbox_pci_emul_post_probe,
	.pre_remove	= sandbox_pci_emul_pre_remove,
	.priv_auto_alloc_size	= sizeof(struct sandbox_pci_priv),
};
