// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (C) 2023 Bhupesh Sharma <bhupesh.sharma@linaro.org>
 *
 * Based on Linux driver
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <generic-phy.h>
#include <malloc.h>
#include <reset.h>

#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define USB2_PHY_USB_PHY_UTMI_CTRL0 (0x3c)
#define SLEEPM BIT(0)
#define OPMODE_MASK GENMASK(4, 3)
#define OPMODE_NORMAL (0x00)
#define OPMODE_NONDRIVING BIT(3)
#define TERMSEL BIT(5)

#define USB2_PHY_USB_PHY_UTMI_CTRL5 (0x50)
#define POR BIT(1)

#define USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON0 (0x54)
#define SIDDQ BIT(2)
#define RETENABLEN BIT(3)
#define FSEL_MASK GENMASK(6, 4)
#define FSEL_DEFAULT (0x3 << 4)

#define USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON1 (0x58)
#define VBUSVLDEXTSEL0 BIT(4)
#define PLLBTUNE BIT(5)

#define USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON2 (0x5c)
#define VREGBYPASS BIT(0)

#define USB2_PHY_USB_PHY_HS_PHY_CTRL1 (0x60)
#define VBUSVLDEXT0 BIT(0)

#define USB2_PHY_USB_PHY_HS_PHY_CTRL2 (0x64)
#define USB2_AUTO_RESUME BIT(0)
#define USB2_SUSPEND_N BIT(2)
#define USB2_SUSPEND_N_SEL BIT(3)

#define USB2_PHY_USB_PHY_CFG0 (0x94)
#define UTMI_PHY_DATAPATH_CTRL_OVERRIDE_EN BIT(0)
#define UTMI_PHY_CMN_CTRL_OVERRIDE_EN BIT(1)

#define USB2_PHY_USB_PHY_REFCLK_CTRL (0xa0)
#define REFCLK_SEL_MASK GENMASK(1, 0)
#define REFCLK_SEL_DEFAULT (0x2 << 0)

struct qcom_snps_hsphy {
	void __iomem *base;
	struct reset_ctl_bulk resets;
};

/*
 * We should just be able to use clrsetbits_le32() here, but this results
 * in crashes on some boards. This is suspected to be because of some bus
 * arbitration quirks with the PHY (i.e. it takes several bus clock cycles
 * for the write to actually go through). The readl_relaxed() at the end will
 * block until the write is completed (and all registers updated), and thus
 * ensure that we don't access the PHY registers when they're in an
 * undetermined state.
 */
static inline void qcom_snps_hsphy_write_mask(void __iomem *base, u32 offset,
					      u32 mask, u32 val)
{
	u32 reg;

	reg = readl_relaxed(base + offset);

	reg &= ~mask;
	reg |= val & mask;
	writel_relaxed(reg, base + offset);

	/* Ensure above write is completed */
	readl_relaxed(base + offset);
}

static int qcom_snps_hsphy_usb_init(struct phy *phy)
{
	struct qcom_snps_hsphy *priv = dev_get_priv(phy->dev);

	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_CFG0,
				   UTMI_PHY_CMN_CTRL_OVERRIDE_EN,
				   UTMI_PHY_CMN_CTRL_OVERRIDE_EN);
	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_UTMI_CTRL5, POR,
				   POR);
	qcom_snps_hsphy_write_mask(priv->base,
				   USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON0, FSEL_MASK, 0);
	qcom_snps_hsphy_write_mask(priv->base,
				   USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON1,
				   PLLBTUNE, PLLBTUNE);
	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_REFCLK_CTRL,
				   REFCLK_SEL_DEFAULT, REFCLK_SEL_MASK);
	qcom_snps_hsphy_write_mask(priv->base,
				   USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON1,
				   VBUSVLDEXTSEL0, VBUSVLDEXTSEL0);
	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_HS_PHY_CTRL1,
				   VBUSVLDEXT0, VBUSVLDEXT0);

	qcom_snps_hsphy_write_mask(priv->base,
				   USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON2,
				   VREGBYPASS, VREGBYPASS);

	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_HS_PHY_CTRL2,
				   USB2_SUSPEND_N_SEL | USB2_SUSPEND_N,
				   USB2_SUSPEND_N_SEL | USB2_SUSPEND_N);

	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_UTMI_CTRL0,
				   SLEEPM, SLEEPM);

	qcom_snps_hsphy_write_mask(
		priv->base, USB2_PHY_USB_PHY_HS_PHY_CTRL_COMMON0, SIDDQ, 0);

	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_UTMI_CTRL5, POR,
				   0);

	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_HS_PHY_CTRL2,
				   USB2_SUSPEND_N_SEL, 0);

	qcom_snps_hsphy_write_mask(priv->base, USB2_PHY_USB_PHY_CFG0,
				   UTMI_PHY_CMN_CTRL_OVERRIDE_EN, 0);

	return 0;
}

static int qcom_snps_hsphy_power_on(struct phy *phy)
{
	struct qcom_snps_hsphy *priv = dev_get_priv(phy->dev);
	int ret;

	ret = reset_deassert_bulk(&priv->resets);
	if (ret)
		return ret;

	ret = qcom_snps_hsphy_usb_init(phy);
	if (ret)
		return ret;

	return 0;
}

static int qcom_snps_hsphy_power_off(struct phy *phy)
{
	struct qcom_snps_hsphy *priv = dev_get_priv(phy->dev);

	reset_assert_bulk(&priv->resets);

	return 0;
}

static int qcom_snps_hsphy_phy_probe(struct udevice *dev)
{
	struct qcom_snps_hsphy *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret < 0) {
		printf("failed to get resets, ret = %d\n", ret);
		return ret;
	}

	reset_deassert_bulk(&priv->resets);

	return 0;
}

static struct phy_ops qcom_snps_hsphy_phy_ops = {
	.power_on = qcom_snps_hsphy_power_on,
	.power_off = qcom_snps_hsphy_power_off,
};

static const struct udevice_id qcom_snps_hsphy_phy_ids[] = {
	{ .compatible = "qcom,sm8150-usb-hs-phy" },
	{ .compatible = "qcom,usb-snps-hs-5nm-phy" },
	{ .compatible = "qcom,usb-snps-hs-7nm-phy" },
	{ .compatible = "qcom,usb-snps-femto-v2-phy" },
	{}
};

U_BOOT_DRIVER(qcom_usb_qcom_snps_hsphy) = {
	.name = "qcom-snps-hsphy",
	.id = UCLASS_PHY,
	.of_match = qcom_snps_hsphy_phy_ids,
	.ops = &qcom_snps_hsphy_phy_ops,
	.probe = qcom_snps_hsphy_phy_probe,
	.priv_auto = sizeof(struct qcom_snps_hsphy),
};
