// SPDX-License-Identifier: GPL-2.0
/*
 * Spacemit K1 PCIe and PCIe/USB 3 combo PHY driver
 *
 * Copyright (C) 2026 RISCstar Ltd.  All rights reserved.
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/ofnode.h>
#include <errno.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <reset.h>

#include <dt-bindings/phy/phy.h>

/*
 * Three PCIe ports are supported in the SpacemiT K1 SoC.
 *
 * Port A (combo PHY): 1 lane, can switch between PCIe and USB 3.
 *                     The only PHY that can auto-calibrate RX/TX termination.
 * Port B (pcie-phy):  2 lanes, PCIe only.
 * Port C (pcie-phy):  2 lanes, PCIe only.
 *
 * Calibration values from port A are used to configure ports B and C.
 * Port B/C must not probe until port A has completed calibration.
 */

/* Offset between lane 0 and lane 1 registers */
#define PHY_LANE_OFFSET			0x0400

/* PHY PLL configuration */
#define PCIE_PU_ADDR_CLK_CFG		0x0008
#define PLL_READY			BIT(0)
#define CFG_INTERNAL_TIMER_ADJ		GENMASK(10, 7)
#define TIMER_ADJ_PCIE			0x6
#define CFG_SW_PHY_INIT_DONE		BIT(11)

#define PCIE_RC_DONE_STATUS		0x0018
#define CFG_FORCE_RCV_RETRY		BIT(10)

/* PCIe PHY lane calibration */
#define PCIE_RC_CAL_REG2		0x0020
#define RC_CAL_TOGGLE			BIT(22)
#define CLKSEL				GENMASK(31, 29)
#define CLKSEL_24M			0x3

/* Additional PHY PLL configuration */
#define PCIE_PU_PLL_1			0x0048
#define REF_100_WSSC			BIT(12)
#define FREF_SEL			GENMASK(15, 13)
#define FREF_24M			0x1
#define SSC_DEP_SEL			GENMASK(19, 16)
#define SSC_DEP_NONE			0x0

/* PCIe PHY configuration */
#define PCIE_PU_PLL_2			0x004c
#define GEN_REF100			BIT(4)

#define PCIE_RX_REG1			0x0050
#define EN_RTERM			BIT(3)
#define AFE_RTERM_REG			GENMASK(11, 8)

#define PCIE_RX_REG2			0x0054
#define RX_RTERM_SEL			BIT(5)

#define PCIE_LTSSM_DIS_ENTRY		0x005c
#define CFG_REFCLK_MODE			GENMASK(9, 8)
#define RFCLK_MODE_DRIVER		0x1
#define RFCLK_MODE_RECEIVER		0x2
#define OVRD_REFCLK_MODE		BIT(10)

#define PCIE_TX_REG1			0x0064
#define TX_RTERM_REG			GENMASK(15, 12)
#define TX_RTERM_SEL			BIT(25)

/* PHY calibration values */
#define PCIE_RCAL_RESULT		0x0084
#define RTERM_VALUE_RX			GENMASK(3, 0)
#define RTERM_VALUE_TX			GENMASK(7, 4)
#define R_TUNE_DONE			BIT(10)

/* PMU register offsets (shared APMU space) */
#define PMUA_USB_PHY_CTRL0		0x0110
#define COMBO_PHY_SEL			BIT(3)	/* 0: PCIe; 1: USB 3 */
#define PCIE_CLK_RES_CTRL		0x03cc
#define PCIE_APP_HOLD_PHY_RST		BIT(30)
#define DEVICE_TYPE_RC			BIT(31)
#define GLOBAL_PHY_RST			BIT(8)

#define APMU_CLK_GATE_MASK		0x3f

#define CALIBRATION_TIMEOUT_US		500000
#define PLL_TIMEOUT_US			500000

/* Global calibration value shared across PHY ports */
static u32 k1_phy_rterm = ~0;

