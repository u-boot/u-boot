// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#include <malloc.h>
#include <miiphy.h>
#include <linux/delay.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include <linux/io.h>
#include "mtk_eth.h"
#include "mt753x.h"

#include "../mdio-mt7531-mmio.h"

static int mt7988_reg_read(struct mt753x_switch_priv *priv, u32 reg, u32 *data)
{
	*data = readl(priv->epriv.ethsys_base + GSW_BASE + reg);

	return 0;
}

static int mt7988_reg_write(struct mt753x_switch_priv *priv, u32 reg, u32 data)
{
	writel(data, priv->epriv.ethsys_base + GSW_BASE + reg);

	return 0;
}

static void mt7988_phy_setting(struct mt753x_switch_priv *priv)
{
	struct mii_dev *mdio_bus = priv->mdio_bus;
	u16 val;
	u32 i;

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		u16 addr = MT753X_PHY_ADDR(priv->phy_base, i);

		/* Set PHY to PHY page 1 */
		mt7531_mdio_mmio_write(mdio_bus, addr, MDIO_DEVAD_NONE,
				       0x1f, 0x1);

		/* Enable HW auto downshift */
		val = mt7531_mdio_mmio_read(mdio_bus, addr, MDIO_DEVAD_NONE,
					    PHY_EXT_REG_14);
		val |= PHY_EN_DOWN_SHFIT;
		mt7531_mdio_mmio_write(mdio_bus, addr, MDIO_DEVAD_NONE,
				       PHY_EXT_REG_14, val);

		/* PHY link down power saving enable */
		val = mt7531_mdio_mmio_read(mdio_bus, addr, MDIO_DEVAD_NONE,
					    PHY_EXT_REG_17);
		val |= PHY_LINKDOWN_POWER_SAVING_EN;
		mt7531_mdio_mmio_write(mdio_bus, addr, MDIO_DEVAD_NONE,
				       PHY_EXT_REG_17, val);

		/* Restore PHY to PHY page 0 */
		mt7531_mdio_mmio_write(mdio_bus, addr, MDIO_DEVAD_NONE,
				       0x1f, 0x0);
	}
}

static void mt7988_mac_control(struct mtk_eth_switch_priv *swpriv, bool enable)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;
	u32 pmcr = FORCE_MODE_LNK;

	if (enable)
		pmcr = priv->pmcr;

	mt7988_reg_write(priv, PMCR_REG(6), pmcr);
}

static int mt7988_mdio_register(struct mt753x_switch_priv *priv)
{
	struct mt7531_mdio_mmio_priv *mdio_priv;
	struct mii_dev *mdio_bus = mdio_alloc();
	int ret;

	if (!mdio_bus)
		return -ENOMEM;

	mdio_priv = malloc(sizeof(*mdio_priv));
	if (!mdio_priv)
		return -ENOMEM;

	mdio_priv->switch_regs = (phys_addr_t)priv->epriv.ethsys_base + GSW_BASE;

	mdio_bus->read = mt7531_mdio_mmio_read;
	mdio_bus->write = mt7531_mdio_mmio_write;
	snprintf(mdio_bus->name, sizeof(mdio_bus->name), priv->epriv.sw->name);

	mdio_bus->priv = mdio_priv;

	ret = mdio_register(mdio_bus);
	if (ret) {
		free(mdio_bus->priv);
		mdio_free(mdio_bus);
		return ret;
	}

	priv->mdio_bus = mdio_bus;

	return 0;
}

