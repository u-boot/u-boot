// SPDX-License-Identifier: GPL-2.0-only
/*
 * Renesas Ethernet PCS Device Driver
 *
 * Based on the Renesas Ethernet SERDES driver and updated for PCS support.
 *
 * Copyright (C) 2025 Renesas Electronics Corporation
 */

#include <asm/io.h>
#include <clk-uclass.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <log.h>
#include <reset.h>
#include <syscon.h>

#define R8A78000_ETH_PCS_NUM			8
#define R8A78000_ETH_PCS_OFFSET			0x0400
#define R8A78000_ETH_PCS_BANK_SELECT		0x03fc
#define R8A78000_ETH_PCS_TIMEOUT_US		100000
#define R8A78000_ETH_PCS_NUM_RETRY_LINKUP	8

struct r8a78000_eth_pcs_drv_data;
struct r8a78000_eth_pcs_channel {
	struct r8a78000_eth_pcs_drv_data *dd;
	struct phy *phy;
	void __iomem *addr;
	phy_interface_t phy_interface;
	int speed;
	int index;
};

struct r8a78000_eth_pcs_drv_data {
	void __iomem *addr;
	struct reset_ctl_bulk reset;
	struct phy mpphy;
	struct r8a78000_eth_pcs_channel channel[R8A78000_ETH_PCS_NUM];
	struct clk_bulk clks;
};

/*
 * The datasheet describes initialization procedure without any information
 * about registers' name/bits. So, this is all black magic to initialize
 * the hardware.
 */
static void r8a78000_eth_pcs_write32(void __iomem *addr, u32 offs, u32 bank, u32 data)
{
	writel(bank, addr + R8A78000_ETH_PCS_BANK_SELECT);
	writel(data, addr + offs);
}

static int
r8a78000_eth_pcs_reg_wait(struct r8a78000_eth_pcs_channel *channel,
			  u32 offs, u32 bank, u32 mask, u32 expected)
{
	u32 val = 0;
	int ret;

	writel(bank, channel->addr + R8A78000_ETH_PCS_BANK_SELECT);

	ret = readl_poll_timeout(channel->addr + offs, val,
				 (val & mask) == expected,
				 R8A78000_ETH_PCS_TIMEOUT_US);
	if (ret)
		dev_dbg(channel->phy->dev,
			"%s: index %d, offs %x, bank %x, mask %x, expected %x\n",
			 __func__, channel->index, offs, bank, mask, expected);

	return ret;
}

static int
r8a78000_eth_pcs_init_ram(struct r8a78000_eth_pcs_channel *channel, struct phy *mpphy)
{
	int ret;

	ret = r8a78000_eth_pcs_reg_wait(channel, 0x026c, 0x180, BIT(0), 0x01);
	if (ret)
		return ret;

	r8a78000_eth_pcs_write32(channel->addr, 0x026c, 0x180, 0x03);

	ret = generic_phy_power_on(mpphy);
	if (ret)
		return ret;

	ret = r8a78000_eth_pcs_reg_wait(channel, 0x0000, 0x300, BIT(15), 0);
	if (ret)
		return ret;

	return 0;
}

