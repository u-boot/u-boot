// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 * Loosely based on Linux lvds-codec.c driver
 */

#include <dm.h>
#include <dm/ofnode_graph.h>
#include <log.h>
#include <panel.h>
#include <video_bridge.h>
#include <asm/gpio.h>
#include <power/regulator.h>

struct lvds_codec_priv {
	struct udevice *panel;
	struct display_timing timing;

	struct gpio_desc powerdown_gpio;
	struct udevice *power;
};

static int lvds_codec_attach(struct udevice *dev)
{
	struct lvds_codec_priv *priv = dev_get_priv(dev);

	regulator_set_enable_if_allowed(priv->power, 1);
	dm_gpio_set_value(&priv->powerdown_gpio, 0);

	return panel_enable_backlight(priv->panel);
}

static int lvds_codec_set_panel(struct udevice *dev, int percent)
{
	struct lvds_codec_priv *priv = dev_get_priv(dev);

	return panel_set_backlight(priv->panel, percent);
}

static int lvds_codec_panel_timings(struct udevice *dev,
				    struct display_timing *timing)
{
	struct lvds_codec_priv *priv = dev_get_priv(dev);

	memcpy(timing, &priv->timing, sizeof(*timing));

	return 0;
}

/* Function is purely for sandbox testing */
static int lvds_codec_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	return 0;
}

static int lvds_codec_get_panel(struct udevice *dev)
{
	struct lvds_codec_priv *priv = dev_get_priv(dev);
	int i, ret;

	u32 num = ofnode_graph_get_port_count(dev_ofnode(dev));

	for (i = 0; i < num; i++) {
		ofnode remote = ofnode_graph_get_remote_node(dev_ofnode(dev), i, -1);

		ret = uclass_get_device_by_of_offset(UCLASS_PANEL,
						     ofnode_to_offset(remote),
						     &priv->panel);
		if (!ret)
			return 0;
	}

	/* If this point is reached, no panels were found */
	return -ENODEV;
}

static int lvds_codec_probe(struct udevice *dev)
{
	struct lvds_codec_priv *priv = dev_get_priv(dev);
	int ret;

	ret = lvds_codec_get_panel(dev);
	if (ret) {
		log_debug("%s: cannot get panel: ret=%d\n", __func__, ret);
		return log_ret(ret);
	}

	panel_get_display_timing(priv->panel, &priv->timing);

	ret = gpio_request_by_name(dev, "powerdown-gpios", 0,
				   &priv->powerdown_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: could not get powerdown-gpios (%d)\n", __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	ret = device_get_supply_regulator(dev, "power-supply", &priv->power);
	if (ret) {
		log_debug("%s: power regulator error: %d\n", __func__, ret);
		if (ret != -ENOENT)
			return log_ret(ret);
	}

	return 0;
}

static const struct video_bridge_ops lvds_codec_ops = {
	.attach			= lvds_codec_attach,
	.set_backlight		= lvds_codec_set_panel,
	.get_display_timing	= lvds_codec_panel_timings,
	.read_edid		= lvds_codec_read_edid,
};

static const struct udevice_id lvds_codec_ids[] = {
	{ .compatible = "lvds-decoder" },
	{ .compatible = "lvds-encoder" },
	{ }
};

U_BOOT_DRIVER(lvds_codec) = {
	.name		= "lvds_codec",
	.id		= UCLASS_VIDEO_BRIDGE,
	.of_match	= lvds_codec_ids,
	.ops		= &lvds_codec_ops,
	.probe		= lvds_codec_probe,
	.priv_auto	= sizeof(struct lvds_codec_priv),
};