static int mt7988_setup(struct mtk_eth_switch_priv *swpriv)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;
	struct mii_dev *mdio_bus;
	u16 phy_addr, phy_val;
	int ret, i;
	u32 pmcr;

	priv->smi_addr = MT753X_DFL_SMI_ADDR;
	priv->phy_base = (priv->smi_addr + 1) & MT753X_SMI_ADDR_MASK;
	priv->reg_read = mt7988_reg_read;
	priv->reg_write = mt7988_reg_write;

	ret = mt7988_mdio_register(priv);
	if (ret)
		return ret;

	mdio_bus = priv->mdio_bus;

	/* Turn off PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->phy_base, i);
		phy_val = mt7531_mdio_mmio_read(mdio_bus, phy_addr,
						MDIO_DEVAD_NONE, MII_BMCR);
		phy_val |= BMCR_PDOWN;
		mt7531_mdio_mmio_write(mdio_bus, phy_addr, MDIO_DEVAD_NONE,
				       MII_BMCR, phy_val);
	}

	switch (priv->epriv.phy_interface) {
	case PHY_INTERFACE_MODE_USXGMII:
		/* Use CPU bridge instead of actual USXGMII path */

		/* Disable GDM1 RX CRC stripping */
		/* mtk_fe_rmw(priv, 0x500, BIT(16), 0); */

		/* Set GDM1 no drop */
		mtk_fe_rmw(priv->epriv.eth, PSE_NO_DROP_CFG_REG, 0,
			   PSE_NO_DROP_GDM1);

		/* Enable GSW CPU bridge as USXGMII */
		/* mtk_fe_rmw(priv, 0x504, BIT(31), BIT(31)); */

		/* Enable GDM1 to GSW CPU bridge */
		mtk_gmac_rmw(priv->epriv.eth, GMAC_MAC_MISC_REG, 0, BIT(0));

		/* XGMAC force link up */
		mtk_gmac_rmw(priv->epriv.eth, GMAC_XGMAC_STS_REG, 0,
			     P1_XGMAC_FORCE_LINK);

		/* Setup GSW CPU bridge IPG */
		mtk_gmac_rmw(priv->epriv.eth, GMAC_GSW_CFG_REG,
			     GSWTX_IPG_M | GSWRX_IPG_M,
			     (0xB << GSWTX_IPG_S) | (0xB << GSWRX_IPG_S));
		break;
	default:
		printf("Error: MT7988 GSW does not support %s interface\n",
		       phy_string_for_interface(priv->epriv.phy_interface));
		break;
	}

	pmcr = MT7988_FORCE_MODE |
	       (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
	       MAC_MODE | MAC_TX_EN | MAC_RX_EN |
	       BKOFF_EN | BACKPR_EN |
	       FORCE_RX_FC | FORCE_TX_FC |
	       (SPEED_1000M << FORCE_SPD_S) | FORCE_DPX |
	       FORCE_LINK;

	priv->pmcr = pmcr;

	/* Keep MAC link down before starting eth */
	mt7988_reg_write(priv, PMCR_REG(6), FORCE_MODE_LNK);

	/* Enable port isolation to block inter-port communication */
	mt753x_port_isolation(priv);

	/* Turn on PHYs */
	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		phy_addr = MT753X_PHY_ADDR(priv->phy_base, i);
		phy_val = mt7531_mdio_mmio_read(mdio_bus, phy_addr,
						MDIO_DEVAD_NONE, MII_BMCR);
		phy_val &= ~BMCR_PDOWN;
		mt7531_mdio_mmio_write(mdio_bus, phy_addr, MDIO_DEVAD_NONE,
				       MII_BMCR, phy_val);
	}

	mt7988_phy_setting(priv);

	return 0;
}

static int mt7988_cleanup(struct mtk_eth_switch_priv *swpriv)
{
	struct mt753x_switch_priv *priv = (struct mt753x_switch_priv *)swpriv;
	struct mii_dev *mdio_bus = priv->mdio_bus;

	mdio_unregister(mdio_bus);
	free(mdio_bus->priv);
	mdio_free(mdio_bus);

	return 0;
}

MTK_ETH_SWITCH(mt7988) = {
	.name = "mt7988",
	.desc = "MediaTek MT7988 built-in switch",
	.priv_size = sizeof(struct mt753x_switch_priv),
	.reset_wait_time = 50,

	.setup = mt7988_setup,
	.cleanup = mt7988_cleanup,
	.mac_control = mt7988_mac_control,
};
