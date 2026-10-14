// SPDX-License-Identifier: GPL-2.0-only
/*
 * SpacemiT K1/K3 USB 2.0 (UTMI) PHY driver
 *
 * Ported from the Linux SpacemiT K1 USB 2.0 PHY driver:
 * Copyright (C) 2025 SpacemiT (Hangzhou) Technology Co. Ltd
 * Copyright (C) 2025 Ze Huang <huang.ze@linux.dev>
 */

#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define PHY_RST_MODE_CTRL		0x04
#define  PHY_PLL_RDY			BIT(0)
#define  PHY_CLK_CDR_EN			BIT(1)
#define  PHY_CLK_PLL_EN			BIT(2)
#define  PHY_CLK_MAC_EN			BIT(3)
#define  PHY_MAC_RSTN			BIT(5)
#define  PHY_CDR_RSTN			BIT(6)
#define  PHY_PLL_RSTN			BIT(7)
#define  PHY_HS_LINE_TX_MODE		BIT(13)
#define  PHY_FS_LINE_TX_MODE		BIT(14)

#define  PHY_INIT_MODE_BITS		(PHY_FS_LINE_TX_MODE | PHY_HS_LINE_TX_MODE)
#define  PHY_CLK_ENABLE_BITS		(PHY_CLK_PLL_EN | PHY_CLK_CDR_EN | \
					 PHY_CLK_MAC_EN)
#define  PHY_DEASSERT_RST_BITS		(PHY_PLL_RSTN | PHY_CDR_RSTN | \
					 PHY_MAC_RSTN)

#define PHY_TX_HOST_CTRL		0x10
#define  PHY_HST_DISC_AUTO_CLR		BIT(2)

#define PHY_HSTXP_HW_CTRL		0x34
#define  PHY_HSTXP_RSTN			BIT(2)
#define  PHY_CLK_HSTXP_EN		BIT(3)
#define  PHY_HSTXP_MODE			BIT(4)

#define PHY_PLL_DIV_CFG			0x98
#define  PHY_FDIV_FRACT_8_15		GENMASK(7, 0)
#define  PHY_FDIV_FRACT_16_19		GENMASK(11, 8)
#define  PHY_FDIV_FRACT_20_21		BIT(12)
#define  PHY_FDIV_FRACT_0_1		GENMASK(14, 13)
#define  PHY_DIV_LOCAL_EN		BIT(15)

#define  PHY_SEL_FREQ_24MHZ		0x01
#define  FDIV_REG_MASK			(PHY_FDIV_FRACT_20_21 | PHY_FDIV_FRACT_16_19 | \
					 PHY_FDIV_FRACT_8_15)
#define  FDIV_REG_VAL			0x1ec4

#define K1_USB2PHY_PLL_TIMEOUT_US	50000

struct spacemit_usb2phy {
	void __iomem *base;
	struct clk clk;
};

static int spacemit_usb2phy_init(struct phy *phy)
{
	struct spacemit_usb2phy *sphy = dev_get_priv(phy->dev);
	void __iomem *base = sphy->base;
	u32 val;
	int ret;

	ret = clk_enable(&sphy->clk);
	if (ret) {
		dev_err(phy->dev, "failed to enable clock: %d\n", ret);
		return ret;
	}

	/* Let any controller reset settle before touching the PHY */
	udelay(200);

	/*
	 * Program the PLL divider for the 24 MHz reference before waiting
	 * for the PLL: it does not lock until the divider is written.
	 */
	val = FIELD_PREP(FDIV_REG_MASK, FDIV_REG_VAL) |
	      FIELD_PREP(PHY_FDIV_FRACT_0_1, PHY_SEL_FREQ_24MHZ) |
	      PHY_DIV_LOCAL_EN;
	writel(val, base + PHY_PLL_DIV_CFG);

	ret = readl_poll_timeout(base + PHY_RST_MODE_CTRL, val,
				 val & PHY_PLL_RDY, K1_USB2PHY_PLL_TIMEOUT_US);
	if (ret) {
		dev_err(phy->dev, "PLL lock timeout\n");
		clk_disable(&sphy->clk);
		return ret;
	}

	/* Release the PHY's internal resets and enable its clocks */
	writel(PHY_INIT_MODE_BITS | PHY_CLK_ENABLE_BITS | PHY_DEASSERT_RST_BITS,
	       base + PHY_RST_MODE_CTRL);

	writel(PHY_HSTXP_RSTN | PHY_CLK_HSTXP_EN | PHY_HSTXP_MODE,
	       base + PHY_HSTXP_HW_CTRL);

	/* Auto-clear the HS host disconnect state on reconnect */
	setbits_le32(base + PHY_TX_HOST_CTRL, PHY_HST_DISC_AUTO_CLR);

	return 0;
}

static int spacemit_usb2phy_exit(struct phy *phy)
{
	struct spacemit_usb2phy *sphy = dev_get_priv(phy->dev);

	return clk_disable(&sphy->clk);
}

static const struct phy_ops spacemit_usb2phy_ops = {
	.init = spacemit_usb2phy_init,
	.exit = spacemit_usb2phy_exit,
};

static int spacemit_usb2phy_probe(struct udevice *dev)
{
	struct spacemit_usb2phy *sphy = dev_get_priv(dev);
	int ret;

	sphy->base = dev_read_addr_ptr(dev);
	if (!sphy->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &sphy->clk);
	if (ret) {
		dev_err(dev, "failed to get clock: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id spacemit_usb2phy_ids[] = {
	{ .compatible = "spacemit,k1-usb2-phy" },
	{ .compatible = "spacemit,k3-usb2-phy" },
	{ }
};

U_BOOT_DRIVER(spacemit_k1_usb2_phy) = {
	.name		= "spacemit_k1_usb2_phy",
	.id		= UCLASS_PHY,
	.of_match	= spacemit_usb2phy_ids,
	.probe		= spacemit_usb2phy_probe,
	.ops		= &spacemit_usb2phy_ops,
	.priv_auto	= sizeof(struct spacemit_usb2phy),
};
