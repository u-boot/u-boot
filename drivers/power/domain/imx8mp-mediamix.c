// SPDX-License-Identifier: GPL-2.0
/*
 * i.MX8 MEDIAMIX control block driver
 * Copyright (C) 2024 Miquel Raynal <miquel.raynal@bootlin.com>
 * Inspired from Marek Vasut <marex@denx.de> work on the hsiomix driver.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <power-domain-uclass.h>
#include <linux/delay.h>

#include <dt-bindings/power/imx8mp-power.h>

#define BLK_SFT_RSTN		0x0
#define BLK_CLK_EN		0x4

struct imx8mp_mediamix_priv {
	void __iomem *base;
	struct clk clk_apb;
	struct clk clk_axi;
	struct clk clk_disp2;
	struct power_domain pd_bus;
	struct power_domain pd_lcdif2;
};

static int imx8mp_mediamix_on(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct imx8mp_mediamix_priv *priv = dev_get_priv(dev);
	struct power_domain *domain;
	struct clk *clk;
	u32 reset;
	int ret;

	switch (power_domain->id) {
	case IMX8MP_MEDIABLK_PD_LCDIF_2:
		domain = &priv->pd_lcdif2;
		clk = &priv->clk_disp2;
		reset = BIT(11) | BIT(12) | BIT(24);
		break;
	default:
		return -EINVAL;
	}

	/* Make sure bus domain is awake */
	ret = power_domain_on(&priv->pd_bus);
	if (ret)
		return ret;

	/* Put devices into reset */
	clrbits_le32(priv->base + BLK_SFT_RSTN, reset);

	/* Enable upstream clocks */
	ret = clk_enable(&priv->clk_apb);
	if (ret)
		goto dis_bus_pd;

	ret = clk_enable(&priv->clk_axi);
	if (ret)
		goto dis_apb_clk;

	/* Enable blk-ctrl clock to allow reset to propagate */
	ret = clk_enable(clk);
	if (ret)
		goto dis_axi_clk;
	setbits_le32(priv->base + BLK_CLK_EN, reset);

	/* Power up upstream GPC domain */
	ret = power_domain_on(domain);
	if (ret)
		goto dis_lcdif_clk;

	/* Wait for reset to propagate */
	udelay(5);

	/* Release reset */
	setbits_le32(priv->base + BLK_SFT_RSTN, reset);

	return 0;

dis_lcdif_clk:
	clk_disable(clk);
dis_axi_clk:
	clk_disable(&priv->clk_axi);
dis_apb_clk:
	clk_disable(&priv->clk_apb);
dis_bus_pd:
	power_domain_off(&priv->pd_bus);

	return ret;
}

static int imx8mp_mediamix_off(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct imx8mp_mediamix_priv *priv = dev_get_priv(dev);
	struct power_domain *domain;
	struct clk *clk;
	u32 reset;

	switch (power_domain->id) {
	case IMX8MP_MEDIABLK_PD_LCDIF_2:
		domain = &priv->pd_lcdif2;
		clk = &priv->clk_disp2;
		reset = BIT(11) | BIT(12) | BIT(24);
		break;
	default:
		return -EINVAL;
	}

	/* Put devices into reset and disable clocks */
	clrbits_le32(priv->base + BLK_SFT_RSTN, reset);
	clrbits_le32(priv->base + BLK_CLK_EN, reset);

	/* Power down upstream GPC domain */
	power_domain_off(domain);

	clk_disable(clk);
	clk_disable(&priv->clk_axi);
	clk_disable(&priv->clk_apb);

	/* Allow bus domain to suspend */
	power_domain_off(&priv->pd_bus);

	return 0;
}

static int imx8mp_mediamix_of_xlate(struct power_domain *power_domain,
				    struct ofnode_phandle_args *args)
{
	power_domain->id = args->args[0];

	return 0;
}

static int imx8mp_mediamix_bind(struct udevice *dev)
{
	/* Bind child lcdif */
	return dm_scan_fdt_dev(dev);
}

static int imx8mp_mediamix_probe(struct udevice *dev)
{
	struct power_domain_plat *plat = dev_get_uclass_plat(dev);
	struct imx8mp_mediamix_priv *priv = dev_get_priv(dev);
	int ret;

	/* Definitions are in imx8mp-power.h */
	plat->subdomains = 9;

	priv->base = dev_read_addr_ptr(dev);

	ret = clk_get_by_name(dev, "apb", &priv->clk_apb);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "axi", &priv->clk_axi);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "disp2", &priv->clk_disp2);
	if (ret < 0)
		return ret;

	ret = power_domain_get_by_name(dev, &priv->pd_bus, "bus");
	if (ret < 0)
		return ret;

	ret = power_domain_get_by_name(dev, &priv->pd_lcdif2, "lcdif2");
	if (ret < 0)
		goto free_bus_pd;

	return 0;

free_bus_pd:
	power_domain_free(&priv->pd_bus);
	return ret;
}

static int imx8mp_mediamix_remove(struct udevice *dev)
{
	struct imx8mp_mediamix_priv *priv = dev_get_priv(dev);

	power_domain_free(&priv->pd_lcdif2);
	power_domain_free(&priv->pd_bus);

	return 0;
}

static const struct udevice_id imx8mp_mediamix_ids[] = {
	{ .compatible = "fsl,imx8mp-media-blk-ctrl" },
	{ }
};

struct power_domain_ops imx8mp_mediamix_ops = {
	.on = imx8mp_mediamix_on,
	.off = imx8mp_mediamix_off,
	.of_xlate = imx8mp_mediamix_of_xlate,
};

U_BOOT_DRIVER(imx8mp_mediamix) = {
	.name		= "imx8mp_mediamix",
	.id		= UCLASS_POWER_DOMAIN,
	.of_match	= imx8mp_mediamix_ids,
	.bind		= imx8mp_mediamix_bind,
	.probe		= imx8mp_mediamix_probe,
	.remove		= imx8mp_mediamix_remove,
	.priv_auto	= sizeof(struct imx8mp_mediamix_priv),
	.ops		= &imx8mp_mediamix_ops,
};
