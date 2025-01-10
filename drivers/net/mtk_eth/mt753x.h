/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#ifndef _MTK_ETH_MT753X_H_
#define _MTK_ETH_MT753X_H_

#include <phy.h>
#include <miiphy.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>

struct mtk_eth_priv;

#define MT753X_NUM_PHYS		5
#define MT753X_NUM_PORTS	7
#define MT753X_DFL_SMI_ADDR	31
#define MT753X_SMI_ADDR_MASK	0x1f

#define MT753X_PHY_ADDR(base, addr) \
	(((base) + (addr)) & 0x1f)

/* MT7530 Registers */
#define PCR_REG(p)			(0x2004 + (p) * 0x100)
#define PORT_MATRIX_S			16
#define PORT_MATRIX_M			0xff0000

#define PVC_REG(p)			(0x2010 + (p) * 0x100)
#define STAG_VPID_S			16
#define STAG_VPID_M			0xffff0000
#define VLAN_ATTR_S			6
#define VLAN_ATTR_M			0xc0

/* VLAN_ATTR: VLAN attributes */
#define VLAN_ATTR_USER			0
#define VLAN_ATTR_STACK			1
#define VLAN_ATTR_TRANSLATION		2
#define VLAN_ATTR_TRANSPARENT		3

#define PMCR_REG(p)			(0x3000 + (p) * 0x100)
/* XXX: all fields of MT7530 are defined under GMAC_PORT_MCR
 * MT7531 specific fields are defined below
 */
#define FORCE_MODE_EEE1G		BIT(25)
#define FORCE_MODE_EEE100		BIT(26)
#define FORCE_MODE_TX_FC		BIT(27)
#define FORCE_MODE_RX_FC		BIT(28)
#define FORCE_MODE_DPX			BIT(29)
#define FORCE_MODE_SPD			BIT(30)
#define FORCE_MODE_LNK			BIT(31)
#define MT7531_FORCE_MODE		FORCE_MODE_TX_FC | FORCE_MODE_RX_FC | \
					FORCE_MODE_DPX   | FORCE_MODE_SPD | \
					FORCE_MODE_LNK
#define MT7988_FORCE_MODE		FORCE_MODE_TX_FC | FORCE_MODE_RX_FC | \
					FORCE_MODE_DPX   | FORCE_MODE_SPD | \
					FORCE_MODE_LNK

/* MT7531 SGMII Registers */
#define MT7531_SGMII_REG_BASE		0x5000
#define MT7531_SGMII_REG_PORT_BASE	0x1000
#define MT7531_SGMII_REG(p, r)		(MT7531_SGMII_REG_BASE + \
					(p) * MT7531_SGMII_REG_PORT_BASE + (r))
#define MT7531_PCS_CONTROL_1(p)		MT7531_SGMII_REG(((p) - 5), 0x00)
#define MT7531_SGMII_MODE(p)		MT7531_SGMII_REG(((p) - 5), 0x20)
#define MT7531_QPHY_PWR_STATE_CTRL(p)	MT7531_SGMII_REG(((p) - 5), 0xe8)
#define MT7531_PHYA_CTRL_SIGNAL3(p)	MT7531_SGMII_REG(((p) - 5), 0x128)
#define MT7531_PHYA_ANA_SYSPLL(p)	MT7531_SGMII_REG(((p) - 5), 0x158)
/* XXX: all fields of MT7531 SGMII  are defined under SGMSYS */

/* MT753x System Control Register */
#define SYS_CTRL_REG			0x7000
#define SW_PHY_RST			BIT(2)
#define SW_SYS_RST			BIT(1)
#define SW_REG_RST			BIT(0)

/* MT7531  */
#define MT7531_PHY_IAC			0x701c
/* XXX: all fields are defined under GMAC_PIAC_REG */

#define MT7531_CLKGEN_CTRL		0x7500
#define CLK_SKEW_OUT_S			8
#define CLK_SKEW_OUT_M			0x300
#define CLK_SKEW_IN_S			6
#define CLK_SKEW_IN_M			0xc0
#define RXCLK_NO_DELAY			BIT(5)
#define TXCLK_NO_REVERSE		BIT(4)
#define GP_MODE_S			1
#define GP_MODE_M			0x06
#define GP_CLK_EN			BIT(0)

/* Values of GP_MODE */
#define GP_MODE_RGMII			0
#define GP_MODE_MII			1
#define GP_MODE_REV_MII			2