struct k1_pcie_phy {
	void __iomem *regs;
	void __iomem *apmu_base;	/* only for combo PHY */
	u32 pcie_lanes;
	bool is_combo;
	struct clk_bulk clks;
	struct reset_ctl_bulk rst;
};

static bool k1_phy_rterm_valid(void)
{
	return !(k1_phy_rterm & ~(RTERM_VALUE_RX | RTERM_VALUE_TX));
}

u32 k1_phy_rterm_rx(void)
{
	return FIELD_GET(RTERM_VALUE_RX, k1_phy_rterm);
}

u32 k1_phy_rterm_tx(void)
{
	return FIELD_GET(RTERM_VALUE_TX, k1_phy_rterm);
}

static void k1_phy_rterm_set(u32 val)
{
	k1_phy_rterm = val & (RTERM_VALUE_RX | RTERM_VALUE_TX);
}

static void k1_combo_phy_sel(struct k1_pcie_phy *k1_phy, bool usb)
{
	void __iomem *pmu = k1_phy->apmu_base;
	u32 val;

	val = readl(pmu + PMUA_USB_PHY_CTRL0);
	if (usb)
		val |= COMBO_PHY_SEL;
	else
		val &= ~COMBO_PHY_SEL;
	writel(val, pmu + PMUA_USB_PHY_CTRL0);
}

int k1_pcie_combo_phy_calibrate(struct udevice *dev, struct k1_pcie_phy *k1_phy)
{
	void __iomem *regs = k1_phy->regs;
	int ret = 0;
	u32 val;

	if (k1_phy_rterm_valid())
		return 0;

	/*
	 * Initialize the APMU control register: set RC mode, enable
	 * clock gates (bits 0-5), deassert PHY global reset, then
	 * release the PHY hold.  Without this the PHY registers are
	 * not accessible and reads will hang.
	 */
	val = readl(k1_phy->apmu_base + PCIE_CLK_RES_CTRL);
	val |= DEVICE_TYPE_RC | PCIE_APP_HOLD_PHY_RST | APMU_CLK_GATE_MASK;
	val &= ~GLOBAL_PHY_RST;
	writel(val, k1_phy->apmu_base + PCIE_CLK_RES_CTRL);
	val &= ~PCIE_APP_HOLD_PHY_RST;
	writel(val, k1_phy->apmu_base + PCIE_CLK_RES_CTRL);

	/* Put the combo PHY into PCIe mode for calibration */
	k1_combo_phy_sel(k1_phy, false);

	/* Set refclk override + clear CFG_REFCLK_MODE */
	val = readl(regs + PCIE_LTSSM_DIS_ENTRY);
	val |= OVRD_REFCLK_MODE;
	writel(val, regs + PCIE_LTSSM_DIS_ENTRY);

	val = readl(regs + PCIE_LTSSM_DIS_ENTRY);
	val &= ~CFG_REFCLK_MODE;
	writel(val, regs + PCIE_LTSSM_DIS_ENTRY);

	val = readl(regs + PCIE_LTSSM_DIS_ENTRY);
	val |= FIELD_PREP(CFG_REFCLK_MODE, RFCLK_MODE_DRIVER);
	writel(val, regs + PCIE_LTSSM_DIS_ENTRY);

	/* Configure PLL: select 24MHz reference, enable 100MHz output */
	val = readl(regs + PCIE_PU_PLL_1);
	val &= ~(FREF_SEL | REF_100_WSSC);
	writel(val, regs + PCIE_PU_PLL_1);

	val = readl(regs + PCIE_PU_PLL_1);
	val |= FIELD_PREP(FREF_SEL, FREF_24M);
	writel(val, regs + PCIE_PU_PLL_1);

	val = readl(regs + PCIE_PU_PLL_2);
	val |= GEN_REF100;
	writel(val, regs + PCIE_PU_PLL_2);

	val = readl(regs + PCIE_PU_PLL_1);
	val &= ~SSC_DEP_SEL;
	writel(val, regs + PCIE_PU_PLL_1);

	/* Start PHY init + force receiver */
	writel(0x00000B78, regs + PCIE_PU_ADDR_CLK_CFG);
	writel(CFG_FORCE_RCV_RETRY, regs + PCIE_RC_DONE_STATUS);

	/* Wait for calibration to complete */
	ret = readl_poll_sleep_timeout(regs + PCIE_RCAL_RESULT,
				       val, val & R_TUNE_DONE,
				       500, CALIBRATION_TIMEOUT_US);
	if (ret)
		dev_err(dev, "PHY calib timeout, RCAL=0x%08x\n", val);

	if (!ret)
		k1_phy_rterm_set(val);

	return ret;
}

