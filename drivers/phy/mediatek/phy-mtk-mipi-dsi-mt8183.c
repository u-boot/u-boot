// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019 MediaTek Inc.
 * Copyright (c) 2026 BayLibre, SAS
 * Author: Jitao Shi <jitao.shi@mediatek.com>
 */

#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/math64.h>

#include "phy-mtk-io.h"
#include "phy-mtk-mipi-dsi.h"

#define MIPITX_LANE_CON		0x000c
#define RG_DSI_BG_LPF_EN		BIT(6)
#define RG_DSI_BG_CORE_EN		BIT(7)
#define RG_DSI_PAD_TIEL_SEL		BIT(8)

#define MIPITX_VOLTAGE_SEL	0x0010
#define RG_DSI_HSTX_LDO_REF_SEL		GENMASK(9, 6)

#define MIPITX_PLL_PWR		0x0028
#define AD_DSI_PLL_SDM_PWR_ON		BIT(0)
#define AD_DSI_PLL_SDM_ISO_EN		BIT(1)

#define MIPITX_PLL_CON0		0x002c

#define MIPITX_PLL_CON1		0x0030
#define RG_DSI_PLL_EN			BIT(4)
#define RG_DSI_PLL_POSDIV		GENMASK(10, 8)

#define MIPITX_PLL_CON4		0x003c
#define RG_DSI_PLL_IBIAS		GENMASK(11, 10)

#define MIPITX_D2P_RTCODE	0x0100
#define MIPITX_D2_SW_CTL_EN	0x0144
#define MIPITX_D0_SW_CTL_EN	0x0244

#define MIPITX_CK_CKMODE_EN	0x0328
#define DSI_CK_CKMODE_EN		BIT(0)

#define MIPITX_CK_SW_CTL_EN	0x0344
#define MIPITX_D1_SW_CTL_EN	0x0444
#define MIPITX_D3_SW_CTL_EN	0x0544
#define DSI_SW_CTL_EN			BIT(0)

/* Reference clock feeding the PLL, fixed at 26MHz on all supported SoCs. */
#define MIPITX_PLL_REF_RATE	26000000

#define MIPITX_DATA_RATE_MIN	125000000ULL
#define MIPITX_DATA_RATE_MAX	1600000000ULL

static u64 mtk_mipi_tx_pll_determine_rate(struct mtk_mipi_tx *mipi_tx, u64 rate)
{
	return clamp_val(rate, MIPITX_DATA_RATE_MIN, MIPITX_DATA_RATE_MAX);
}

static int mtk_mipi_tx_pll_enable(struct mtk_mipi_tx *mipi_tx)
{
	void __iomem *base = mipi_tx->regs;
	unsigned int txdiv, txdiv0;
	u64 pcw;

	dev_dbg(mipi_tx->dev, "enable: %llu bps\n", mipi_tx->data_rate);

	if (mipi_tx->data_rate >= 2000000000) {
		txdiv = 1;
		txdiv0 = 0;
	} else if (mipi_tx->data_rate >= 1000000000) {
		txdiv = 2;
		txdiv0 = 1;
	} else if (mipi_tx->data_rate >= 500000000) {
		txdiv = 4;
		txdiv0 = 2;
	} else if (mipi_tx->data_rate > 250000000) {
		txdiv = 8;
		txdiv0 = 3;
	} else if (mipi_tx->data_rate >= 125000000) {
		txdiv = 16;
		txdiv0 = 4;
	} else {
		return -EINVAL;
	}

	mtk_phy_clear_bits(base + MIPITX_PLL_CON4, RG_DSI_PLL_IBIAS);

	mtk_phy_set_bits(base + MIPITX_PLL_PWR, AD_DSI_PLL_SDM_PWR_ON);
	mtk_phy_clear_bits(base + MIPITX_PLL_CON1, RG_DSI_PLL_EN);
	udelay(1);
	mtk_phy_clear_bits(base + MIPITX_PLL_PWR, AD_DSI_PLL_SDM_ISO_EN);
	pcw = div_u64((mipi_tx->data_rate * txdiv) << 24, MIPITX_PLL_REF_RATE);
	writel(pcw, base + MIPITX_PLL_CON0);
	mtk_phy_update_field(base + MIPITX_PLL_CON1, RG_DSI_PLL_POSDIV, txdiv0);
	mtk_phy_set_bits(base + MIPITX_PLL_CON1, RG_DSI_PLL_EN);

	return 0;
}

