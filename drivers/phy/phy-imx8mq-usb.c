// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 *
 */

#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <clk.h>
#include <dm/device_compat.h>
#include <power/regulator.h>

#define PHY_CTRL0			0x0
#define PHY_CTRL0_REF_SSP_EN		BIT(2)
#define PHY_CTRL0_FSEL_MASK		GENMASK(10, 5)
#define PHY_CTRL0_FSEL_24M		0x2a
#define PHY_CTRL0_FSEL_100M		0x27
#define PHY_CTRL0_SSC_RANGE_MASK	GENMASK(23, 21)
#define PHY_CTRL0_SSC_RANGE_4003PPM	(0x2 << 21)

#define PHY_CTRL1			0x4
#define PHY_CTRL1_RESET			BIT(0)
#define PHY_CTRL1_COMMONONN		BIT(1)
#define PHY_CTRL1_ATERESET		BIT(3)
#define PHY_CTRL1_DCDENB		BIT(17)
#define PHY_CTRL1_CHRGSEL		BIT(18)
#define PHY_CTRL1_VDATSRCENB0		BIT(19)
#define PHY_CTRL1_VDATDETENB0		BIT(20)

#define PHY_CTRL2			0x8
#define PHY_CTRL2_TXENABLEN0		BIT(8)
#define PHY_CTRL2_OTG_DISABLE		BIT(9)

#define PHY_CTRL3			0xc
#define PHY_CTRL3_COMPDISTUNE_MASK	GENMASK(2, 0)
#define PHY_CTRL3_TXPREEMP_TUNE_MASK	GENMASK(16, 15)
#define PHY_CTRL3_TXPREEMP_TUNE_SHIFT	15
#define PHY_CTRL3_TXRISE_TUNE_MASK	GENMASK(21, 20)
#define PHY_CTRL3_TXRISE_TUNE_SHIFT	20
/* 1111: +24% ... 0000: -6% step: 2% */
#define PHY_CTRL3_TXVREF_TUNE_MASK	GENMASK(25, 22)
#define PHY_CTRL3_TXVREF_TUNE_SHIFT	22
#define PHY_CTRL3_TX_VBOOST_LEVEL_MASK	GENMASK(31, 29)
#define PHY_CTRL3_TX_VBOOST_LEVEL_SHIFT	29

#define PHY_CTRL4			0x10
#define PHY_CTRL4_PCS_TX_DEEMPH_3P5DB_MASK	GENMASK(20, 15)
#define PHY_CTRL4_PCS_TX_DEEMPH_3P5DB_SHIFT	15

#define PHY_CTRL5			0x14
#define PHY_CTRL5_DMPWD_OVERRIDE_SEL	BIT(23)
#define PHY_CTRL5_DMPWD_OVERRIDE	BIT(22)
#define PHY_CTRL5_DPPWD_OVERRIDE_SEL	BIT(21)
#define PHY_CTRL5_DPPWD_OVERRIDE	BIT(20)
#define PHY_CTRL5_PCS_TX_SWING_FULL_MASK	GENMASK(6, 0)

#define PHY_CTRL6			0x18
#define PHY_CTRL6_RXTERM_OVERRIDE_SEL	BIT(29)
#define PHY_CTRL6_ALT_CLK_EN		BIT(1)
#define PHY_CTRL6_ALT_CLK_SEL		BIT(0)

#define PHY_STS0			0x40
#define PHY_STS0_OTGSESSVLD		BIT(7)
#define PHY_STS0_CHGDET			BIT(4)
#define PHY_STS0_FSVPLUS		BIT(3)
#define PHY_STS0_FSVMINUS		BIT(2)

#define TCA_CLK_RST			0x00
#define TCA_CLK_RST_SW			BIT(9)
#define TCA_CLK_RST_REF_CLK_EN		BIT(1)
#define TCA_CLK_RST_SUSPEND_CLK_EN	BIT(0)