/* Values of CLK_SKEW_IN */
#define CLK_SKEW_IN_NO_CHANGE		0
#define CLK_SKEW_IN_DELAY_100PPS	1
#define CLK_SKEW_IN_DELAY_200PPS	2
#define CLK_SKEW_IN_REVERSE		3

/* Values of CLK_SKEW_OUT */
#define CLK_SKEW_OUT_NO_CHANGE		0
#define CLK_SKEW_OUT_DELAY_100PPS	1
#define CLK_SKEW_OUT_DELAY_200PPS	2
#define CLK_SKEW_OUT_REVERSE		3

#define HWTRAP_REG			0x7800
/* MT7530 Modified Hardware Trap Status Registers */
#define MHWTRAP_REG			0x7804
#define CHG_TRAP			BIT(16)
#define LOOPDET_DIS			BIT(14)
#define P5_INTF_SEL_S			13
#define P5_INTF_SEL_M			0x2000
#define SMI_ADDR_S			11
#define SMI_ADDR_M			0x1800
#define XTAL_FSEL_S			9
#define XTAL_FSEL_M			0x600
#define P6_INTF_DIS			BIT(8)
#define P5_INTF_MODE_S			7
#define P5_INTF_MODE_M			0x80
#define P5_INTF_DIS			BIT(6)
#define C_MDIO_BPS			BIT(5)
#define CHIP_MODE_S			0
#define CHIP_MODE_M			0x0f

/* P5_INTF_SEL: Interface type of Port5 */
#define P5_INTF_SEL_GPHY		0
#define P5_INTF_SEL_GMAC5		1

/* P5_INTF_MODE: Interface mode of Port5 */
#define P5_INTF_MODE_GMII_MII		0
#define P5_INTF_MODE_RGMII		1

#define MT7530_P6ECR			0x7830
#define P6_INTF_MODE_M			0x3
#define P6_INTF_MODE_S			0

/* P6_INTF_MODE: Interface mode of Port6 */
#define P6_INTF_MODE_RGMII		0
#define P6_INTF_MODE_TRGMII		1

#define MT7530_TRGMII_RD(n)		(0x7a10 + (n) * 8)
#define RD_TAP_S			0
#define RD_TAP_M			0x7f

#define MT7530_TRGMII_TD_ODT(n)		(0x7a54 + (n) * 8)
/* XXX: all fields are defined under GMAC_TRGMII_TD_ODT */

/* TOP Signals Status Register */
#define MT7531_TOP_SIG_SR		0x780c
#define PAD_MCM_SMI_EN			BIT(0)
#define PAD_DUAL_SGMII_EN		BIT(1)

/* MT7531 PLLGP Registers */
#define MT7531_PLLGP_EN			0x7820
#define EN_COREPLL			BIT(2)
#define SW_CLKSW			BIT(1)
#define SW_PLLGP			BIT(0)

#define MT7531_PLLGP_CR0		0x78a8
#define RG_COREPLL_EN			BIT(22)
#define RG_COREPLL_POSDIV_S		23
#define RG_COREPLL_POSDIV_M		0x3800000
#define RG_COREPLL_SDM_PCW_S		1
#define RG_COREPLL_SDM_PCW_M		0x3ffffe
#define RG_COREPLL_SDM_PCW_CHG		BIT(0)

/* MT7531 RGMII and SGMII PLL clock */
#define MT7531_ANA_PLLGP_CR2		0x78b0
#define MT7531_ANA_PLLGP_CR5		0x78bc

/* MT7531 GPIO GROUP IOLB SMT0 Control */
#define MT7531_SMT0_IOLB		0x7f04
#define SMT_IOLB_5_SMI_MDC_EN		BIT(5)

/* MT7530 GPHY MDIO MMD Registers */
#define CORE_PLL_GROUP2			0x401
#define RG_SYSPLL_EN_NORMAL		BIT(15)
#define RG_SYSPLL_VODEN			BIT(14)
#define RG_SYSPLL_POSDIV_S		5
#define RG_SYSPLL_POSDIV_M		0x60

#define CORE_PLL_GROUP4			0x403
#define MT7531_BYPASS_MODE		BIT(4)
#define MT7531_POWER_ON_OFF		BIT(5)
#define RG_SYSPLL_DDSFBK_EN		BIT(12)
#define RG_SYSPLL_BIAS_EN		BIT(11)
#define RG_SYSPLL_BIAS_LPF_EN		BIT(10)

