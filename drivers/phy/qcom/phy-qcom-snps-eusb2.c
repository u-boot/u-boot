// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023-2024, Linaro Limited
 *
 * Based on the Linux phy-qcom-snps-eusb2.c driver
 */

#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <generic-phy.h>
#include <malloc.h>
#include <reset.h>

#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>

#define USB_PHY_UTMI_CTRL0		(0x3c)
#define SLEEPM				BIT(0)
#define OPMODE_MASK			GENMASK(4, 3)
#define OPMODE_NONDRIVING		BIT(3)

#define USB_PHY_UTMI_CTRL5		(0x50)
#define POR				BIT(1)

#define USB_PHY_HS_PHY_CTRL_COMMON0	(0x54)
#define PHY_ENABLE			BIT(0)
#define SIDDQ_SEL			BIT(1)
#define SIDDQ				BIT(2)
#define RETENABLEN			BIT(3)
#define FSEL_MASK			GENMASK(6, 4)
#define FSEL_19_2_MHZ_VAL		(0x0)
#define FSEL_38_4_MHZ_VAL		(0x4)

#define USB_PHY_CFG_CTRL_1		(0x58)
#define PHY_CFG_PLL_CPBIAS_CNTRL_MASK	GENMASK(7, 1)

#define USB_PHY_CFG_CTRL_2		(0x5c)
#define PHY_CFG_PLL_FB_DIV_7_0_MASK	GENMASK(7, 0)
#define DIV_7_0_19_2_MHZ_VAL		(0x90)
#define DIV_7_0_38_4_MHZ_VAL		(0xc8)

#define USB_PHY_CFG_CTRL_3		(0x60)
#define PHY_CFG_PLL_FB_DIV_11_8_MASK	GENMASK(3, 0)
#define DIV_11_8_19_2_MHZ_VAL		(0x1)
#define DIV_11_8_38_4_MHZ_VAL		(0x0)

#define PHY_CFG_PLL_REF_DIV		GENMASK(7, 4)
#define PLL_REF_DIV_VAL			(0x0)

#define USB_PHY_HS_PHY_CTRL2		(0x64)
#define VBUSVLDEXT0			BIT(0)
#define USB2_SUSPEND_N			BIT(2)
#define USB2_SUSPEND_N_SEL		BIT(3)
#define VBUS_DET_EXT_SEL		BIT(4)

#define USB_PHY_CFG_CTRL_4		(0x68)
#define PHY_CFG_PLL_GMP_CNTRL_MASK	GENMASK(1, 0)
#define PHY_CFG_PLL_INT_CNTRL_MASK	GENMASK(7, 2)

#define USB_PHY_CFG_CTRL_5		(0x6c)
#define PHY_CFG_PLL_PROP_CNTRL_MASK	GENMASK(4, 0)
#define PHY_CFG_PLL_VREF_TUNE_MASK	GENMASK(7, 6)

#define USB_PHY_CFG_CTRL_6		(0x70)
#define PHY_CFG_PLL_VCO_CNTRL_MASK	GENMASK(2, 0)

#define USB_PHY_CFG_CTRL_7		(0x74)

#define USB_PHY_CFG_CTRL_8		(0x78)
#define PHY_CFG_TX_FSLS_VREF_TUNE_MASK	GENMASK(1, 0)
#define PHY_CFG_TX_FSLS_VREG_BYPASS	BIT(2)
#define PHY_CFG_TX_HS_VREF_TUNE_MASK	GENMASK(5, 3)
#define PHY_CFG_TX_HS_XV_TUNE_MASK	GENMASK(7, 6)

#define USB_PHY_CFG_CTRL_9		(0x7c)
#define PHY_CFG_TX_PREEMP_TUNE_MASK	GENMASK(2, 0)
#define PHY_CFG_TX_RES_TUNE_MASK	GENMASK(4, 3)
#define PHY_CFG_TX_RISE_TUNE_MASK	GENMASK(6, 5)
#define PHY_CFG_RCAL_BYPASS		BIT(7)

#define USB_PHY_CFG_CTRL_10		(0x80)

#define USB_PHY_CFG0			(0x94)
#define DATAPATH_CTRL_OVERRIDE_EN	BIT(0)
#define CMN_CTRL_OVERRIDE_EN		BIT(1)

#define UTMI_PHY_CMN_CTRL0		(0x98)
#define TESTBURNIN			BIT(6)

#define USB_PHY_FSEL_SEL		(0xb8)
#define FSEL_SEL			BIT(0)

#define USB_PHY_APB_ACCESS_CMD		(0x130)
#define RW_ACCESS			BIT(0)
#define APB_START_CMD			BIT(1)
#define APB_LOGIC_RESET			BIT(2)

