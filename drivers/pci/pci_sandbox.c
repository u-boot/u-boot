// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <inttypes.h>
#include <pci.h>

static int sandbox_pci_write_config(struct udevice *bus, pci_dev_t devfn,
				    uint offset, ulong value,
				    enum pci_size_t size)
{
	struct dm_pci_emul_ops *ops;
	struct udevice *emul;
	int ret;

	ret = sandbox_pci_get_emul(bus, devfn, &emul);
	if (ret)
		return ret == -ENODEV ? 0 : ret;
	ops = pci_get_emul_ops(emul);
	if (!ops || !ops->write_config)
		return -ENOSYS;

	return ops->write_config(emul, offset, value, size);
}

static int sandbox_pci_read_config(struct udevice *bus, pci_dev_t devfn,
				   uint offset, ulong *valuep,
				   enum pci_size_t size)
{
	struct dm_pci_emul_ops *ops;
	struct udevice *emul;
	int ret;

	/* Prepare the default response */
	*valuep = pci_get_ff(size);
	ret = sandbox_pci_get_emul(bus, devfn, &emul);
	if (ret)
		return ret == -ENODEV ? 0 : ret;
	ops = pci_get_emul_ops(emul);
	if (!ops || !ops->read_config)
		return -ENOSYS;

	return ops->read_config(emul, offset, valuep, size);
}

static const struct dm_pci_ops sandbox_pci_ops = {
	.read_config = sandbox_pci_read_config,
	.write_config = sandbox_pci_write_config,
};

static const struct udevice_id sandbox_pci_ids[] = {
	{ .compatible = "sandbox,pci" },
	{ }
};

U_BOOT_DRIVER(pci_sandbox) = {
	.name	= "pci_sandbox",
	.id	= UCLASS_PCI,
	.of_match = sandbox_pci_ids,
	.ops	= &sandbox_pci_ops,

	/* Attach an emulator if we can */
	.child_post_bind = dm_scan_fdt_dev,
	.per_child_platdata_auto_alloc_size =
			sizeof(struct pci_child_platdata),
};
