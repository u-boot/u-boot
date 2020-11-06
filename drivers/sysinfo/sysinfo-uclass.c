// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <sysinfo.h>

int sysinfo_get(struct udevice **devp)
{
	return uclass_first_device_err(UCLASS_SYSINFO, devp);
}

int sysinfo_detect(struct udevice *dev)
{
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!ops->detect)
		return -ENOSYS;

	return ops->detect(dev);
}

int sysinfo_get_fit_loadable(struct udevice *dev, int index, const char *type,
			     const char **strp)
{
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!ops->get_fit_loadable)
		return -ENOSYS;

	return ops->get_fit_loadable(dev, index, type, strp);
}

int sysinfo_get_bool(struct udevice *dev, int id, bool *val)
{
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!ops->get_bool)
		return -ENOSYS;

	return ops->get_bool(dev, id, val);
}

int sysinfo_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!ops->get_int)
		return -ENOSYS;

	return ops->get_int(dev, id, val);
}

int sysinfo_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!ops->get_str)
		return -ENOSYS;

	return ops->get_str(dev, id, size, val);
}

UCLASS_DRIVER(sysinfo) = {
	.id		= UCLASS_SYSINFO,
	.name		= "sysinfo",
	.post_bind	= dm_scan_fdt_dev,
};
