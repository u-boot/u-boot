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

#define CHIP_REV			0x7ffc
#define CHIP_NAME_S			16
#define CHIP_NAME_M			0xffff0000
#define CHIP_REV_S			0
#define CHIP_REV_M			0x0f

static void mt7530_core_reg_write(struct mt753x_switch_priv *priv, u32 reg,
				  u32 val)
{
	u8 phy_addr = MT753X_PHY_ADDR(priv->phy_base, 0);

	mtk_mmd_ind_write(priv->epriv.eth, phy_addr, 0x1f, reg, val);
}

static int mt7530_pad_clk_setup(struct mt753x_switch_priv *priv, int mode)
{
	u32 ncpo1, ssc_delta;

	switch (mode) {
	case PHY_INTERFACE_MODE_RGMII:
		ncpo1 = 0x0c80;
		ssc_delta = 0x87;
		break;

	default:
		printf("error: xMII mode %d is not supported\n", mode);
		return -EINVAL;
	}

	/* Disable MT7530 core clock */
	mt7530_core_reg_write(priv, CORE_TRGMII_GSW_CLK_CG, 0);

	/* Disable MT7530 PLL */
	mt7530_core_reg_write(priv, CORE_GSWPLL_GRP1,
			      (2 << RG_GSWPLL_POSDIV_200M_S) |
			      (32 << RG_GSWPLL_FBKDIV_200M_S));

	/* For MT7530 core clock = 500Mhz */
	mt7530_core_reg_write(priv, CORE_GSWPLL_GRP2,
			      (1 << RG_GSWPLL_POSDIV_500M_S) |
			      (25 << RG_GSWPLL_FBKDIV_500M_S));

	/* Enable MT7530 PLL */
	mt7530_core_reg_write(priv, CORE_GSWPLL_GRP1,
			      (2 << RG_GSWPLL_POSDIV_200M_S) |
			      (32 << RG_GSWPLL_FBKDIV_200M_S) |
			      RG_GSWPLL_EN_PRE);

	udelay(20);

	mt7530_core_reg_write(priv, CORE_TRGMII_GSW_CLK_CG, REG_GSWCK_EN);

	/* Setup the MT7530 TRGMII Tx Clock */
	mt7530_core_reg_write(priv, CORE_PLL_GROUP5, ncpo1);
	mt7530_core_reg_write(priv, CORE_PLL_GROUP6, 0);
	mt7530_core_reg_write(priv, CORE_PLL_GROUP10, ssc_delta);
	mt7530_core_reg_write(priv, CORE_PLL_GROUP11, ssc_delta);
	mt7530_core_reg_write(priv, CORE_PLL_GROUP4, RG_SYSPLL_DDSFBK_EN |
			      RG_SYSPLL_BIAS_EN | RG_SYSPLL_BIAS_LPF_EN);

	mt7530_core_reg_write(priv, CORE_PLL_GROUP2,
			      RG_SYSPLL_EN_NORMAL | RG_SYSPLL_VODEN |
			      (1 << RG_SYSPLL_POSDIV_S));

	mt7530_core_reg_write(priv, CORE_PLL_GROUP7,
			      RG_LCDDS_PCW_NCPO_CHG | (3 << RG_LCCDS_C_S) |
			      RG_LCDDS_PWDB | RG_LCDDS_ISO_EN);

	/* Enable MT7530 core clock */
	mt7530_core_reg_write(priv, CORE_TRGMII_GSW_CLK_CG,
			      REG_GSWCK_EN | REG_TRGMIICK_EN);

	return 0;
}

static void mt7530_mac_control(struct mtk_eth_switch_priv *swpriv, bool enable)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;
	u32 pmcr = FORCE_MODE;

	if (enable)
		pmcr = priv->pmcr;

	mt753x_reg_write(priv, PMCR_REG(6), pmcr);
}

static int mt7530_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mt753x_switch_priv *priv = bus->priv;

	if (devad < 0)
		return mtk_mii_read(priv->epriv.eth, addr, reg);

	return mtk_mmd_ind_read(priv->epriv.eth, addr, devad, reg);
}

static int mt7530_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			     u16 val)
{
	struct mt753x_switch_priv *priv = bus->priv;

	if (devad < 0)
		return mtk_mii_write(priv->epriv.eth, addr, reg, val);

	return mtk_mmd_ind_write(priv->epriv.eth, addr, devad, reg, val);
}

static int mt7530_mdio_register(struct mt753x_switch_priv *priv)
{
	struct mii_dev *mdio_bus = mdio_alloc();
	int ret;

	if (!mdio_bus)
		return -ENOMEM;

	mdio_bus->read = mt7530_mdio_read;
	mdio_bus->write = mt7530_mdio_write;
	snprintf(mdio_bus->name, sizeof(mdio_bus->name), priv->epriv.sw->name);

	mdio_bus->priv = priv;

	ret = mdio_register(mdio_bus);
	if (ret) {
		mdio_free(mdio_bus);
		return ret;
	}

	priv->mdio_bus = mdio_bus;

	return 0;
}

