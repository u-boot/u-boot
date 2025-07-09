// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Samsung Exynos SoC series USB DRD PHY driver.
 * Based on Linux kernel PHY driver: drivers/phy/samsung/phy-exynos5-usbdrd.c
 */

#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>

/* Offset of PMU register controlling USB PHY output isolation */
#define EXYNOS_USBDRD_PHY_CONTROL		0x0704
#define EXYNOS_PHY_ENABLE			BIT(0)

/* Exynos USB PHY registers */
#define EXYNOS5_FSEL_9MHZ6			0x0
#define EXYNOS5_FSEL_10MHZ			0x1
#define EXYNOS5_FSEL_12MHZ			0x2
#define EXYNOS5_FSEL_19MHZ2			0x3
#define EXYNOS5_FSEL_20MHZ			0x4
#define EXYNOS5_FSEL_24MHZ			0x5
#define EXYNOS5_FSEL_26MHZ			0x6
#define EXYNOS5_FSEL_50MHZ			0x7

/* Exynos850: USB DRD PHY registers */
#define EXYNOS850_DRD_LINKCTRL			0x04
#define LINKCTRL_FORCE_QACT			BIT(8)
#define LINKCTRL_BUS_FILTER_BYPASS		GENMASK(7, 4)

#define EXYNOS850_DRD_CLKRST			0x20
#define CLKRST_LINK_SW_RST			BIT(0)
#define CLKRST_PORT_RST				BIT(1)
#define CLKRST_PHY_SW_RST			BIT(3)

#define EXYNOS850_DRD_SSPPLLCTL			0x30
#define SSPPLLCTL_FSEL				GENMASK(2, 0)

#define EXYNOS850_DRD_UTMI			0x50
#define UTMI_FORCE_SLEEP			BIT(0)
#define UTMI_FORCE_SUSPEND			BIT(1)
#define UTMI_DM_PULLDOWN			BIT(2)
#define UTMI_DP_PULLDOWN			BIT(3)
#define UTMI_FORCE_BVALID			BIT(4)
#define UTMI_FORCE_VBUSVALID			BIT(5)

#define EXYNOS850_DRD_HSP			0x54
#define HSP_COMMONONN				BIT(8)
#define HSP_EN_UTMISUSPEND			BIT(9)
#define HSP_VBUSVLDEXT				BIT(12)
#define HSP_VBUSVLDEXTSEL			BIT(13)
#define HSP_FSV_OUT_EN				BIT(24)

#define EXYNOS850_DRD_HSP_TEST			0x5c
#define HSP_TEST_SIDDQ				BIT(24)

#define KHZ					1000
#define MHZ					(KHZ * KHZ)

/**
 * struct exynos_usbdrd_phy - driver data for Exynos USB PHY
 * @reg_phy: USB PHY controller register memory base
 * @clk: clock for register access
 * @core_clk: core clock for phy (ref clock)
 * @reg_pmu: regmap for PMU block
 * @extrefclk: frequency select settings when using 'separate reference clocks'
 */
struct exynos_usbdrd_phy {
	void __iomem *reg_phy;
	struct clk *clk;
	struct clk *core_clk;
	struct regmap *reg_pmu;
	u32 extrefclk;
};

static void exynos_usbdrd_phy_isol(struct regmap *reg_pmu, bool isolate)
{
	unsigned int val;

	if (!reg_pmu)
		return;

	val = isolate ? 0 : EXYNOS_PHY_ENABLE;
	regmap_update_bits(reg_pmu, EXYNOS_USBDRD_PHY_CONTROL,
			   EXYNOS_PHY_ENABLE, val);
}

/*
 * Convert the supplied clock rate to the value that can be written to the PHY
 * register.
 */
