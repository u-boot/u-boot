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
#include <fdt_support.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dm/platdata.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <linux/err.h>
#include <linux/list.h>

DECLARE_GLOBAL_DATA_PTR;

int device_bind(struct udevice *parent, const struct driver *drv,
		const char *name, void *platdata, int of_offset,
		struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int size, ret = 0;

	if (devp)
		*devp = NULL;
	if (!name)
		return -EINVAL;

	ret = uclass_get(drv->id, &uc);
	if (ret) {
		debug("Missing uclass for driver %s\n", drv->name);
		return ret;
	}

	dev = calloc(1, sizeof(struct udevice));
	if (!dev)
		return -ENOMEM;

	INIT_LIST_HEAD(&dev->sibling_node);
	INIT_LIST_HEAD(&dev->child_head);
	INIT_LIST_HEAD(&dev->uclass_node);
#ifdef CONFIG_DEVRES
	INIT_LIST_HEAD(&dev->devres_head);
#endif
	dev->platdata = platdata;
	dev->name = name;
	dev->of_offset = of_offset;
	dev->parent = parent;
	dev->driver = drv;
	dev->uclass = uc;

	dev->seq = -1;
	dev->req_seq = -1;
	if (CONFIG_IS_ENABLED(OF_CONTROL) && CONFIG_IS_ENABLED(DM_SEQ_ALIAS)) {
		/*
		 * Some devices, such as a SPI bus, I2C bus and serial ports
		 * are numbered using aliases.
		 *
		 * This is just a 'requested' sequence, and will be
		 * resolved (and ->seq updated) when the device is probed.
		 */
		if (uc->uc_drv->flags & DM_UC_FLAG_SEQ_ALIAS) {
			if (uc->uc_drv->name && of_offset != -1) {
				fdtdec_get_alias_seq(gd->fdt_blob,
						uc->uc_drv->name, of_offset,
						&dev->req_seq);
			}
		}
	}

	if (!dev->platdata && drv->platdata_auto_alloc_size) {
		dev->flags |= DM_FLAG_ALLOC_PDATA;
		dev->platdata = calloc(1, drv->platdata_auto_alloc_size);
		if (!dev->platdata) {
			ret = -ENOMEM;
			goto fail_alloc1;
		}
	}

	size = uc->uc_drv->per_device_platdata_auto_alloc_size;
	if (size) {
		dev->flags |= DM_FLAG_ALLOC_UCLASS_PDATA;
		dev->uclass_platdata = calloc(1, size);
		if (!dev->uclass_platdata) {
			ret = -ENOMEM;
			goto fail_alloc2;
		}
	}

	if (parent) {
		size = parent->driver->per_child_platdata_auto_alloc_size;
		if (!size) {
			size = parent->uclass->uc_drv->
					per_child_platdata_auto_alloc_size;
		}
		if (size) {
			dev->flags |= DM_FLAG_ALLOC_PARENT_PDATA;
			dev->parent_platdata = calloc(1, size);
			if (!dev->parent_platdata) {
				ret = -ENOMEM;
				goto fail_alloc3;
			}
		}
	}

	/* put dev into parent's successor list */
	if (parent)
		list_add_tail(&dev->sibling_node, &parent->child_head);

	ret = uclass_bind_device(dev);
	if (ret)
		goto fail_uclass_bind;

	/* if we fail to bind we remove device from successors and free it */
	if (drv->bind) {
		ret = drv->bind(dev);
		if (ret)
			goto fail_bind;
	}
	if (parent && parent->driver->child_post_bind) {
		ret = parent->driver->child_post_bind(dev);
		if (ret)
			goto fail_child_post_bind;
	}
	if (uc->uc_drv->post_bind) {
		ret = uc->uc_drv->post_bind(dev);
		if (ret)
			goto fail_uclass_post_bind;
	}

	if (parent)
		dm_dbg("Bound device %s to %s\n", dev->name, parent->name);
	if (devp)
		*devp = dev;

	dev->flags |= DM_FLAG_BOUND;

	return 0;

fail_uclass_post_bind:
	/* There is no child unbind() method, so no clean-up required */
fail_child_post_bind:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		if (drv->unbind && drv->unbind(dev)) {
			dm_warn("unbind() method failed on dev '%s' on error path\n",
				dev->name);
		}
	}

fail_bind:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		if (uclass_unbind_device(dev)) {
			dm_warn("Failed to unbind dev '%s' on error path\n",
				dev->name);
		}
	}
fail_uclass_bind:
	if (CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)) {
		list_del(&dev->sibling_node);
		if (dev->flags & DM_FLAG_ALLOC_PARENT_PDATA) {
			free(dev->parent_platdata);
			dev->parent_platdata = NULL;
		}
	}