static int
r8a78000_eth_pcs_common_setting(struct r8a78000_eth_pcs_channel *channel)
{
	int ret;

	switch (channel->phy_interface) {
	case PHY_INTERFACE_MODE_SGMII:
		r8a78000_eth_pcs_write32(channel->addr, 0x001c, 0x300, 0x0001);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x380, 0x2000);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x1f00, 0x0140);
		r8a78000_eth_pcs_write32(channel->addr, 0x0258, 0x180, 0x0018);
		r8a78000_eth_pcs_write32(channel->addr, 0x01dc, 0x180, 0x000d);
		r8a78000_eth_pcs_write32(channel->addr, 0x00f8, 0x180, 0x0016);
		r8a78000_eth_pcs_write32(channel->addr, 0x0248, 0x180, 0x0016);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x300, 0x0c40);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0040, 0x380, GENMASK(4, 2), 0x06 << 2);
		if (ret)
			return ret;

		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x300, 0x0440);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0040, 0x380, GENMASK(4, 2), 0x04 << 2);
		if (ret)
			return ret;

		r8a78000_eth_pcs_write32(channel->addr, 0x0014, 0x380, 0x0050);
		r8a78000_eth_pcs_write32(channel->addr, 0x00d8, 0x180, 0x3000);
		r8a78000_eth_pcs_write32(channel->addr, 0x00dc, 0x180, 0x0000);

		return 0;
	case PHY_INTERFACE_MODE_USXGMII:
		r8a78000_eth_pcs_write32(channel->addr, 0x001c, 0x300, 0x0000);
		r8a78000_eth_pcs_write32(channel->addr, 0x001c, 0x380, 0x0000);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x380, 0x2200);
		r8a78000_eth_pcs_write32(channel->addr, 0x0258, 0x180, 0x0018);
		r8a78000_eth_pcs_write32(channel->addr, 0x01dc, 0x180, 0x000d);
		r8a78000_eth_pcs_write32(channel->addr, 0x00f8, 0x180, 0x001b);
		r8a78000_eth_pcs_write32(channel->addr, 0x0248, 0x180, 0x001b);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x300, 0x0c40);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0040, 0x380, GENMASK(4, 2), 0x06 << 2);
		if (ret)
			return ret;

		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x300, 0x0440);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0040, 0x380, GENMASK(4, 2), 0x04 << 2);
		if (ret)
			return ret;

		r8a78000_eth_pcs_write32(channel->addr, 0x0014, 0x380, 0x0050);
		r8a78000_eth_pcs_write32(channel->addr, 0x00d8, 0x180, 0x1800);
		r8a78000_eth_pcs_write32(channel->addr, 0x00dc, 0x180, 0x0012);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0080, 0x180, BIT(12), BIT(12));
		if (ret)
			return ret;

		r8a78000_eth_pcs_write32(channel->addr, 0x0170, 0x180, 0x1000);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0260, 0x180, BIT(12), BIT(12));
		if (ret)
			return ret;

		r8a78000_eth_pcs_write32(channel->addr, 0x0170, 0x180, 0x0000);

		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static int
r8a78000_eth_pcs_chan_setting(struct r8a78000_eth_pcs_channel *channel)
{
	int ret;

	switch (channel->phy_interface) {
	case PHY_INTERFACE_MODE_SGMII:
		/* For AN_ON */
		r8a78000_eth_pcs_write32(channel->addr, 0x0004, 0x1f80, 0x0005);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x1f80, 0x2200);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x1f00, 0x3140);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0008, 0x1f80, BIT(0), BIT(0));
		if (ret)
			return ret;

		break;
	case PHY_INTERFACE_MODE_USXGMII:
		/* For AN_ON */
		r8a78000_eth_pcs_write32(channel->addr, 0x0004, 0x1f80, 0x0001);
		r8a78000_eth_pcs_write32(channel->addr, 0x0028, 0x1f80, 0x0001);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x1f80, 0x2008);
		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x1f00, 0x3140);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0008, 0x1f80, BIT(0), BIT(0));
		if (ret)
			return ret;

		r8a78000_eth_pcs_write32(channel->addr, 0x0008, 0x1f80, 0x0000);

		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int
r8a78000_eth_pcs_chan_speed(struct r8a78000_eth_pcs_channel *channel)
{
	int ret;

	switch (channel->phy_interface) {
	case PHY_INTERFACE_MODE_SGMII:
		/* Do nothing */
		break;
	case PHY_INTERFACE_MODE_USXGMII:
		if (channel->speed == 10000)
			r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x1f00, 0x2140);
		else if (channel->speed == 2500)
			r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x1f00, 0x0120);
		else
			return -EOPNOTSUPP;

		r8a78000_eth_pcs_write32(channel->addr, 0x0000, 0x380, 0x2600);

		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0000, 0x380, BIT(10), 0);
		if (ret)
			return ret;

		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int r8a78000_eth_pcs_monitor_linkup(struct r8a78000_eth_pcs_channel *channel)
{
	int i, ret;

	for (i = 0; i < R8A78000_ETH_PCS_NUM_RETRY_LINKUP; i++) {
		ret = r8a78000_eth_pcs_reg_wait(channel, 0x0004, 0x300,
						BIT(2), BIT(2));
		if (!ret)
			break;

		/* restart */
		r8a78000_eth_pcs_write32(channel->addr, 0x0144, 0x180, 0x0010);
		udelay(1);
		r8a78000_eth_pcs_write32(channel->addr, 0x0144, 0x180, 0x0000);
	}

	return ret;
}

static int r8a78000_eth_pcs_init(struct phy *p)
{
	struct r8a78000_eth_pcs_drv_data *dd = dev_get_priv(p->dev);
	struct r8a78000_eth_pcs_channel *channel = dd->channel + p->id;
	int ret;

	ret = generic_phy_init(&dd->mpphy);
	if (ret) {
		dev_dbg(channel->phy->dev, "XPCS: Failed to init MPPHY\n");
		return ret;
	}

	ret = r8a78000_eth_pcs_init_ram(channel, &dd->mpphy);
	if (ret)
		return ret;

	ret = r8a78000_eth_pcs_common_setting(channel);

	return ret;
}

