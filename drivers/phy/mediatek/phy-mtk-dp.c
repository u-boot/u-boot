// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek DisplayPort PHY driver
 *
 * Copyright (c) 2019-2022 MediaTek Inc.
 * Copyright (c) 2022, 2026 BayLibre, SAS.
 *                          Markus Schneider-Pargmann <msp@baylibre.com>
 *                          David Lechner <dlechner@baylibre.com>
 * Copyright (c) 2026 Collabora Ltd.
 *                    AngeloGioacchino Del Regno <angelogioacchino.delregno@collabora.com>
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <phy-dp.h>

#define MTK_DP_PHY_MAX_LANES		4

/* analog global registers */
#define DP_PHYA_GLB_OFFSET		0x400
#define DP_PHYA_GLB_BIAS_GEN_0		(DP_PHYA_GLB_OFFSET + 0x0)
#define XTP_GLB_BIAS_INT_R_CTRL			GENMASK(20, 16)
#define DP_PHYA_GLB_DPAUX_TX		(DP_PHYA_GLB_OFFSET + 0x8)
#define CKM_PT0_CKTX_IMPSEL			GENMASK(23, 20)
#define DP_PHYA_GLB_FORCE_CTRL_1	(DP_PHYA_GLB_OFFSET + 0x34)
#define CKM_CKTX0_EN_FORCE_MODE			BIT(10)

/* analog per lane registers */
#define DP_PHYA_LANE_TX_0(lane)		(0x100 * (lane) + 0x4)
#define XTP_LN_TX_IMPSEL_PMOS			GENMASK(15, 12)
#define XTP_LN_TX_IMPSEL_NMOS			GENMASK(19, 16)

/* digital per lane registers */
#define DP_PHYD_LANE_DRIVING_FORCE(lane) (0x1000 + 0x100 * (lane) + 0x18)
#define TX_FORCE_VOLT_SWING_EN			BIT(0)
#define TX_FORCE_VOLT_SWING_VAL			GENMASK(2, 1)
#define TX_FORCE_PRE_EMPH_EN			BIT(4)
#define TX_FORCE_PRE_EMPH_VAL			GENMASK(6, 5)

/* digital global registers */
#define DP_PHYD_GLB_OFFSET		0x1400
#define DP_PHYD_PLL_CTL_0		(DP_PHYD_GLB_OFFSET + 0x10)
#define PLL_CTL_0_EN				GENMASK(2, 0)
#define DP_PHYD_PLL_CTL_1		(DP_PHYD_GLB_OFFSET + 0x14)
/*
 * Spread spectrum on the transmit PLL is an active high enable at BIT(3), as
 * on the MT8195 DisplayPort PHY. MediaTek confirmed this for the MT8189.
 */
#define TPLL_SSC_EN				BIT(3)
#define DP_PHYD_SW_RST			(DP_PHYD_GLB_OFFSET + 0x38)
#define DP_GLB_SW_RST_PHYD			BIT(0)
#define DP_PHYD_BIT_RATE		(DP_PHYD_GLB_OFFSET + 0x3c)
#define BIT_RATE_RBR				0
#define BIT_RATE_HBR				1
#define BIT_RATE_HBR2				2
#define BIT_RATE_HBR3				3
#define DP_PHYD_AUX_RX_CTL		(DP_PHYD_GLB_OFFSET + 0x40)
#define DPAUX_RX_EN				BIT(0)
#define XTP_GLB_CKDET_EN			BIT(1)
#define DPAUX_RX_DEGLITCH_EN			BIT(2)
#define DP_PHYD_TX_CTL_0		(DP_PHYD_GLB_OFFSET + 0x44)
#define TX_LN_EN				GENMASK(7, 4)
#define DP_PHYD_IPMUX_CONTROL		(DP_PHYD_GLB_OFFSET + 0x98)
#define EDPTX_DSI_PHYD_SEL			BIT(0)

/*
 * Calibration normally comes from efuse, which is not programmed on the
 * development boards this runs on, and the MT8189 device tree describes no
 * nvmem cells to read it from. These are the values Linux falls back to in
 * the same situation, which are also the reset values of the registers.
 */
