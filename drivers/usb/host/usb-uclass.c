/*
 * (C) Copyright 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <usb.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

extern bool usb_started; /* flag for the started/stopped USB status */
static bool asynch_allowed;

int usb_disable_asynch(int disable)
{
	int old_value = asynch_allowed;

	asynch_allowed = !disable;
	return old_value;
}

int submit_int_msg(struct usb_device *udev, unsigned long pipe, void *buffer,
		   int length, int interval)
{
	struct udevice *bus = udev->controller_dev;
	struct dm_usb_ops *ops = usb_get_ops(bus);

	if (!ops->interrupt)
		return -ENOSYS;

	return ops->interrupt(bus, udev, pipe, buffer, length, interval);
}

int submit_control_msg(struct usb_device *udev, unsigned long pipe,
		       void *buffer, int length, struct devrequest *setup)
{
	struct udevice *bus = udev->controller_dev;
	struct dm_usb_ops *ops = usb_get_ops(bus);

	if (!ops->control)
		return -ENOSYS;

	return ops->control(bus, udev, pipe, buffer, length, setup);
}

int submit_bulk_msg(struct usb_device *udev, unsigned long pipe, void *buffer,
		    int length)
{
	struct udevice *bus = udev->controller_dev;
	struct dm_usb_ops *ops = usb_get_ops(bus);

	if (!ops->bulk)
		return -ENOSYS;

	return ops->bulk(bus, udev, pipe, buffer, length);
}

int usb_alloc_device(struct usb_device *udev)
{
	struct udevice *bus = udev->controller_dev;
	struct dm_usb_ops *ops = usb_get_ops(bus);

	/* This is only requird by some controllers - current XHCI */
	if (!ops->alloc_device)
		return 0;

	return ops->alloc_device(bus, udev);
}