#define CORE_PLL_GROUP5			0x404
#define RG_LCDDS_PCW_NCPO1_S		0
#define RG_LCDDS_PCW_NCPO1_M		0xffff

#define CORE_PLL_GROUP6			0x405
#define RG_LCDDS_PCW_NCPO0_S		0
#define RG_LCDDS_PCW_NCPO0_M		0xffff

#define CORE_PLL_GROUP7			0x406
#define RG_LCDDS_PWDB			BIT(15)
#define RG_LCDDS_ISO_EN			BIT(13)
#define RG_LCCDS_C_S			4
#define RG_LCCDS_C_M			0x70
#define RG_LCDDS_PCW_NCPO_CHG		BIT(3)

#define CORE_PLL_GROUP10		0x409
#define RG_LCDDS_SSC_DELTA_S		0
#define RG_LCDDS_SSC_DELTA_M		0xfff

#define CORE_PLL_GROUP11		0x40a
#define RG_LCDDS_SSC_DELTA1_S		0
#define RG_LCDDS_SSC_DELTA1_M		0xfff

#define CORE_GSWPLL_GRP1		0x40d
#define RG_GSWPLL_POSDIV_200M_S		12
#define RG_GSWPLL_POSDIV_200M_M		0x3000
#define RG_GSWPLL_EN_PRE		BIT(11)
#define RG_GSWPLL_FBKDIV_200M_S		0
#define RG_GSWPLL_FBKDIV_200M_M		0xff

#define CORE_GSWPLL_GRP2		0x40e
#define RG_GSWPLL_POSDIV_500M_S		8
#define RG_GSWPLL_POSDIV_500M_M		0x300
#define RG_GSWPLL_FBKDIV_500M_S		0
#define RG_GSWPLL_FBKDIV_500M_M		0xff

#define CORE_TRGMII_GSW_CLK_CG		0x410
#define REG_GSWCK_EN			BIT(0)
#define REG_TRGMIICK_EN			BIT(1)

/* Extend PHY Control Register 3 */
#define PHY_EXT_REG_14			0x14

/* Fields of PHY_EXT_REG_14 */
#define PHY_EN_DOWN_SHFIT		BIT(4)

/* Extend PHY Control Register 4 */
#define PHY_EXT_REG_17			0x17

/* Fields of PHY_EXT_REG_17 */
#define PHY_LINKDOWN_POWER_SAVING_EN	BIT(4)

/* PHY RXADC Control Register 7 */
#define PHY_DEV1E_REG_0C6		0x0c6

/* Fields of PHY_DEV1E_REG_0C6 */
#define PHY_POWER_SAVING_S		8
#define PHY_POWER_SAVING_M		0x300
#define PHY_POWER_SAVING_TX		0x0

struct mt753x_switch_priv {
	struct mtk_eth_switch_priv epriv;
	struct mii_dev *mdio_bus;
	u32 smi_addr;
	u32 phy_base;
	u32 pmcr;

	int (*reg_read)(struct mt753x_switch_priv *priv, u32 reg, u32 *data);
	int (*reg_write)(struct mt753x_switch_priv *priv, u32 reg, u32 data);
};

int __mt753x_mdio_reg_read(struct mtk_eth_priv *priv, u32 smi_addr, u32 reg,
			   u32 *data);
int mt753x_mdio_reg_read(struct mt753x_switch_priv *priv, u32 reg, u32 *data);
int mt753x_mdio_reg_write(struct mt753x_switch_priv *priv, u32 reg, u32 data);

int mt753x_reg_read(struct mt753x_switch_priv *priv, u32 reg, u32 *data);
int mt753x_reg_write(struct mt753x_switch_priv *priv, u32 reg, u32 data);
void mt753x_reg_rmw(struct mt753x_switch_priv *priv, u32 reg, u32 clr, u32 set);

int mt7531_mii_read(struct mt753x_switch_priv *priv, u8 phy, u8 reg);
int mt7531_mii_write(struct mt753x_switch_priv *priv, u8 phy, u8 reg, u16 val);
int mt7531_mmd_read(struct mt753x_switch_priv *priv, u8 addr, u8 devad,
		    u16 reg);
int mt7531_mmd_write(struct mt753x_switch_priv *priv, u8 addr, u8 devad,
		     u16 reg, u16 val);

int mt7531_mdio_register(struct mt753x_switch_priv *priv);

void mt753x_port_isolation(struct mt753x_switch_priv *priv);

#endif /* _MTK_ETH_MT753X_H_ */