#define TCA_INTR_EN			0x04
#define TCA_INTR_STS			0x08

#define TCA_GCFG			0x10
#define TCA_GCFG_ROLE_HSTDEV		BIT(4)
#define TCA_GCFG_OP_MODE		GENMASK(1, 0)
#define TCA_GCFG_OP_MODE_SYSMODE	0
#define TCA_GCFG_OP_MODE_SYNCMODE	1

#define TCA_TCPC			0x14
#define TCA_TCPC_VALID			BIT(4)
#define TCA_TCPC_LOW_POWER_EN		BIT(3)
#define TCA_TCPC_ORIENTATION_NORMAL	BIT(2)
#define TCA_TCPC_MUX_CONTRL		GENMASK(1, 0)
#define TCA_TCPC_MUX_CONTRL_NO_CONN	0
#define TCA_TCPC_MUX_CONTRL_USB_CONN	1

#define TCA_SYSMODE_CFG			0x18
#define TCA_SYSMODE_TCPC_DISABLE	BIT(3)
#define TCA_SYSMODE_TCPC_FLIP		BIT(2)

#define TCA_CTRLSYNCMODE_CFG0		0x20
#define TCA_CTRLSYNCMODE_CFG1           0x20

#define TCA_PSTATE			0x30
#define TCA_PSTATE_CM_STS		BIT(4)
#define TCA_PSTATE_TX_STS		BIT(3)
#define TCA_PSTATE_RX_PLL_STS		BIT(2)
#define TCA_PSTATE_PIPE0_POWER_DOWN	GENMASK(1, 0)

#define TCA_GEN_STATUS			0x34
#define TCA_GEN_DEV_POR			BIT(12)
#define TCA_GEN_REF_CLK_SEL		BIT(8)
#define TCA_GEN_TYPEC_FLIP_INVERT	BIT(4)
#define TCA_GEN_PHY_TYPEC_DISABLE	BIT(3)
#define TCA_GEN_PHY_TYPEC_FLIP		BIT(2)

#define TCA_VBUS_CTRL			0x40
#define TCA_VBUS_STATUS			0x44

#define TCA_INFO			0xfc

enum imx8mpq_phy_type {
	IMX8MQ_PHY,
	IMX8MP_PHY,
	IMX95_PHY,
};

struct imx8mq_usb_phy {
	struct clk phy_clk;
	void __iomem *base;
	enum imx8mpq_phy_type type;
	struct udevice *vbus_supply;
	void __iomem *tca_base;
};

static const struct udevice_id imx8mq_usb_phy_of_match[] = {
	{ .compatible = "fsl,imx8mq-usb-phy", .data = IMX8MQ_PHY },
	{ .compatible = "fsl,imx8mp-usb-phy", .data = IMX8MP_PHY },
	{ .compatible = "fsl,imx95-usb-phy", .data = IMX95_PHY },
	{},
};

static void tca_blk_init(struct phy *usb_phy)
{
	struct udevice *dev = usb_phy->dev;
	struct imx8mq_usb_phy *imx_phy = dev_get_priv(dev);
	void __iomem *base = imx_phy->tca_base;
	u32 val;

	/* reset XBar block */
	val = readl(base + TCA_CLK_RST);
	val &= ~TCA_CLK_RST_SW;
	writel(val, base + TCA_CLK_RST);

	udelay(100);

	/* clear reset */
	val |= TCA_CLK_RST_SW;
	writel(val, base + TCA_CLK_RST);

	/*
	 * use Controller Synced Mode for TCA low power enable and
	 * put PHY to USB safe state.
	 */
	val = FIELD_PREP(TCA_GCFG_OP_MODE, TCA_GCFG_OP_MODE_SYNCMODE);
	writel(val, base + TCA_GCFG);

	val = TCA_TCPC_VALID | TCA_TCPC_LOW_POWER_EN;
	writel(val, base + TCA_TCPC);

	/* use System Configuration Mode for TCA mux control. */
	val = FIELD_PREP(TCA_GCFG_OP_MODE, TCA_GCFG_OP_MODE_SYSMODE);
	writel(val, base + TCA_GCFG);
}

