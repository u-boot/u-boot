// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <dm.h>
#include <errno.h>
#include <wdt.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define CORE0_POKE_OFFSET	0x50000
#define CORE0_POKE_OFFSET_MASK	0xfffffULL

struct octeontx_wdt {
	void __iomem *reg;
};

static int octeontx_wdt_reset(struct udevice *dev)
{
	struct octeontx_wdt *priv = dev_get_priv(dev);

	writeq(~0ULL, priv->reg);

	return 0;
}

static int octeontx_wdt_probe(struct udevice *dev)
{
	struct octeontx_wdt *priv = dev_get_priv(dev);

	priv->reg = dev_remap_addr(dev);
	if (!priv->reg)
		return -EINVAL;

	/*
	 * Save core poke register address in reg (its not 0xa0000 as
	 * extracted from the DT but 0x50000 instead)
	 */
	priv->reg = (void __iomem *)(((u64)priv->reg &
				      ~CORE0_POKE_OFFSET_MASK) |
				     CORE0_POKE_OFFSET);

	return 0;
}

static const struct wdt_ops octeontx_wdt_ops = {
	.reset = octeontx_wdt_reset,
};

static const struct udevice_id octeontx_wdt_ids[] = {
	{ .compatible = "arm,sbsa-gwdt" },
	{}
};

U_BOOT_DRIVER(wdt_octeontx) = {
	.name = "wdt_octeontx",
	.id = UCLASS_WDT,
	.of_match = octeontx_wdt_ids,
	.ops = &octeontx_wdt_ops,
	.priv_auto_alloc_size = sizeof(struct octeontx_wdt),
	.probe = octeontx_wdt_probe,
};
