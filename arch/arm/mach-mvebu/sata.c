/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ahci.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Dummy implementation that can be overwritten by a board
 * specific function
 */
__weak int board_ahci_enable(void)
{
	return 0;
}

#ifdef CONFIG_ARMADA_8K
/* CP110 has different AHCI port addresses */
void __iomem *ahci_port_base(void __iomem *base, u32 port)
{
	return base + 0x10000 + (port * 0x10000);
}
#endif

static int mvebu_ahci_probe(struct udevice *dev)
{
	/*
	 * Board specific SATA / AHCI enable code, e.g. enable the
	 * AHCI power or deassert reset
	 */
	board_ahci_enable();

	ahci_init(dev_get_addr_ptr(dev));

	return 0;
}

static const struct udevice_id mvebu_ahci_ids[] = {
	{ .compatible = "marvell,armada-3700-ahci" },
	{ .compatible = "marvell,armada-8k-ahci" },
	{ }
};

U_BOOT_DRIVER(ahci_mvebu_drv) = {
	.name		= "ahci_mvebu",
	.id		= UCLASS_AHCI,
	.of_match	= mvebu_ahci_ids,
	.probe		= mvebu_ahci_probe,
};