static int imx8mq_usb_phy_init(struct phy *usb_phy)
{
	struct udevice *dev = usb_phy->dev;
	struct imx8mq_usb_phy *imx_phy = dev_get_priv(dev);
	u32 value;

	value = readl(imx_phy->base + PHY_CTRL1);
	value &= ~(PHY_CTRL1_VDATSRCENB0 | PHY_CTRL1_VDATDETENB0 |
		   PHY_CTRL1_COMMONONN);
	value |= PHY_CTRL1_RESET | PHY_CTRL1_ATERESET;
	writel(value, imx_phy->base + PHY_CTRL1);

	value = readl(imx_phy->base + PHY_CTRL0);
	value |= PHY_CTRL0_REF_SSP_EN;
	value &= ~PHY_CTRL0_SSC_RANGE_MASK;
	value |= PHY_CTRL0_SSC_RANGE_4003PPM;
	writel(value, imx_phy->base + PHY_CTRL0);

	value = readl(imx_phy->base + PHY_CTRL2);
	value |= PHY_CTRL2_TXENABLEN0;
	writel(value, imx_phy->base + PHY_CTRL2);

	value = readl(imx_phy->base + PHY_CTRL1);
	value &= ~(PHY_CTRL1_RESET | PHY_CTRL1_ATERESET);
	writel(value, imx_phy->base + PHY_CTRL1);

	return 0;
}

static int imx8mp_usb_phy_init(struct phy *usb_phy)
{
	struct udevice *dev = usb_phy->dev;
	struct imx8mq_usb_phy *imx_phy = dev_get_priv(dev);
	u32 value;

	/* USB3.0 PHY signal fsel for 24M ref */
	value = readl(imx_phy->base + PHY_CTRL0);
	value &= ~PHY_CTRL0_FSEL_MASK;
	value |= FIELD_PREP(PHY_CTRL0_FSEL_MASK, PHY_CTRL0_FSEL_24M);
	writel(value, imx_phy->base + PHY_CTRL0);

	/* Disable alt_clk_en and use internal MPLL clocks */
	value = readl(imx_phy->base + PHY_CTRL6);
	value &= ~(PHY_CTRL6_ALT_CLK_SEL | PHY_CTRL6_ALT_CLK_EN);
	writel(value, imx_phy->base + PHY_CTRL6);

	value = readl(imx_phy->base + PHY_CTRL1);
	value &= ~(PHY_CTRL1_VDATSRCENB0 | PHY_CTRL1_VDATDETENB0);
	value |= PHY_CTRL1_RESET | PHY_CTRL1_ATERESET;
	writel(value, imx_phy->base + PHY_CTRL1);

	value = readl(imx_phy->base + PHY_CTRL0);
	value |= PHY_CTRL0_REF_SSP_EN;
	writel(value, imx_phy->base + PHY_CTRL0);

	value = readl(imx_phy->base + PHY_CTRL2);
	value |= PHY_CTRL2_TXENABLEN0 | PHY_CTRL2_OTG_DISABLE;
	writel(value, imx_phy->base + PHY_CTRL2);

	udelay(10);

	value = readl(imx_phy->base + PHY_CTRL1);
	value &= ~(PHY_CTRL1_RESET | PHY_CTRL1_ATERESET);
	writel(value, imx_phy->base + PHY_CTRL1);

	if (imx_phy->tca_base)
		tca_blk_init(usb_phy);

	return 0;
}

static int imx8mpq_usb_phy_init(struct phy *usb_phy)
{
	struct udevice *dev = usb_phy->dev;
	struct imx8mq_usb_phy *imx_phy = dev_get_priv(dev);

	if (imx_phy->type == IMX8MP_PHY || imx_phy->type == IMX95_PHY)
		return imx8mp_usb_phy_init(usb_phy);
	else
		return imx8mq_usb_phy_init(usb_phy);
}