static int mt7530_setup(struct mtk_eth_switch_priv *swpriv)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;
	u16 phy_addr, phy_val;
	u32 i, val, txdrv;

	priv->smi_addr = MT753X_DFL_SMI_ADDR;
	priv->reg_read = mt753x_mdio_reg_read;
	priv->reg_write = mt753x_mdio_reg_write;

	if (!MTK_HAS_CAPS(priv->epriv.soc->caps, MTK_TRGMII_MT7621_CLK)) {
		/* Select 250MHz clk for RGMII mode */
		mtk_ethsys_rmw(priv->epriv.eth, ETHSYS_CLKCFG0_REG,
			       ETHSYS_TRGMII_CLK_SEL362_5, 0);

		txdrv = 8;
	} else {
		txdrv = 4;
	}

	/* Modify HWTRAP first to allow direct access to internal PHYs */
	mt753x_reg_read(priv, HWTRAP_REG, &val);
	val |= CHG_TRAP;
	val &= ~C_MDIO_BPS;
	mt753x_reg_write(priv, MHWTRAP_REG, val);

	/* Calculate the phy base address */
	val = ((val & SMI_ADDR_M) >> SMI_ADDR_S) << 3;
	priv->phy_base = (val | 0x7) + 1;

	/* Turn off PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->phy_base, i);
		phy_val = mtk_mii_read(priv->epriv.eth, phy_addr, MII_BMCR);
		phy_val |= BMCR_PDOWN;
		mtk_mii_write(priv->epriv.eth, phy_addr, MII_BMCR, phy_val);
	}

	/* Force MAC link down before reset */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE);
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE);

	/* MT7530 reset */
	mt753x_reg_write(priv, SYS_CTRL_REG, SW_SYS_RST | SW_REG_RST);
	udelay(100);

	val = (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
	      MAC_MODE | FORCE_MODE |
	      MAC_TX_EN | MAC_RX_EN |
	      BKOFF_EN | BACKPR_EN |
	      (SPEED_1000M << FORCE_SPD_S) |
	      FORCE_DPX | FORCE_LINK;

	/* MT7530 Port6: Forced 1000M/FD, FC disabled */
	priv->pmcr = val;

	/* MT7530 Port5: Forced link down */
	mt753x_reg_write(priv, PMCR_REG(5), FORCE_MODE);

	/* Keep MAC link down before starting eth */
	mt753x_reg_write(priv, PMCR_REG(6), FORCE_MODE);

	/* MT7530 Port6: Set to RGMII */
	mt753x_reg_rmw(priv, MT7530_P6ECR, P6_INTF_MODE_M, P6_INTF_MODE_RGMII);

	/* Hardware Trap: Enable Port6, Disable Port5 */
	mt753x_reg_read(priv, HWTRAP_REG, &val);
	val |= CHG_TRAP | LOOPDET_DIS | P5_INTF_DIS |
	       (P5_INTF_SEL_GMAC5 << P5_INTF_SEL_S) |
	       (P5_INTF_MODE_RGMII << P5_INTF_MODE_S);
	val &= ~(C_MDIO_BPS | P6_INTF_DIS);
	mt753x_reg_write(priv, MHWTRAP_REG, val);

	/* Setup switch core pll */
	mt7530_pad_clk_setup(priv, priv->epriv.phy_interface);

	/* Lower Tx Driving for TRGMII path */
	for (i = 0 ; i < NUM_TRGMII_CTRL ; i++)
		mt753x_reg_write(priv, MT7530_TRGMII_TD_ODT(i),
				 (txdrv << TD_DM_DRVP_S) |
				 (txdrv << TD_DM_DRVN_S));

	for (i = 0 ; i < NUM_TRGMII_CTRL; i++)
		mt753x_reg_rmw(priv, MT7530_TRGMII_RD(i), RD_TAP_M, 16);

	/* Enable port isolation to block inter-port communication */
	mt753x_port_isolation(priv);

	/* Turn on PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->phy_base, i);
		phy_val = mtk_mii_read(priv->epriv.eth, phy_addr, MII_BMCR);
		phy_val &= ~BMCR_PDOWN;
		mtk_mii_write(priv->epriv.eth, phy_addr, MII_BMCR, phy_val);
	}

	return mt7530_mdio_register(priv);
}

static int mt7530_cleanup(struct mtk_eth_switch_priv *swpriv)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;

	mdio_unregister(priv->mdio_bus);

	return 0;
}

static int mt7530_detect(struct mtk_eth_priv *priv)
{
	int ret;
	u32 rev;

	ret = __mt753x_mdio_reg_read(priv, MT753X_DFL_SMI_ADDR, CHIP_REV, &rev);
	if (ret)
		return ret;

	if (((rev & CHIP_NAME_M) >> CHIP_NAME_S) == 0x7530)
		return 0;

	return -ENODEV;
}

MTK_ETH_SWITCH(mt7530) = {
	.name = "mt7530",
	.desc = "MediaTek MT7530",
	.priv_size = sizeof(struct mt753x_switch_priv),
	.reset_wait_time = 1000,

	.detect = mt7530_detect,
	.setup = mt7530_setup,
	.cleanup = mt7530_cleanup,
	.mac_control = mt7530_mac_control,
};
