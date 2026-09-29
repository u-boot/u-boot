// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Stanley Chu <stanley.chu@mediatek.com>
 *
 * Copyright (c) 2025, Igor Belwon <igor.belwon@mentallysanemainliners.org>
 */

#include <asm/io.h>
#include <clk.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/read.h>
#include <generic-phy.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <mapmem.h>

/* mphy register and offsets */
#define MP_GLB_DIG_8C		0x008C
#define FRC_PLL_ISO_EN		BIT(8)
#define PLL_ISO_EN		BIT(9)
#define FRC_FRC_PWR_ON		BIT(10)
#define PLL_PWR_ON		BIT(11)

#define MP_LN_DIG_RX_9C		0xA09C
#define FSM_DIFZ_FRC		BIT(18)

#define MP_LN_DIG_RX_AC		0xA0AC
#define FRC_RX_SQ_EN		BIT(0)
#define RX_SQ_EN		BIT(1)

#define MP_LN_RX_44		0xB044
#define FRC_CDR_PWR_ON		BIT(17)
#define CDR_PWR_ON		BIT(18)
#define FRC_CDR_ISO_EN		BIT(19)
#define CDR_ISO_EN		BIT(20)

struct mtk_ufs_phy {
	struct udevice *dev;
	void __iomem *mmio;

	struct clk_bulk clk_bulk;
};

static void ufs_mtk_phy_set_active(struct mtk_ufs_phy *phy)
{
	/* release DA_MP_PLL_PWR_ON */
	setbits_le32(phy->mmio + MP_GLB_DIG_8C, PLL_PWR_ON);
	clrbits_le32(phy->mmio + MP_GLB_DIG_8C, FRC_FRC_PWR_ON);

	/* release DA_MP_PLL_ISO_EN */
	clrbits_le32(phy->mmio + MP_GLB_DIG_8C, PLL_ISO_EN);
	clrbits_le32(phy->mmio + MP_GLB_DIG_8C, FRC_PLL_ISO_EN);

	/* release DA_MP_CDR_PWR_ON */
	setbits_le32(phy->mmio + MP_LN_RX_44, CDR_PWR_ON);
	clrbits_le32(phy->mmio + MP_LN_RX_44, FRC_CDR_PWR_ON);

	/* release DA_MP_CDR_ISO_EN */
	clrbits_le32(phy->mmio + MP_LN_RX_44, CDR_ISO_EN);
	clrbits_le32(phy->mmio + MP_LN_RX_44, FRC_CDR_ISO_EN);

	/* release DA_MP_RX0_SQ_EN */
	setbits_le32(phy->mmio + MP_LN_DIG_RX_AC, RX_SQ_EN);
	clrbits_le32(phy->mmio + MP_LN_DIG_RX_AC, FRC_RX_SQ_EN);

	/* delay 1us to wait DIFZ stable */
	udelay(1);

	/* release DIFZ */
	clrbits_le32(phy->mmio + MP_LN_DIG_RX_9C, FSM_DIFZ_FRC);
}

static int mtk_phy_power_on(struct phy *phy)
{
	struct mtk_ufs_phy *ufs_phy = dev_get_priv(phy->dev);
	int ret;

	ret = clk_enable_bulk(&ufs_phy->clk_bulk);
	if (ret) {
		dev_err(phy->dev, "failed to enable clocks (ret=%d)\n", ret);
		return ret;
	}

	ufs_mtk_phy_set_active(ufs_phy);

	return 0;
}

static void ufs_mtk_phy_set_inactive(struct mtk_ufs_phy *phy)
{
	/* Set PHY to Deep Hibernate mode */
	setbits_le32(phy->mmio + MP_LN_DIG_RX_9C, FSM_DIFZ_FRC);

	/* force DA_MP_RX0_SQ_EN */
	setbits_le32(phy->mmio + MP_LN_DIG_RX_AC, FRC_RX_SQ_EN);
	clrbits_le32(phy->mmio + MP_LN_DIG_RX_AC, RX_SQ_EN);

	/* force DA_MP_CDR_ISO_EN */
	setbits_le32(phy->mmio + MP_LN_RX_44, FRC_CDR_ISO_EN);
	setbits_le32(phy->mmio + MP_LN_RX_44, CDR_ISO_EN);

	/* force DA_MP_CDR_PWR_ON */
	setbits_le32(phy->mmio + MP_LN_RX_44, FRC_CDR_PWR_ON);
	clrbits_le32(phy->mmio + MP_LN_RX_44, CDR_PWR_ON);

	/* force DA_MP_PLL_ISO_EN */
	setbits_le32(phy->mmio + MP_GLB_DIG_8C, FRC_PLL_ISO_EN);
	setbits_le32(phy->mmio + MP_GLB_DIG_8C, PLL_ISO_EN);

	/* force DA_MP_PLL_PWR_ON */
	setbits_le32(phy->mmio + MP_GLB_DIG_8C, FRC_FRC_PWR_ON);
	clrbits_le32(phy->mmio + MP_GLB_DIG_8C, PLL_PWR_ON);
}

static int mtk_phy_power_off(struct phy *phy)
{
	struct mtk_ufs_phy *ufs_phy = dev_get_priv(phy->dev);
	int ret;

	ufs_mtk_phy_set_inactive(ufs_phy);

	ret = clk_disable_bulk(&ufs_phy->clk_bulk);
	if (ret)
		dev_err(phy->dev, "failed to disable clocks (ret=%d)\n", ret);

	return ret;
}

static const struct phy_ops mtk_ufs_phy_ops = {
	.power_on	= mtk_phy_power_on,
	.power_off	= mtk_phy_power_off,
};

static int mtk_ufs_phy_probe(struct udevice *dev)
{
	struct mtk_ufs_phy *phy = dev_get_priv(dev);
	fdt_addr_t addr;
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -ENOMEM;

	phy->dev = dev;
	phy->mmio = map_sysmem(addr, 0);

	ret = clk_get_bulk(dev, &phy->clk_bulk);
	if (ret)
		dev_err(dev, "Failed to get clocks (ret=%d)\n", ret);

	return ret;
}

static const struct udevice_id mtk_ufs_phy_id_table[] = {
	{.compatible = "mediatek,mt8183-ufsphy"},
	{},
};

U_BOOT_DRIVER(mtk_ufs_phy) = {
	.name		= "mtk-ufs_phy",
	.id		= UCLASS_PHY,
	.of_match	= mtk_ufs_phy_id_table,
	.ops		= &mtk_ufs_phy_ops,
	.probe		= mtk_ufs_phy_probe,
	.priv_auto	= sizeof(struct mtk_ufs_phy),
};
