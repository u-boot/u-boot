/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Spacemit K1 DesignWare PCIe host controller driver
 *
 * Copyright (c) 2023, Spacemit Corporation.
 * Copyright (c) 2026, RISCstar Ltd.
 */

#ifndef _PCIE_DW_SPACEMIT_H_
#define _PCIE_DW_SPACEMIT_H_

#include <linux/bitops.h>

/* PCIe DW common registers */
#define PCIE_LINK_CAPABILITY		0x7c
#define PCIE_LINK_CTL_2			0xa0
#define TARGET_LINK_SPEED_MASK		0xf
#define LINK_SPEED_GEN_1		0x1
#define LINK_SPEED_GEN_2		0x2
#define LINK_SPEED_GEN_3		0x3

#define PCIE_MISC_CONTROL_1_OFF	0x8bc
#define PCIE_DBI_RO_WR_EN		BIT(0)

#define PLR_OFFSET			0x700
#define PCIE_PORT_DEBUG0		(PLR_OFFSET + 0x28)
#define PORT_LOGIC_LTSSM_STATE_MASK	0x1f
#define PORT_LOGIC_LTSSM_STATE_L0	0x11

#define PCIE_LINK_UP_TIMEOUT_MS		1000

/* Vendor and device IDs */
#define PCIE_VENDORID_MASK		GENMASK(15, 0)
#define PCIE_DEVICEID_SHIFT		16
#define SPACEMIT_PCIE_VENDOR_ID		0x201F
#define SPACEMIT_PCIE_DEVICE_ID		0x0001

/* Application register offsets */
#define PCIE_CMD_STATUS			0x04
#define LTSSM_EN_VAL			BIT(0)

/* K1X_CONF_DEVICE_CMD register */
#define PCIECTRL_K1X_CONF_DEVICE_CMD	0x0000
#define LTSSM_EN			BIT(6)
#define PCIE_PERST_IN			BIT(7)
#define GLOBAL_PHY_RST			BIT(8)
#define PCIE_AUX_PWR_DET		BIT(9)
#define PCIE_CLKREQ_IN			BIT(10)	/* read-only: CLKREQ# IO input value */
#define PCIE_REFCLK_EN			BIT(11)
#define PCIE_RC_PERST			BIT(12)
#define PCIE_EP_WAKE			BIT(13)
#define APP_HOLD_PHY_RST		BIT(30)
#define DEVICE_TYPE_RC			BIT(31)

/* PCIE_CTRL_LOGIC register */
#define PCIE_CTRL_LOGIC			0x0004
#define PCIE_SOFT_RESET			BIT(0)
#define PCIE_IGNORE_PERSTN		BIT(2)

/* PHY AHB link status */
#define K1X_PHY_AHB_LINK_STS		0x0004
#define SMLH_LINK_UP			BIT(1)
#define RDLH_LINK_UP			BIT(12)

/* PHY lane control mask (bits 0-5 in DEVICE_CMD) */
#define PCIE_PHY_LANE_CTRL_MASK		0x3f

/* PCIe DBI address calculation */
#define SPACEMIT_PCIE_DBI_BASE		0xca000000
#define SPACEMIT_PCIE_DBI_STRIDE	0x400000

enum dw_pcie_device_mode {
	DW_PCIE_UNKNOWN_TYPE,
	DW_PCIE_EP_TYPE,
	DW_PCIE_RC_TYPE,
};

#endif /* _PCIE_DW_SPACEMIT_H_ */
