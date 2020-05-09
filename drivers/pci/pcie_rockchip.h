/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Rockchip PCIe Headers
 *
 * Copyright (c) 2016 Rockchip, Inc.
 * Copyright (c) 2020 Amarula Solutions(India)
 * Copyright (c) 2020 Jagan Teki <jagan@amarulasolutions.com>
 * Copyright (c) 2019 Patrick Wildt <patrick@blueri.se>
 *
 */

#define HIWORD_UPDATE(mask, val)        (((mask) << 16) | (val))
#define HIWORD_UPDATE_BIT(val)          HIWORD_UPDATE(val, val)

#define ENCODE_LANES(x)                 ((((x) >> 1) & 3) << 4)
#define PCIE_CLIENT_BASE                0x0
#define PCIE_CLIENT_CONFIG              (PCIE_CLIENT_BASE + 0x00)
#define PCIE_CLIENT_CONF_ENABLE         HIWORD_UPDATE_BIT(0x0001)
#define PCIE_CLIENT_LINK_TRAIN_ENABLE   HIWORD_UPDATE_BIT(0x0002)
#define PCIE_CLIENT_MODE_RC             HIWORD_UPDATE_BIT(0x0040)
#define PCIE_CLIENT_GEN_SEL_1           HIWORD_UPDATE(0x0080, 0)
#define PCIE_CLIENT_BASIC_STATUS1	0x0048
#define PCIE_CLIENT_LINK_STATUS_UP	GENMASK(21, 20)
#define PCIE_CLIENT_LINK_STATUS_MASK	GENMASK(21, 20)
#define PCIE_LINK_UP(x) \
	(((x) & PCIE_CLIENT_LINK_STATUS_MASK) == PCIE_CLIENT_LINK_STATUS_UP)
#define PCIE_RC_NORMAL_BASE		0x800000
#define PCIE_LM_BASE			0x900000
#define PCIE_LM_VENDOR_ID              (PCIE_LM_BASE + 0x44)
#define PCIE_LM_VENDOR_ROCKCHIP		0x1d87
#define PCIE_LM_RCBAR			(PCIE_LM_BASE + 0x300)
#define PCIE_LM_RCBARPIE		BIT(19)
#define PCIE_LM_RCBARPIS		BIT(20)
#define PCIE_RC_BASE			0xa00000
#define PCIE_RC_CONFIG_DCR		(PCIE_RC_BASE + 0x0c4)
#define PCIE_RC_CONFIG_DCR_CSPL_SHIFT	18
#define PCIE_RC_CONFIG_DCR_CPLS_SHIFT	26
#define PCIE_RC_PCIE_LCAP		(PCIE_RC_BASE + 0x0cc)
#define PCIE_RC_PCIE_LCAP_APMS_L0S	BIT(10)
#define PCIE_ATR_BASE			0xc00000
#define PCIE_ATR_OB_ADDR0(i)		(PCIE_ATR_BASE + 0x000 + (i) * 0x20)
#define PCIE_ATR_OB_ADDR1(i)		(PCIE_ATR_BASE + 0x004 + (i) * 0x20)
#define PCIE_ATR_OB_DESC0(i)		(PCIE_ATR_BASE + 0x008 + (i) * 0x20)
#define PCIE_ATR_OB_DESC1(i)		(PCIE_ATR_BASE + 0x00c + (i) * 0x20)
#define PCIE_ATR_IB_ADDR0(i)		(PCIE_ATR_BASE + 0x800 + (i) * 0x8)
#define PCIE_ATR_IB_ADDR1(i)		(PCIE_ATR_BASE + 0x804 + (i) * 0x8)
#define PCIE_ATR_HDR_MEM		0x2
#define PCIE_ATR_HDR_IO			0x6
#define PCIE_ATR_HDR_CFG_TYPE0		0xa
#define PCIE_ATR_HDR_CFG_TYPE1		0xb
#define PCIE_ATR_HDR_RID		BIT(23)

#define PCIE_ATR_OB_REGION0_SIZE	(32 * 1024 * 1024)
#define PCIE_ATR_OB_REGION_SIZE		(1 * 1024 * 1024)

struct rockchip_pcie {
	fdt_addr_t axi_base;
	fdt_addr_t apb_base;
	int first_busno;
	struct udevice *dev;

	/* resets */
	struct reset_ctl core_rst;
	struct reset_ctl mgmt_rst;
	struct reset_ctl mgmt_sticky_rst;
	struct reset_ctl pipe_rst;
	struct reset_ctl pm_rst;
	struct reset_ctl pclk_rst;
	struct reset_ctl aclk_rst;

	/* gpio */
	struct gpio_desc ep_gpio;

	/* vpcie regulators */
	struct udevice *vpcie12v;
	struct udevice *vpcie3v3;
	struct udevice *vpcie1v8;
	struct udevice *vpcie0v9;
};