fail_alloc3:
	if (dev->flags & DM_FLAG_ALLOC_UCLASS_PDATA) {
		free(dev->uclass_platdata);
		dev->uclass_platdata = NULL;
	}
fail_alloc2:
	if (dev->flags & DM_FLAG_ALLOC_PDATA) {
		free(dev->platdata);
		dev->platdata = NULL;
	}
fail_alloc1:
	devres_release_all(dev);

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

static void *alloc_priv(int size, uint flags)
{
	void *priv;

	if (flags & DM_FLAG_ALLOC_PRIV_DMA) {
		priv = memalign(ARCH_DMA_MINALIGN, size);
		if (priv)
			memset(priv, '\0', size);
	} else {
		priv = calloc(1, size);
	}

	return priv;
}

int device_probe(struct udevice *dev)
{
	const struct driver *drv;
	int size = 0;
	int ret;
	int seq;

	if (!dev)
		return -EINVAL;

	if (dev->flags & DM_FLAG_ACTIVATED)
		return 0;

	drv = dev->driver;
	assert(drv);

	/* Allocate private data if requested and not reentered */
	if (drv->priv_auto_alloc_size && !dev->priv) {
		dev->priv = alloc_priv(drv->priv_auto_alloc_size, drv->flags);
		if (!dev->priv) {
			ret = -ENOMEM;
			goto fail;
		}
	}
	/* Allocate private data if requested and not reentered */
	size = dev->uclass->uc_drv->per_device_auto_alloc_size;
	if (size && !dev->uclass_priv) {
		dev->uclass_priv = calloc(1, size);
		if (!dev->uclass_priv) {
			ret = -ENOMEM;
			goto fail;
		}
	}

	/* Ensure all parents are probed */
	if (dev->parent) {
		size = dev->parent->driver->per_child_auto_alloc_size;
		if (!size) {
			size = dev->parent->uclass->uc_drv->
					per_child_auto_alloc_size;
		}
		if (size && !dev->parent_priv) {
			dev->parent_priv = alloc_priv(size, drv->flags);
			if (!dev->parent_priv) {
				ret = -ENOMEM;
				goto fail;
			}
		}

		ret = device_probe(dev->parent);
		if (ret)
			goto fail;

		/*
		 * The device might have already been probed during
		 * the call to device_probe() on its parent device
		 * (e.g. PCI bridge devices). Test the flags again
		 * so that we don't mess up the device.
		 */
		if (dev->flags & DM_FLAG_ACTIVATED)
			return 0;
	}

	seq = uclass_resolve_seq(dev);
	if (seq < 0) {
		ret = seq;
		goto fail;
	}
	dev->seq = seq;

	dev->flags |= DM_FLAG_ACTIVATED;

	/*
	 * Process pinctrl for everything except the root device, and
	 * continue regardless of the result of pinctrl. Don't process pinctrl
	 * settings for pinctrl devices since the device may not yet be
	 * probed.
	 */
	if (dev->parent && device_get_uclass_id(dev) != UCLASS_PINCTRL)
		pinctrl_select_state(dev, "default");

	ret = uclass_pre_probe_device(dev);
	if (ret)
		goto fail;

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
		if (ret) {
			dev->flags &= ~DM_FLAG_ACTIVATED;
			goto fail;
		}
	}

	ret = uclass_post_probe_device(dev);
	if (ret)
		goto fail_uclass;

	if (dev->parent && device_get_uclass_id(dev) == UCLASS_PINCTRL)
		pinctrl_select_state(dev, "default");

	return 0;
fail_uclass:
	if (device_remove(dev)) {
		dm_warn("%s: Device '%s' failed to remove on error path\n",
			__func__, dev->name);
	}
fail:
	dev->flags &= ~DM_FLAG_ACTIVATED;

	dev->seq = -1;
	device_free(dev);

	return ret;
}

void *dev_get_platdata(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->platdata;
}

void *dev_get_parent_platdata(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->parent_platdata;
}

void *dev_get_uclass_platdata(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->uclass_platdata;
}

void *dev_get_priv(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->priv;
}

void *dev_get_uclass_priv(struct udevice *dev)
{
	if (!dev) {
		dm_warn("%s: null device\n", __func__);
		return NULL;
	}

	return dev->uclass_priv;
}

void *dev_get_parent_priv(struct udevice *dev)
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

int device_get_child_by_of_offset(struct udevice *parent, int node,
				  struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	*devp = NULL;
	ret = device_find_child_by_of_offset(parent, node, &dev);
	return device_get_device_tail(dev, ret, devp);
}