static unsigned int exynos_rate_to_clk(unsigned long rate, u32 *reg)
{
	switch (rate) {
	case 9600 * KHZ:
		*reg = EXYNOS5_FSEL_9MHZ6;
		break;
	case 10 * MHZ:
		*reg = EXYNOS5_FSEL_10MHZ;
		break;
	case 12 * MHZ:
		*reg = EXYNOS5_FSEL_12MHZ;
		break;
	case 19200 * KHZ:
		*reg = EXYNOS5_FSEL_19MHZ2;
		break;
	case 20 * MHZ:
		*reg = EXYNOS5_FSEL_20MHZ;
		break;
	case 24 * MHZ:
		*reg = EXYNOS5_FSEL_24MHZ;
		break;
	case 26 * MHZ:
		*reg = EXYNOS5_FSEL_26MHZ;
		break;
	case 50 * MHZ:
		*reg = EXYNOS5_FSEL_50MHZ;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static void exynos850_usbdrd_utmi_init(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg;

	/*
	 * Disable HWACG (hardware auto clock gating control). This will force
	 * QACTIVE signal in Q-Channel interface to HIGH level, to make sure
	 * the PHY clock is not gated by the hardware.
	 */
	reg = readl(regs_base + EXYNOS850_DRD_LINKCTRL);
	reg |= LINKCTRL_FORCE_QACT;
	writel(reg, regs_base + EXYNOS850_DRD_LINKCTRL);

	/* Start PHY Reset (POR=high) */
	reg = readl(regs_base + EXYNOS850_DRD_CLKRST);
	reg |= CLKRST_PHY_SW_RST;
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);

	/* Enable UTMI+ */
	reg = readl(regs_base + EXYNOS850_DRD_UTMI);
	reg &= ~(UTMI_FORCE_SUSPEND | UTMI_FORCE_SLEEP | UTMI_DP_PULLDOWN |
		 UTMI_DM_PULLDOWN);
	writel(reg, regs_base + EXYNOS850_DRD_UTMI);

	/* Set PHY clock and control HS PHY */
	reg = readl(regs_base + EXYNOS850_DRD_HSP);
	reg |= HSP_EN_UTMISUSPEND | HSP_COMMONONN;
	writel(reg, regs_base + EXYNOS850_DRD_HSP);

	/* Set VBUS Valid and D+ pull-up control by VBUS pad usage */
	reg = readl(regs_base + EXYNOS850_DRD_LINKCTRL);
	reg |= FIELD_PREP(LINKCTRL_BUS_FILTER_BYPASS, 0xf);
	writel(reg, regs_base + EXYNOS850_DRD_LINKCTRL);

	reg = readl(regs_base + EXYNOS850_DRD_UTMI);
	reg |= UTMI_FORCE_BVALID | UTMI_FORCE_VBUSVALID;
	writel(reg, regs_base + EXYNOS850_DRD_UTMI);

	reg = readl(regs_base + EXYNOS850_DRD_HSP);
	reg |= HSP_VBUSVLDEXT | HSP_VBUSVLDEXTSEL;
	writel(reg, regs_base + EXYNOS850_DRD_HSP);

	reg = readl(regs_base + EXYNOS850_DRD_SSPPLLCTL);
	reg &= ~SSPPLLCTL_FSEL;
	switch (phy_drd->extrefclk) {
	case EXYNOS5_FSEL_50MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 7);
		break;
	case EXYNOS5_FSEL_26MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 6);
		break;
	case EXYNOS5_FSEL_24MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 2);
		break;
	case EXYNOS5_FSEL_20MHZ:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 1);
		break;
	case EXYNOS5_FSEL_19MHZ2:
		reg |= FIELD_PREP(SSPPLLCTL_FSEL, 0);
		break;
	default:
		dev_warn(phy->dev, "unsupported ref clk: %#.2x\n",
			 phy_drd->extrefclk);
		break;
	}
	writel(reg, regs_base + EXYNOS850_DRD_SSPPLLCTL);

	/* Power up PHY analog blocks */
	reg = readl(regs_base + EXYNOS850_DRD_HSP_TEST);
	reg &= ~HSP_TEST_SIDDQ;
	writel(reg, regs_base + EXYNOS850_DRD_HSP_TEST);

	/* Finish PHY reset (POR=low) */
	udelay(10); /* required before doing POR=low */
	reg = readl(regs_base + EXYNOS850_DRD_CLKRST);
	reg &= ~(CLKRST_PHY_SW_RST | CLKRST_PORT_RST);
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);
	udelay(75); /* required after POR=low for guaranteed PHY clock */

	/* Disable single ended signal out */
	reg = readl(regs_base + EXYNOS850_DRD_HSP);
	reg &= ~HSP_FSV_OUT_EN;
	writel(reg, regs_base + EXYNOS850_DRD_HSP);
}