#define MTK_DP_PHY_CAL_BIAS_R		0xf
#define MTK_DP_PHY_CAL_CKTX_IMP		0x8
#define MTK_DP_PHY_CAL_LANE_IMP		0x8

struct mtk_dp_phy_priv {
	void __iomem *regs;
};

static void mtk_dp_phy_mask(struct mtk_dp_phy_priv *priv, u32 offset, u32 val,
			    u32 mask)
{
	clrsetbits_le32(priv->regs + offset, mask, val & mask);
}

static int mtk_dp_phy_init(struct phy *phy)
{
	struct mtk_dp_phy_priv *priv = dev_get_priv(phy->dev);
	int lane;

	/* the PHY is shared with DSI; steer it to eDP */
	mtk_dp_phy_mask(priv, DP_PHYD_IPMUX_CONTROL, 0, EDPTX_DSI_PHYD_SEL);
	mtk_dp_phy_mask(priv, DP_PHYD_PLL_CTL_0, PLL_CTL_0_EN, PLL_CTL_0_EN);

	mtk_dp_phy_mask(priv, DP_PHYA_GLB_BIAS_GEN_0,
			FIELD_PREP(XTP_GLB_BIAS_INT_R_CTRL, MTK_DP_PHY_CAL_BIAS_R),
			XTP_GLB_BIAS_INT_R_CTRL);
	mtk_dp_phy_mask(priv, DP_PHYA_GLB_DPAUX_TX,
			FIELD_PREP(CKM_PT0_CKTX_IMPSEL, MTK_DP_PHY_CAL_CKTX_IMP),
			CKM_PT0_CKTX_IMPSEL);

	for (lane = 0; lane < MTK_DP_PHY_MAX_LANES; lane++)
		mtk_dp_phy_mask(priv, DP_PHYA_LANE_TX_0(lane),
				FIELD_PREP(XTP_LN_TX_IMPSEL_PMOS,
					   MTK_DP_PHY_CAL_LANE_IMP) |
				FIELD_PREP(XTP_LN_TX_IMPSEL_NMOS,
					   MTK_DP_PHY_CAL_LANE_IMP),
				XTP_LN_TX_IMPSEL_PMOS | XTP_LN_TX_IMPSEL_NMOS);

	return 0;
}

static int mtk_dp_phy_power_on(struct phy *phy)
{
	struct mtk_dp_phy_priv *priv = dev_get_priv(phy->dev);

	/* the AUX channel, with RX deglitch and input clock detection */
	writel(DPAUX_RX_EN | XTP_GLB_CKDET_EN | DPAUX_RX_DEGLITCH_EN,
	       priv->regs + DP_PHYD_AUX_RX_CTL);
	mtk_dp_phy_mask(priv, DP_PHYA_GLB_FORCE_CTRL_1, 0,
			CKM_CKTX0_EN_FORCE_MODE);

	return 0;
}

static void mtk_dp_phy_disable_lanes(struct mtk_dp_phy_priv *priv)
{
	u32 lanes = FIELD_GET(TX_LN_EN, readl(priv->regs + DP_PHYD_TX_CTL_0));

	/* lanes have to be disabled one at a time, from the last one down */
	while (lanes) {
		lanes &= ~BIT(fls(lanes) - 1);
		mtk_dp_phy_mask(priv, DP_PHYD_TX_CTL_0,
				FIELD_PREP(TX_LN_EN, lanes), TX_LN_EN);
	}
}

static int mtk_dp_phy_power_off(struct phy *phy)
{
	struct mtk_dp_phy_priv *priv = dev_get_priv(phy->dev);

	mtk_dp_phy_mask(priv, DP_PHYA_GLB_FORCE_CTRL_1, CKM_CKTX0_EN_FORCE_MODE,
			CKM_CKTX0_EN_FORCE_MODE);
	writel(0, priv->regs + DP_PHYD_AUX_RX_CTL);
	mtk_dp_phy_disable_lanes(priv);

	return 0;
}

/*
 * Reset the digital part of the PHY, keeping the lanes that were enabled and
 * dropping any forced voltage swing and pre-emphasis.
 */
