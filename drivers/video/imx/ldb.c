// SPDX-License-Identifier: GPL-2.0+
/*
 * Derived work from:
 *   Philippe Cornu <philippe.cornu@st.com>
 *   Yannick Fertre <yannick.fertre@st.com>
 * Adapted by Miquel Raynal <miquel.raynal@bootlin.com>
 */

#define LOG_CATEGORY UCLASS_VIDEO_BRIDGE

#include <clk.h>
#include <dm.h>
#include <log.h>
#include <panel.h>
#include <video_bridge.h>
#include <asm/io.h>
#include <linux/delay.h>

#define LDB_CTRL_CH0_ENABLE BIT(0)
#define LDB_CTRL_CH1_ENABLE BIT(2)
#define LDB_CTRL_CH0_DATA_WIDTH BIT(5)
#define LDB_CTRL_CH0_BIT_MAPPING BIT(6)
#define LDB_CTRL_CH1_DATA_WIDTH BIT(7)
#define LDB_CTRL_CH1_BIT_MAPPING BIT(8)
#define LDB_CTRL_DI0_VSYNC_POLARITY BIT(9)
#define LDB_CTRL_DI1_VSYNC_POLARITY BIT(10)

#define LVDS_CTRL_CH0_EN BIT(0)
#define LVDS_CTRL_CH1_EN BIT(1)
#define LVDS_CTRL_VBG_EN BIT(2)
#define LVDS_CTRL_PRE_EMPH_EN BIT(4)
#define LVDS_CTRL_PRE_EMPH_ADJ(n) (((n) & 0x7) << 5)
#define LVDS_CTRL_CC_ADJ(n) (((n) & 0x7) << 11)

struct imx_ldb_priv {
	struct clk ldb_clk;
	void __iomem *ldb_ctrl;
	void __iomem *lvds_ctrl;
	struct udevice *lvds1;
	struct udevice *lvds2;
};

static int imx_ldb_set_backlight(struct udevice *dev, int percent)
{
	struct imx_ldb_priv *priv = dev_get_priv(dev);
	int ret;

	if (priv->lvds1) {
		ret = panel_enable_backlight(priv->lvds1);
		if (ret) {
			debug("ldb: Cannot enable lvds1 backlight\n");
			return ret;
		}

		ret = panel_set_backlight(priv->lvds1, percent);
		if (ret)
			return ret;
	}

	if (priv->lvds2) {
		ret = panel_enable_backlight(priv->lvds2);
		if (ret) {
			debug("ldb: Cannot enable lvds2 backlight\n");
			return ret;
		}

		ret = panel_set_backlight(priv->lvds2, percent);
		if (ret)
			return ret;
	}

	return 0;
}

static int imx_ldb_of_to_plat(struct udevice *dev)
{
	struct imx_ldb_priv *priv = dev_get_priv(dev);
	int ret;

	uclass_get_device_by_endpoint(UCLASS_PANEL, dev, 1, -1, &priv->lvds1);
	uclass_get_device_by_endpoint(UCLASS_PANEL, dev, 2, -1, &priv->lvds2);
	if (!priv->lvds1 && !priv->lvds2) {
		debug("ldb: No remote panel for '%s' (ret=%d)\n",
		      dev_read_name(dev), ret);
		return ret;
	}

	return 0;
}

/* The block has a mysterious x7 internal divisor (x3.5 in dual configuration) */
#define IMX_LDB_INTERNAL_DIVISOR(x) (((x) * 70) / 10)
#define IMX_LDB_INTERNAL_DIVISOR_DUAL(x) (((x) * 35) / 10)

static ulong imx_ldb_input_rate(struct imx_ldb_priv *priv,
				struct display_timing *timings)
{
	ulong target_rate = timings->pixelclock.typ;

	if (priv->lvds1 && priv->lvds2)
		return IMX_LDB_INTERNAL_DIVISOR_DUAL(target_rate);

	return IMX_LDB_INTERNAL_DIVISOR(target_rate);
}

