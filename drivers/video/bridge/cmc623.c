// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Ion Agorria <ion@agorria.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/ofnode_graph.h>
#include <i2c.h>
#include <log.h>
#include <backlight.h>
#include <panel.h>
#include <video_bridge.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <power/regulator.h>
#include <asm/gpio.h>

static const char * const cmc623_supplies[] = {
	"vdd3v0-supply", "vdd1v2-supply", "vddio1v8-supply"
};

struct cmc623_priv {
	struct udevice *panel;
	struct display_timing timing;

	struct udevice *supplies[ARRAY_SIZE(cmc623_supplies)];

	struct gpio_desc enable_gpio; /* also known as FAILSAFE */
	struct gpio_desc bypass_gpio;
};

static int cmc623_attach(struct udevice *dev)
{
	struct cmc623_priv *priv = dev_get_priv(dev);
	int ret;

	/* Perform panel setup */
	ret = panel_enable_backlight(priv->panel);
	if (ret)
		return ret;

	return 0;
}

static int cmc623_set_backlight(struct udevice *dev, int percent)
{
	struct cmc623_priv *priv = dev_get_priv(dev);

	return panel_set_backlight(priv->panel, percent);
}

static int cmc623_panel_timings(struct udevice *dev, struct display_timing *timing)
{
	struct cmc623_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));

	return 0;
}

static int cmc623_hw_init(struct udevice *dev)
{
	struct cmc623_priv *priv = dev_get_priv(dev);
	struct video_bridge_priv *uc_priv = dev_get_uclass_priv(dev);
	int i, ret;

	/* enable supplies */
	for (i = 0; i < ARRAY_SIZE(cmc623_supplies); i++) {
		ret = regulator_set_enable_if_allowed(priv->supplies[i], 1);
		if (ret) {
			log_debug("%s: cannot enable %s %d\n", __func__,
				  cmc623_supplies[i], ret);
			return ret;
		}
	}

	mdelay(10);

	ret = dm_gpio_set_value(&uc_priv->reset, 1);
	if (ret) {
		log_debug("%s: error at reset = 1 (%d)\n", __func__, ret);
		return ret;
	}

	ret = dm_gpio_set_value(&priv->enable_gpio, 0);
	if (ret) {
		log_debug("%s: error at enable = 0 (%d)\n", __func__, ret);
		return ret;
	}

	ret = dm_gpio_set_value(&priv->bypass_gpio, 0);
	if (ret) {
		log_debug("%s: error at bypass = 0 (%d)\n", __func__, ret);
		return ret;
	}

	ret = dm_gpio_set_value(&uc_priv->sleep, 0);
	if (ret) {
		log_debug("%s: error at sleep = 0 (%d)\n", __func__, ret);
		return ret;
	}

	udelay(2000);

	ret = dm_gpio_set_value(&priv->enable_gpio, 1);
	if (ret) {
		log_debug("%s: error at enable = 1 (%d)\n", __func__, ret);
		return ret;
	}

	udelay(2000);

	ret = dm_gpio_set_value(&priv->bypass_gpio, 1);
	if (ret) {
		log_debug("%s: error at bypass = 1 (%d)\n", __func__, ret);
		return ret;
	}

	udelay(2000);

	ret = dm_gpio_set_value(&uc_priv->sleep, 1);
	if (ret) {
		log_debug("%s: error at sleep = 1 (%d)\n", __func__, ret);
		return ret;
	}

	udelay(2000);

	ret = dm_gpio_set_value(&uc_priv->reset, 0);
	if (ret) {
		log_debug("%s: error at sleep = 0 (%d)\n", __func__, ret);
		return ret;
	}

	mdelay(10);

	ret = dm_gpio_set_value(&uc_priv->reset, 1);
	if (ret) {
		log_debug("%s: error at sleep = 1 (%d)\n", __func__, ret);
		return ret;
	}

	mdelay(10);

	return 0;
}

static int cmc623_get_panel(struct udevice *dev)
{
	struct cmc623_priv *priv = dev_get_priv(dev);
	int i, ret;

	u32 num = ofnode_graph_get_port_count(dev_ofnode(dev));

	for (i = 0; i < num; i++) {
		ofnode remote = ofnode_graph_get_remote_node(dev_ofnode(dev), i, -1);

		ret = uclass_get_device_by_ofnode(UCLASS_PANEL, remote, &priv->panel);
		if (!ret)
			return 0;
	}

	/* If this point is reached, no panels were found */
	return -ENODEV;
}

static int cmc623_probe(struct udevice *dev)
{
	struct cmc623_priv *priv = dev_get_priv(dev);
	int i, ret;

	/* get supplies */
	for (i = 0; i < ARRAY_SIZE(cmc623_supplies); i++) {
		ret = device_get_supply_regulator(dev, cmc623_supplies[i], &priv->supplies[i]);
		if (ret) {
			log_debug("%s: cannot get %s %d\n", __func__, cmc623_supplies[i], ret);
			if (ret != -ENOENT)
				return log_ret(ret);
		}
	}

	/* get control gpios */
	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not get enable-gpios (%d)\n", __func__, ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "bypass-gpios", 0, &priv->bypass_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not get bypass-gpios (%d)\n", __func__, ret);
		return ret;
	}

	ret = cmc623_hw_init(dev);
	if (ret) {
		log_debug("%s: error doing hw init, ret %d\n", __func__, ret);
		return ret;
	}

	ret = cmc623_get_panel(dev);
	if (ret) {
		log_debug("%s: panel not found, ret %d\n", __func__, ret);
		return ret;
	}

	panel_get_display_timing(priv->panel, &priv->timing);

	return 0;
}

static const struct video_bridge_ops cmc623_ops = {
	.attach			= cmc623_attach,
	.set_backlight		= cmc623_set_backlight,
	.get_display_timing	= cmc623_panel_timings,
};

static const struct udevice_id cmc623_ids[] = {
	{ .compatible = "samsung,cmc623" },
	{ }
};

U_BOOT_DRIVER(samsung_cmc623) = {
	.name		= "samsung_cmc623",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= cmc623_ids,
	.ops		= &cmc623_ops,
	.bind		= dm_scan_fdt_dev,
	.probe		= cmc623_probe,
	.priv_auto	= sizeof(struct cmc623_priv),
};