static int mtk_dp_phy_reset(struct phy *phy)
{
	struct mtk_dp_phy_priv *priv = dev_get_priv(phy->dev);
	u32 lanes = readl(priv->regs + DP_PHYD_TX_CTL_0) & TX_LN_EN;
	int lane;

	mtk_dp_phy_mask(priv, DP_PHYD_SW_RST, 0, DP_GLB_SW_RST_PHYD);
	udelay(50);
	mtk_dp_phy_mask(priv, DP_PHYD_SW_RST, DP_GLB_SW_RST_PHYD,
			DP_GLB_SW_RST_PHYD);
	mtk_dp_phy_mask(priv, DP_PHYD_TX_CTL_0, lanes, TX_LN_EN);

	for (lane = 0; lane < MTK_DP_PHY_MAX_LANES; lane++)
		mtk_dp_phy_mask(priv, DP_PHYD_LANE_DRIVING_FORCE(lane), 0,
				TX_FORCE_VOLT_SWING_EN | TX_FORCE_VOLT_SWING_VAL |
				TX_FORCE_PRE_EMPH_EN | TX_FORCE_PRE_EMPH_VAL);

	return 0;
}

static int mtk_dp_phy_configure(struct phy *phy, void *params)
{
	struct mtk_dp_phy_priv *priv = dev_get_priv(phy->dev);
	struct phy_configure_opts_dp *opts = params;
	unsigned int lane;
	u32 val;

	if (opts->lanes > MTK_DP_PHY_MAX_LANES) {
		dev_err(phy->dev, "invalid lane count %u\n", opts->lanes);
		return -EINVAL;
	}

	if (opts->set_rate) {
		switch (opts->link_rate) {
		case 1620:
			val = BIT_RATE_RBR;
			break;
		case 2700:
			val = BIT_RATE_HBR;
			break;
		case 5400:
			val = BIT_RATE_HBR2;
			break;
		case 8100:
			val = BIT_RATE_HBR3;
			break;
		default:
			dev_err(phy->dev, "unsupported link rate %u\n",
				opts->link_rate);
			return -EINVAL;
		}

		writel(val, priv->regs + DP_PHYD_BIT_RATE);
	}

	if (opts->set_lanes)
		mtk_dp_phy_mask(priv, DP_PHYD_TX_CTL_0,
				FIELD_PREP(TX_LN_EN, BIT(opts->lanes) - 1),
				TX_LN_EN);

	if (opts->set_voltages) {
		for (lane = 0; lane < opts->lanes; lane++) {
			u32 reg = DP_PHYD_LANE_DRIVING_FORCE(lane);

			mtk_dp_phy_mask(priv, reg,
					FIELD_PREP(TX_FORCE_VOLT_SWING_VAL,
						   opts->voltage[lane]) |
					FIELD_PREP(TX_FORCE_PRE_EMPH_VAL,
						   opts->pre[lane]),
					TX_FORCE_VOLT_SWING_VAL |
					TX_FORCE_PRE_EMPH_VAL);
			mtk_dp_phy_mask(priv, reg,
					TX_FORCE_VOLT_SWING_EN |
					TX_FORCE_PRE_EMPH_EN,
					TX_FORCE_VOLT_SWING_EN |
					TX_FORCE_PRE_EMPH_EN);
		}
	}

	mtk_dp_phy_mask(priv, DP_PHYD_PLL_CTL_1, opts->ssc ? TPLL_SSC_EN : 0,
			TPLL_SSC_EN);

	return 0;
}

static int mtk_dp_phy_probe(struct udevice *dev)
{
	struct mtk_dp_phy_priv *priv = dev_get_priv(dev);

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

static const struct phy_ops mtk_dp_phy_ops = {
	.init = mtk_dp_phy_init,
	.power_on = mtk_dp_phy_power_on,
	.power_off = mtk_dp_phy_power_off,
	.reset = mtk_dp_phy_reset,
	.configure = mtk_dp_phy_configure,
};

static const struct udevice_id mtk_dp_phy_ids[] = {
	{ .compatible = "mediatek,mt8189-edp-phy" },
	{ }
};

U_BOOT_DRIVER(mtk_dp_phy) = {
	.name = "mtk-dp-phy",
	.id = UCLASS_PHY,
	.of_match = mtk_dp_phy_ids,
	.ops = &mtk_dp_phy_ops,
	.probe = mtk_dp_phy_probe,
	.priv_auto = sizeof(struct mtk_dp_phy_priv),
};
