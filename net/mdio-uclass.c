// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <miiphy.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/uclass-internal.h>
#include <linux/compat.h>

/* DT node properties for MAC-PHY interface */
#define PHY_MODE_STR_CNT	2
static const char *phy_mode_str[PHY_MODE_STR_CNT] = { "phy-mode",
						      "phy-connection-type" };
/* DT node properties that reference a PHY node */
#define PHY_HANDLE_STR_CNT	3
const char *phy_handle_str[PHY_HANDLE_STR_CNT] = { "phy-handle",
						   "phy",
						   "phy-device" };

void dm_mdio_probe_devices(void)
{
	struct udevice *it;
	struct uclass *uc;

	uclass_get(UCLASS_MDIO, &uc);
	uclass_foreach_dev(it, uc) {
		device_probe(it);
	}
}

static int dm_mdio_post_bind(struct udevice *dev)
{
	const char *dt_name;

	/* set a custom name for the MDIO device, if present in DT */
	if (ofnode_valid(dev->node)) {
		dt_name = ofnode_read_string(dev->node, "device-name");
		if (dt_name) {
			debug("renaming dev %s to %s\n", dev->name, dt_name);
			device_set_name(dev, dt_name);
		}
	}

	/*
	 * MDIO command doesn't like spaces in names, don't allow them to keep
	 * it happy
	 */
	if (strchr(dev->name, ' ')) {
		debug("\nError: MDIO device name \"%s\" has a space!\n",
		      dev->name);
		return -EINVAL;
	}

	return 0;
}

/*
 * Following read/write/reset functions are registered with legacy MII code.
 * These are called for PHY operations by upper layers and we further call the
 * DM MDIO driver functions.
 */
static int mdio_read(struct mii_dev *mii_bus, int addr, int devad, int reg)
{
	struct udevice *dev = mii_bus->priv;

	return mdio_get_ops(dev)->read(dev, addr, devad, reg);
}

static int mdio_write(struct mii_dev *mii_bus, int addr, int devad, int reg,
		      u16 val)
{
	struct udevice *dev = mii_bus->priv;

	return mdio_get_ops(dev)->write(dev, addr, devad, reg, val);
}

static int mdio_reset(struct mii_dev *mii_bus)
{
	struct udevice *dev = mii_bus->priv;

	if (mdio_get_ops(dev)->reset)
		return mdio_get_ops(dev)->reset(dev);
	else
		return 0;
}

static int dm_mdio_post_probe(struct udevice *dev)
{
	struct mdio_perdev_priv *pdata = dev_get_uclass_priv(dev);

	pdata->mii_bus = mdio_alloc();
	pdata->mii_bus->read = mdio_read;
	pdata->mii_bus->write = mdio_write;
	pdata->mii_bus->reset = mdio_reset;
	pdata->mii_bus->priv = dev;
	strncpy(pdata->mii_bus->name, dev->name, MDIO_NAME_LEN - 1);

	return mdio_register(pdata->mii_bus);
}

static int dm_mdio_pre_remove(struct udevice *dev)
{
	struct mdio_perdev_priv *pdata = dev_get_uclass_priv(dev);
	struct mdio_ops *ops = mdio_get_ops(dev);

	if (ops->reset)
		ops->reset(dev);
	mdio_unregister(pdata->mii_bus);
	mdio_free(pdata->mii_bus);

	return 0;
}

struct phy_device *dm_mdio_phy_connect(struct udevice *mdiodev, int phyaddr,
				       struct udevice *ethdev,
				       phy_interface_t interface)
{
	struct mdio_perdev_priv *pdata = dev_get_uclass_priv(mdiodev);

	if (device_probe(mdiodev))
		return NULL;

	return phy_connect(pdata->mii_bus, phyaddr, ethdev, interface);
}

static struct phy_device *dm_eth_connect_phy_handle(struct udevice *ethdev,
						    phy_interface_t interface)
{
	u32 phy_addr;
	struct udevice *mdiodev;
	struct phy_device *phy;
	struct ofnode_phandle_args phandle = {.node = ofnode_null()};
	int i;

	for (i = 0; i < PHY_HANDLE_STR_CNT; i++)
		if (!dev_read_phandle_with_args(ethdev, phy_handle_str[i], NULL,
						0, 0, &phandle))
			break;

	if (!ofnode_valid(phandle.node)) {
		dev_dbg(dev, "can't find PHY node\n");
		return NULL;
	}

	/*
	 * reading 'reg' directly should be fine.  This is a PHY node, the
	 * address is always size 1 and requires no translation
	 */
	if (ofnode_read_u32(phandle.node, "reg", &phy_addr)) {
		dev_dbg(ethdev, "missing reg property in phy node\n");
		return NULL;
	}

	if (uclass_get_device_by_ofnode(UCLASS_MDIO,
					ofnode_get_parent(phandle.node),
					&mdiodev)) {
		dev_dbg(dev, "can't find MDIO bus for node %s\n",
			ofnode_get_name(ofnode_get_parent(phandle.node)));
		return NULL;
	}

	phy = dm_mdio_phy_connect(mdiodev, phy_addr, ethdev, interface);

	if (phy)
		phy->node = phandle.node;

	return phy;
}

/* Connect to a PHY linked in eth DT node */
struct phy_device *dm_eth_phy_connect(struct udevice *ethdev)
{
	const char *if_str;
	phy_interface_t interface;
	struct phy_device *phy;
	int i;

	if (!ofnode_valid(ethdev->node)) {
		debug("%s: supplied eth dev has no DT node!\n", ethdev->name);
		return NULL;
	}

	interface = PHY_INTERFACE_MODE_NONE;
	for (i = 0; i < PHY_MODE_STR_CNT; i++) {
		if_str = ofnode_read_string(ethdev->node, phy_mode_str[i]);
		if (if_str) {
			interface = phy_get_interface_by_name(if_str);
			break;
		}
	}
	if (interface < 0)
		interface = PHY_INTERFACE_MODE_NONE;
	if (interface == PHY_INTERFACE_MODE_NONE)
		dev_dbg(ethdev, "can't find interface mode, default to NONE\n");

	phy = dm_eth_connect_phy_handle(ethdev, interface);

	if (!phy)
		return NULL;

	phy->interface = interface;

	return phy;
}

UCLASS_DRIVER(mdio) = {
	.id = UCLASS_MDIO,
	.name = "mdio",
	.post_bind  = dm_mdio_post_bind,
	.post_probe = dm_mdio_post_probe,
	.pre_remove = dm_mdio_pre_remove,
	.per_device_auto_alloc_size = sizeof(struct mdio_perdev_priv),
};