static struct udevice *_device_find_global_by_of_offset(struct udevice *parent,
							int of_offset)
{
	struct udevice *dev, *found;

	if (parent->of_offset == of_offset)
		return parent;

	list_for_each_entry(dev, &parent->child_head, sibling_node) {
		found = _device_find_global_by_of_offset(dev, of_offset);
		if (found)
			return found;
	}

	return NULL;
}

int device_get_global_by_of_offset(int of_offset, struct udevice **devp)
{
	struct udevice *dev;

	dev = _device_find_global_by_of_offset(gd->dm_root, of_offset);
	return device_get_device_tail(dev, dev ? 0 : -ENOENT, devp);
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

ulong dev_get_driver_data(struct udevice *dev)
{
	return dev->driver_data;
}

const void *dev_get_driver_ops(struct udevice *dev)
{
	if (!dev || !dev->driver->ops)
		return NULL;

	return dev->driver->ops;
}

enum uclass_id device_get_uclass_id(struct udevice *dev)
{
	return dev->uclass->uc_drv->id;
}

const char *dev_get_uclass_name(struct udevice *dev)
{
	if (!dev)
		return NULL;

	return dev->uclass->uc_drv->name;
}

fdt_addr_t dev_get_addr_index(struct udevice *dev, int index)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	fdt_addr_t addr;

	if (CONFIG_IS_ENABLED(OF_TRANSLATE)) {
		const fdt32_t *reg;
		int len = 0;
		int na, ns;

		na = fdt_address_cells(gd->fdt_blob, dev->parent->of_offset);
		if (na < 1) {
			debug("bad #address-cells\n");
			return FDT_ADDR_T_NONE;
		}

		ns = fdt_size_cells(gd->fdt_blob, dev->parent->of_offset);
		if (ns < 0) {
			debug("bad #size-cells\n");
			return FDT_ADDR_T_NONE;
		}

		reg = fdt_getprop(gd->fdt_blob, dev->of_offset, "reg", &len);
		if (!reg || (len <= (index * sizeof(fdt32_t) * (na + ns)))) {
			debug("Req index out of range\n");
			return FDT_ADDR_T_NONE;
		}

		reg += index * (na + ns);

		/*
		 * Use the full-fledged translate function for complex
		 * bus setups.
		 */
		addr = fdt_translate_address((void *)gd->fdt_blob,
					     dev->of_offset, reg);
	} else {
		/*
		 * Use the "simple" translate function for less complex
		 * bus setups.
		 */
		addr = fdtdec_get_addr_size_auto_parent(gd->fdt_blob,
							dev->parent->of_offset,
							dev->of_offset, "reg",
							index, NULL);
		if (CONFIG_IS_ENABLED(SIMPLE_BUS) && addr != FDT_ADDR_T_NONE) {
			if (device_get_uclass_id(dev->parent) ==
			    UCLASS_SIMPLE_BUS)
				addr = simple_bus_translate(dev->parent, addr);
		}
	}

	/*
	 * Some platforms need a special address translation. Those
	 * platforms (e.g. mvebu in SPL) can configure a translation
	 * offset in the DM by calling dm_set_translation_offset() that
	 * will get added to all addresses returned by dev_get_addr().
	 */
	addr += dm_get_translation_offset();

	return addr;
#else
	return FDT_ADDR_T_NONE;
#endif
}

fdt_addr_t dev_get_addr_name(struct udevice *dev, const char *name)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	int index;

	index = fdt_find_string(gd->fdt_blob, dev->parent->of_offset,
				"reg-names", name);
	if (index < 0)
		return index;

	return dev_get_addr_index(dev, index);
#else
	return FDT_ADDR_T_NONE;
#endif
}

fdt_addr_t dev_get_addr(struct udevice *dev)
{
	return dev_get_addr_index(dev, 0);
}

void *dev_get_addr_ptr(struct udevice *dev)
{
	return (void *)(uintptr_t)dev_get_addr_index(dev, 0);
}

bool device_has_children(struct udevice *dev)
{
	return !list_empty(&dev->child_head);
}

bool device_has_active_children(struct udevice *dev)
{
	struct udevice *child;

	for (device_find_first_child(dev, &child);
	     child;
	     device_find_next_child(&child)) {
		if (device_active(child))
			return true;
	}

	return false;
}

bool device_is_last_sibling(struct udevice *dev)
{
	struct udevice *parent = dev->parent;

	if (!parent)
		return false;
	return list_is_last(&dev->sibling_node, &parent->child_head);
}

int device_set_name(struct udevice *dev, const char *name)
{
	name = strdup(name);
	if (!name)
		return -ENOMEM;
	dev->name = name;

	return 0;
}