static void exynos850_usbdrd_utmi_exit(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	void __iomem *regs_base = phy_drd->reg_phy;
	u32 reg;

	/* Set PHY clock and control HS PHY */
	reg = readl(regs_base + EXYNOS850_DRD_UTMI);
	reg &= ~(UTMI_DP_PULLDOWN | UTMI_DM_PULLDOWN);
	reg |= UTMI_FORCE_SUSPEND | UTMI_FORCE_SLEEP;
	writel(reg, regs_base + EXYNOS850_DRD_UTMI);

	/* Power down PHY analog blocks */
	reg = readl(regs_base + EXYNOS850_DRD_HSP_TEST);
	reg |= HSP_TEST_SIDDQ;
	writel(reg, regs_base + EXYNOS850_DRD_HSP_TEST);

	/* Link reset */
	reg = readl(regs_base + EXYNOS850_DRD_CLKRST);
	reg |= CLKRST_LINK_SW_RST;
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);
	udelay(10); /* required before doing POR=low */
	reg &= ~CLKRST_LINK_SW_RST;
	writel(reg, regs_base + EXYNOS850_DRD_CLKRST);
}

static int exynos_usbdrd_phy_init(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	int ret;

	ret = clk_prepare_enable(phy_drd->clk);
	if (ret)
		return ret;

	exynos850_usbdrd_utmi_init(phy);

	clk_disable_unprepare(phy_drd->clk);

	return 0;
}

static int exynos_usbdrd_phy_exit(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	int ret;

	ret = clk_prepare_enable(phy_drd->clk);
	if (ret)
		return ret;

	exynos850_usbdrd_utmi_exit(phy);

	clk_disable_unprepare(phy_drd->clk);

	return 0;
}

static int exynos_usbdrd_phy_power_on(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);
	int ret;

	dev_dbg(phy->dev, "Request to power_on usbdrd_phy phy\n");

	ret = clk_prepare_enable(phy_drd->core_clk);
	if (ret)
		return ret;

	/* Power-on PHY */
	exynos_usbdrd_phy_isol(phy_drd->reg_pmu, false);

	return 0;
}

static int exynos_usbdrd_phy_power_off(struct phy *phy)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(phy->dev);

	dev_dbg(phy->dev, "Request to power_off usbdrd_phy phy\n");

	/* Power-off the PHY */
	exynos_usbdrd_phy_isol(phy_drd->reg_pmu, true);

	clk_disable_unprepare(phy_drd->core_clk);

	return 0;
}

static int exynos_usbdrd_phy_init_clk(struct udevice *dev)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(dev);
	unsigned long ref_rate;
	int err;

	phy_drd->clk = devm_clk_get(dev, "phy");
	if (IS_ERR(phy_drd->clk)) {
		err = PTR_ERR(phy_drd->clk);
		dev_err(dev, "Failed to get phy clock (err=%d)\n", err);
		return err;
	}

	phy_drd->core_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(phy_drd->core_clk)) {
		err = PTR_ERR(phy_drd->core_clk);
		dev_err(dev, "Failed to get ref clock (err=%d)\n", err);
		return err;
	}

	ref_rate = clk_get_rate(phy_drd->core_clk);
	err = exynos_rate_to_clk(ref_rate, &phy_drd->extrefclk);
	if (err) {
		dev_err(dev, "Clock rate %lu not supported\n", ref_rate);
		return err;
	}

	return 0;
}

static int exynos_usbdrd_phy_probe(struct udevice *dev)
{
	struct exynos_usbdrd_phy *phy_drd = dev_get_priv(dev);
	int err;

	phy_drd->reg_phy = dev_read_addr_ptr(dev);
	if (!phy_drd->reg_phy)
		return -EINVAL;

	err = exynos_usbdrd_phy_init_clk(dev);
	if (err)
		return err;

	phy_drd->reg_pmu = syscon_regmap_lookup_by_phandle(dev,
							  "samsung,pmu-syscon");
	if (IS_ERR(phy_drd->reg_pmu)) {
		err = PTR_ERR(phy_drd->reg_pmu);
		dev_err(dev, "Failed to lookup PMU regmap\n");
		return err;
	}

	return 0;
}

static const struct phy_ops exynos_usbdrd_phy_ops = {
	.init		= exynos_usbdrd_phy_init,
	.exit		= exynos_usbdrd_phy_exit,
	.power_on	= exynos_usbdrd_phy_power_on,
	.power_off	= exynos_usbdrd_phy_power_off,
};

static const struct udevice_id exynos_usbdrd_phy_of_match[] = {
	{
		.compatible = "samsung,exynos850-usbdrd-phy",
	},
	{ }
};

U_BOOT_DRIVER(exynos_usbdrd_phy) = {
	.name		= "exynos-usbdrd-phy",
	.id		= UCLASS_PHY,
	.of_match	= exynos_usbdrd_phy_of_match,
	.probe		= exynos_usbdrd_phy_probe,
	.ops		= &exynos_usbdrd_phy_ops,
	.priv_auto	= sizeof(struct exynos_usbdrd_phy),
};
