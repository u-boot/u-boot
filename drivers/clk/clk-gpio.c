// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <asm/gpio.h>
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>

struct clk_gpio_priv {
	struct gpio_desc	enable;
};

static int clk_gpio_enable(struct clk *clk)
{
	struct clk_gpio_priv *priv = dev_get_priv(clk->dev);

	dm_gpio_set_value(&priv->enable, 1);

	return 0;
}

static int clk_gpio_disable(struct clk *clk)
{
	struct clk_gpio_priv *priv = dev_get_priv(clk->dev);

	dm_gpio_set_value(&priv->enable, 0);

	return 0;
}

const struct clk_ops clk_gpio_ops = {
	.enable		= clk_gpio_enable,
	.disable	= clk_gpio_disable,
};

static int clk_gpio_probe(struct udevice *dev)
{
	struct clk_gpio_priv *priv = dev_get_priv(dev);

	return gpio_request_by_name(dev, "enable-gpios", 0,
				    &priv->enable, GPIOD_IS_OUT);
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
