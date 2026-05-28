// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <power/regulator.h>
#include <linux/clk-provider.h>

#include <asm/gpio.h>

struct clk_gpio_priv {
	struct gpio_desc	enable;	/* GPIO, controlling the gate */
	struct clk		*clk;	/* Gated clock */
	struct udevice		*vdd_supply;
};

static int clk_gpio_enable(struct clk *clk)
{
	struct clk_gpio_priv *priv = dev_get_priv(clk->dev);

	if (priv->clk)
		clk_enable(priv->clk);

	if (priv->enable.dev)
		dm_gpio_set_value(&priv->enable, 1);

	return 0;
}

static int clk_gpio_disable(struct clk *clk)
{
	struct clk_gpio_priv *priv = dev_get_priv(clk->dev);

	if (priv->enable.dev)
		dm_gpio_set_value(&priv->enable, 0);

	if (priv->clk)
		clk_disable(priv->clk);

	return 0;
}

static ulong clk_gpio_get_rate(struct clk *clk)
{
	struct clk_gpio_priv *priv = dev_get_priv(clk->dev);

	return (priv->clk) ? clk_get_rate(priv->clk) : -1;
}

const struct clk_ops clk_gpio_ops = {
	.enable		= clk_gpio_enable,
	.disable	= clk_gpio_disable,
	.get_rate	= clk_gpio_get_rate,
};

static int clk_gpio_probe(struct udevice *dev)
{
	struct clk_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	priv->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->clk)) {
		log_debug("%s: Could not get gated clock: %ld\n",
			  __func__, PTR_ERR(priv->clk));
		priv->clk = 0;
	}

	ret = gpio_request_by_name(dev, "enable-gpios", 0,
				   &priv->enable, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: Could not decode enable-gpios (%d)\n",
			  __func__, ret);
	}

	ret = device_get_supply_regulator(dev, "vdd-supply",
					  &priv->vdd_supply);
	if (ret == 0)
		ret = regulator_set_enable(priv->vdd_supply, true);

	log_debug("%s: %s regulator = %d\n", __func__, dev->name, ret);

	return 0;
}

/*
 * When implementing clk-mux-clock, use gpio_request_list_by_name
 * and implement get_rate/set_rate/set_parent ops. This should be
 * in a separate driver and with separate Kconfig option to enable
 * that driver, since unlike Linux implementation, the U-Boot DM
 * integration would be orthogonal to this driver.
 */
static const struct udevice_id clk_gpio_match[] = {
	{ .compatible = "gpio-gate-clock" },
	{ .compatible = "gated-fixed-clock" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gpio_gate_clock) = {
	.name		= "gpio_clock",
	.id		= UCLASS_CLK,
	.of_match	= clk_gpio_match,
	.probe		= clk_gpio_probe,
	.priv_auto	= sizeof(struct clk_gpio_priv),
	.ops		= &clk_gpio_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};