static int r8a78000_eth_pcs_hw_init_late(struct r8a78000_eth_pcs_channel *channel)
{
	int ret;

	ret = r8a78000_eth_pcs_chan_setting(channel);
	if (ret)
		return ret;

	ret = r8a78000_eth_pcs_chan_speed(channel);
	if (ret)
		return ret;

	r8a78000_eth_pcs_write32(channel->addr, 0x03c0, 0x380, 0x0000);

	r8a78000_eth_pcs_write32(channel->addr, 0x03d0, 0x380, 0x0000);

	return r8a78000_eth_pcs_monitor_linkup(channel);
}

static int r8a78000_eth_pcs_power_on(struct phy *p)
{
	struct r8a78000_eth_pcs_drv_data *dd = dev_get_priv(p->dev);
	struct r8a78000_eth_pcs_channel *channel = dd->channel + p->id;

	return r8a78000_eth_pcs_hw_init_late(channel);
}

static int r8a78000_eth_pcs_set_mode(struct phy *p, enum phy_mode mode,
				     int submode)
{
	struct r8a78000_eth_pcs_drv_data *dd = dev_get_priv(p->dev);
	struct r8a78000_eth_pcs_channel *channel = dd->channel + p->id;

	if (mode != PHY_MODE_ETHERNET)
		return -EOPNOTSUPP;

	switch (submode) {
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_USXGMII:
		channel->phy_interface = submode;
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static int r8a78000_eth_pcs_set_speed(struct phy *p, int speed)
{
	struct r8a78000_eth_pcs_drv_data *dd = dev_get_priv(p->dev);
	struct r8a78000_eth_pcs_channel *channel = dd->channel + p->id;

	channel->speed = speed;

	return 0;
}

static int r8a78000_eth_pcs_of_xlate(struct phy *phy,
				     struct ofnode_phandle_args *args)
{
	if (args->args_count < 1)
		return -ENODEV;

	if (args->args[0] >= R8A78000_ETH_PCS_NUM)
		return -ENODEV;

	phy->id = args->args[0];

	return 0;
}

static const struct phy_ops r8a78000_eth_pcs_ops = {
	.init		= r8a78000_eth_pcs_init,
	.power_on	= r8a78000_eth_pcs_power_on,
	.set_mode	= r8a78000_eth_pcs_set_mode,
	.set_speed	= r8a78000_eth_pcs_set_speed,
	.of_xlate	= r8a78000_eth_pcs_of_xlate,
};

static const struct udevice_id r8a78000_eth_pcs_of_table[] = {
	{ .compatible = "renesas,r8a78000-ether-pcs", },
	{ }
};

static int r8a78000_eth_pcs_probe(struct udevice *dev)
{
	struct r8a78000_eth_pcs_drv_data *dd = dev_get_priv(dev);
	int i, ret;

	dd->addr = dev_read_addr_ptr(dev);
	if (!dd->addr)
		return -EINVAL;

	ret = reset_get_bulk(dev, &dd->reset);
	if (ret)
		return ret;

	ret = clk_get_bulk(dev, &dd->clks);
	if (ret < 0)
		goto err_clk_get;

	ret = clk_enable_bulk(&dd->clks);
	if (ret)
		goto err_clk_enable;

	reset_assert_bulk(&dd->reset);
	reset_deassert_bulk(&dd->reset);

	ret = generic_phy_get_by_index(dev, 0, &dd->mpphy);
	if (ret)
		goto err_phy_get;

	for (i = 0; i < R8A78000_ETH_PCS_NUM; i++) {
		struct r8a78000_eth_pcs_channel *channel = &dd->channel[i];

		channel->addr = dd->addr + R8A78000_ETH_PCS_OFFSET * i;
		channel->dd = dd;
		channel->index = i;
	}

	return 0;

err_phy_get:
	clk_disable_bulk(&dd->clks);
err_clk_enable:
	clk_release_bulk(&dd->clks);
err_clk_get:
	reset_release_bulk(&dd->reset);
	return ret;
}

U_BOOT_DRIVER(r8a78000_eth_pcs_driver_platform) = {
	.name		= "r8a78000_eth_pcs",
	.id		= UCLASS_PHY,
	.of_match	= r8a78000_eth_pcs_of_table,
	.probe		= r8a78000_eth_pcs_probe,
	.ops		= &r8a78000_eth_pcs_ops,
	.priv_auto	= sizeof(struct r8a78000_eth_pcs_drv_data),
};
