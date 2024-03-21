// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Linaro Ltd.
 *
 * Derived from Linux counterpart driver
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/clk-provider.h>
#include <linux/iopoll.h>
#include <syscon.h>
#include <regmap.h>
#include <reset.h>

#include <dt-bindings/phy/phy-imx8-pcie.h>

#define IMX8MM_PCIE_PHY_CMN_REG061	0x184
#define  ANA_PLL_CLK_OUT_TO_EXT_IO_EN	BIT(0)
#define IMX8MM_PCIE_PHY_CMN_REG062	0x188
#define  ANA_PLL_CLK_OUT_TO_EXT_IO_SEL	BIT(3)
#define IMX8MM_PCIE_PHY_CMN_REG063	0x18C
#define  AUX_PLL_REFCLK_SEL_SYS_PLL	GENMASK(7, 6)
#define IMX8MM_PCIE_PHY_CMN_REG064	0x190
#define  ANA_AUX_RX_TX_SEL_TX		BIT(7)
#define  ANA_AUX_RX_TERM_GND_EN		BIT(3)
#define  ANA_AUX_TX_TERM		BIT(2)
#define IMX8MM_PCIE_PHY_CMN_REG065	0x194
#define  ANA_AUX_RX_TERM		(BIT(7) | BIT(4))
#define  ANA_AUX_TX_LVL			GENMASK(3, 0)
#define IMX8MM_PCIE_PHY_CMN_REG075	0x1D4
#define  ANA_PLL_DONE			0x3
#define PCIE_PHY_TRSV_REG5		0x414
#define PCIE_PHY_TRSV_REG6		0x418

#define IMX8MM_GPR_PCIE_REF_CLK_SEL	GENMASK(25, 24)
#define IMX8MM_GPR_PCIE_REF_CLK_PLL	FIELD_PREP(IMX8MM_GPR_PCIE_REF_CLK_SEL, 0x3)
#define IMX8MM_GPR_PCIE_REF_CLK_EXT	FIELD_PREP(IMX8MM_GPR_PCIE_REF_CLK_SEL, 0x2)
#define IMX8MM_GPR_PCIE_AUX_EN		BIT(19)
#define IMX8MM_GPR_PCIE_CMN_RST		BIT(18)
#define IMX8MM_GPR_PCIE_POWER_OFF	BIT(17)
#define IMX8MM_GPR_PCIE_SSC_EN		BIT(16)
#define IMX8MM_GPR_PCIE_AUX_EN_OVERRIDE	BIT(9)

#define IOMUXC_GPR14_OFFSET		0x38

enum imx8_pcie_phy_type {
	IMX8MM,
	IMX8MP,
};

struct imx8_pcie_phy_drvdata {
	const	char			*gpr;
	enum	imx8_pcie_phy_type	variant;
};

struct imx8_pcie_phy {
	ulong			base;
	struct clk		hsio_clk;
	struct regmap		*iomuxc_gpr;
	struct reset_ctl	perst;
	struct reset_ctl	reset;
	u32			refclk_pad_mode;
	u32			tx_deemph_gen1;
	u32			tx_deemph_gen2;
	bool			clkreq_unused;
	const struct imx8_pcie_phy_drvdata	*drvdata;
};

