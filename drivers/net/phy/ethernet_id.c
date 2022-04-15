// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx ethernet phy reset driver
 *
 * Copyright (C) 2022 Xilinx, Inc.
 */

#include <common.h>
#include <dm/device_compat.h>
#include <phy.h>
#include <linux/delay.h>
#include <asm/gpio.h>

struct phy_device *phy_connect_phy_id(struct mii_dev *bus, struct udevice *dev,
				      int phyaddr)
{
	struct phy_device *phydev;
	struct ofnode_phandle_args phandle_args;
	struct gpio_desc gpio;
	ofnode node;
	u32 id, assert, deassert;
	u16 vendor, device;
	int ret;

	if (dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
				       &phandle_args))
		return NULL;

	if (!ofnode_valid(phandle_args.node))
		return NULL;

	node = phandle_args.node;

	ret = ofnode_read_eth_phy_id(node, &vendor, &device);
	if (ret) {
		debug("Failed to read eth PHY id, err: %d\n", ret);
		return NULL;
	}

	if (!IS_ENABLED(CONFIG_DM_ETH_PHY)) {
		ret = gpio_request_by_name_nodev(node, "reset-gpios", 0, &gpio,
						 GPIOD_ACTIVE_LOW);
		if (!ret) {
			assert = ofnode_read_u32_default(node,
							 "reset-assert-us", 0);
			deassert = ofnode_read_u32_default(node,
							   "reset-deassert-us",
							   0);
			ret = dm_gpio_set_value(&gpio, 1);
			if (ret) {
				dev_err(dev,
					"Failed assert gpio, err: %d\n", ret);
				return NULL;
			}

			udelay(assert);

			ret = dm_gpio_set_value(&gpio, 0);
			if (ret) {
				dev_err(dev,
					"Failed deassert gpio, err: %d\n",
					ret);
				return NULL;
			}

			udelay(deassert);
		}
	}

	id =  vendor << 16 | device;
	phydev = phy_device_create(bus, phyaddr, id, false);
	if (phydev)
		phydev->node = node;

	return phydev;
}
