// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#include <errno.h>
#include <time.h>
#include "mtk_eth.h"
#include "mt753x.h"

/*
 * MT753x Internal Register Address Bits
 * -------------------------------------------------------------------
 * | 15  14  13  12  11  10   9   8   7   6 | 5   4   3   2 | 1   0  |
 * |----------------------------------------|---------------|--------|
 * |              Page Address              |  Reg Address  | Unused |
 * -------------------------------------------------------------------
 */

int __mt753x_mdio_reg_read(struct mtk_eth_priv *priv, u32 smi_addr, u32 reg,
			   u32 *data)
{
	int ret, low_word, high_word;

	/* Write page address */
	ret = mtk_mii_write(priv, smi_addr, 0x1f, reg >> 6);
	if (ret)
		return ret;

	/* Read low word */
	low_word = mtk_mii_read(priv, smi_addr, (reg >> 2) & 0xf);
	if (low_word < 0)
		return low_word;

	/* Read high word */
	high_word = mtk_mii_read(priv, smi_addr, 0x10);
	if (high_word < 0)
		return high_word;

	if (data)
		*data = ((u32)high_word << 16) | (low_word & 0xffff);

	return 0;
}

int mt753x_mdio_reg_read(struct mt753x_switch_priv *priv, u32 reg, u32 *data)
{
	return __mt753x_mdio_reg_read(priv->epriv.eth, priv->smi_addr, reg,
				      data);
}

int mt753x_mdio_reg_write(struct mt753x_switch_priv *priv, u32 reg, u32 data)
{
	int ret;

	/* Write page address */
	ret = mtk_mii_write(priv->epriv.eth, priv->smi_addr, 0x1f, reg >> 6);
	if (ret)
		return ret;

	/* Write low word */
	ret = mtk_mii_write(priv->epriv.eth, priv->smi_addr, (reg >> 2) & 0xf,
			    data & 0xffff);
	if (ret)
		return ret;

	/* Write high word */
	return mtk_mii_write(priv->epriv.eth, priv->smi_addr, 0x10, data >> 16);
}

int mt753x_reg_read(struct mt753x_switch_priv *priv, u32 reg, u32 *data)
{
	return priv->reg_read(priv, reg, data);
}

int mt753x_reg_write(struct mt753x_switch_priv *priv, u32 reg, u32 data)
{
	return priv->reg_write(priv, reg, data);
}

void mt753x_reg_rmw(struct mt753x_switch_priv *priv, u32 reg, u32 clr, u32 set)
{
	u32 val;

	priv->reg_read(priv, reg, &val);
	val &= ~clr;
	val |= set;
	priv->reg_write(priv, reg, val);
}

/* Indirect MDIO clause 22/45 access */
static int mt7531_mii_rw(struct mt753x_switch_priv *priv, int phy, int reg,
			 u16 data, u32 cmd, u32 st)
{
	u32 val, timeout_ms;
	ulong timeout;
	int ret = 0;

	val = (st << MDIO_ST_S) |
	      ((cmd << MDIO_CMD_S) & MDIO_CMD_M) |
	      ((phy << MDIO_PHY_ADDR_S) & MDIO_PHY_ADDR_M) |
	      ((reg << MDIO_REG_ADDR_S) & MDIO_REG_ADDR_M);

	if (cmd == MDIO_CMD_WRITE || cmd == MDIO_CMD_ADDR)
		val |= data & MDIO_RW_DATA_M;

	mt753x_reg_write(priv, MT7531_PHY_IAC, val | PHY_ACS_ST);

	timeout_ms = 100;
	timeout = get_timer(0);
	while (1) {
		mt753x_reg_read(priv, MT7531_PHY_IAC, &val);

		if ((val & PHY_ACS_ST) == 0)
			break;

		if (get_timer(timeout) > timeout_ms)
			return -ETIMEDOUT;
	}

	if (cmd == MDIO_CMD_READ || cmd == MDIO_CMD_READ_C45) {
		mt753x_reg_read(priv, MT7531_PHY_IAC, &val);
		ret = val & MDIO_RW_DATA_M;
	}

	return ret;
}

