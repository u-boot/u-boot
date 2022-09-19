// SPDX-License-Identifier: GPL-2.0+
#include <asm/io.h>
#include <common.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <timer.h>

#define TIMER_CTRL		0x00
#define TIMER0_EN		BIT(0)
#define TIMER0_RELOAD_EN	BIT(1)
#define TIMER0_RELOAD		0x10
#define TIMER0_VAL		0x14

struct orion_timer_priv {
	void *base;
};

static uint64_t orion_timer_get_count(struct udevice *dev)
{
	struct orion_timer_priv *priv = dev_get_priv(dev);

	return timer_conv_64(~readl(priv->base + TIMER0_VAL));
}

static int orion_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct orion_timer_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_remap_addr_index(dev, 0);
	if (!priv->base) {
		debug("unable to map registers\n");
		return -ENOMEM;
	}

	uc_priv->clock_rate = CONFIG_SYS_TCLK;

	writel(~0, priv->base + TIMER0_VAL);
	writel(~0, priv->base + TIMER0_RELOAD);

	/* enable timer */
	setbits_le32(priv->base + TIMER_CTRL, TIMER0_EN | TIMER0_RELOAD_EN);

	return 0;
}

static const struct timer_ops orion_timer_ops = {
	.get_count = orion_timer_get_count,
};

static const struct udevice_id orion_timer_ids[] = {
	{ .compatible = "marvell,orion-timer" },
	{}
};

U_BOOT_DRIVER(orion_timer) = {
	.name	= "orion_timer",
	.id	= UCLASS_TIMER,
	.of_match = orion_timer_ids,
	.probe = orion_timer_probe,
	.ops	= &orion_timer_ops,
	.priv_auto	= sizeof(struct orion_timer_priv),
};
