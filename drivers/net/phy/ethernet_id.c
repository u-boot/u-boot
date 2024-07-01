// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx ethernet phy reset driver
 *
 * Copyright (C) 2022 Xilinx, Inc.
 */

#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <phy.h>
#include <linux/delay.h>
#include <asm/gpio.h>

struct phy_device *phy_connect_phy_id(struct mii_dev *bus, struct udevice *dev,
				      int phyaddr)
{
	struct phy_device *phydev;
	struct ofnode_phandle_args phandle_args;
	const char *node_name;
	struct udevice *pdev;
	u16 vendor, device;
	ofnode node;
	u32 id;
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

	ret = phy_gpio_reset(dev);
	if (ret)
		return NULL;

	if (phyaddr == -1)
		phyaddr = ofnode_read_u32_default(phandle_args.node, "reg", -1);

	id =  vendor << 16 | device;
	phydev = phy_device_create(bus, phyaddr, id, false);
	if (phydev)
		phydev->node = node;

	if (IS_ENABLED(CONFIG_DM_ETH_PHY) && ofnode_valid(node)) {
		node_name = ofnode_get_name(node);
		ret = device_bind_driver_to_node(dev, "eth_phy_generic_drv",
						 node_name, node,
						 &pdev);
		if (ret)
			return NULL;

		ret = device_probe(pdev);
		if (ret)
			return NULL;
	}

	return phydev;
}