static int imx8_pcie_phy_power_on(struct phy *phy)
{
	int ret;
	u32 val, pad_mode;
	struct imx8_pcie_phy *imx8_phy = dev_get_priv(phy->dev);

	pad_mode = imx8_phy->refclk_pad_mode;
	switch (imx8_phy->drvdata->variant) {
	case IMX8MM:
		reset_assert(&imx8_phy->reset);

		/* Tune PHY de-emphasis setting to pass PCIe compliance. */
		if (imx8_phy->tx_deemph_gen1)
			writel(imx8_phy->tx_deemph_gen1,
			       imx8_phy->base + PCIE_PHY_TRSV_REG5);
		if (imx8_phy->tx_deemph_gen2)
			writel(imx8_phy->tx_deemph_gen2,
			       imx8_phy->base + PCIE_PHY_TRSV_REG6);
		break;
	case IMX8MP: /* Do nothing. */
		break;
	}

	if (pad_mode == IMX8_PCIE_REFCLK_PAD_INPUT ||
	    pad_mode == IMX8_PCIE_REFCLK_PAD_UNUSED) {
		/* Configure the pad as input */
		val = readl(imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG061);
		writel(val & ~ANA_PLL_CLK_OUT_TO_EXT_IO_EN,
		       imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG061);
	} else {
		/* Configure the PHY to output the refclock via pad */
		writel(ANA_PLL_CLK_OUT_TO_EXT_IO_EN,
		       imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG061);
	}

	if (pad_mode == IMX8_PCIE_REFCLK_PAD_OUTPUT ||
	    pad_mode == IMX8_PCIE_REFCLK_PAD_UNUSED) {
		/* Source clock from SoC internal PLL */
		writel(ANA_PLL_CLK_OUT_TO_EXT_IO_SEL,
		       imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG062);
		writel(AUX_PLL_REFCLK_SEL_SYS_PLL,
		       imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG063);
		val = ANA_AUX_RX_TX_SEL_TX | ANA_AUX_TX_TERM;
		writel(val | ANA_AUX_RX_TERM_GND_EN,
		       imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG064);
		writel(ANA_AUX_RX_TERM | ANA_AUX_TX_LVL,
		       imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG065);
	}

	/* Set AUX_EN_OVERRIDE 1'b0, when the CLKREQ# isn't hooked */
	regmap_update_bits(imx8_phy->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8MM_GPR_PCIE_AUX_EN_OVERRIDE,
			   imx8_phy->clkreq_unused ?
			   0 : IMX8MM_GPR_PCIE_AUX_EN_OVERRIDE);
	regmap_update_bits(imx8_phy->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8MM_GPR_PCIE_AUX_EN,
			   IMX8MM_GPR_PCIE_AUX_EN);
	regmap_update_bits(imx8_phy->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8MM_GPR_PCIE_POWER_OFF, 0);
	regmap_update_bits(imx8_phy->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8MM_GPR_PCIE_SSC_EN, 0);

	regmap_update_bits(imx8_phy->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8MM_GPR_PCIE_REF_CLK_SEL,
			   pad_mode == IMX8_PCIE_REFCLK_PAD_INPUT ?
			   IMX8MM_GPR_PCIE_REF_CLK_EXT :
			   IMX8MM_GPR_PCIE_REF_CLK_PLL);
	udelay(200);

	/* Do the PHY common block reset */
	regmap_update_bits(imx8_phy->iomuxc_gpr, IOMUXC_GPR14_OFFSET,
			   IMX8MM_GPR_PCIE_CMN_RST,
			   IMX8MM_GPR_PCIE_CMN_RST);

	switch (imx8_phy->drvdata->variant) {
	case IMX8MP:
		reset_deassert(&imx8_phy->perst);
		fallthrough;
	case IMX8MM:
		reset_deassert(&imx8_phy->reset);
		udelay(500);
		break;
	}

	/* Polling to check the phy is ready or not. */
	ret = readl_poll_timeout(imx8_phy->base + IMX8MM_PCIE_PHY_CMN_REG075,
				 val, val == ANA_PLL_DONE, 20000);
	return ret;
}

static int imx8_pcie_phy_init(struct phy *phy)
{
	struct imx8_pcie_phy *imx8_phy = dev_get_priv(phy->dev);

	return clk_enable(&imx8_phy->hsio_clk);
}

static int imx8_pcie_phy_exit(struct phy *phy)
{
	struct imx8_pcie_phy *imx8_phy = dev_get_priv(phy->dev);

	return clk_disable(&imx8_phy->hsio_clk);
}

