// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <power-domain-uclass.h>

#include <dt-bindings/power/imx8mp-power.h>

#define GPR_REG0		0x0
#define  PCIE_CLOCK_MODULE_EN	BIT(0)
#define  USB_CLOCK_MODULE_EN	BIT(1)
#define  PCIE_PHY_APB_RST	BIT(4)
#define  PCIE_PHY_INIT_RST	BIT(5)

struct imx8mp_hsiomix_priv {
	void __iomem *base;
	struct clk clk_usb;
	struct clk clk_pcie;
	struct power_domain pd_bus;
	struct power_domain pd_usb;
	struct power_domain pd_pcie;
	struct power_domain pd_usb_phy1;
	struct power_domain pd_usb_phy2;
	struct power_domain pd_pcie_phy;
};

static int imx8mp_hsiomix_set(struct power_domain *power_domain, bool power_on)
{
	struct udevice *dev = power_domain->dev;
	struct imx8mp_hsiomix_priv *priv = dev_get_priv(dev);
	struct power_domain *domain = NULL;
	struct clk *clk = NULL;
	u32 gpr_reg0_bits = 0;
	int ret;

	switch (power_domain->id) {
	case IMX8MP_HSIOBLK_PD_USB:
		domain = &priv->pd_usb;
		clk = &priv->clk_usb;
		gpr_reg0_bits |= USB_CLOCK_MODULE_EN;
		break;
	case IMX8MP_HSIOBLK_PD_USB_PHY1:
		domain = &priv->pd_usb_phy1;
		break;
	case IMX8MP_HSIOBLK_PD_USB_PHY2:
		domain = &priv->pd_usb_phy2;
		break;
	case IMX8MP_HSIOBLK_PD_PCIE:
		domain = &priv->pd_pcie;
		clk = &priv->clk_pcie;
		gpr_reg0_bits |= PCIE_CLOCK_MODULE_EN;
		break;
	case IMX8MP_HSIOBLK_PD_PCIE_PHY:
		domain = &priv->pd_pcie_phy;
		/* Bits to deassert PCIe PHY reset */
		gpr_reg0_bits |= PCIE_PHY_APB_RST | PCIE_PHY_INIT_RST;
		break;
	default:
		dev_err(dev, "unknown power domain id: %ld\n",
			power_domain->id);
		return -EINVAL;
	}

	if (power_on) {
		ret = power_domain_on(&priv->pd_bus);
		if (ret)
			return ret;

		ret = power_domain_on(domain);
		if (ret)
			goto err_pd;

		if (clk) {
			ret = clk_enable(clk);
			if (ret)
				goto err_clk;
		}

		if (gpr_reg0_bits)
			setbits_le32(priv->base + GPR_REG0, gpr_reg0_bits);
	} else {
		if (gpr_reg0_bits)
			clrbits_le32(priv->base + GPR_REG0, gpr_reg0_bits);

		if (clk)
			clk_disable(clk);

		power_domain_off(domain);
		power_domain_off(&priv->pd_bus);
	}

	return 0;

err_clk:
	power_domain_off(domain);
err_pd:
	power_domain_off(&priv->pd_bus);
	return ret;
}

static int imx8mp_hsiomix_on(struct power_domain *power_domain)
{
	return imx8mp_hsiomix_set(power_domain, true);
}

static int imx8mp_hsiomix_off(struct power_domain *power_domain)
{
	return imx8mp_hsiomix_set(power_domain, false);
}

static int imx8mp_hsiomix_of_xlate(struct power_domain *power_domain,
				   struct ofnode_phandle_args *args)
{
	power_domain->id = args->args[0];

	return 0;
}

static int imx8mp_hsiomix_probe(struct udevice *dev)
{
	struct imx8mp_hsiomix_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);

	ret = clk_get_by_name(dev, "usb", &priv->clk_usb);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "pcie", &priv->clk_pcie);
	if (ret < 0)
		return ret;

	ret = power_domain_get_by_name(dev, &priv->pd_bus, "bus");
	if (ret < 0)
		return ret;

	ret = power_domain_get_by_name(dev, &priv->pd_usb, "usb");
	if (ret < 0)
		goto err_pd_usb;

	ret = power_domain_get_by_name(dev, &priv->pd_usb_phy1, "usb-phy1");
	if (ret < 0)
		goto err_pd_usb_phy1;

	ret = power_domain_get_by_name(dev, &priv->pd_usb_phy2, "usb-phy2");
	if (ret < 0)
		goto err_pd_usb_phy2;

	ret = power_domain_get_by_name(dev, &priv->pd_pcie, "pcie");
	if (ret < 0)
		goto err_pd_pcie;

	ret = power_domain_get_by_name(dev, &priv->pd_pcie_phy, "pcie-phy");
	if (ret < 0)
		goto err_pd_pcie_phy;

	return 0;

err_pd_pcie_phy:
	power_domain_free(&priv->pd_pcie);
err_pd_pcie:
	power_domain_free(&priv->pd_usb_phy2);
err_pd_usb_phy2:
	power_domain_free(&priv->pd_usb_phy1);
err_pd_usb_phy1:
	power_domain_free(&priv->pd_usb);
err_pd_usb:
	power_domain_free(&priv->pd_bus);
	return ret;
}

static const struct udevice_id imx8mp_hsiomix_ids[] = {
	{ .compatible = "fsl,imx8mp-hsio-blk-ctrl" },
	{ }
};

struct power_domain_ops imx8mp_hsiomix_ops = {
	.on = imx8mp_hsiomix_on,
	.off = imx8mp_hsiomix_off,
	.of_xlate = imx8mp_hsiomix_of_xlate,
};

U_BOOT_DRIVER(imx8mp_hsiomix) = {
	.name		= "imx8mp_hsiomix",
	.id		= UCLASS_POWER_DOMAIN,
	.of_match	= imx8mp_hsiomix_ids,
	.probe		= imx8mp_hsiomix_probe,
	.priv_auto	= sizeof(struct imx8mp_hsiomix_priv),
	.ops		= &imx8mp_hsiomix_ops,
};
