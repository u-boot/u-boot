/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015-2016 Marvell International Ltd.
 */

#ifndef _COMPHY_A3700_H_
#define _COMPHY_A3700_H_

#include "comphy_core.h"

#define MVEBU_REG(offs)			((uintptr_t)MVEBU_REGISTER(offs))

#define PLL_LOCK_TIMEOUT		1000
#define POLL_16B_REG			1
#define POLL_32B_REG			0

/*
 * USB definitions
 */
#define USB32_BASE			MVEBU_REG(0x050000) /* usb3 device */
#define USB2PHY_BASE			MVEBU_REG(0x05D000)
#define USB2PHY2_BASE			MVEBU_REG(0x05F000)

#define USB3_CTRPUL_VAL_REG		(0x20 + USB32_BASE)
#define USB3_TOP_INT_STATUS_REG		(0xd8 + USB32_BASE)
#define vbus_int_state			BIT(5)
#define USB3_TOP_INT_ENABLE_REG		(0xdc + USB32_BASE)
#define vbus_int_enable			BIT(5)
#define rb_usb3_ctr_100ns		0xff000000

#define USB2_OTG_PHY_CTRL_ADDR		(0x820 + USB2PHY_BASE)
#define rb_usb2phy_suspm		BIT(14)
#define rb_usb2phy_pu			BIT(0)
#define rb_usb2_dp_pulldn_dev_mode	BIT(5)
#define rb_usb2_dm_pulldn_dev_mode	BIT(6)

#define USB2_PHY_OTG_CTRL_ADDR		(0x34 + USB2PHY_BASE)
#define rb_pu_otg			BIT(4)

#define USB2_PHY_CHRGR_DET_ADDR		(0x38 + USB2PHY_BASE)
#define rb_cdp_en			BIT(2)
#define rb_dcp_en			BIT(3)
#define rb_pd_en			BIT(4)
#define rb_pu_chrg_dtc			BIT(5)
#define rb_cdp_dm_auto			BIT(7)
#define rb_enswitch_dp			BIT(12)
#define rb_enswitch_dm			BIT(13)

#define USB2_CAL_CTRL_ADDR		(0x8 + USB2PHY_BASE)
#define rb_usb2phy_pllcal_done		BIT(31)
#define rb_usb2phy_impcal_done		BIT(23)

#define USB2_PLL_CTRL0_ADDR		(0x0 + USB2PHY_BASE)
#define rb_usb2phy_pll_ready		BIT(31)

#define USB2_RX_CHAN_CTRL1_ADDR		(0x18 + USB2PHY_BASE)
#define rb_usb2phy_sqcal_done		BIT(31)

#define USB2_PHY2_CTRL_ADDR		(0x804 + USB2PHY2_BASE)
#define rb_usb2phy2_suspm		BIT(7)
#define rb_usb2phy2_pu			BIT(0)
#define USB2_PHY2_CAL_CTRL_ADDR		(0x8 + USB2PHY2_BASE)
#define USB2_PHY2_PLL_CTRL0_ADDR	(0x0 + USB2PHY2_BASE)
#define USB2_PHY2_RX_CHAN_CTRL1_ADDR	(0x18 + USB2PHY2_BASE)

#define USB2_PHY_BASE(usb32)	(usb32 == 0 ? USB2PHY2_BASE : USB2PHY_BASE)
#define USB2_PHY_CTRL_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_CTRL_ADDR : USB2_OTG_PHY_CTRL_ADDR)
#define RB_USB2PHY_SUSPM(usb32) \
	(usb32 == 0 ? rb_usb2phy2_suspm : rb_usb2phy_suspm)
#define RB_USB2PHY_PU(usb32) \
	(usb32 == 0 ? rb_usb2phy2_pu : rb_usb2phy_pu)
#define USB2_PHY_CAL_CTRL_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_CAL_CTRL_ADDR : USB2_CAL_CTRL_ADDR)
#define USB2_PHY_RX_CHAN_CTRL1_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_RX_CHAN_CTRL1_ADDR : USB2_RX_CHAN_CTRL1_ADDR)
#define USB2_PHY_PLL_CTRL0_ADDR(usb32) \
	(usb32 == 0 ? USB2_PHY2_PLL_CTRL0_ADDR : USB2_PLL_CTRL0_ADDR)

/*
 * SATA definitions
 */
#define AHCI_BASE			MVEBU_REG(0xE0000)

#define rh_vs0_a			(AHCI_BASE + 0xA0)
#define rh_vs0_d			(AHCI_BASE + 0xA4)

#define vsata_ctrl_reg			0x00
#define bs_phy_pu_pll			BIT(6)

/*
 * SDIO/eMMC definitions
 */
#define SDIO_BASE			MVEBU_REG(0xD8000)

#define SDIO_HOST_CTRL1_ADDR		(SDIO_BASE + 0x28)
#define SDIO_SDHC_FIFO_ADDR		(SDIO_BASE + 0x12C)
#define SDIO_CAP_12_ADDR		(SDIO_BASE + 0x40)
#define SDIO_ENDIAN_ADDR		(SDIO_BASE + 0x1A4)
#define SDIO_PHY_TIMING_ADDR		(SDIO_BASE + 0x170)
#define SDIO_PHY_PAD_CTRL0_ADDR		(SDIO_BASE + 0x178)
#define SDIO_DLL_RST_ADDR		(SDIO_BASE + 0x148)

#endif /* _COMPHY_A3700_H_ */
