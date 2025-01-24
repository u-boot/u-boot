// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#include <miiphy.h>
#include <linux/delay.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include "mtk_eth.h"
#include "mt753x.h"

#define CHIP_REV			0x781C
#define CHIP_NAME_S			16
#define CHIP_NAME_M			0xffff0000
#define CHIP_REV_S			0
#define CHIP_REV_M			0x0f
#define CHIP_REV_E1			0x0

static int mt7531_core_reg_read(struct mt753x_switch_priv *priv, u32 reg)
{
	u8 phy_addr = MT753X_PHY_ADDR(priv->phy_base, 0);

	return mt7531_mmd_read(priv, phy_addr, 0x1f, reg);
}

static void mt7531_core_reg_write(struct mt753x_switch_priv *priv, u32 reg,
				  u32 val)
{
	u8 phy_addr = MT753X_PHY_ADDR(priv->phy_base, 0);

	mt7531_mmd_write(priv, phy_addr, 0x1f, reg, val);
}

static void mt7531_core_pll_setup(struct mt753x_switch_priv *priv)
{
	/* Step 1 : Disable MT7531 COREPLL */
	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, EN_COREPLL, 0);

	/* Step 2: switch to XTAL output */
	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, SW_CLKSW, SW_CLKSW);

	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_EN, 0);

	/* Step 3: disable PLLGP and enable program PLLGP */
	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, SW_PLLGP, SW_PLLGP);

	/* Step 4: program COREPLL output frequency to 500MHz */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_POSDIV_M,
		       2 << RG_COREPLL_POSDIV_S);
	udelay(25);

	/* Currently, support XTAL 25Mhz only */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_SDM_PCW_M,
		       0x140000 << RG_COREPLL_SDM_PCW_S);

	/* Set feedback divide ratio update signal to high */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_SDM_PCW_CHG,
		       RG_COREPLL_SDM_PCW_CHG);

	/* Wait for at least 16 XTAL clocks */
	udelay(10);

	/* Step 5: set feedback divide ratio update signal to low */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_SDM_PCW_CHG, 0);

	/* add enable 325M clock for SGMII */
	mt753x_reg_write(priv, MT7531_ANA_PLLGP_CR5, 0xad0000);

	/* add enable 250SSC clock for RGMII */
	mt753x_reg_write(priv, MT7531_ANA_PLLGP_CR2, 0x4f40000);

	/*Step 6: Enable MT7531 PLL */
	mt753x_reg_rmw(priv, MT7531_PLLGP_CR0, RG_COREPLL_EN, RG_COREPLL_EN);

	mt753x_reg_rmw(priv, MT7531_PLLGP_EN, EN_COREPLL, EN_COREPLL);

	udelay(25);
}

static int mt7531_port_sgmii_init(struct mt753x_switch_priv *priv, u32 port)
{
	if (port != 5 && port != 6) {
		printf("mt7531: port %d is not a SGMII port\n", port);
		return -EINVAL;
	}

	/* Set SGMII GEN2 speed(2.5G) */
	mt753x_reg_rmw(priv, MT7531_PHYA_CTRL_SIGNAL3(port), SGMSYS_SPEED_MASK,
		       FIELD_PREP(SGMSYS_SPEED_MASK, SGMSYS_SPEED_2500));

	/* Disable SGMII AN */
	mt753x_reg_rmw(priv, MT7531_PCS_CONTROL_1(port),
		       SGMII_AN_ENABLE, 0);

	/* SGMII force mode setting */
	mt753x_reg_write(priv, MT7531_SGMII_MODE(port), SGMII_FORCE_MODE);

	/* Release PHYA power down state */
	mt753x_reg_rmw(priv, MT7531_QPHY_PWR_STATE_CTRL(port),
		       SGMII_PHYA_PWD, 0);

	return 0;
}

static int mt7531_port_rgmii_init(struct mt753x_switch_priv *priv, u32 port)
{
	u32 val;

	if (port != 5) {
		printf("error: RGMII mode is not available for port %d\n",
		       port);
		return -EINVAL;
	}

	mt753x_reg_read(priv, MT7531_CLKGEN_CTRL, &val);
	val |= GP_CLK_EN;
	val &= ~GP_MODE_M;
	val |= GP_MODE_RGMII << GP_MODE_S;
	val |= TXCLK_NO_REVERSE;
	val |= RXCLK_NO_DELAY;
	val &= ~CLK_SKEW_IN_M;
	val |= CLK_SKEW_IN_NO_CHANGE << CLK_SKEW_IN_S;
	val &= ~CLK_SKEW_OUT_M;
	val |= CLK_SKEW_OUT_NO_CHANGE << CLK_SKEW_OUT_S;
	mt753x_reg_write(priv, MT7531_CLKGEN_CTRL, val);

	return 0;
}