static int imx_ldb_attach(struct udevice *dev)
{
	struct imx_ldb_priv *priv = dev_get_priv(dev);
	struct display_timing timings;
	bool format_jeida = false;
	bool format_24bpp = true;
	u32 ldb_ctrl = 0, lvds_ctrl;
	ulong ldb_rate;
	int ret;

	/* TODO: update the 24bpp/jeida booleans with proper checks when they
	 * will be supported.
	 */
	if (priv->lvds1) {
		ret = panel_get_display_timing(priv->lvds1, &timings);
		if (ret) {
			ret = ofnode_decode_display_timing(dev_ofnode(priv->lvds1),
							   0, &timings);
			if (ret) {
				printf("Cannot decode lvds1 timings (%d)\n", ret);
				return ret;
			}
		}

		ldb_ctrl |= LDB_CTRL_CH0_ENABLE;
		if (format_24bpp)
			ldb_ctrl |= LDB_CTRL_CH0_DATA_WIDTH;
		if (format_jeida)
			ldb_ctrl |= LDB_CTRL_CH0_BIT_MAPPING;
		if (timings.flags & DISPLAY_FLAGS_VSYNC_HIGH)
			ldb_ctrl |= LDB_CTRL_DI0_VSYNC_POLARITY;
	}

	if (priv->lvds2) {
		ret = panel_get_display_timing(priv->lvds2, &timings);
		if (ret) {
			ret = ofnode_decode_display_timing(dev_ofnode(priv->lvds2),
							   0, &timings);
			if (ret) {
				printf("Cannot decode lvds2 timings (%d)\n", ret);
				return ret;
			}
		}

		ldb_ctrl |= LDB_CTRL_CH1_ENABLE;
		if (format_24bpp)
			ldb_ctrl |= LDB_CTRL_CH1_DATA_WIDTH;
		if (format_jeida)
			ldb_ctrl |= LDB_CTRL_CH1_BIT_MAPPING;
		if (timings.flags & DISPLAY_FLAGS_VSYNC_HIGH)
			ldb_ctrl |= LDB_CTRL_DI1_VSYNC_POLARITY;
	}

	/*
	 * Not all pixel clocks will work, as the final rate (after internal
	 * integer division) should be identical to the LCDIF clock, otherwise
	 * the rendering will appear resized/shimmering.
	 */
	ldb_rate = imx_ldb_input_rate(priv, &timings);
	clk_set_rate(&priv->ldb_clk, ldb_rate);

	writel(ldb_ctrl, priv->ldb_ctrl);

	lvds_ctrl = LVDS_CTRL_CC_ADJ(2) | LVDS_CTRL_PRE_EMPH_EN |
		    LVDS_CTRL_PRE_EMPH_ADJ(3) | LVDS_CTRL_VBG_EN;
	writel(lvds_ctrl, priv->lvds_ctrl);

	/* Wait for VBG to stabilize. */
	udelay(15);

	if (priv->lvds1)
		lvds_ctrl |= LVDS_CTRL_CH0_EN;
	if (priv->lvds2)
		lvds_ctrl |= LVDS_CTRL_CH1_EN;

	writel(lvds_ctrl, priv->lvds_ctrl);

	return 0;
}

static int imx_ldb_probe(struct udevice *dev)
{
	struct imx_ldb_priv *priv = dev_get_priv(dev);
	struct udevice *parent = dev_get_parent(dev);
	fdt_addr_t parent_addr, child_addr;
	int ret;

	ret = clk_get_by_name(dev, "ldb", &priv->ldb_clk);
	if (ret < 0)
		return ret;

	parent_addr = dev_read_addr(parent);
	if (parent_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	child_addr = dev_read_addr_name(dev, "ldb");
	if (child_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->ldb_ctrl = map_physmem(parent_addr + child_addr, 0, MAP_NOCACHE);
	if (!priv->ldb_ctrl)
		return -EINVAL;

	child_addr = dev_read_addr_name(dev, "lvds");
	if (child_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->lvds_ctrl = map_physmem(parent_addr + child_addr, 0, MAP_NOCACHE);
	if (!priv->lvds_ctrl)
		return -EINVAL;

	ret = clk_enable(&priv->ldb_clk);
	if (ret)
		return ret;

	ret = video_bridge_set_active(dev, true);
	if (ret)
		goto dis_clk;

	return 0;

dis_clk:
	clk_disable(&priv->ldb_clk);

	return ret;
}

struct video_bridge_ops imx_ldb_ops = {
	.attach = imx_ldb_attach,
	.set_backlight	= imx_ldb_set_backlight,
};

static const struct udevice_id imx_ldb_ids[] = {
	{ .compatible = "fsl,imx8mp-ldb"},
	{ }
};

U_BOOT_DRIVER(imx_ldb) = {
	.name = "imx-lvds-display-bridge",
	.id = UCLASS_VIDEO_BRIDGE,
	.of_match = imx_ldb_ids,
	.probe = imx_ldb_probe,
	.of_to_plat = imx_ldb_of_to_plat,
	.ops = &imx_ldb_ops,
	.priv_auto = sizeof(struct imx_ldb_priv),
};
