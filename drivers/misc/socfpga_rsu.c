// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 * SoC FPGA Remote System Update — driver model binding (UCLASS_MISC).
 */

#include <dm.h>
#include <asm/arch/socfpga_rsu_dm.h>

static int socfpga_rsu_probe(struct udevice *dev)
{
	struct socfpga_rsu_priv *priv = dev_get_priv(dev);

	priv->ll = NULL;
	return 0;
}

static const struct udevice_id socfpga_rsu_ids[] = {
	{ .compatible = "altr,socfpga-rsu" },
	{ }
};

U_BOOT_DRIVER(socfpga_rsu) = {
	.name		= "socfpga_rsu",
	.id		= UCLASS_MISC,
	.of_match	= socfpga_rsu_ids,
	.probe		= socfpga_rsu_probe,
	.priv_auto	= sizeof(struct socfpga_rsu_priv),
};
