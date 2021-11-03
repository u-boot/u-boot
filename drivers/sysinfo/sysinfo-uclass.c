// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#define LOG_CATEGORY UCLASS_SYSINFO

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <sysinfo.h>

struct sysinfo_priv {
	bool detected;
};

int sysinfo_get(struct udevice **devp)
{
	return uclass_first_device_err(UCLASS_SYSINFO, devp);
}

int sysinfo_detect(struct udevice *dev)
{
	int ret;
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	ret = ops->detect ? ops->detect(dev) : 0;
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

int sysinfo_get_str_list(struct udevice *dev, int id, unsigned idx, size_t size,
			 char *val)
{
	struct sysinfo_priv *priv = dev_get_uclass_priv(dev);
	struct sysinfo_ops *ops = sysinfo_get_ops(dev);

	if (!priv->detected)
		return -EPERM;

	if (!ops->get_str_list)
		return -ENOSYS;

	return ops->get_str_list(dev, id, idx, size, val);
}

int sysinfo_get_str_list_max_len(struct udevice *dev, int id)
{
	int maxlen = 0;
	unsigned i;

	/* first find out length of the longest string in the list */
	for (i = 0; ; ++i) {
		char dummy[1];
		int len;

		len = sysinfo_get_str_list(dev, id, i, 0, dummy);
		if (len == -ERANGE)
			break;
		else if (len < 0)
			return len;
		else if (len > maxlen)
			maxlen = len;
	}

	return maxlen;
}

struct sysinfo_str_list_iter {
	struct udevice *dev;
	int id;
	size_t size;
	unsigned idx;
	char value[];
};

char *sysinfo_str_list_first(struct udevice *dev, int id, void *_iter)
{
	struct sysinfo_str_list_iter *iter, **iterp = _iter;
	int maxlen, res;

	maxlen = sysinfo_get_str_list_max_len(dev, id);
	if (maxlen < 0)
		return NULL;

	iter = malloc(sizeof(struct sysinfo_str_list_iter) + maxlen + 1);
	if (!iter) {
		printf("No memory for sysinfo string list iterator\n");
		return NULL;
	}

	iter->dev = dev;
	iter->id = id;
	iter->size = maxlen + 1;
	iter->idx = 0;

	res = sysinfo_get_str_list(dev, id, 0, iter->size, iter->value);
	if (res < 0) {
		*iterp = NULL;
		free(iter);
		return NULL;
	}

	*iterp = iter;

	return iter->value;
}

char *sysinfo_str_list_next(void *_iter)
{
	struct sysinfo_str_list_iter **iterp = _iter, *iter = *iterp;
	int res;

	res = sysinfo_get_str_list(iter->dev, iter->id, ++iter->idx, iter->size,
				   iter->value);
	if (res < 0) {
		*iterp = NULL;
		free(iter);
		return NULL;
	}

	return iter->value;
}

UCLASS_DRIVER(sysinfo) = {
	.id		= UCLASS_SYSINFO,
	.name		= "sysinfo",
	.post_bind	= dm_scan_fdt_dev,
	.per_device_auto	= sizeof(bool),
};