#define USB_PHY_APB_ACCESS_STATUS	(0x134)
#define ACCESS_DONE			BIT(0)
#define TIMED_OUT			BIT(1)
#define ACCESS_ERROR			BIT(2)
#define ACCESS_IN_PROGRESS		BIT(3)

#define USB_PHY_APB_ADDRESS		(0x138)
#define APB_REG_ADDR_MASK		GENMASK(7, 0)

#define USB_PHY_APB_WRDATA_LSB		(0x13c)
#define APB_REG_WRDATA_7_0_MASK		GENMASK(3, 0)

#define USB_PHY_APB_WRDATA_MSB		(0x140)
#define APB_REG_WRDATA_15_8_MASK	GENMASK(7, 4)

#define USB_PHY_APB_RDDATA_LSB		(0x144)
#define APB_REG_RDDATA_7_0_MASK		GENMASK(3, 0)

#define USB_PHY_APB_RDDATA_MSB		(0x148)
#define APB_REG_RDDATA_15_8_MASK	GENMASK(7, 4)

struct qcom_snps_eusb2_phy_priv {
	void __iomem *base;
	struct clk *ref_clk;
	struct reset_ctl_bulk resets;
};

static void qcom_snps_eusb2_hsphy_write_mask(void __iomem *base, u32 offset,
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

static void qcom_eusb2_default_parameters(struct qcom_snps_eusb2_phy_priv *qcom_snps_eusb2)
{
	/* default parameters: tx pre-emphasis */
	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_9,
					 PHY_CFG_TX_PREEMP_TUNE_MASK,
					 FIELD_PREP(PHY_CFG_TX_PREEMP_TUNE_MASK, 0));

	/* tx rise/fall time */
	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_9,
					 PHY_CFG_TX_RISE_TUNE_MASK,
					 FIELD_PREP(PHY_CFG_TX_RISE_TUNE_MASK, 0x2));

	/* source impedance adjustment */
	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_9,
					 PHY_CFG_TX_RES_TUNE_MASK,
					 FIELD_PREP(PHY_CFG_TX_RES_TUNE_MASK, 0x1));

	/* dc voltage level adjustement */
	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_8,
					 PHY_CFG_TX_HS_VREF_TUNE_MASK,
					 FIELD_PREP(PHY_CFG_TX_HS_VREF_TUNE_MASK, 0x3));

	/* transmitter HS crossover adjustement */
	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_8,
					 PHY_CFG_TX_HS_XV_TUNE_MASK,
					 FIELD_PREP(PHY_CFG_TX_HS_XV_TUNE_MASK, 0x0));
}

static int qcom_eusb2_ref_clk_init(struct qcom_snps_eusb2_phy_priv *qcom_snps_eusb2)
{
	unsigned long ref_clk_freq = clk_get_rate(qcom_snps_eusb2->ref_clk);

	switch (ref_clk_freq) {
	case 19200000:
		qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL_COMMON0,
						 FSEL_MASK,
						 FIELD_PREP(FSEL_MASK, FSEL_19_2_MHZ_VAL));

		qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_2,
						 PHY_CFG_PLL_FB_DIV_7_0_MASK,
						 DIV_7_0_19_2_MHZ_VAL);

		qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_3,
						 PHY_CFG_PLL_FB_DIV_11_8_MASK,
						 DIV_11_8_19_2_MHZ_VAL);
		break;

	case 38400000:
		qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL_COMMON0,
						 FSEL_MASK,
						 FIELD_PREP(FSEL_MASK, FSEL_38_4_MHZ_VAL));

		qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_2,
						 PHY_CFG_PLL_FB_DIV_7_0_MASK,
						 DIV_7_0_38_4_MHZ_VAL);

		qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_3,
						 PHY_CFG_PLL_FB_DIV_11_8_MASK,
						 DIV_11_8_38_4_MHZ_VAL);
		break;

	default:
		printf("%s: unsupported ref_clk_freq:%lu\n", __func__, ref_clk_freq);
		return -EINVAL;
	}

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_3,
					 PHY_CFG_PLL_REF_DIV, PLL_REF_DIV_VAL);

	return 0;
}

