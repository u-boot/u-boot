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

/*
 * The higher 16-bit of this register is used for write protection
 * only if BIT(x + 16) set to 1 the BIT(x) can be written.
 */
#define HIWORD_UPDATE_MASK(val, mask, shift) \
		((val) << (shift) | (mask) << ((shift) + 16))

#define PHY_CFG_DATA_SHIFT    7
#define PHY_CFG_ADDR_SHIFT    1
#define PHY_CFG_DATA_MASK     0xf
#define PHY_CFG_ADDR_MASK     0x3f
#define PHY_CFG_RD_MASK       0x3ff
#define PHY_CFG_WR_ENABLE     1
#define PHY_CFG_WR_DISABLE    1
#define PHY_CFG_WR_SHIFT      0
#define PHY_CFG_WR_MASK       1
#define PHY_CFG_PLL_LOCK      0x10
#define PHY_CFG_CLK_TEST      0x10
#define PHY_CFG_CLK_SCC       0x12
#define PHY_CFG_SEPE_RATE     BIT(3)
#define PHY_CFG_PLL_100M      BIT(3)
#define PHY_PLL_LOCKED        BIT(9)
#define PHY_PLL_OUTPUT        BIT(10)
#define PHY_LANE_IDLE_OFF     0x1
#define PHY_LANE_IDLE_MASK    0x1
#define PHY_LANE_IDLE_A_SHIFT 3
#define PHY_LANE_IDLE_B_SHIFT 4
#define PHY_LANE_IDLE_C_SHIFT 5
#define PHY_LANE_IDLE_D_SHIFT 6

#define PCIE_PHY_CONF		0xe220
#define PCIE_PHY_STATUS		0xe2a4
#define PCIE_PHY_LANEOFF	0xe214

struct rockchip_pcie_phy {
	void *reg_base;
	struct clk refclk;
	struct reset_ctl phy_rst;
	struct rockchip_pcie_phy_ops *ops;
};

struct rockchip_pcie_phy_ops {
	int (*init)(struct rockchip_pcie_phy *phy);
	int (*exit)(struct rockchip_pcie_phy *phy);
	int (*power_on)(struct rockchip_pcie_phy *phy);
	int (*power_off)(struct rockchip_pcie_phy *phy);
};

struct rockchip_pcie {
	fdt_addr_t axi_base;
	fdt_addr_t apb_base;
	int first_busno;
	struct udevice *dev;
	struct rockchip_pcie_phy rk_phy;
	struct rockchip_pcie_phy *phy;

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

int rockchip_pcie_phy_get(struct udevice *dev);

inline struct rockchip_pcie_phy *pcie_get_phy(struct rockchip_pcie *pcie)
{
	return pcie->phy;
}

inline
struct rockchip_pcie_phy_ops *phy_get_ops(struct rockchip_pcie_phy *phy)
{
	return (struct rockchip_pcie_phy_ops *)phy->ops;
}