int mt7531_mii_read(struct mt753x_switch_priv *priv, u8 phy, u8 reg)
{
	u8 phy_addr;

	if (phy >= MT753X_NUM_PHYS)
		return -EINVAL;

	phy_addr = MT753X_PHY_ADDR(priv->phy_base, phy);

	return mt7531_mii_rw(priv, phy_addr, reg, 0, MDIO_CMD_READ,
			     MDIO_ST_C22);
}

int mt7531_mii_write(struct mt753x_switch_priv *priv, u8 phy, u8 reg, u16 val)
{
	u8 phy_addr;

	if (phy >= MT753X_NUM_PHYS)
		return -EINVAL;

	phy_addr = MT753X_PHY_ADDR(priv->phy_base, phy);

	return mt7531_mii_rw(priv, phy_addr, reg, val, MDIO_CMD_WRITE,
			     MDIO_ST_C22);
}

int mt7531_mmd_read(struct mt753x_switch_priv *priv, u8 addr, u8 devad,
		    u16 reg)
{
	u8 phy_addr;
	int ret;

	if (addr >= MT753X_NUM_PHYS)
		return -EINVAL;

	phy_addr = MT753X_PHY_ADDR(priv->phy_base, addr);

	ret = mt7531_mii_rw(priv, phy_addr, devad, reg, MDIO_CMD_ADDR,
			    MDIO_ST_C45);
	if (ret)
		return ret;

	return mt7531_mii_rw(priv, phy_addr, devad, 0, MDIO_CMD_READ_C45,
			     MDIO_ST_C45);
}

int mt7531_mmd_write(struct mt753x_switch_priv *priv, u8 addr, u8 devad,
		     u16 reg, u16 val)
{
	u8 phy_addr;
	int ret;

	if (addr >= MT753X_NUM_PHYS)
		return 0;

	phy_addr = MT753X_PHY_ADDR(priv->phy_base, addr);

	ret = mt7531_mii_rw(priv, phy_addr, devad, reg, MDIO_CMD_ADDR,
			    MDIO_ST_C45);
	if (ret)
		return ret;

	return mt7531_mii_rw(priv, phy_addr, devad, val, MDIO_CMD_WRITE,
			     MDIO_ST_C45);
}

static int mt7531_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mt753x_switch_priv *priv = bus->priv;

	if (devad < 0)
		return mt7531_mii_read(priv, addr, reg);

	return mt7531_mmd_read(priv, addr, devad, reg);
}

static int mt7531_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			     u16 val)
{
	struct mt753x_switch_priv *priv = bus->priv;

	if (devad < 0)
		return mt7531_mii_write(priv, addr, reg, val);

	return mt7531_mmd_write(priv, addr, devad, reg, val);
}

int mt7531_mdio_register(struct mt753x_switch_priv *priv)
{
	struct mii_dev *mdio_bus = mdio_alloc();
	int ret;

	if (!mdio_bus)
		return -ENOMEM;

	mdio_bus->read = mt7531_mdio_read;
	mdio_bus->write = mt7531_mdio_write;
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

void mt753x_port_isolation(struct mt753x_switch_priv *priv)
{
	u32 i;

	for (i = 0; i < MT753X_NUM_PORTS; i++) {
		/* Set port matrix mode */
		if (i != 6)
			mt753x_reg_write(priv, PCR_REG(i),
					 (0x40 << PORT_MATRIX_S));
		else
			mt753x_reg_write(priv, PCR_REG(i),
					 (0x3f << PORT_MATRIX_S));

		/* Set port mode to user port */
		mt753x_reg_write(priv, PVC_REG(i),
				 (0x8100 << STAG_VPID_S) |
				 (VLAN_ATTR_USER << VLAN_ATTR_S));
	}
}