static int qcom_snps_eusb2_usb_init(struct phy *phy)
{
	struct qcom_snps_eusb2_phy_priv *qcom_snps_eusb2 = dev_get_priv(phy->dev);
	int ret;

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG0,
					 CMN_CTRL_OVERRIDE_EN, CMN_CTRL_OVERRIDE_EN);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_UTMI_CTRL5, POR, POR);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL_COMMON0,
					 PHY_ENABLE | RETENABLEN, PHY_ENABLE | RETENABLEN);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_APB_ACCESS_CMD,
					 APB_LOGIC_RESET, APB_LOGIC_RESET);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, UTMI_PHY_CMN_CTRL0, TESTBURNIN, 0);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_FSEL_SEL,
					 FSEL_SEL, FSEL_SEL);

	/* update ref_clk related registers */
	ret = qcom_eusb2_ref_clk_init(qcom_snps_eusb2);
	if (ret)
		return ret;

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_1,
					 PHY_CFG_PLL_CPBIAS_CNTRL_MASK,
					 FIELD_PREP(PHY_CFG_PLL_CPBIAS_CNTRL_MASK, 0x1));

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_4,
					 PHY_CFG_PLL_INT_CNTRL_MASK,
					 FIELD_PREP(PHY_CFG_PLL_INT_CNTRL_MASK, 0x8));

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_4,
					 PHY_CFG_PLL_GMP_CNTRL_MASK,
					 FIELD_PREP(PHY_CFG_PLL_GMP_CNTRL_MASK, 0x1));

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_5,
					 PHY_CFG_PLL_PROP_CNTRL_MASK,
					 FIELD_PREP(PHY_CFG_PLL_PROP_CNTRL_MASK, 0x10));

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_6,
					 PHY_CFG_PLL_VCO_CNTRL_MASK,
					 FIELD_PREP(PHY_CFG_PLL_VCO_CNTRL_MASK, 0x0));

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_CFG_CTRL_5,
					 PHY_CFG_PLL_VREF_TUNE_MASK,
					 FIELD_PREP(PHY_CFG_PLL_VREF_TUNE_MASK, 0x1));

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL2,
					 VBUS_DET_EXT_SEL, VBUS_DET_EXT_SEL);

	/* set default parameters */
	qcom_eusb2_default_parameters(qcom_snps_eusb2);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL2,
					 USB2_SUSPEND_N_SEL | USB2_SUSPEND_N,
					 USB2_SUSPEND_N_SEL | USB2_SUSPEND_N);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_UTMI_CTRL0, SLEEPM, SLEEPM);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL_COMMON0,
					 SIDDQ_SEL, SIDDQ_SEL);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL_COMMON0,
					 SIDDQ, 0);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_UTMI_CTRL5, POR, 0);

	qcom_snps_eusb2_hsphy_write_mask(qcom_snps_eusb2->base, USB_PHY_HS_PHY_CTRL2,
					 USB2_SUSPEND_N_SEL, 0);

	return 0;
}

static int qcom_snps_eusb2_phy_power_on(struct phy *phy)
{
	struct qcom_snps_eusb2_phy_priv *qcom_snps_eusb2 = dev_get_priv(phy->dev);
	int ret;

	/* TODO Repeater */

	clk_prepare_enable(qcom_snps_eusb2->ref_clk);

	ret = reset_deassert_bulk(&qcom_snps_eusb2->resets);
	if (ret)
		return ret;

	ret = qcom_snps_eusb2_usb_init(phy);
	if (ret)
		return ret;

	return 0;
}

static int qcom_snps_eusb2_phy_power_off(struct phy *phy)
{
	struct qcom_snps_eusb2_phy_priv *qcom_snps_eusb2 = dev_get_priv(phy->dev);

	reset_assert_bulk(&qcom_snps_eusb2->resets);
	clk_disable_unprepare(qcom_snps_eusb2->ref_clk);

	return 0;
}

static int qcom_snps_eusb2_phy_probe(struct udevice *dev)
{
	struct qcom_snps_eusb2_phy_priv *qcom_snps_eusb2 = dev_get_priv(dev);
	int ret;

	qcom_snps_eusb2->base = (void __iomem *)dev_read_addr(dev);
	if (IS_ERR(qcom_snps_eusb2->base))
		return PTR_ERR(qcom_snps_eusb2->base);

	qcom_snps_eusb2->ref_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(qcom_snps_eusb2->ref_clk)) {
		ret = PTR_ERR(qcom_snps_eusb2->ref_clk);
		printf("%s: failed to get ref clk %d\n", __func__, ret);
		return ret;
	}

	ret = reset_get_bulk(dev, &qcom_snps_eusb2->resets);
	if (ret < 0) {
		printf("failed to get resets, ret = %d\n", ret);
		return ret;
	}

	return 0;
}

static struct phy_ops qcom_snps_eusb2_phy_ops = {
	.power_on = qcom_snps_eusb2_phy_power_on,
	.power_off = qcom_snps_eusb2_phy_power_off,
};

static const struct udevice_id qcom_snps_eusb2_phy_ids[] = {
	{
		.compatible = "qcom,sm8550-snps-eusb2-phy",
	},
	{}
};

U_BOOT_DRIVER(qcom_usb_qcom_snps_eusb2) = {
	.name = "qcom-snps-eusb2-hsphy",
	.id = UCLASS_PHY,
	.of_match = qcom_snps_eusb2_phy_ids,
	.ops = &qcom_snps_eusb2_phy_ops,
	.probe = qcom_snps_eusb2_phy_probe,
	.priv_auto = sizeof(struct qcom_snps_eusb2_phy_priv),
};