static int imx8mq_usb_phy_power_on(struct phy *usb_phy)
{
	struct udevice *dev = usb_phy->dev;
	struct imx8mq_usb_phy *imx_phy = dev_get_priv(dev);
	u32 value;
	int ret;

	if (CONFIG_IS_ENABLED(CLK)) {
		ret = clk_enable(&imx_phy->phy_clk);
		if (ret) {
			dev_err(dev, "Failed to enable usb phy clock: %d\n", ret);
			return ret;
		}
	}

	if (CONFIG_IS_ENABLED(DM_REGULATOR) && imx_phy->vbus_supply) {
		ret = regulator_set_enable_if_allowed(imx_phy->vbus_supply, true);
		if (ret && ret != -ENOSYS) {
			dev_err(dev, "Failed to enable VBUS regulator: %d\n", ret);
			goto err;
		}
	}

	/* Disable rx term override */
	value = readl(imx_phy->base + PHY_CTRL6);
	value &= ~PHY_CTRL6_RXTERM_OVERRIDE_SEL;
	writel(value, imx_phy->base + PHY_CTRL6);

	return 0;

err:
	if (CONFIG_IS_ENABLED(CLK))
		clk_disable(&imx_phy->phy_clk);
	return ret;
}

static int imx8mq_usb_phy_power_off(struct phy *usb_phy)
{
	struct udevice *dev = usb_phy->dev;
	struct imx8mq_usb_phy *imx_phy = dev_get_priv(dev);
	u32 value;
	int ret;

	/* Override rx term to be 0 */
	value = readl(imx_phy->base + PHY_CTRL6);
	value |= PHY_CTRL6_RXTERM_OVERRIDE_SEL;
	writel(value, imx_phy->base + PHY_CTRL6);

	if (CONFIG_IS_ENABLED(CLK))
		clk_disable(&imx_phy->phy_clk);

	if (CONFIG_IS_ENABLED(DM_REGULATOR) && imx_phy->vbus_supply) {
		ret = regulator_set_enable_if_allowed(imx_phy->vbus_supply, false);
		if (ret && ret != -ENOSYS) {
			dev_err(dev, "Failed to disable VBUS regulator: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

struct phy_ops imx8mq_usb_phy_ops = {
	.init = imx8mpq_usb_phy_init,
	.power_on = imx8mq_usb_phy_power_on,
	.power_off = imx8mq_usb_phy_power_off,
};

int imx8mq_usb_phy_probe(struct udevice *dev)
{
	struct imx8mq_usb_phy *priv = dev_get_priv(dev);
	int ret;

	priv->type = dev_get_driver_data(dev);
	priv->base = dev_read_addr_ptr(dev);

	if (!priv->base)
		return -EINVAL;

	if (CONFIG_IS_ENABLED(CLK)) {
		ret = clk_get_by_name(dev, "phy", &priv->phy_clk);
		if (ret) {
			dev_err(dev, "Failed to get usb phy clock %d\n", ret);
			return ret;
		}
	}

	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		ret = device_get_supply_regulator(dev, "vbus-supply",
						  &priv->vbus_supply);
		if (ret && ret != -ENOENT) {
			dev_err(dev, "Failed to get VBUS regulator: %d\n", ret);
			return ret;
		}
	}

	if (priv->type == IMX95_PHY)
		priv->tca_base = dev_read_addr_index_ptr(dev, 1);

	return 0;
}

U_BOOT_DRIVER(nxp_imx8mq_usb_phy) = {
	.name = "nxp_imx8mq_usb_phy",
	.id = UCLASS_PHY,
	.of_match = imx8mq_usb_phy_of_match,
	.probe = imx8mq_usb_phy_probe,
	.ops = &imx8mq_usb_phy_ops,
	.priv_auto	= sizeof(struct imx8mq_usb_phy),
};