static void mt7531_phy_setting(struct mt753x_switch_priv *priv)
{
	int i;
	u32 val;

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		/* Enable HW auto downshift */
		mt7531_mii_write(priv, i, 0x1f, 0x1);
		val = mt7531_mii_read(priv, i, PHY_EXT_REG_14);
		val |= PHY_EN_DOWN_SHFIT;
		mt7531_mii_write(priv, i, PHY_EXT_REG_14, val);

		/* PHY link down power saving enable */
		val = mt7531_mii_read(priv, i, PHY_EXT_REG_17);
		val |= PHY_LINKDOWN_POWER_SAVING_EN;
		mt7531_mii_write(priv, i, PHY_EXT_REG_17, val);

		val = mt7531_mmd_read(priv, i, 0x1e, PHY_DEV1E_REG_0C6);
		val &= ~PHY_POWER_SAVING_M;
		val |= PHY_POWER_SAVING_TX << PHY_POWER_SAVING_S;
		mt7531_mmd_write(priv, i, 0x1e, PHY_DEV1E_REG_0C6, val);
	}
}

static void mt7531_mac_control(struct mtk_eth_switch_priv *swpriv, bool enable)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;
	u32 pmcr = FORCE_MODE_LNK;

	if (enable)
		pmcr = priv->pmcr;

	mt753x_reg_write(priv, PMCR_REG(5), pmcr);
	mt753x_reg_write(priv, PMCR_REG(6), pmcr);
}

static int mt7531_setup(struct mtk_eth_switch_priv *swpriv)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;
	u32 i, val, pmcr, port5_sgmii;
	u16 phy_addr, phy_val;

	priv->smi_addr = MT753X_DFL_SMI_ADDR;
	priv->phy_base = (priv->smi_addr + 1) & MT753X_SMI_ADDR_MASK;
	priv->reg_read = mt753x_mdio_reg_read;
	priv->reg_write = mt753x_mdio_reg_write;

	/* Turn off PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->phy_base, i);
		phy_val = mt7531_mii_read(priv, phy_addr, MII_BMCR);
		phy_val |= BMCR_PDOWN;
		mt7531_mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	/* Force MAC link down before reset */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE_LNK);
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE_LNK);

	/* Switch soft reset */
	mt753x_reg_write(priv, SYS_CTRL_REG, SW_SYS_RST | SW_REG_RST);
	udelay(100);

	/* Enable MDC input Schmitt Trigger */
	mt753x_reg_rmw(priv, MT7531_SMT0_IOLB, SMT_IOLB_5_SMI_MDC_EN,
		       SMT_IOLB_5_SMI_MDC_EN);

	mt7531_core_pll_setup(priv);

	mt753x_reg_read(priv, MT7531_TOP_SIG_SR, &val);
	port5_sgmii = !!(val & PAD_DUAL_SGMII_EN);

	/* port5 support either RGMII or SGMII, port6 only support SGMII. */
	switch (priv->epriv.phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
		if (!port5_sgmii)
			mt7531_port_rgmii_init(priv, 5);
		break;

	case PHY_INTERFACE_MODE_2500BASEX:
		mt7531_port_sgmii_init(priv, 6);
		if (port5_sgmii)
			mt7531_port_sgmii_init(priv, 5);
		break;

	default:
		break;
	}

	pmcr = MT7531_FORCE_MODE |
	       (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
	       MAC_MODE | MAC_TX_EN | MAC_RX_EN |
	       BKOFF_EN | BACKPR_EN |
	       FORCE_RX_FC | FORCE_TX_FC |
	       (SPEED_1000M << FORCE_SPD_S) | FORCE_DPX |
	       FORCE_LINK;

	priv->pmcr = pmcr;

	/* Keep MAC link down before starting eth */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE_LNK);
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE_LNK);

	/* Enable port isolation to block inter-port communication */
	mt753x_port_isolation(priv);

	/* Turn on PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->phy_base, i);
		phy_val = mt7531_mii_read(priv, phy_addr, MII_BMCR);
		phy_val &= ~BMCR_PDOWN;
		mt7531_mii_write(priv, phy_addr, MII_BMCR, phy_val);
	}

	mt7531_phy_setting(priv);

	/* Enable Internal PHYs */
	val = mt7531_core_reg_read(priv, CORE_PLL_GROUP4);
	val |= MT7531_BYPASS_MODE;
	val &= ~MT7531_POWER_ON_OFF;
	mt7531_core_reg_write(priv, CORE_PLL_GROUP4, val);

	return mt7531_mdio_register(priv);
}

static int mt7531_cleanup(struct mtk_eth_switch_priv *swpriv)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;

	mdio_unregister(priv->mdio_bus);

	return 0;
}

static int mt7531_detect(struct mtk_eth_priv *priv)
{
	int ret;
	u32 rev;

	ret = __mt753x_mdio_reg_read(priv, MT753X_DFL_SMI_ADDR, CHIP_REV, &rev);
	if (ret)
		return ret;

	if (((rev & CHIP_NAME_M) >> CHIP_NAME_S) == 0x7531)
		return 0;

	return -ENODEV;
}

MTK_ETH_SWITCH(mt7531) = {
	.name = "mt7531",
	.desc = "MediaTek MT7531",
	.priv_size = sizeof(struct mt753x_switch_priv),
	.reset_wait_time = 200,

	.detect = mt7531_detect,
	.setup = mt7531_setup,
	.cleanup = mt7531_cleanup,
	.mac_control = mt7531_mac_control,
};
