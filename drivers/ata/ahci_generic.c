// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <ahci.h>
#include <dm.h>
#include <log.h>

/*
 * Dummy implementation that can be overwritten by a board
 * specific function
 */
__weak int board_ahci_enable(void)
{
	return 0;
}

static int generic_ahci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;
	int ret;

	ret = ahci_bind_scsi(dev, &scsi_dev);
	if (ret) {
		debug("%s: Failed to bind (err=%d\n)", __func__, ret);
		return ret;
	}

	return 0;
}

static int generic_ahci_probe(struct udevice *dev)
{
	/*
	 * Board specific SATA / AHCI enable code, e.g. enable the
	 * AHCI power or deassert reset
	 */
	board_ahci_enable();

	ahci_probe_scsi(dev, (ulong)dev_remap_addr(dev));

	return 0;
}

static const struct udevice_id generic_ahci_ids[] = {
	{ .compatible = "marvell,armada-380-ahci" },
	{ .compatible = "marvell,armada-3700-ahci" },
	{ .compatible = "marvell,armada-8k-ahci" },
	{ .compatible = "cavium,octeon-7130-ahci" },
	{ .compatible = "generic-ahci" },
	{ }
};

U_BOOT_DRIVER(ahci_generic_drv) = {
	.name		= "ahci_generic",
	.id		= UCLASS_AHCI,
	.of_match	= generic_ahci_ids,
	.bind		= generic_ahci_bind,
	.probe		= generic_ahci_probe,
};
