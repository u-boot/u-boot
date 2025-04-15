// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 SiFive
 */

#include <cache.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/device.h>
#include <linux/bitfield.h>

#define SIFIVE_CCACHE_CONFIG		0x000
#define SIFIVE_CCACHE_CONFIG_WAYS	GENMASK(15, 8)

#define SIFIVE_CCACHE_WAY_ENABLE	0x008

#define SIFIVE_CCACHE_TRUNKCLOCKGATE	0x1000
#define SIFIVE_CCACHE_TRUNKCLOCKGATE_DISABLE	BIT(0)
#define SIFIVE_CCACHE_REGIONCLOCKGATE_DISABLE	BIT(1)

struct sifive_ccache {
	void __iomem *base;
	bool has_cg;
};

struct sifive_ccache_quirks {
	bool has_cg;
};

static int sifive_ccache_enable(struct udevice *dev)
{
	struct sifive_ccache *priv = dev_get_priv(dev);
	u32 config;
	u32 ways;

	/* Enable all ways of composable cache */
	config = readl(priv->base + SIFIVE_CCACHE_CONFIG);
	ways = FIELD_GET(SIFIVE_CCACHE_CONFIG_WAYS, config);

	writel(ways - 1, priv->base + SIFIVE_CCACHE_WAY_ENABLE);

	if (priv->has_cg) {
		/* enable clock gating bits */
		config = readl(priv->base + SIFIVE_CCACHE_TRUNKCLOCKGATE);
		config &= ~(SIFIVE_CCACHE_TRUNKCLOCKGATE_DISABLE |
				SIFIVE_CCACHE_REGIONCLOCKGATE_DISABLE);
		writel(config, priv->base + SIFIVE_CCACHE_TRUNKCLOCKGATE);
	}

	return 0;
}

static int sifive_ccache_get_info(struct udevice *dev, struct cache_info *info)
{
	struct sifive_ccache *priv = dev_get_priv(dev);

	info->base = (uintptr_t)priv->base;

	return 0;
}

static const struct cache_ops sifive_ccache_ops = {
	.enable = sifive_ccache_enable,
	.get_info = sifive_ccache_get_info,
};

static int sifive_ccache_probe(struct udevice *dev)
{
	struct sifive_ccache *priv = dev_get_priv(dev);
	const struct sifive_ccache_quirks *quirk = (void *)dev_get_driver_data(dev);

	priv->has_cg = quirk->has_cg;
	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct sifive_ccache_quirks fu540_ccache = {
	.has_cg = false,
};

static const struct sifive_ccache_quirks ccache0 = {
	.has_cg = true,
};

static const struct udevice_id sifive_ccache_ids[] = {
	{ .compatible = "sifive,fu540-c000-ccache", .data = (ulong)&fu540_ccache },
	{ .compatible = "sifive,fu740-c000-ccache", .data = (ulong)&fu540_ccache },
	{ .compatible = "sifive,ccache0", .data = (ulong)&ccache0 },
	{}
};

U_BOOT_DRIVER(sifive_ccache) = {
	.name = "sifive_ccache",
	.id = UCLASS_CACHE,
	.of_match = sifive_ccache_ids,
	.probe = sifive_ccache_probe,
	.priv_auto = sizeof(struct sifive_ccache),
	.ops = &sifive_ccache_ops,
};