int usb_stop(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int err = 0, ret;

	/* De-activate any devices that have been activated */
	ret = uclass_get(UCLASS_USB, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(bus, uc) {
		ret = device_remove(bus);
		if (ret && !err)
			err = ret;
	}

	usb_stor_reset();
	usb_hub_reset();
	usb_started = 0;

	return err;
}

static int usb_scan_bus(struct udevice *bus, bool recurse)
{
	struct usb_bus_priv *priv;
	struct udevice *dev;
	int ret;

	priv = dev_get_uclass_priv(bus);

	assert(recurse);	/* TODO: Support non-recusive */

	ret = usb_scan_device(bus, 0, USB_SPEED_FULL, &dev);
	if (ret)
		return ret;

	return priv->next_addr;
}

int usb_init(void)
{
	int controllers_initialized = 0;
	struct udevice *bus;
	struct uclass *uc;
	int count = 0;
	int ret;

	asynch_allowed = 1;
	usb_hub_reset();

	ret = uclass_get(UCLASS_USB, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(bus, uc) {
		/* init low_level USB */
		count++;
		printf("USB");
		printf("%d:   ", bus->seq);
		ret = device_probe(bus);
		if (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			controllers_initialized++;
			continue;
		}

		if (ret) {		/* Other error. */
			printf("probe failed, error %d\n", ret);
			continue;
		}
		/*
		 * lowlevel init is OK, now scan the bus for devices
		 * i.e. search HUBs and configure them
		 */
		controllers_initialized++;
		printf("scanning bus %d for devices... ", bus->seq);
		debug("\n");
		ret = usb_scan_bus(bus, true);
		if (ret < 0)
			printf("failed, error %d\n", ret);
		else if (!ret)
			printf("No USB Device found\n");
		else
			printf("%d USB Device(s) found\n", ret);
		usb_started = true;
	}

	debug("scan end\n");
	/* if we were not able to find at least one working bus, bail out */
	if (!count)
		printf("No controllers found\n");
	else if (controllers_initialized == 0)
		printf("USB error: all controllers failed lowlevel init\n");

	return usb_started ? 0 : -1;
}

int usb_reset_root_port(void)
{
	return -ENOSYS;
}

static struct usb_device *find_child_devnum(struct udevice *parent, int devnum)
{
	struct usb_device *udev;
	struct udevice *dev;

	if (!device_active(parent))
		return NULL;
	udev = dev_get_parentdata(parent);
	if (udev->devnum == devnum)
		return udev;

	for (device_find_first_child(parent, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		udev = find_child_devnum(dev, devnum);
		if (udev)
			return udev;
	}

	return NULL;
}

struct usb_device *usb_get_dev_index(struct udevice *bus, int index)
{
	struct udevice *hub;
	int devnum = index + 1; /* Addresses are allocated from 1 on USB */

	device_find_first_child(bus, &hub);
	if (device_get_uclass_id(hub) == UCLASS_USB_HUB)
		return find_child_devnum(hub, devnum);

	return NULL;
}

int usb_post_bind(struct udevice *dev)
{
	/* Scan the bus for devices */
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

int usb_port_reset(struct usb_device *parent, int portnr)
{
	unsigned short portstatus;
	int ret;

	debug("%s: start\n", __func__);

	if (parent) {
		/* reset the port for the second time */
		assert(portnr > 0);
		debug("%s: reset %d\n", __func__, portnr - 1);
		ret = legacy_hub_port_reset(parent, portnr - 1, &portstatus);
		if (ret < 0) {
			printf("\n     Couldn't reset port %i\n", portnr);
			return ret;
		}
	} else {
		debug("%s: reset root\n", __func__);
		usb_reset_root_port();
	}

	return 0;
}

int usb_legacy_port_reset(struct usb_device *parent, int portnr)
{
	return usb_port_reset(parent, portnr);
}

int usb_scan_device(struct udevice *parent, int port,
		    enum usb_device_speed speed, struct udevice **devp)
{
	struct udevice *dev;
	bool created = false;
	struct usb_dev_platdata *plat;
	struct usb_bus_priv *priv;
	struct usb_device *parent_udev;
	int ret;
	ALLOC_CACHE_ALIGN_BUFFER(struct usb_device, udev, 1);
	struct usb_interface_descriptor *iface = &udev->config.if_desc[0].desc;

	*devp = NULL;
	memset(udev, '\0', sizeof(*udev));
	ret = usb_get_bus(parent, &udev->controller_dev);
	if (ret)
		return ret;
	priv = dev_get_uclass_priv(udev->controller_dev);

	/*
	 * Somewhat nasty, this. We create a local device and use the normal
	 * USB stack to read its descriptor. Then we know what type of device
	 * to create for real.
	 *
	 * udev->dev is set to the parent, since we don't have a real device
	 * yet. The USB stack should not access udev.dev anyway, except perhaps
	 * to find the controller, and the controller will either be @parent,
	 * or some parent of @parent.
	 *
	 * Another option might be to create the device as a generic USB
	 * device, then morph it into the correct one when we know what it
	 * should be. This means that a generic USB device would morph into
	 * a network controller, or a USB flash stick, for example. However,
	 * we don't support such morphing and it isn't clear that it would
	 * be easy to do.
	 *
	 * Yet another option is to split out the USB stack parts of udev
	 * into something like a 'struct urb' (as Linux does) which can exist
	 * independently of any device. This feels cleaner, but calls for quite
	 * a big change to the USB stack.
	 *
	 * For now, the approach is to set up an empty udev, read its
	 * descriptor and assign it an address, then bind a real device and
	 * stash the resulting information into the device's parent
	 * platform data. Then when we probe it, usb_child_pre_probe() is called
	 * and it will pull the information out of the stash.
	 */
	udev->dev = parent;
	udev->speed = speed;
	udev->devnum = priv->next_addr + 1;
	udev->portnr = port;
	debug("Calling usb_setup_device(), portnr=%d\n", udev->portnr);
	parent_udev = device_get_uclass_id(parent) == UCLASS_USB_HUB ?
		dev_get_parentdata(parent) : NULL;
	ret = usb_setup_device(udev, priv->desc_before_addr, parent_udev, port);
	debug("read_descriptor for '%s': ret=%d\n", parent->name, ret);
	if (ret)
		return ret;
	ret = usb_find_child(parent, &udev->descriptor, iface, &dev);
	debug("** usb_find_child returns %d\n", ret);

	/* TODO: Find a suitable driver and create the device */
	return -ENOENT;
}

int usb_child_post_bind(struct udevice *dev)
{
	struct usb_dev_platdata *plat = dev_get_parent_platdata(dev);
	const void *blob = gd->fdt_blob;
	int val;

	if (dev->of_offset == -1)
		return 0;

	/* We only support matching a few things */
	val = fdtdec_get_int(blob, dev->of_offset, "usb,device-class", -1);
	if (val != -1) {
		plat->id.match_flags |= USB_DEVICE_ID_MATCH_DEV_CLASS;
		plat->id.bDeviceClass = val;
	}
	val = fdtdec_get_int(blob, dev->of_offset, "usb,interface-class", -1);
	if (val != -1) {
		plat->id.match_flags |= USB_DEVICE_ID_MATCH_INT_CLASS;
		plat->id.bInterfaceClass = val;
	}

	return 0;
}

int usb_get_bus(struct udevice *dev, struct udevice **busp)
{
	struct udevice *bus;

	*busp = NULL;
	for (bus = dev; bus && device_get_uclass_id(bus) != UCLASS_USB; )
		bus = bus->parent;
	if (!bus) {
		/* By design this cannot happen */
		assert(bus);
		debug("USB HUB '%s' does not have a controller\n", dev->name);
		return -EXDEV;
	}
	*busp = bus;

	return 0;
}

int usb_child_pre_probe(struct udevice *dev)
{
	struct udevice *bus;
	struct usb_device *udev = dev_get_parentdata(dev);
	struct usb_dev_platdata *plat = dev_get_parent_platdata(dev);
	int ret;

	ret = usb_get_bus(dev, &bus);
	if (ret)
		return ret;
	udev->controller_dev = bus;
	udev->dev = dev;
	udev->devnum = plat->devnum;
	udev->slot_id = plat->slot_id;
	udev->portnr = plat->portnr;
	udev->speed = plat->speed;
	debug("** device '%s': getting slot_id=%d\n", dev->name, plat->slot_id);

	ret = usb_select_config(udev);
	if (ret)
		return ret;

	return 0;
}

UCLASS_DRIVER(usb) = {
	.id		= UCLASS_USB,
	.name		= "usb",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_bind	= usb_post_bind,
	.per_child_auto_alloc_size = sizeof(struct usb_device),
	.per_device_auto_alloc_size = sizeof(struct usb_bus_priv),
	.child_post_bind = usb_child_post_bind,
	.child_pre_probe = usb_child_pre_probe,
	.per_child_platdata_auto_alloc_size = sizeof(struct usb_dev_platdata),
};