static int k1_pcie_phy_init(struct phy *phy)
{
	struct k1_pcie_phy *k1_phy = dev_get_priv(phy->dev);
	void __iomem *regs = k1_phy->regs;
	int ret;
	u32 val;
	int i, lane;

	/* If combo PHY is configured for USB 3 mode */
	if (k1_phy->is_combo && phy->id == PHY_TYPE_USB3) {
		k1_combo_phy_sel(k1_phy, true);
		return 0;
	}

	/* For combo PHY, ensure it's in PCIe mode */
	if (k1_phy->is_combo)
		k1_combo_phy_sel(k1_phy, false);

	dev_dbg(phy->dev, "init: combo=%d lanes=%u rterm=0x%02x\n",
		k1_phy->is_combo, k1_phy->pcie_lanes, k1_phy_rterm);

	lane = (k1_phy->pcie_lanes == 1) ? 1 : 2;

	/* Apply rterm calibration value: LSB to RX_REG1 */
	for (i = 0; i < lane; i++) {
		val = readl(regs + PCIE_RX_REG1 + PHY_LANE_OFFSET * i);
		val |= FIELD_PREP(AFE_RTERM_REG, k1_phy_rterm_rx());
		writel(val, regs + PCIE_RX_REG1 + PHY_LANE_OFFSET * i);
	}

	/* Disable RX_RTERM_SEL for all lanes */
	for (i = 0; i < lane; i++) {
		val = readl(regs + PCIE_RX_REG2 + PHY_LANE_OFFSET * i);
		val &= ~RX_RTERM_SEL;
		writel(val, regs + PCIE_RX_REG2 + PHY_LANE_OFFSET * i);
	}

	/* Apply rterm MSB to TX_REG1 */
	for (i = 0; i < lane; i++) {
		val = readl(regs + PCIE_TX_REG1 + PHY_LANE_OFFSET * i);
		val |= FIELD_PREP(TX_RTERM_REG, k1_phy_rterm_tx());
		writel(val, regs + PCIE_TX_REG1 + PHY_LANE_OFFSET * i);
	}

	/* Enable TX_RTERM_SEL for all lanes */
	for (i = 0; i < lane; i++) {
		val = readl(regs + PCIE_TX_REG1 + PHY_LANE_OFFSET * i);
		val |= TX_RTERM_SEL;
		writel(val, regs + PCIE_TX_REG1 + PHY_LANE_OFFSET * i);
	}

	/* Set CLKSEL_24M on lane0 */
	val = readl(regs + PCIE_RC_CAL_REG2);
	val |= FIELD_PREP(CLKSEL, CLKSEL_24M);
	writel(val, regs + PCIE_RC_CAL_REG2);

	/* Toggle RC_CAL_TOGGLE for all lanes */
	for (i = 0; i < lane; i++) {
		val = readl(regs + PCIE_RC_CAL_REG2 + PHY_LANE_OFFSET * i);
		val &= ~RC_CAL_TOGGLE;
		writel(val, regs + PCIE_RC_CAL_REG2 + PHY_LANE_OFFSET * i);
	}
	for (i = 0; i < lane; i++) {
		val = readl(regs + PCIE_RC_CAL_REG2 + PHY_LANE_OFFSET * i);
		val |= RC_CAL_TOGGLE;
		writel(val, regs + PCIE_RC_CAL_REG2 + PHY_LANE_OFFSET * i);
	}

	/* Configure refclk as driver mode, override */
	val = readl(regs + PCIE_LTSSM_DIS_ENTRY);
	val |= OVRD_REFCLK_MODE;
	writel(val, regs + PCIE_LTSSM_DIS_ENTRY);

	val = readl(regs + PCIE_LTSSM_DIS_ENTRY);
	val &= ~CFG_REFCLK_MODE;
	writel(val, regs + PCIE_LTSSM_DIS_ENTRY);

	val = readl(regs + PHY_LANE_OFFSET + PCIE_LTSSM_DIS_ENTRY);
	val |= OVRD_REFCLK_MODE;
	writel(val, regs + PHY_LANE_OFFSET + PCIE_LTSSM_DIS_ENTRY);

	val = readl(regs + PHY_LANE_OFFSET + PCIE_LTSSM_DIS_ENTRY);
	val &= ~CFG_REFCLK_MODE;
	writel(val, regs + PHY_LANE_OFFSET + PCIE_LTSSM_DIS_ENTRY);

	/* Set driver mode for both lanes */
	val = readl(regs + PCIE_LTSSM_DIS_ENTRY);
	val |= FIELD_PREP(CFG_REFCLK_MODE, RFCLK_MODE_DRIVER);
	writel(val, regs + PCIE_LTSSM_DIS_ENTRY);

	val = readl(regs + PHY_LANE_OFFSET + PCIE_LTSSM_DIS_ENTRY);
	val |= FIELD_PREP(CFG_REFCLK_MODE, RFCLK_MODE_DRIVER);
	writel(val, regs + PHY_LANE_OFFSET + PCIE_LTSSM_DIS_ENTRY);

	/* Configure PLL: select 24MHz reference */
	val = readl(regs + PCIE_PU_PLL_1);
	val &= ~FREF_SEL;
	writel(val, regs + PCIE_PU_PLL_1);

	val = readl(regs + PCIE_PU_PLL_1);
	val |= FIELD_PREP(FREF_SEL, FREF_24M);
	writel(val, regs + PCIE_PU_PLL_1);

	/* Enable 100MHz reference output */
	val = readl(regs + PCIE_PU_PLL_2);
	val |= GEN_REF100;
	writel(val, regs + PCIE_PU_PLL_2);

	/* Disable SSC */
	val = readl(regs + PCIE_PU_PLL_1);
	val &= ~SSC_DEP_SEL;
	writel(val, regs + PCIE_PU_PLL_1);

	/* Set PU_ADDR_CLK_CFG for both lanes */
	val = 0x00000B78;
	writel(val, regs + PCIE_PU_ADDR_CLK_CFG);
	writel(val, regs + PHY_LANE_OFFSET + PCIE_PU_ADDR_CLK_CFG);

	/* Force receiver done */
	val = CFG_FORCE_RCV_RETRY;
	writel(val, regs + PCIE_RC_DONE_STATUS);

	/* Wait for PLL lock */
	ret = readl_poll_timeout(regs + PCIE_PU_ADDR_CLK_CFG, val,
				 val & PLL_READY, PLL_TIMEOUT_US);
	if (ret) {
		dev_err(phy->dev, "PLL lock timeout! reg[0x08]=0x%08x\n", val);
		return ret;
	}

	dev_dbg(phy->dev, "PLL locked (combo=%d, lanes=%d)\n",
		k1_phy->is_combo, k1_phy->pcie_lanes);

	return 0;
}

