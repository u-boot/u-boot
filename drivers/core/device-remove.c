/*
 * Device manager
 *
 * Copyright (c) 2014 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>

int device_unbind_children(struct udevice *dev)
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

int device_remove_children(struct udevice *dev)
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

int device_unbind(struct udevice *dev)
{
	const struct driver *drv;
	int ret;

	if (!dev)
		return -EINVAL;

	if (dev->flags & DM_FLAG_ACTIVATED)
		return -EINVAL;

	if (!(dev->flags & DM_FLAG_BOUND))
		return -EINVAL;

	drv = dev->driver;
	assert(drv);

	if (drv->unbind) {
		ret = drv->unbind(dev);
		if (ret)
			return ret;
	}

	ret = device_unbind_children(dev);
	if (ret)
		return ret;

	if (dev->flags & DM_FLAG_ALLOC_PDATA) {
		free(dev->platdata);
		dev->platdata = NULL;
	}
	if (dev->flags & DM_FLAG_ALLOC_UCLASS_PDATA) {
		free(dev->uclass_platdata);
		dev->uclass_platdata = NULL;
	}
	if (dev->flags & DM_FLAG_ALLOC_PARENT_PDATA) {
		free(dev->parent_platdata);
		dev->parent_platdata = NULL;
	}
	ret = uclass_unbind_device(dev);
	if (ret)
		return ret;

	if (dev->parent)
		list_del(&dev->sibling_node);

	devres_release_all(dev);

	free(dev);

	return 0;
}

/**
 * device_free() - Free memory buffers allocated by a device
 * @dev:	Device that is to be started
 */
void device_free(struct udevice *dev)
{
	int size;

	if (dev->driver->priv_auto_alloc_size) {
		free(dev->priv);
		dev->priv = NULL;
	}
	size = dev->uclass->uc_drv->per_device_auto_alloc_size;
	if (size) {
		free(dev->uclass_priv);
		dev->uclass_priv = NULL;
	}
	if (dev->parent) {
		size = dev->parent->driver->per_child_auto_alloc_size;
		if (!size) {
			size = dev->parent->uclass->uc_drv->
					per_child_auto_alloc_size;
		}
		if (size) {
			free(dev->parent_priv);
			dev->parent_priv = NULL;
		}
	}

	devres_release_probe(dev);
}

int device_remove(struct udevice *dev)
{
	const struct driver *drv;
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

	ret = device_remove_children(dev);
	if (ret)
		goto err;

	if (drv->remove) {
		ret = drv->remove(dev);
		if (ret)
			goto err_remove;
	}

	if (dev->parent && dev->parent->driver->child_post_remove) {
		ret = dev->parent->driver->child_post_remove(dev);
		if (ret) {
			dm_warn("%s: Device '%s' failed child_post_remove()",
				__func__, dev->name);
		}
	}

	device_free(dev);

	dev->seq = -1;
	dev->flags &= ~DM_FLAG_ACTIVATED;

	return ret;

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
