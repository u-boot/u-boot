/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <spi.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <dm/root.h>
#include <dm/lists.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

static int spi_set_speed_mode(struct udevice *bus, int speed, int mode)
{
	struct dm_spi_ops *ops;
	int ret;

	ops = spi_get_ops(bus);
	if (ops->set_speed)
		ret = ops->set_speed(bus, speed);
	else
		ret = -EINVAL;
	if (ret) {
		printf("Cannot set speed (err=%d)\n", ret);
		return ret;
	}

	if (ops->set_mode)
		ret = ops->set_mode(bus, mode);
	else
		ret = -EINVAL;
	if (ret) {
		printf("Cannot set mode (err=%d)\n", ret);
		return ret;
	}

	return 0;
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct udevice *dev = slave->dev;
	struct udevice *bus = dev->parent;
	struct dm_spi_ops *ops = spi_get_ops(bus);
	struct dm_spi_bus *spi = dev_get_uclass_priv(bus);
	int speed;
	int ret;

	speed = slave->max_hz;
	if (spi->max_hz) {
		if (speed)
			speed = min(speed, (int)spi->max_hz);
		else
			speed = spi->max_hz;
	}
	if (!speed)
		speed = 100000;
	if (speed != slave->speed) {
		ret = spi_set_speed_mode(bus, speed, slave->mode);
		if (ret)
			return ret;
		slave->speed = speed;
	}

	return ops->claim_bus ? ops->claim_bus(dev) : 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct udevice *dev = slave->dev;
	struct udevice *bus = dev->parent;
	struct dm_spi_ops *ops = spi_get_ops(bus);

	if (ops->release_bus)
		ops->release_bus(dev);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	struct udevice *dev = slave->dev;
	struct udevice *bus = dev->parent;

	if (bus->uclass->uc_drv->id != UCLASS_SPI)
		return -EOPNOTSUPP;

	return spi_get_ops(bus)->xfer(dev, bitlen, dout, din, flags);
}

int spi_post_bind(struct udevice *dev)
{
	/* Scan the bus for devices */
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

int spi_child_post_bind(struct udevice *dev)
{
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);

	if (dev->of_offset == -1)
		return 0;

	return spi_slave_ofdata_to_platdata(gd->fdt_blob, dev->of_offset, plat);
}

int spi_post_probe(struct udevice *bus)
{
	struct dm_spi_bus *spi = dev_get_uclass_priv(bus);

	spi->max_hz = fdtdec_get_int(gd->fdt_blob, bus->of_offset,
				     "spi-max-frequency", 0);

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	struct dm_spi_ops *ops = spi_get_ops(bus);


	if (ops->claim_bus)
		ops->claim_bus += gd->reloc_off;
	if (ops->release_bus)
		ops->release_bus += gd->reloc_off;
	if (ops->set_wordlen)
		ops->set_wordlen += gd->reloc_off;
	if (ops->xfer)
		ops->xfer += gd->reloc_off;
	if (ops->set_speed)
		ops->set_speed += gd->reloc_off;
	if (ops->set_mode)
		ops->set_mode += gd->reloc_off;
	if (ops->cs_info)
		ops->cs_info += gd->reloc_off;
#endif

	return 0;
}

int spi_child_pre_probe(struct udevice *dev)
{
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);
	struct spi_slave *slave = dev_get_parentdata(dev);

	/*
	 * This is needed because we pass struct spi_slave around the place
	 * instead slave->dev (a struct udevice). So we have to have some
	 * way to access the slave udevice given struct spi_slave. Once we
	 * change the SPI API to use udevice instead of spi_slave, we can
	 * drop this.
	 */
	slave->dev = dev;

	slave->max_hz = plat->max_hz;
	slave->mode = plat->mode;

	return 0;
}

int spi_chip_select(struct udevice *dev)
{
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);

	return plat ? plat->cs : -ENOENT;
}