static int k1_pcie_phy_exit(struct phy *phy)
{
	return 0;
}

static int k1_pcie_phy_of_xlate(struct phy *phy,
				struct ofnode_phandle_args *args)
{
	struct k1_pcie_phy *k1_phy = dev_get_priv(phy->dev);

	if (k1_phy->is_combo) {
		if (args->args_count != 1)
			return -EINVAL;
		if (args->args[0] != PHY_TYPE_PCIE &&
		    args->args[0] != PHY_TYPE_USB3)
			return -EINVAL;
	}

	if (args->args_count > 0)
		phy->id = args->args[0];
	else
		phy->id = 0;

	return 0;
}

static const struct phy_ops k1_pcie_phy_ops = {
	.of_xlate	= k1_pcie_phy_of_xlate,
	.init		= k1_pcie_phy_init,
	.exit		= k1_pcie_phy_exit,
};

static int k1_pcie_phy_probe(struct udevice *dev)
{
	struct k1_pcie_phy *k1_phy = dev_get_priv(dev);
	struct ofnode_phandle_args apmu_args;
	bool is_combo;
	int ret;
	u32 lanes;

	is_combo = (bool)dev_get_driver_data(dev);
	k1_phy->is_combo = is_combo;

	k1_phy->regs = dev_read_addr_ptr(dev);
	if (!k1_phy->regs)
		return -EINVAL;

	/* Determine number of lanes */
	if (is_combo) {
		lanes = 1;
	} else {
		u32 count = 0;

		ret = dev_read_u32(dev, "num-lanes", &count);
		if (ret == 0 && count == 1)
			lanes = 1;
		else
			lanes = 2;
	}
	k1_phy->pcie_lanes = lanes;

	/* Get APMU base for combo PHY (for mode select + calibration) */
	if (is_combo) {
		ret = dev_read_phandle_with_args(dev, "spacemit,apmu",
						 NULL, 0, 0,
						 &apmu_args);
		if (ret)
			return ret;

		k1_phy->apmu_base = (void __iomem *)ofnode_get_addr(apmu_args.node);
		if (!k1_phy->apmu_base)
			return -EINVAL;

		ret = clk_get_bulk(dev, &k1_phy->clks);
		if (ret)
			dev_warn(dev, "failed to get clocks: %d\n", ret);

		ret = reset_get_bulk(dev, &k1_phy->rst);
		if (ret)
			dev_warn(dev, "failed to get resets: %d\n", ret);

		ret = k1_pcie_combo_phy_calibrate(dev, k1_phy);
		if (ret)
			dev_warn(dev, "calibration failed: %d\n", ret);
	} else {
		/*
		 * PCIe-only PHYs need the calibration value from the
		 * combo PHY.  In device-tree order the combo PHY probes
		 * first, but handle the case where it hasn't yet by
		 * explicitly looking it up.
		 */
		if (!k1_phy_rterm_valid()) {
			struct udevice *combo_dev;

			ret = uclass_first_device_drvdata(UCLASS_PHY, 1UL,
							  &combo_dev);
			if (ret)
				dev_warn(dev,
					 "failed to probe combo PHY: %d\n",
					 ret);
		}

		if (!k1_phy_rterm_valid()) {
			dev_err(dev,
				"calibration value not available; "
				"is the combo PHY enabled?\n");
			return -ENOENT;
		}

		ret = clk_get_bulk(dev, &k1_phy->clks);
		if (ret)
			dev_warn(dev, "failed to get clocks: %d\n", ret);

		ret = reset_get_bulk(dev, &k1_phy->rst);
		if (ret)
			dev_warn(dev, "failed to get resets: %d\n", ret);
	}

	dev_dbg(dev, "probed (combo=%d lanes=%u rterm=0x%02x)\n",
		is_combo, lanes, k1_phy_rterm);

	return 0;
}

static const struct udevice_id k1_pcie_phy_ids[] = {
	{ .compatible = "spacemit,k1-combo-phy", .data = 1UL },
	{ .compatible = "spacemit,k1-pcie-phy", .data = 0UL },
	{ }
};

U_BOOT_DRIVER(k1_pcie_phy) = {
	.name		= "k1_pcie_phy",
	.id		= UCLASS_PHY,
	.of_match	= k1_pcie_phy_ids,
	.ops		= &k1_pcie_phy_ops,
	.probe		= k1_pcie_phy_probe,
	.priv_auto	= sizeof(struct k1_pcie_phy),
};
