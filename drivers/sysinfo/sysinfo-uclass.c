// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#define LOG_CATEGORY UCLASS_SYSINFO

#include <dm.h>
#include <sysinfo.h>

struct sysinfo_priv {
	bool detected;
};

int sysinfo_get(struct udevice **devp)
{
	int ret = uclass_first_device_err(UCLASS_SYSINFO, devp);

	/*
	 * There is some very dodgy error handling in gazerbeam,
	 * do not return a device on error.
	 */
	if (ret)
		*devp = NULL;
	return ret;
}

int sysinfo_detect(struct udevice *dev)
{
	int ret;
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!ops->detect)
		return -ENOSYS;

	ret = ops->detect(dev);
	if (!ret)
		priv->detected = true;

	return ret;
}

int sysinfo_get_fit_loadable(struct udevice *dev, int index, const char *type,
			     const char **strp)
{
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_fit_loadable)
		return -ENOSYS;

	return ops->get_fit_loadable(dev, index, type, strp);
}

int sysinfo_get_bool(struct udevice *dev, int id, bool *val)
{
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_bool)
		return -ENOSYS;

	return ops->get_bool(dev, id, val);
}

int sysinfo_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_int)
		return -ENOSYS;

	return ops->get_int(dev, id, val);
}

int sysinfo_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_str)
		return -ENOSYS;

	return ops->get_str(dev, id, size, val);
}

int sysinfo_get_data(struct udevice *dev, int id, void **data, size_t *size)
{
	struct sysinfo_priv *priv;
	struct sysinfo_ops *ops;

	if (!dev)
		return -ENOSYS;

	priv = dev_get_uclass_priv(dev);
	ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_data)
		return -ENOSYS;

	return ops->get_data(dev, id, data, size);
}

int sysinfo_get_item_count(struct udevice *dev, int id)
{
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_item_count)
		return -ENOSYS;

	return ops->get_item_count(dev, id);
}

int sysinfo_get_data_by_index(struct udevice *dev, int id, int index,
			      void **data, size_t *size)
{
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_data_by_index)
		return -ENOSYS;

	return ops->get_data_by_index(dev, id, index, data, size);
}

UCLASS_DRIVER(sysinfo) = {
	.id		= UCLASS_SYSINFO,
	.name		= "sysinfo",
	.post_bind	= dm_scan_fdt_dev,
	.per_device_auto	= sizeof(bool),
};