static const struct phy_ops imx8_pcie_phy_ops = {
	.init		= imx8_pcie_phy_init,
	.exit		= imx8_pcie_phy_exit,
	.power_on	= imx8_pcie_phy_power_on,
};

static const struct imx8_pcie_phy_drvdata imx8mm_drvdata = {
	.gpr = "fsl,imx8mm-iomuxc-gpr",
	.variant = IMX8MM,
};

static const struct imx8_pcie_phy_drvdata imx8mp_drvdata = {
	.gpr = "fsl,imx8mp-iomuxc-gpr",
	.variant = IMX8MP,
};

static const struct udevice_id imx8_pcie_phy_of_match[] = {
	{.compatible = "fsl,imx8mm-pcie-phy", .data = (ulong)&imx8mm_drvdata, },
	{.compatible = "fsl,imx8mp-pcie-phy", .data = (ulong)&imx8mp_drvdata, },
	{ },
};

static int imx8_pcie_phy_probe(struct udevice *dev)
{
	struct imx8_pcie_phy *imx8_phy = dev_get_priv(dev);
	ofnode gpr;
	int ret = 0;

	imx8_phy->drvdata = (void *)dev_get_driver_data(dev);
	imx8_phy->base = dev_read_addr(dev);
	if (!imx8_phy->base)
		return -EINVAL;

	/* get PHY refclk pad mode */
	dev_read_u32(dev, "fsl,refclk-pad-mode", &imx8_phy->refclk_pad_mode);

	imx8_phy->tx_deemph_gen1 = dev_read_u32_default(dev,
							"fsl,tx-deemph-gen1",
							0);
	imx8_phy->tx_deemph_gen2 = dev_read_u32_default(dev,
							"fsl,tx-deemph-gen2",
							0);
	imx8_phy->clkreq_unused = dev_read_bool(dev, "fsl,clkreq-unsupported");

	/* Grab GPR config register range */
	gpr = ofnode_by_compatible(ofnode_null(), imx8_phy->drvdata->gpr);
	if (ofnode_equal(gpr, ofnode_null())) {
		dev_err(dev, "unable to find GPR node\n");
		return -ENODEV;
	}

	imx8_phy->iomuxc_gpr = syscon_node_to_regmap(gpr);
	if (IS_ERR(imx8_phy->iomuxc_gpr)) {
		dev_err(dev, "unable to find iomuxc registers\n");
		return PTR_ERR(imx8_phy->iomuxc_gpr);
	}

	ret = clk_get_by_name(dev, "ref", &imx8_phy->hsio_clk);
	if (ret) {
		dev_err(dev, "Failed to get PCIEPHY ref clock\n");
		return ret;
	}

	ret = reset_get_by_name(dev, "pciephy", &imx8_phy->reset);
	if (ret) {
		dev_err(dev, "Failed to get PCIEPHY reset control\n");
		return ret;
	}

	if (imx8_phy->drvdata->variant == IMX8MP) {
		ret = reset_get_by_name(dev, "perst", &imx8_phy->perst);
		if (ret) {
			dev_err(dev,
				"Failed to get PCIEPHY PHY PERST control\n");
			goto err_perst;
		}
	}

	return 0;

err_perst:
	reset_free(&imx8_phy->reset);
	return ret;
}

static int imx8_pcie_phy_remove(struct udevice *dev)
{
	struct imx8_pcie_phy *imx8_phy = dev_get_priv(dev);

	if (imx8_phy->drvdata->variant == IMX8MP)
		reset_free(&imx8_phy->perst);

	reset_free(&imx8_phy->reset);

	return 0;
}

U_BOOT_DRIVER(nxp_imx8_pcie_phy) = {
	.name = "nxp_imx8_pcie_phy",
	.id = UCLASS_PHY,
	.of_match = imx8_pcie_phy_of_match,
	.probe = imx8_pcie_phy_probe,
	.remove = imx8_pcie_phy_remove,
	.ops = &imx8_pcie_phy_ops,
	.priv_auto	= sizeof(struct imx8_pcie_phy),
};
