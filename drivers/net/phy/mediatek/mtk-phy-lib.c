// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc. All Rights Reserved.
 *
 * Author: Sky Huang <SkyLake.Huang@mediatek.com>
 */
#include <dm/device_compat.h>
#include <phy.h>

#include "mtk.h"

void mtk_phy_select_page(struct phy_device *phydev, int page)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MTK_EXT_PAGE_ACCESS, page);
}

void mtk_phy_restore_page(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MTK_EXT_PAGE_ACCESS,
		  MTK_PHY_PAGE_STANDARD);
}

/* Difference between functions with mtk_tr* and __mtk_tr* prefixes is
 * mtk_tr* functions: wrapped by page switching operations
 * __mtk_tr* functions: no page switching operations
 */
static void __mtk_tr_access(struct phy_device *phydev, bool read, u8 ch_addr,
			    u8 node_addr, u8 data_addr)
{
	u16 tr_cmd = BIT(15); /* bit 14 & 0 are reserved */

	if (read)
		tr_cmd |= BIT(13);

	tr_cmd |= (((ch_addr & 0x3) << 11) |
		   ((node_addr & 0xf) << 7) |
		   ((data_addr & 0x3f) << 1));
	dev_dbg(phydev->dev, "tr_cmd: 0x%x\n", tr_cmd);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x10, tr_cmd);
}

static void __mtk_tr_read(struct phy_device *phydev, u8 ch_addr, u8 node_addr,
			  u8 data_addr, u16 *tr_high, u16 *tr_low)
{
	__mtk_tr_access(phydev, true, ch_addr, node_addr, data_addr);
	*tr_low = phy_read(phydev, MDIO_DEVAD_NONE, 0x11);
	*tr_high = phy_read(phydev, MDIO_DEVAD_NONE, 0x12);
	dev_dbg(phydev->dev, "tr_high read: 0x%x, tr_low read: 0x%x\n",
		*tr_high, *tr_low);
}

u32 mtk_tr_read(struct phy_device *phydev, u8 ch_addr, u8 node_addr,
		u8 data_addr)
{
	u16 tr_high;
	u16 tr_low;

	mtk_phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_52B5);
	__mtk_tr_read(phydev, ch_addr, node_addr, data_addr, &tr_high, &tr_low);
	mtk_phy_restore_page(phydev);

	return (tr_high << 16) | tr_low;
}

static void __mtk_tr_write(struct phy_device *phydev, u8 ch_addr, u8 node_addr,
			   u8 data_addr, u32 tr_data)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 0x11, tr_data & 0xffff);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x12, tr_data >> 16);
	dev_dbg(phydev->dev, "tr_high write: 0x%x, tr_low write: 0x%x\n",
		tr_data >> 16, tr_data & 0xffff);
	__mtk_tr_access(phydev, false, ch_addr, node_addr, data_addr);
}

void __mtk_tr_modify(struct phy_device *phydev, u8 ch_addr, u8 node_addr,
		     u8 data_addr, u32 mask, u32 set)
{
	u32 tr_data;
	u16 tr_high;
	u16 tr_low;

	__mtk_tr_read(phydev, ch_addr, node_addr, data_addr, &tr_high, &tr_low);
	tr_data = (tr_high << 16) | tr_low;
	tr_data = (tr_data & ~mask) | set;
	__mtk_tr_write(phydev, ch_addr, node_addr, data_addr, tr_data);
}

void mtk_tr_modify(struct phy_device *phydev, u8 ch_addr, u8 node_addr,
		   u8 data_addr, u32 mask, u32 set)
{
	mtk_phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_52B5);
	__mtk_tr_modify(phydev, ch_addr, node_addr, data_addr, mask, set);
	mtk_phy_restore_page(phydev);
}

void __mtk_tr_set_bits(struct phy_device *phydev, u8 ch_addr, u8 node_addr,
		       u8 data_addr, u32 set)
{
	__mtk_tr_modify(phydev, ch_addr, node_addr, data_addr, 0, set);
}

void __mtk_tr_clr_bits(struct phy_device *phydev, u8 ch_addr, u8 node_addr,
		       u8 data_addr, u32 clr)
{
	__mtk_tr_modify(phydev, ch_addr, node_addr, data_addr, clr, 0);
}
