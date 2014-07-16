/*
 * Device manager
 *
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/platdata.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <linux/err.h>
#include <linux/list.h>

/**
 * device_chld_unbind() - Unbind all device's children from the device
 *
 * On error, the function continues to unbind all children, and reports the
 * first error.
 *
 * @dev:	The device that is to be stripped of its children
 * @return 0 on success, -ve on error
 */
static int device_chld_unbind(struct udevice *dev)
{
	struct udevice *pos, *n;
	int ret, saved_ret = 0;

	assert(dev);

	list_for_each_entry_safe(pos, n, &dev->child_head, sibling_node) {
		ret = device_unbind(pos);
		if (ret && !saved_ret)
			saved_ret = ret;
	}

	return saved_ret;
}

/**
 * device_chld_remove() - Stop all device's children
 * @dev:	The device whose children are to be removed
 * @return 0 on success, -ve on error
 */
static int device_chld_remove(struct udevice *dev)
{
	struct udevice *pos, *n;
	int ret;

	assert(dev);

	list_for_each_entry_safe(pos, n, &dev->child_head, sibling_node) {
		ret = device_remove(pos);
		if (ret)
			return ret;
	}

	return 0;
}

int device_bind(struct udevice *parent, struct driver *drv, const char *name,
		void *platdata, int of_offset, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret = 0;

	*devp = NULL;
	if (!name)
		return -EINVAL;

	ret = uclass_get(drv->id, &uc);
	if (ret)
		return ret;

	dev = calloc(1, sizeof(struct udevice));
	if (!dev)
		return -ENOMEM;

	INIT_LIST_HEAD(&dev->sibling_node);
	INIT_LIST_HEAD(&dev->child_head);
	INIT_LIST_HEAD(&dev->uclass_node);
	dev->platdata = platdata;
	dev->name = name;
	dev->of_offset = of_offset;
	dev->parent = parent;
	dev->driver = drv;
	dev->uclass = uc;
	if (!dev->platdata && drv->platdata_auto_alloc_size)
		dev->flags |= DM_FLAG_ALLOC_PDATA;

	/* put dev into parent's successor list */
	if (parent)
		list_add_tail(&dev->sibling_node, &parent->child_head);

	ret = uclass_bind_device(dev);
	if (ret)
		goto fail_bind;

	/* if we fail to bind we remove device from successors and free it */
	if (drv->bind) {
		ret = drv->bind(dev);
		if (ret) {
			if (uclass_unbind_device(dev)) {
				dm_warn("Failed to unbind dev '%s' on error path\n",
					dev->name);
			}
			goto fail_bind;
		}
	}
	if (parent)
		dm_dbg("Bound device %s to %s\n", dev->name, parent->name);
	*devp = dev;

	return 0;

fail_bind:
	list_del(&dev->sibling_node);
	free(dev);
	return ret;
}

int device_bind_by_name(struct udevice *parent, const struct driver_info *info,
			struct udevice **devp)
{
	struct driver *drv;

	drv = lists_driver_lookup_name(info->name);
	if (!drv)
		return -ENOENT;

	return device_bind(parent, drv, info->name, (void *)info->platdata,
			   -1, devp);
}

int device_unbind(struct udevice *dev)
{
	struct driver *drv;
	int ret;

	if (!dev)
		return -EINVAL;

	if (dev->flags & DM_FLAG_ACTIVATED)
		return -EINVAL;

	drv = dev->driver;
	assert(drv);

	if (drv->unbind) {
		ret = drv->unbind(dev);
		if (ret)
			return ret;
	}

	ret = device_chld_unbind(dev);
	if (ret)
		return ret;

	ret = uclass_unbind_device(dev);
	if (ret)
		return ret;

	if (dev->parent)
		list_del(&dev->sibling_node);
	free(dev);

	return 0;
}

/**
 * device_free() - Free memory buffers allocated by a device
 * @dev:	Device that is to be started
 */
static void device_free(struct udevice *dev)
{
	int size;

	if (dev->driver->priv_auto_alloc_size) {
		free(dev->priv);
		dev->priv = NULL;
	}
	if (dev->flags & DM_FLAG_ALLOC_PDATA) {
		free(dev->platdata);
		dev->platdata = NULL;
	}
	size = dev->uclass->uc_drv->per_device_auto_alloc_size;
	if (size) {
		free(dev->uclass_priv);
		dev->uclass_priv = NULL;
	}
}

int device_probe(struct udevice *dev)
{
	struct driver *drv;
	int size = 0;
	int ret;

	if (!dev)
		return -EINVAL;

	if (dev->flags & DM_FLAG_ACTIVATED)
		return 0;

	drv = dev->driver;
	assert(drv);

	/* Allocate private data and platdata if requested */
	if (drv->priv_auto_alloc_size) {
		dev->priv = calloc(1, drv->priv_auto_alloc_size);
		if (!dev->priv) {
			ret = -ENOMEM;
			goto fail;
		}
	}
	/* Allocate private data if requested */
	if (dev->flags & DM_FLAG_ALLOC_PDATA) {
		dev->platdata = calloc(1, drv->platdata_auto_alloc_size);
		if (!dev->platdata) {
			ret = -ENOMEM;
			goto fail;
		}
	}
	size = dev->uclass->uc_drv->per_device_auto_alloc_size;
	if (size) {
		dev->uclass_priv = calloc(1, size);
		if (!dev->uclass_priv) {
			ret = -ENOMEM;
			goto fail;
		}
	}

	/* Ensure all parents are probed */
	if (dev->parent) {
		ret = device_probe(dev->parent);
		if (ret)
			goto fail;
	}

	if (drv->ofdata_to_platdata && dev->of_offset >= 0) {
		ret = drv->ofdata_to_platdata(dev);
		if (ret)
			goto fail;
	}

	if (drv->probe) {
		ret = drv->probe(dev);
		if (ret)
			goto fail;
	}

	dev->flags |= DM_FLAG_ACTIVATED;

	ret = uclass_post_probe_device(dev);
	if (ret) {
		dev->flags &= ~DM_FLAG_ACTIVATED;
		goto fail_uclass;
	}

	return 0;
fail_uclass:
	if (device_remove(dev)) {
		dm_warn("%s: Device '%s' failed to remove on error path\n",
			__func__, dev->name);
	}
fail:
	device_free(dev);

	return ret;
}

int device_remove(struct udevice *dev)
{
	struct driver *drv;
	int ret;

	if (!dev)
		return -EINVAL;

	if (!(dev->flags & DM_FLAG_ACTIVATED))
		return 0;

	drv = dev->driver;
	assert(drv);

	ret = uclass_pre_remove_device(dev);
	if (ret)
		return ret;

	ret = device_chld_remove(dev);
	if (ret)
		goto err;

	if (drv->remove) {
		ret = drv->remove(dev);
		if (ret)
			goto err_remove;
	}

	device_free(dev);

	dev->flags &= ~DM_FLAG_ACTIVATED;

	return 0;

err_remove:
	/* We can't put the children back */
	dm_warn("%s: Device '%s' failed to remove, but children are gone\n",
		__func__, dev->name);
err:
	ret = uclass_post_probe_device(dev);
	if (ret) {
		dm_warn("%s: Device '%s' failed to post_probe on error path\n",
			__func__, dev->name);
	}

	return ret;
}

void *dev_get_platdata(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device", __func__);
		return NULL;
	}

	return dev->platdata;
}

void *dev_get_priv(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device", __func__);
		return NULL;
	}

	return dev->priv;
}
