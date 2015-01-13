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
#include <fdtdec.h>
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

DECLARE_GLOBAL_DATA_PTR;

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

	/*
	 * For some devices, such as a SPI or I2C bus, the 'reg' property
	 * is a reasonable indicator of the sequence number. But if there is
	 * an alias, we use that in preference. In any case, this is just
	 * a 'requested' sequence, and will be resolved (and ->seq updated)
	 * when the device is probed.
	 */
	dev->seq = -1;
#ifdef CONFIG_OF_CONTROL
	dev->req_seq = fdtdec_get_int(gd->fdt_blob, of_offset, "reg", -1);
	if (!IS_ERR_VALUE(dev->req_seq))
		dev->req_seq &= INT_MAX;
	if (uc->uc_drv->name && of_offset != -1) {
		fdtdec_get_alias_seq(gd->fdt_blob, uc->uc_drv->name, of_offset,
				     &dev->req_seq);
	}
#else
	dev->req_seq = -1;
#endif
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

int device_bind_by_name(struct udevice *parent, bool pre_reloc_only,
			const struct driver_info *info, struct udevice **devp)
{
	struct driver *drv;

	drv = lists_driver_lookup_name(info->name);
	if (!drv)
		return -ENOENT;
	if (pre_reloc_only && !(drv->flags & DM_FLAG_PRE_RELOC))
		return -EPERM;

	return device_bind(parent, drv, info->name, (void *)info->platdata,
			   -1, devp);
}

int device_probe_child(struct udevice *dev, void *parent_priv)
{
	struct driver *drv;
	int size = 0;
	int ret;
	int seq;

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
		size = dev->parent->driver->per_child_auto_alloc_size;
		if (size) {
			dev->parent_priv = calloc(1, size);
			if (!dev->parent_priv) {
				ret = -ENOMEM;
				goto fail;
			}
			if (parent_priv)
				memcpy(dev->parent_priv, parent_priv, size);
		}

		ret = device_probe(dev->parent);
		if (ret)
			goto fail;
	}

	seq = uclass_resolve_seq(dev);
	if (seq < 0) {
		ret = seq;
		goto fail;
	}
	dev->seq = seq;

	if (dev->parent && dev->parent->driver->child_pre_probe) {
		ret = dev->parent->driver->child_pre_probe(dev);
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
	dev->seq = -1;
	device_free(dev);

	return ret;
}

int device_probe(struct udevice *dev)
{
	return device_probe_child(dev, NULL);
}

void *dev_get_platdata(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->platdata;
}

void *dev_get_priv(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->priv;
}

void *dev_get_parentdata(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->parent_priv;
}

static int device_get_device_tail(struct udevice *dev, int ret,
				  struct udevice **devp)
{
	if (ret)
		return ret;

	ret = device_probe(dev);
	if (ret)
		return ret;

	*devp = dev;

	return 0;
}

int device_get_child(struct udevice *parent, int index, struct udevice **devp)
{
	struct udevice *dev;

	list_for_each_entry(dev, &parent->child_head, sibling_node) {
		if (!index--)
			return device_get_device_tail(dev, 0, devp);
	}

	return -ENODEV;
}

int device_find_child_by_seq(struct udevice *parent, int seq_or_req_seq,
			     bool find_req_seq, struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;
	if (seq_or_req_seq == -1)
		return -ENODEV;

	list_for_each_entry(dev, &parent->child_head, sibling_node) {
		if ((find_req_seq ? dev->req_seq : dev->seq) ==
				seq_or_req_seq) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int device_get_child_by_seq(struct udevice *parent, int seq,
			    struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = device_find_child_by_seq(parent, seq, false, &dev);
	if (ret == -ENODEV) {
		/*
		 * We didn't find it in probed devices. See if there is one
		 * that will request this seq if probed.
		 */
		ret = device_find_child_by_seq(parent, seq, true, &dev);
	}
	return device_get_device_tail(dev, ret, devp);
}

int device_find_child_by_of_offset(struct udevice *parent, int of_offset,
				   struct udevice **devp)
{
	struct udevice *dev;

	*devp = NULL;

	list_for_each_entry(dev, &parent->child_head, sibling_node) {
		if (dev->of_offset == of_offset) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int device_get_child_by_of_offset(struct udevice *parent, int seq,
				  struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = device_find_child_by_of_offset(parent, seq, &dev);
	return device_get_device_tail(dev, ret, devp);
}

int device_find_first_child(struct udevice *parent, struct udevice **devp)
{
	if (list_empty(&parent->child_head)) {
		*devp = NULL;
	} else {
		*devp = list_first_entry(&parent->child_head, struct udevice,
					 sibling_node);
	}

	return 0;
}

int device_find_next_child(struct udevice **devp)
{
	struct udevice *dev = *devp;
	struct udevice *parent = dev->parent;

	if (list_is_last(&dev->sibling_node, &parent->child_head)) {
		*devp = NULL;
	} else {
		*devp = list_entry(dev->sibling_node.next, struct udevice,
				   sibling_node);
	}

	return 0;
}

struct udevice *dev_get_parent(struct udevice *child)
{
	return child->parent;
}

ulong dev_get_of_data(struct udevice *dev)
{
	return dev->of_id->data;
}