static void mtk_mipi_tx_pll_disable(struct mtk_mipi_tx *mipi_tx)
{
	void __iomem *base = mipi_tx->regs;

	mtk_phy_clear_bits(base + MIPITX_PLL_CON1, RG_DSI_PLL_EN);

	mtk_phy_set_bits(base + MIPITX_PLL_PWR, AD_DSI_PLL_SDM_ISO_EN);
	mtk_phy_clear_bits(base + MIPITX_PLL_PWR, AD_DSI_PLL_SDM_PWR_ON);
}

static void mtk_mipi_tx_config_calibration_data(struct mtk_mipi_tx *mipi_tx)
{
	int i, j;

	for (i = 0; i < 5; i++) {
		if ((mipi_tx->rt_code[i] & 0x1f) == 0)
			mipi_tx->rt_code[i] |= 0x10;

		if ((mipi_tx->rt_code[i] >> 5 & 0x1f) == 0)
			mipi_tx->rt_code[i] |= 0x10 << 5;

		for (j = 0; j < 10; j++)
			mtk_phy_update_bits(mipi_tx->regs +
				MIPITX_D2P_RTCODE * (i + 1) + j * 4,
				1, mipi_tx->rt_code[i] >> j & 1);
	}
}

static void mtk_mipi_tx_power_on_signal(struct mtk_mipi_tx *mipi_tx)
{
	void __iomem *base = mipi_tx->regs;

	/* BG_LPF_EN / BG_CORE_EN */
	writel(RG_DSI_PAD_TIEL_SEL | RG_DSI_BG_CORE_EN, base + MIPITX_LANE_CON);
	udelay(30);
	writel(RG_DSI_BG_CORE_EN | RG_DSI_BG_LPF_EN, base + MIPITX_LANE_CON);

	/* Switch OFF each Lane */
	mtk_phy_clear_bits(base + MIPITX_D0_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_clear_bits(base + MIPITX_D1_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_clear_bits(base + MIPITX_D2_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_clear_bits(base + MIPITX_D3_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_clear_bits(base + MIPITX_CK_SW_CTL_EN, DSI_SW_CTL_EN);

	mtk_phy_update_field(base + MIPITX_VOLTAGE_SEL, RG_DSI_HSTX_LDO_REF_SEL,
			     (mipi_tx->mipitx_drive - 3000) / 200);

	mtk_mipi_tx_config_calibration_data(mipi_tx);

	mtk_phy_set_bits(base + MIPITX_CK_CKMODE_EN, DSI_CK_CKMODE_EN);
}

static void mtk_mipi_tx_power_off_signal(struct mtk_mipi_tx *mipi_tx)
{
	void __iomem *base = mipi_tx->regs;

	/* Switch ON each Lane */
	mtk_phy_set_bits(base + MIPITX_D0_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_set_bits(base + MIPITX_D1_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_set_bits(base + MIPITX_D2_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_set_bits(base + MIPITX_D3_SW_CTL_EN, DSI_SW_CTL_EN);
	mtk_phy_set_bits(base + MIPITX_CK_SW_CTL_EN, DSI_SW_CTL_EN);

	writel(RG_DSI_PAD_TIEL_SEL | RG_DSI_BG_CORE_EN, base + MIPITX_LANE_CON);
	writel(RG_DSI_PAD_TIEL_SEL, base + MIPITX_LANE_CON);
}

const struct mtk_mipitx_data mt8183_mipitx_data = {
	.pll_determine_rate = mtk_mipi_tx_pll_determine_rate,
	.pll_enable = mtk_mipi_tx_pll_enable,
	.pll_disable = mtk_mipi_tx_pll_disable,
	.mipi_tx_enable_signal = mtk_mipi_tx_power_on_signal,
	.mipi_tx_disable_signal = mtk_mipi_tx_power_off_signal,
};