int spi_find_chip_select(struct udevice *bus, int cs, struct udevice **devp)
{
	struct udevice *dev;

	for (device_find_first_child(bus, &dev); dev;
	     device_find_next_child(&dev)) {
		struct dm_spi_slave_platdata *plat;

		plat = dev_get_parent_platdata(dev);
		debug("%s: plat=%p, cs=%d\n", __func__, plat, plat->cs);
		if (plat->cs == cs) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int spi_cs_is_valid(unsigned int busnum, unsigned int cs)
{
	struct spi_cs_info info;
	struct udevice *bus;
	int ret;

	ret = uclass_find_device_by_seq(UCLASS_SPI, busnum, false, &bus);
	if (ret) {
		debug("%s: No bus %d\n", __func__, busnum);
		return ret;
	}

	return spi_cs_info(bus, cs, &info);
}

int spi_cs_info(struct udevice *bus, uint cs, struct spi_cs_info *info)
{
	struct spi_cs_info local_info;
	struct dm_spi_ops *ops;
	int ret;

	if (!info)
		info = &local_info;

	/* If there is a device attached, return it */
	info->dev = NULL;
	ret = spi_find_chip_select(bus, cs, &info->dev);
	if (!ret)
		return 0;

	/*
	 * Otherwise ask the driver. For the moment we don't have CS info.
	 * When we do we could provide the driver with a helper function
	 * to figure out what chip selects are valid, or just handle the
	 * request.
	 */
	ops = spi_get_ops(bus);
	if (ops->cs_info)
		return ops->cs_info(bus, cs, info);

	/*
	 * We could assume there is at least one valid chip select, but best
	 * to be sure and return an error in this case. The driver didn't
	 * care enough to tell us.
	 */
	return -ENODEV;
}

int spi_find_bus_and_cs(int busnum, int cs, struct udevice **busp,
			struct udevice **devp)
{
	struct udevice *bus, *dev;
	int ret;

	ret = uclass_find_device_by_seq(UCLASS_SPI, busnum, false, &bus);
	if (ret) {
		debug("%s: No bus %d\n", __func__, busnum);
		return ret;
	}
	ret = spi_find_chip_select(bus, cs, &dev);
	if (ret) {
		debug("%s: No cs %d\n", __func__, cs);
		return ret;
	}
	*busp = bus;
	*devp = dev;

	return ret;
}

int spi_get_bus_and_cs(int busnum, int cs, int speed, int mode,
		       const char *drv_name, const char *dev_name,
		       struct udevice **busp, struct spi_slave **devp)
{
	struct udevice *bus, *dev;
	bool created = false;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_SPI, busnum, &bus);
	if (ret) {
		printf("Invalid bus %d (err=%d)\n", busnum, ret);
		return ret;
	}
	ret = spi_find_chip_select(bus, cs, &dev);

	/*
	 * If there is no such device, create one automatically. This means
	 * that we don't need a device tree node or platform data for the
	 * SPI flash chip - we will bind to the correct driver.
	 */
	if (ret == -ENODEV && drv_name) {
		struct dm_spi_slave_platdata *plat;

		debug("%s: Binding new device '%s', busnum=%d, cs=%d, driver=%s\n",
		      __func__, dev_name, busnum, cs, drv_name);
		ret = device_bind_driver(bus, drv_name, dev_name, &dev);
		if (ret)
			return ret;
		plat = dev_get_parent_platdata(dev);
		plat->cs = cs;
		plat->max_hz = speed;
		plat->mode = mode;
		created = true;
	} else if (ret) {
		printf("Invalid chip select %d:%d (err=%d)\n", busnum, cs,
		       ret);
		return ret;
	}

	if (!device_active(dev)) {
		struct spi_slave *slave;

		ret = device_probe(dev);
		if (ret)
			goto err;
		slave = dev_get_parentdata(dev);
		slave->dev = dev;
	}

	ret = spi_set_speed_mode(bus, speed, mode);
	if (ret)
		goto err;

	*busp = bus;
	*devp = dev_get_parentdata(dev);
	debug("%s: bus=%p, slave=%p\n", __func__, bus, *devp);

	return 0;

err:
	debug("%s: Error path, credted=%d, device '%s'\n", __func__,
	      created, dev->name);
	if (created) {
		device_remove(dev);
		device_unbind(dev);
	}

	return ret;
}

/* Compatibility function - to be removed */
struct spi_slave *spi_setup_slave_fdt(const void *blob, int node,
				      int bus_node)
{
	struct udevice *bus, *dev;
	int ret;

	ret = uclass_get_device_by_of_offset(UCLASS_SPI, bus_node, &bus);
	if (ret)
		return NULL;
	ret = device_get_child_by_of_offset(bus, node, &dev);
	if (ret)
		return NULL;
	return dev_get_parentdata(dev);
}

/* Compatibility function - to be removed */
struct spi_slave *spi_setup_slave(unsigned int busnum, unsigned int cs,
				  unsigned int speed, unsigned int mode)
{
	struct spi_slave *slave;
	struct udevice *dev;
	int ret;

	ret = spi_get_bus_and_cs(busnum, cs, speed, mode, NULL, 0, &dev,
				  &slave);
	if (ret)
		return NULL;

	return slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	device_remove(slave->dev);
	slave->dev = NULL;
}

int spi_slave_ofdata_to_platdata(const void *blob, int node,
				 struct dm_spi_slave_platdata *plat)
{
	int mode = 0;

	plat->cs = fdtdec_get_int(blob, node, "reg", -1);
	plat->max_hz = fdtdec_get_int(blob, node, "spi-max-frequency", 0);
	if (fdtdec_get_bool(blob, node, "spi-cpol"))
		mode |= SPI_CPOL;
	if (fdtdec_get_bool(blob, node, "spi-cpha"))
		mode |= SPI_CPHA;
	if (fdtdec_get_bool(blob, node, "spi-cs-high"))
		mode |= SPI_CS_HIGH;
	if (fdtdec_get_bool(blob, node, "spi-half-duplex"))
		mode |= SPI_PREAMBLE;
	plat->mode = mode;

	return 0;
}

UCLASS_DRIVER(spi) = {
	.id		= UCLASS_SPI,
	.name		= "spi",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_bind	= spi_post_bind,
	.post_probe	= spi_post_probe,
	.child_pre_probe = spi_child_pre_probe,
	.per_device_auto_alloc_size = sizeof(struct dm_spi_bus),
	.per_child_auto_alloc_size = sizeof(struct spi_slave),
	.per_child_platdata_auto_alloc_size =
			sizeof(struct dm_spi_slave_platdata),
	.child_post_bind = spi_child_post_bind,
};

UCLASS_DRIVER(spi_generic) = {
	.id		= UCLASS_SPI_GENERIC,
	.name		= "spi_generic",
};

U_BOOT_DRIVER(spi_generic_drv) = {
	.name		= "spi_generic_drv",
	.id		= UCLASS_SPI_GENERIC,
};
